# 04 — Attack Detection & Move Generation

**Source Files:** `src/attack.c`, `src/movegen.c`, `src/io.c`, `src/defs.h`

Move generation and attack detection sit at the inner loop of the search tree. BadBishop uses reverse ray-casting for `SqAttacked()`, packs moves into 32-bit integers, and generates pseudo-legal moves using table-driven piece loops.

---

## 1. Reverse Attack Detection (`SqAttacked` in `attack.c`)

To determine whether square `sq` is attacked by `side` (used for check detection and castling validation), we **do not** generate all moves for `side`. Instead, we project rays **outward from `sq`**:

```text
                    (-11)       (-10)       (-9)
                        \         │         /
                         \        │        /
                   (-1) ─────── [ sq ] ─────── (+1)
                         /        │        \
                        /         │         \
                    (+9)        (+10)       (+11)

```

1. **Pawn Attacks ($O(1)$, 2 array lookups):**
* Attacked by `WHITE`? Check if `pieces[sq - 11] == wP` or `pieces[sq - 9] == wP`.
* Attacked by `BLACK`? Check if `pieces[sq + 11] == bP` or `pieces[sq + 9] == bP`.


2. **Knight Attacks ($O(1)$, 8 array lookups):**
* Step through `KnDir[8] = { -8, -19, -21, -12, 8, 19, 21, 12 }`. If `PieceKnight[pce]` is `TRUE` and `PieceCol[pce] == side`, return `TRUE`.


3. **Orthogonal Rays — Rooks & Queens ($O(R)$, up to 4 rays $\times$ 7 squares):**
* Slide along `RkDir[4] = { -1, -10, 1, 10 }`. Stop on the first non-empty square or `OFFBOARD`. If `PieceRookQueen[pce]` and `PieceCol[pce] == side`, return `TRUE`.


4. **Diagonal Rays — Bishops & Queens ($O(R)$, up to 4 rays $\times$ 7 squares):**
* Slide along `BiDir[4] = { -9, -11, 11, 9 }`. Stop on the first non-empty square or `OFFBOARD`. If `PieceBishopQueen[pce]` and `PieceCol[pce] == side`, return `TRUE`.


5. **King Attacks ($O(1)$, 8 array lookups):**
* Step 1 square along `KiDir[8]`. If `PieceKing[pce]` and `PieceCol[pce] == side`, return `TRUE`.



---

## 2. 25-Bit Move Encoding (`defs.h`, `io.c`)

Every move is packed into a single 32-bit `int` (`4 bytes`), allowing moves to be copied, compared, and stored in registers in a single instruction.

```text
Bit:  24     23..20     19       18      17..14     13..7      6..0
    ┌────┬──────────┬────────┬────────┬──────────┬─────────┬─────────┐
    │ CA │ Promoted │ PawnSt │ EnPas  │ Captured │ To Sq   │ From Sq │
    │ 1b │  4 bits  │ 1 bit  │ 1 bit  │  4 bits  │ 7 bits  │ 7 bits  │
    └────┴──────────┴────────┴────────┴──────────┴─────────┴─────────┘

```

### Bit-Packing Macros & Examples

| Macro | Bitwise Expression | Purpose |
| --- | --- | --- |
| `MOVE(f, t, ca, pro, fl)` | `((f) | ((t) << 7) | ((ca) << 14) | ((pro) << 20) | (fl))` | Packs a move into 32 bits |
| `FROMSQ(m)` | `((m) & 0x7F)` | Extracts 120-based origin square (`21..98`) |
| `TOSQ(m)` | `(((m) >> 7) & 0x7F)` | Extracts 120-based target square (`21..98`) |
| `CAPTURED(m)` | `(((m) >> 14) & 0xF)` | Extracts captured piece ID (`0..12`) |
| `PROMOTED(m)` | `(((m) >> 20) & 0xF)` | Extracts promoted piece ID (`0..12`) |
| `MFLAGEP` | `0x40000` (Bit 18) | En Passant capture flag |
| `MFLAGPS` | `0x80000` (Bit 19) | Double Pawn Push flag (sets `enPas`) |
| `MFLAGCA` | `0x1000000` (Bit 24) | Castling move flag |

**Worked Example:** White promotes a pawn on `B7` (`82`) by capturing a Black Rook (`bR = 10`) on `A8` (`91`), promoting to a White Knight (`wN = 2`):

* `from = 82` (`0x52`)
* `to = 91 << 7` (`0x2D80`)
* `cap = 10 << 14` (`0x28000`)
* `pro = 2 << 20` (`0x200000`)
* Packed integer: `0x22ADDA` $\rightarrow$ `PrMove()` formats this as `"b7a8n"`.

---

## 3. Pseudo-Legal Move Generation (`GenerateAllMoves` in `movegen.c`)

`GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list)` populates a stack-allocated `S_MOVELIST` containing all pseudo-legal moves for `pos->side`:

1. **Pawns:** Iterates `pos->pList[wP/bP]`. Generates single pushes (`\pm 10`), Rank 2/7 double pushes (`\pm 20` with `MFLAGPS`), diagonal captures (`\pm 9, \pm 11`), 4-way promotions on Rank 7/2 (`Q, R, B, N`), and En Passant captures onto `pos->enPas` (`MFLAGEP`).
2. **Castling:** Checks `pos->castlePerm` (`WKCA, WQCA, BKCA, BQCA`), verifies empty intervening squares, and calls `SqAttacked()` to ensure the King is not in check and does not pass through an attacked square (`E1/F1` or `E1/D1`).
3. **Sliders (`wB, wR, wQ` / `bB, bR, bQ`):** Driven by `LoopSlidePce[]`, `PceDir[][]`, and `NumDir[]`. Slides along each ray until hitting `OFFBOARD` or a piece (adding a capture if enemy, then breaking).
4. **Non-Sliders (`wN, wK` / `bN, bK`):** Driven by `LoopNonSlidePce[]`. Steps once per direction in `PceDir[][]`.

---

## 4. Storage & Runtime Profile

| Component | Storage Size | Allocation | Runtime Complexity |
| --- | --- | --- | --- |
| **`S_MOVE`** | `8 B` (`int move`, `int score`) | Inside `S_MOVELIST` | $O(1)$ read/write |
| **`S_MOVELIST`** | `256 * 8 B + 4 B = 2,052 B` (~2 KB) | Stack-allocated per ply | Zero heap fragmentation |
| **`SqAttacked()`** | `32 B` (`KnDir, RkDir, BiDir, KiDir`) | Static ROM/Flash | **Best:** $O(1)$ (2 ops); **Worst:** $37$ array checks |
| **`GenerateAllMoves()`** | `480 B` (`PceDir, NumDir, Loop*`) | Static ROM/Flash | $O(P + S \cdot R)$ (~35 moves avg in middlegame, ~400–600 CPU cycles) |

---

## References

1. **Pseudo-Legal vs. Legal Move Generation:** Chess Programming Wiki — [Pseudo-Legal Move](https://www.chessprogramming.org/Pseudo-Legal_Move)
2. **Move Bit-Packing:** Chess Programming Wiki — [Encoding Moves](https://www.chessprogramming.org/Encoding_Moves)
3. **Kiwipete Perft Benchmark Position:** Peter McKenzie — [Perft Results](https://www.google.com/search?q=https://www.chessprogramming.org/Perft_Results%23Position_2)