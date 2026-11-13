# 06 — Static Evaluation & Engine Personalities

**Source Files:** `src/evaluate.c`, `src/init.c`, `src/defs.h`

At leaf nodes of the search tree, `EvalPosition(const S_BOARD *pos)` computes a static score in **centipawns** (`100 cp = 1 pawn`) from the side-to-move's perspective. To support distinct playing styles without floating-point overhead, BadBishop decomposes evaluation into five structural feature buckets and weights them via a compile-time `S_PERSONALITY` table.

---

## 1. Precomputed 64-Bit Pawn Evaluation Masks (`init.c`)

Checking whether a pawn is isolated or passed by looping over mailbox squares requires up to 24 array lookups per pawn. BadBishop precomputes five 64-bit bitboard masks in `InitEvalMasks()` so every pawn structure query executes in **a single 64-bit bitwise `AND` instruction ($O(1)$)**:

1. **`FileBBMask[8]` & `RankBBMask[8]`:** 64-bit masks with bits set along each file (`FILE_A..FILE_H`) and rank (`RANK_1..RANK_8`).
2. **`IsolatedMask[64]`:** For any square `sq64`, contains the union of the adjacent files (`FileBBMask[file - 1] | FileBBMask[file + 1]`).
   * **Test:** `(IsolatedMask[sq64] & pos->pawns[WHITE]) == 0` $\rightarrow$ White pawn has zero friendly neighbors on adjacent files (`-10 cp`).
3. **`WhitePassedMask[64]` & `BlackPassedMask[64]`:** Contains the 3-file forward span (`file - 1`, `file`, `file + 1`) strictly ahead of `sq64` toward the promotion rank.
   * **Test:** `(WhitePassedMask[sq64] & pos->pawns[BLACK]) == 0` $\rightarrow$ No Black pawn can block or capture the White pawn on its march to Rank 8 (`+5 cp` to `+200 cp` by rank).

```text
WhitePassedMask[D4] (Bitboard Forward Span)      IsolatedMask[D4] (Adjacent Files)
8 │ .  .  #  #  #  .  .  .                       8 │ .  .  #  .  #  .  .  .
7 │ .  .  #  #  #  .  .  .                       7 │ .  .  #  .  #  .  .  .
6 │ .  .  #  #  #  .  .  .                       6 │ .  .  #  .  #  .  .  .
5 │ .  .  #  #  #  .  .  .                       5 │ .  .  #  .  #  .  .  .
4 │ .  .  . [wP] .  .  .  .                      4 │ .  .  # [wP]#  .  .  .
3 │ .  .  .  .  .  .  .  .                       3 │ .  .  #  .  #  .  .  .
2 │ .  .  .  .  .  .  .  .                       2 │ .  .  #  .  #  .  .  .
1 │ .  .  .  .  .  .  .  .                       1 │ .  .  #  .  #  .  .  .
  └────────────────────────                        └────────────────────────
    a  b  c  d  e  f  g  h                           a  b  c  d  e  f  g  h
```

---

## 2. The Five Evaluation Feature Buckets (`evaluate.c`)

`EvalPosition()` first checks `MaterialDraw(pos)` to return `0` immediately in dead-drawn bare-piece endgames ($K\text{ vs }K$, $KN\text{ vs }K$, $KB\text{ vs }K$, $KNN\text{ vs }K$). Otherwise, it accumulates scores from White's perspective across 5 buckets:

1. **`matScore` (Material & Bishop Pair):**
   * Base difference: `pos->material[WHITE] - pos->material[BLACK]`.
   * Adds `+30 cp` (`BishopPair`) if a side retains 2 or more Bishops.
2. **`pstScore` (Piece-Square Tables & Phase-Tapered King):**
   * Uses 64-element lookup tables (`PawnTable`, `KnightTable`, `BishopTable`, `RookTable`). Black piece coordinates are flipped vertically via `Mirror64[sq64]`.
   * **King Phase Transition:** When opponent non-King material drops to or below `ENDGAME_MAT` (`1,350 cp`), the King switches from `KingO[64]` (penalizing center exposure, rewarding castled shelter) to `KingE[64]` (rewarding `+40 cp` centralization to support endgame pawns).
3. **`pawnScore` (Bitboard Pawn Structure):**
   * Penalizes isolated pawns (`-10 cp`) and rewards passed pawns by rank (`{0, 5, 10, 20, 35, 60, 100, 200}`).
4. **`mobScore` (Open & Semi-Open File Control):**
   * Checks `(pos->pawns[BOTH] & FileBBMask[file]) == 0` for Rooks (`+10 cp` open, `+5 cp` semi-open) and Queens (`+5 cp` open, `+3 cp` semi-open).
5. **`whiteKingSafety` & `blackKingSafety` (Middlegame King Shield):**
   * In the middlegame, if the file occupied by the King has no friendly pawn (`(pos->pawns[col] & FileBBMask[kFile]) == 0`), applies `-25 cp` (`KingOpenFilePenalty`).

---

## 3. Three-Mode Personality Engine (`S_PERSONALITY`)

BadBishop supports three distinct personalities via fixed-point integer percentage weights (`100 = 1.0x`), avoiding floating-point arithmetic on the RP2350:

| Parameter | `STYLE_BALANCED` (0) | `STYLE_POSITIONAL` (1) | `STYLE_SHARP` (2) | Tactical / Strategic Effect |
| :--- | :---: | :---: | :---: | :--- |
| **`materialWeight`** | `100%` | `105%` | `92%` | Sharp slightly discounts material to permit sacrifices |
| **`pstWeight`** | `100%` | `110%` | `110%` | Both specialized styles value active squares |
| **`pawnStructWeight`** | `100%` | **`160%`** | `75%` | Positional heavily prioritizes clean pawn structures |
| **`mobilityWeight`** | `100%` | `110%` | **`145%`** | Sharp prizes open files for Rooks and Queens |
| **`ownKingSafety`** | `100%` | **`140%`** | `80%` | Positional plays defensively; Sharp tolerates risk |
| **`oppKingAttack`** | `100%` | `85%` | **`185%`** | Sharp nearly doubles bonuses for ripping open the enemy King |
| **`contempt`** | `0 cp` | `0 cp` | **`-50 cp`** | Sharp treats draws as `-0.50` pawns, avoiding repetitions |

### Asymmetric King Safety & Final Score Formula
Notice that `ownKingSafety` and `oppKingAttack` are applied **asymmetrically** based on whose turn it is (`pos->side`):
```c
if (pos->side == WHITE) {
    kingSafetyTerm = (whiteKingSafety * p->ownKingSafety - blackKingSafety * p->oppKingAttack);
} else {
    kingSafetyTerm = (whiteKingSafety * p->oppKingAttack - blackKingSafety * p->ownKingSafety);
}

int finalScore = (matScore * p->materialWeight +
                  pstScore * p->pstWeight +
                  pawnScore * p->pawnStructWeight +
                  mobScore * p->mobilityWeight +
                  kingSafetyTerm) / 100;

return (pos->side == WHITE) ? finalScore : -finalScore;
```

---

## 4. Storage & Runtime Profile

| Component | Storage Footprint | Memory Region | Time Complexity |
| :--- | :--- | :--- | :--- |
| **Bitboard Eval Masks** | `208 * 8 B = 1,664 B` | Static RAM (`init.c`) | $O(1)$ per bitwise `&` check |
| **PSTs & `Mirror64`** | `7 * 64 * 4 B = 1,792 B` | Static ROM/Flash | $O(1)$ array lookup |
| **`Personalities[3]`** | `3 * 28 B = 84 B` | Static ROM/Flash | $O(1)$ integer multiply/divide |
| **`EvalPosition(pos)`** | `0 B` heap (`~48 B` stack) | Stack | $O(P)$ where $P \le 30$ non-King pieces (~120–180 CPU cycles) |

---

## References
1. **Bitboard Pawn Structure Evaluation:** Chess Programming Wiki — [Pawn Structure](https://www.chessprogramming.org/Pawn_Structure) & [Passed Pawn](https://www.chessprogramming.org/Passed_Pawn)
2. **Simplified Evaluation Function (PSTs):** Tomasz Michniewski — [Simplified Evaluation Function](https://www.chessprogramming.org/Simplified_Evaluation_Function)
3. **Contempt Factor & Engine Personalities:** Chess Programming Wiki — [Contempt Factor](https://www.chessprogramming.org/Contempt_Factor)