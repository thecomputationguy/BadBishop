# 07 — Transposition Table & Alpha-Beta Search

**Source Files:** `src/pvtable.c`, `src/search.c`, `src/defs.h`

BadBishop's search engine combines **Iterative Deepening**, **Negamax Alpha-Beta**, **Quiescence Search** with **Delta Pruning**, a **zero-malloc Transposition Table**, and a four-tier **Move Ordering** pipeline.

---

## 1. Zero-Malloc Transposition Table (`pvtable.c`)

To guarantee zero heap fragmentation and deterministic memory usage on embedded targets, `HashTable` is allocated statically at compile time and scaled via `PICO_BUILD`:

| Build Target | Macro | `HASH_ENTRIES` | Entry Size (`S_HASHENTRY`) | Total Static RAM |
| :--- | :--- | :---: | :---: | :---: |
| **Raspberry Pi Pico 2 (RP2350)** | `-DPICO_BUILD` | $8,192$ ($2^{13}$) | `24 B` | **`196,608 B` (`192 KB`)** |
| **Desktop PC (x86-64 / ARM64)** | *(Default)* | $262,144$ ($2^{18}$) | `24 B` | **`6,291,456 B` (`6 MB`)** |

### Power-of-Two Indexing & Bound Flags
Because `HASH_ENTRIES` is a power of two, `HASH_INDEX(key)` maps any 64-bit `posKey` to a table slot using a single bitwise AND:
```c
#define HASH_INDEX(key) ((key) & (HASH_ENTRIES - 1))
```
Each entry stores one of three bound types (`flags`):
* **`HFEXACT`:** True minimax score inside $(\alpha, \beta)$ ($s > \alpha$ and $s < \beta$).
* **`HFBETA` (Lower Bound):** Move failed high ($s \ge \beta$, caused a $\beta$-cutoff); true score is *at least* $\beta$.
* **`HFALPHA` (Upper Bound):** All moves failed low ($s \le \alpha$); true score is *at most* $\alpha$.

### Mate Score Distance Normalization
Checkmate scores are relative to the root (`-INF_BOUND + pos->ply`, so a mate in 3 plies scores higher than a mate in 5 plies). When storing a mate score (`|score| > ISMATE`) in the TT at depth `ply`, `StoreHashEntry()` adjusts it to be relative to the *current node* (`score += pos->ply`), and `ProbeHashEntry()` converts it back (`score -= pos->ply`) upon retrieval.

---

## 2. Four-Tier Move Ordering Pipeline (`search.c`)

Alpha-Beta pruning efficiency depends entirely on searching the best move first:
* **Worst-case ordering:** Searches $O(B^D)$ nodes (same as raw Minimax).
* **Optimal ordering ($\beta$-cutoff on move 1):** Searches $O(B^{D/2})$ nodes—**doubling the reachable search depth** in the same time budget.

`ScoreMoves()` assigns a priority integer to every generated move, and `PickNextMove()` performs an in-place selection swap just before each move is searched:

```text
Priority Tier 1:  2,000,000          ──► Transposition Table / PV Move (ProbeHashEntry)
Priority Tier 2:  1,000,105..1,000,605 ──► Captures ordered by MVV-LVA (Most Valuable Victim / Least Valuable Attacker)
Priority Tier 3:    900,000 / 800,000  ──► Killer Moves (2 quiet moves at current ply that recently caused a Beta-cutoff)
Priority Tier 4:          0..N         ──► History Heuristic (searchHistory[pce][to] incremented by depth^2 on cutoffs)
```

### MVV-LVA Formula (`InitMvvLva`)
$$\text{MvvLvaScores}[\text{Victim}][\text{Attacker}] = \text{VictimScore}[\text{Victim}] + 6 - \frac{\text{VictimScore}[\text{Attacker}]}{100}$$
* `Pawn takes Queen` ($500 + 6 - 1 = 505$) is searched first.
* `Queen takes Queen` ($500 + 6 - 5 = 501$) is searched after `PxQ`, `NxQ`, `BxQ`, `RxQ`.
* `Queen takes Pawn` ($100 + 6 - 5 = 101$) is searched last among captures.

---

## 3. Quiescence Search & Delta Pruning (`Quiescence`)

When `AlphaBeta()` reaches `depth == 0`, it calls `Quiescence(alpha, beta, pos, info)` to resolve ongoing tactical exchanges using `GenerateAllCaps()`:

1. **Stand-Pat Evaluation:** Computes `Score = EvalPosition(pos)`. Since making a capture is rarely compulsory, if `Score >= beta`, it returns `beta` immediately; if `Score > alpha`, it raises `alpha = Score`.
2. **Delta Pruning:** Before calling `MakeMove()` on a non-promoting capture, checks whether the captured piece plus a `+200 cp` positional margin could possibly lift `standPat` up to `alpha`:
   $$\text{if } (\text{standPat} + \text{PieceVal}[\text{captured}] + 200 < \alpha) \implies \text{skip move}$$
   In hopeless positions (e.g., down a Rook), this prunes futile pawn/minor exchanges instantly.

---

## 4. Storage & Runtime Profile

| Component | Storage Footprint | Memory Region | Time Complexity |
| :--- | :--- | :--- | :--- |
| **`HashTable[1]`** | `192 KB` (Pico) / `6 MB` (PC) | Static `.bss` RAM | $O(1)$ probe & store |
| **`MvvLvaScores[13][13]`** | `169 * 4 B = 676 B` | Static RAM | $O(1)$ lookup |
| **`searchHistory[13][120]`** | `1,560 * 4 B = 6,240 B` | Inside `S_BOARD` | $O(1)$ lookup & update |
| **`searchKillers[2][64]`** | `128 * 4 B = 512 B` | Inside `S_BOARD` | $O(1)$ lookup & update |
| **`PickNextMove()`** | `0 B` | Stack | $O(M)$ remaining moves ($M \le 35$ avg) |

---

## References
1. **Negamax Alpha-Beta & Iterative Deepening:** Chess Programming Wiki — [Alpha-Beta](https://www.chessprogramming.org/Alpha-Beta)
2. **Quiescence Search & Delta Pruning:** Chess Programming Wiki — [Quiescence Search](https://www.chessprogramming.org/Quiescence_Search) & [Delta Pruning](https://www.chessprogramming.org/Delta_Pruning)
3. **MVV-LVA, Killer & History Heuristics:** Chess Programming Wiki — [Move Ordering](https://www.chessprogramming.org/Move_Ordering)