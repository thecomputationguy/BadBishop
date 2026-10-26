# 02 — Zobrist Hashing & Position Keys

**Source Files:** `src/hashkeys.c`, `src/init.c`, `src/defs.h`

To detect three-fold repetitions and index positions in a Transposition Table (TT), BadBishop compresses the entire state of a chess position into a single 64-bit unsigned integer (`U64 posKey`) using **Zobrist Hashing**.

---

## 1. How Zobrist Hashing Works

At engine startup (`InitHashKeys()` in `init.c`), we populate three tables of 64-bit pseudo-random numbers using `RAND_64`:

1. **`PieceKeys[13][120]`:** A unique 64-bit random number for every `[piece][square]` pair (plus index `0` used for the En Passant square `[EMPTY][pos->enPas]`).
2. **`SideKey`:** A single 64-bit random number XORed in if and only if it is `WHITE`'s turn to move.
3. **`CastleKeys[16]`:** 16 random 64-bit numbers corresponding to every possible 4-bit combination of castling rights (`0000` to `1111` for `WKCA | WQCA | BKCA | BQCA`).

### Full Key Generation (`GeneratePosKey`)
When a FEN is loaded, `GeneratePosKey(const S_BOARD *pos)` computes `posKey` from scratch by XORing together the active features:

$$\text{posKey} = \left( \bigoplus_{s \in \text{Board}} \text{PieceKeys}[\text{piece}(s)][s] \right) \oplus \text{SideKey}_{\text{if White}} \oplus \text{PieceKeys}[0][\text{enPas}]_{\text{if valid}} \oplus \text{CastleKeys}[\text{castlePerm}]$$

---

## 2. Why XOR ($\oplus$) Is Crucial: $O(1)$ Incremental Updates

The bitwise XOR operator is **self-inverting** ($A \oplus B \oplus B = A$) and **commutative** ($A \oplus B = B \oplus A$).

Instead of calling `GeneratePosKey()` after every move in the search tree (which would cost 64 loop iterations per node), `MakeMove()` and `TakeMove()` update `pos->posKey` incrementally in $O(1)$ time (just 3 to 5 clock cycles):

### Example: Moving a White Knight from `G1` to `F3`
```c
// 1. XOR out the White Knight on G1
pos->posKey ^= PieceKeys[wN][G1];

// 2. XOR in the White Knight on F3
pos->posKey ^= PieceKeys[wN][F3];

// 3. Flip the side to move
pos->posKey ^= SideKey;

```

If the Knightlater retreats from `F3` to `G1`, XORing `PieceKeys[wN][F3]` and `PieceKeys[wN][G1]` again restores the exact original bit pattern.

---

## 3. Collision Probability (Birthday Paradox)

With a 64-bit key space ($2^{64} \approx 1.84 \times 10^{19}$ possible keys), the probability of two distinct chess positions producing the exact same `posKey` during a search of $N$ unique positions is governed by the birthday approximation:

$$P(\text{collision}) \approx \frac{N^2}{2 \cdot 2^{64}}$$

* **On Raspberry Pi Pico 2 WH** ($\sim 50,000$ nodes/sec, searching $5 \times 10^5$ nodes over 10 seconds):

$$P \approx \frac{(5 \times 10^5)^2}{3.68 \times 10^{19}} \approx 6.8 \times 10^{-9} \text{ (virtually zero)}$$


* **On Desktop PC** ($\sim 2 \times 10^6$ nodes/sec, searching $2 \times 10^7$ nodes):

$$P \approx \frac{(2 \times 10^7)^2}{3.68 \times 10^{19}} \approx 1.08 \times 10^{-5}$$



---

## 4. Storage & Runtime Profile

### Memory Footprint (Static RAM in `init.c`)

| Table | Dimensions | Calculation | Size (Bytes) |
| --- | --- | --- | --- |
| `PieceKeys` | `13 * 120` of `U64` | $1,560 \times 8\text{ B}$ | `12,480 B` |
| `SideKey` | `1` of `U64` | $1 \times 8\text{ B}$ | `8 B` |
| `CastleKeys` | `16` of `U64` | $16 \times 8\text{ B}$ | `128 B` |
| **Total Zobrist Tables** | — | — | **`12,616 B` (~12.3 KB)** |

### Runtime Complexity

| Operation | Function / Macro | Time Complexity | CPU Cost |
| --- | --- | --- | --- |
| **Table Initialization** | `InitHashKeys()` | $O(1)$ (runs once at boot) | ~1,577 PRNG calls |
| **Full Board Hash** | `GeneratePosKey(pos)` | $O(N)$ ($N = 64$ squares) | ~80–120 cycles (used only in `ParseFen` and `CheckBoard`) |
| **Incremental Hash Update** | `HASH_PCE`, `HASH_SIDE`, `HASH_CA`, `HASH_EP` | $O(1)$ | **1–2 CPU cycles** per state change |

---

## References

1. **Zobrist, A. L. (1970):** *"A New Hashing Method with Application for Game Playing"*, Technical Report #88, University of Wisconsin, Madison.
2. **Chess Programming Wiki:** [Zobrist Hashing](https://www.chessprogramming.org/Zobrist_Hashing)