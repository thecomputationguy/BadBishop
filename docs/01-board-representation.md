# 01 — Hybrid Board Representation

**Source Files:** `src/defs.h`, `src/init.c`, `src/bitboards.c`

A chess engine executes board queries and updates millions of times per second. BadBishop combines three complementary data structures inside `S_BOARD` so that every query type runs in $O(1)$ time without dynamic memory allocation or boundary-checking branch penalties.

---

## 1. The 120-Square Mailbox (`pieces[120]`)

An $8 \times 8$ chessboard has 64 squares, but storing it in a raw 64-element array creates **ray-wrapping bugs**. For example, if a Knight on `B1` steps two ranks down and one file left, simple arithmetic wraps around to the opposite side of the board or indexes out of bounds. Checking `if (file >= 0 && file < 8 && rank >= 0 && rank < 8)` on every step introduces costly CPU branch mispredictions.

BadBishop embeds the $8 \times 8$ play area inside a flat **$10 \times 12$ (120-element) 1D array** surrounded by `OFFBOARD` sentinel borders:

```text
Rank 8 │  90  [ 91(A8)  92(B8)  93(C8)  94(D8)  95(E8)  96(F8)  97(G8)  98(H8) ]  99
Rank 7 │  80  [ 81(A7)  82(B7)  83(C7)  84(D7)  85(E7)  86(F7)  87(G7)  88(H7) ]  89
Rank 6 │  70  [ 71(A6)  72(B6)  73(C6)  74(D6)  75(E6)  76(F6)  77(G7)  78(H6) ]  79
Rank 5 │  60  [ 61(A5)  62(B5)  63(C5)  64(D5)  65(E5)  66(F5)  67(G5)  68(H5) ]  69
Rank 4 │  50  [ 51(A4)  52(B4)  53(C4)  54(D4)  55(E4)  56(F4)  57(G4)  58(H4) ]  59
Rank 3 │  40  [ 41(A3)  42(B3)  43(C3)  44(D3)  45(E3)  46(F3)  47(G3)  48(H3) ]  49
Rank 2 │  30  [ 31(A2)  32(B2)  33(C2)  34(D2)  35(E2)  36(F2)  37(G2)  38(H2) ]  39
Rank 1 │  20  [ 21(A1)  22(B1)  23(C1)  24(D1)  25(E1)  26(F1)  27(G1)  28(H1) ]  29
Pad    │  10    11      12      13      14      15      16      17      18        19
Pad    │   0     1       2       3       4       5       6       7       8         9

```

*(Top padding ranks `100..109` and `110..119` mirror the bottom two ranks).*

### Why Two Padding Ranks on Top and Bottom, One on Sides?

A Knight is the longest-reaching leaper in a single step: it jumps 2 ranks and 1 file (or 1 rank and 2 files).

* A vertical jump of 2 ranks from Rank 1 (`21..28`) lands on indices `1..8`. Two bottom padding ranks (`0..19`) and two top padding ranks (`100..119`) catch all vertical Knight leaps inside valid array bounds.
* A horizontal jump of 2 files from File A (`21`) lands on index `19` (the right guard of the previous row), which is also marked `OFFBOARD`. Thus, **1 sentinel file per side** (`*0` and `*9`) catches all horizontal wrap-arounds.

### Coordinate Formula (`FR2SQ`)

Given `file` $\in [0, 7]$ and `rank` $\in [0, 7]$:


$$\text{sq120} = 21 + \text{file} + (\text{rank} \times 10)$$

**Example (Square `E4`):**

* $\text{file} = 4$ (`FILE_E`), $\text{rank} = 3$ (`RANK_4`)
* $\text{sq120} = 21 + 4 + (3 \times 10) = 55$

---

## 2. 64-Bit Pawn Bitboards (`pawns[3]`)

While the 120-square mailbox excels at generating sliding rays, evaluating pawn structures (isolated pawns, passed pawns, doubled pawns) across a mailbox requires slow nested loops. BadBishop maintains three 64-bit unsigned integers (`U64`, `uint64_t`):

* `pawns[WHITE]` (index `0`)
* `pawns[BLACK]` (index `1`)
* `pawns[BOTH]`  (index `2`)

Bit $i \in [0, 63]$ corresponds to `sq64`:

* Bit `0` = `A1`, Bit `7` = `H1`, Bit `56` = `A8`, Bit `63` = `H8`.

### Bit Manipulation Operations (`bitboards.c`, `defs.h`)

| Operation | Implementation | Runtime | Description |
| --- | --- | --- | --- |
| `SETBIT(bb, sq)` | `(bb) |= SetMask[(sq)]` | $O(1)$ (1 cycle) | Sets bit at `sq64` |
| `CLRBIT(bb, sq)` | `(bb) &= ClearMask[(sq)]` | $O(1)$ (1 cycle) | Clears bit at `sq64` |
| `CountBits(bb)` | `for(r = 0; bb; r++, bb &= bb - 1);` | $O(k)$ set bits | Brian Kernighan's population count |
| `PopBit(&bb)` | De Bruijn multiplication (`0x783a9b23`) + `BitTable[64]` | $O(1)$ (~4 cycles) | Extracts and clears the least significant set bit (LSB) |

**Example (`CountBits` via Kernighan's trick):**
Subtracting `1` from any binary integer flips the lowest set bit (`1`) to `0` and turns all trailing `0`s into `1`s. Bitwise ANDing `bb &= (bb - 1)` strips the lowest set bit in a single CPU instruction. For 8 White pawns on the board, the loop runs at most 8 times instead of 64.

---

## 3. Piece Lists (`pList[13][10]`)

Scanning 64 squares to find pieces during move generation wastes cycles, especially in the endgame when only 4 or 5 pieces remain. `pList[piece_type][instance_index]` stores the exact 120-square coordinate of every piece on the board.

**Example:** White has two Knights on `B1` (`22`) and `F3` (`46`):

* `pos->pceNum[wN] == 2`
* `pos->pList[wN][0] == 22` (`B1`)
* `pos->pList[wN][1] == 46` (`F3`)

Iterating over all White Knights visits exactly 2 array slots (`O(K)` where $K$ is the count of that piece).

---

## 4. Storage & Runtime Profile

| Data Structure | Location | Memory Footprint | Access Time |
| --- | --- | --- | --- |
| `Sq120ToSq64[120]` | Static RAM (`init.c`) | `120 * 4 B = 480 B` | $O(1)$ |
| `Sq64ToSq120[64]` | Static RAM (`init.c`) | `64 * 4 B = 256 B` | $O(1)$ |
| `FilesBrd[120]` & `RanksBrd[120]` | Static RAM (`init.c`) | `2 * 480 B = 960 B` | $O(1)$ |
| `SetMask[64]` & `ClearMask[64]` | Static RAM (`init.c`) | `2 * 512 B = 1,024 B` | $O(1)$ |
| `pos->pieces[120]` | `S_BOARD` | `120 * 4 B = 480 B` | $O(1)$ |
| `pos->pawns[3]` | `S_BOARD` | `3 * 8 B = 24 B` | $O(1)$ |
| `pos->pList[13][10]` | `S_BOARD` | `130 * 4 B = 520 B` | $O(1)$ |

---

## References

1. **10x12 Mailbox Representation:** Chess Programming Wiki — [10x12 Board](https://www.chessprogramming.org/10x12_Board)
2. **De Bruijn Bitscan (LSB Extraction):** Leiserson, Prokop, Randall (1998), *"Using de Bruijn Sequences to Index a 1 in a Computer Word"*.
3. **Piece Lists:** Chess Programming Wiki — [Piece-Lists](https://www.chessprogramming.org/Piece-Lists)
