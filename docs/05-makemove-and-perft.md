# 05 — State Transitions (`MakeMove` / `TakeMove`) & Perft Verification

**Source Files:** `src/makemove.c`, `src/perft.c`, `src/defs.h`, `tests/test_perft.c`

During search, BadBishop never clones `S_BOARD` structs. Instead, a single `S_BOARD` instance is mutated in-place by `MakeMove()` as the engine descends the search tree and restored bit-for-bit by `TakeMove()` when backtracking.

---

## 1. Atomic Piece Manipulation Primitives (`makemove.c`)

All board mutations are built out of three `static` primitives that keep the 120-square mailbox, 64-bit pawn bitboards, material counters, piece lists, and Zobrist `posKey` synchronized in $O(1)$ time.

### A. `ClearPiece(const int sq, S_BOARD *pos)`
Removes a piece from `sq`:
1. **Zobrist Update:** `HASH_PCE(pce, sq)` XORs out `PieceKeys[pce][sq]`.
2. **Mailbox & Material:** Sets `pos->pieces[sq] = EMPTY` and subtracts `PieceVal[pce]` from `pos->material[col]`.
3. **Category Counters / Bitboards:**
   * If `PieceBig[pce]` is true, decrements `bigPce[col]` and either `majPce[col]` or `minPce[col]`.
   * If the piece is a pawn, clears `SQ64(sq)` in both `pos->pawns[col]` and `pos->pawns[BOTH]`.
4. **$O(1)$ Swap-Erase in `pList`:** Scans `pos->pList[pce][0 .. pceNum[pce]-1]` (at most 8 elements for pawns, 2 for knights/rooks/bishops, 1 for queens/kings), decrements `pos->pceNum[pce]`, and overwrites the removed slot with the final element in the array:

```text
Before ClearPiece(C4):  pList[wP] = [ A2, B2, C4, D4, E4 ]  (pceNum = 5)
                                              ▲       │
                                              └───────┘ (Swap last element into index 2)
After ClearPiece(C4):   pList[wP] = [ A2, B2, E4, D4, -- ]  (pceNum = 4)

```

### B. `AddPiece(const int sq, S_BOARD *pos, const int pce)`

Places `pce` onto an empty square `sq`:

1. XORs `PieceKeys[pce][sq]` into `pos->posKey`.
2. Sets `pos->pieces[sq] = pce` and adds `PieceVal[pce]` to `pos->material[col]`.
3. Increments `bigPce`/`majPce`/`minPce` (or sets `pos->pawns[col]` and `pos->pawns[BOTH]` bits).
4. Appends `sq` to the end of the piece list: `pos->pList[pce][pos->pceNum[pce]++] = sq`.

### C. `MovePiece(const int from, const int to, S_BOARD *pos)`

Translates an existing piece from `from` to `to` without altering material or piece counts:

1. XORs out `PieceKeys[pce][from]` and XORs in `PieceKeys[pce][to]`.
2. Updates `pos->pieces[from] = EMPTY` and `pos->pieces[to] = pce`.
3. If the piece is a pawn, clears bit `SQ64(from)` and sets bit `SQ64(to)` in `pawns[col]` and `pawns[BOTH]`.
4. Updates the coordinate in `pos->pList[pce][]` from `from` to `to`.

---

## 2. Branchless Castling Permission Mask (`CastlePerm[120]`)

Castling rights (`castlePerm`, 4 bits: `WKCA=1, WQCA=2, BKCA=4, BQCA=8`) can be revoked in three ways:

1. A King moves from `E1` or `E8`.
2. A Rook moves from its starting corner (`A1`, `H1`, `A8`, `H8`).
3. An enemy piece captures a Rook *on* its starting corner (`to == A1/H1/A8/H8`).

Instead of branching with `if/else` ladders, `MakeMove()` uses a 120-element bitmask table `CastlePerm[120]` where every square is `15` (`0b1111`), except the 6 critical squares:

| Square | 120-Index | Binary Mask | Decimal | Effect on `pos->castlePerm` |
| --- | --- | --- | --- | --- |
| **`A1`** | `21` | `0b1101` (`~WQCA`) | `13` | Clears White Queenside castling |
| **`E1`** | `25` | `0b1100` (`~(WKCA|WQCA)`) | `12` | Clears all White castling |
| **`H1`** | `28` | `0b1110` (`~WKCA`) | `14` | Clears White Kingside castling |
| **`A8`** | `91` | `0b0111` (`~BQCA`) | `7` | Clears Black Queenside castling |
| **`E8`** | `95` | `0b0011` (`~(BKCA|BQCA)`) | `3` | Clears all Black castling |
| **`H8`** | `98` | `0b1011` (`~BKCA`) | `11` | Clears Black Kingside castling |

On every move, two bitwise AND instructions handle all castling right updates:

```c
HASH_CA; // XOR out old castling permission key
pos->castlePerm &= CastlePerm[from];
pos->castlePerm &= CastlePerm[to];
HASH_CA; // XOR in new castling permission key

```

---

## 3. `MakeMove()` Execution & Pseudo-Legal Filtering

`int MakeMove(S_BOARD *pos, int move)` executes in 9 sequential phases:

1. **Snapshot Irreversible State:** Saves `posKey`, `move`, `fiftyMove`, `enPas`, and `castlePerm` into `pos->history[pos->hisPly]`.
2. **Special Move Pre-Processing:**
* **En Passant (`MFLAGEP`):** Calls `ClearPiece(to - 10)` (White) or `ClearPiece(to + 10)` (Black) to remove the captured pawn.
* **Castling (`MFLAGCA`):** Calls `MovePiece()` to jump the corresponding Rook (`A1->D1`, `H1->F1`, `A8->D8`, or `H8->F8`).


3. **Castling & EP Hash Reset:** Unhashes any active `enPas` square (`HASH_EP`), updates `castlePerm` via `CastlePerm[from/to]`, resets `pos->enPas = NO_SQ`, and re-hashes `HASH_CA`.
4. **Captures & Fifty-Move Rule:** Increments `pos->fiftyMove`. If `CAPTURED(move) != EMPTY`, calls `ClearPiece(to, pos)` and resets `pos->fiftyMove = 0`.
5. **Ply Advancement & Double Pawn Push:** Increments `pos->hisPly` and `pos->ply`. If a pawn moved, resets `pos->fiftyMove = 0`; if `MFLAGPS` is set, sets `pos->enPas = from ± 10` and hashes `HASH_EP`.
6. **Primary Piece Translation:** Calls `MovePiece(from, to, pos)`.
7. **Promotions:** If `PROMOTED(move) != EMPTY`, clears the pawn that just arrived at `to` and calls `AddPiece(to, pos, prPce)`.
8. **King Tracking & Side Flip:** Updates `pos->KingSq[side] = to` if a King moved, flips `pos->side ^= 1`, and toggles `HASH_SIDE`.
9. **Post-Move Check Legality Filter:**
Calls `SqAttacked(pos->KingSq[side], pos->side, pos)`. If the mover's own King is left under attack by the opponent, the move is illegal: `MakeMove()` immediately calls `TakeMove(pos)` and returns `FALSE` (`0`). Otherwise, it returns `TRUE` (`1`).

---

## 4. `TakeMove()` State Restoration

`void TakeMove(S_BOARD *pos)` reverses `MakeMove()` in exact mirror order:

1. Decrements `pos->hisPly` and `pos->ply`, and fetches `move` from `pos->history[pos->hisPly]`.
2. Restores `castlePerm`, `fiftyMove`, and `enPas` from `pos->history[pos->hisPly]`, keeping `pos->posKey` synchronized via `HASH_EP` and `HASH_CA`.
3. Flips `pos->side ^= 1` and toggles `HASH_SIDE`.
4. Undoes En Passant (restoring the enemy pawn on `to \mp 10`) or Castling (moving the Rook back to its corner).
5. Calls `MovePiece(to, from, pos)` and restores `pos->KingSq[pos->side]` if the King moved.
6. If `CAPTURED(move) != EMPTY`, calls `AddPiece(to, pos, captured)` to resurrect the captured piece on `to`.
7. If `PROMOTED(move) != EMPTY`, clears the promoted piece on `from` and restores a pawn (`wP` or `bP`) on `from`.

---

## 5. Perft Tree Verification (`perft.c`)

**Perft** (*Performance Test*) traverses the complete legal game tree to depth $D$ without pruning, counting every leaf node at `depth == 0`. BadBishop is verified against 4 canonical Chess Programming Wiki benchmark positions in `tests/test_perft.c`:

| Position | FEN | Edge Cases Tested | Depth | Verified Leaf Nodes |
| --- | --- | --- | --- | --- |
| **1. Start Position** | `rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1` | Opening pawn pushes, knight leaps, baseline tree growth | `4` | `197,281` |
| **2. Kiwipete** | `r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1` | Both-side castling rights loss, EP, promotions, captures | `3` | `97,862` |
| **3. Endgame EP/Pin** | `8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1` | Horizontal EP discovered check pins, king opposition | `4` | `43,238` |
| **4. Promotion Stress** | `r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1` | 4-way underpromotions, rook-corner captures revoking castling | `3` | `9,467` |

---

## 6. Storage & Runtime Profile

| Component | Storage Footprint | Memory Region | Time Complexity |
| --- | --- | --- | --- |
| **`CastlePerm[120]`** | `120 * 4 B = 480 B` | Static ROM/Flash | $O(1)$ (2 bitwise ANDs per move) |
| **`S_UNDO`** | `24 B` (`4 * int + U64` padded) | Inside `S_BOARD.history[]` | $O(1)$ array index via `hisPly` |
| **`MakeMove()`** | `0 B` heap (`~32 B` stack locals) | Stack | $O(1)$ (~45–80 CPU cycles + `SqAttacked`) |
| **`TakeMove()`** | `0 B` heap (`~24 B` stack locals) | Stack | $O(1)$ (~35–60 CPU cycles) |
| **`Perft(D)`** | `2,052 B * D` (`S_MOVELIST` per ply) | Stack (`~10 KB` at $D=5$) | $O(B^D)$ ($B \approx 35$ branching factor) |

---

## References

1. **Make and Unmake Move Architecture:** Chess Programming Wiki — [Make Move](https://www.chessprogramming.org/Make_Move) & [Unmake Move](https://www.chessprogramming.org/Unmake_Move)
2. **Canonical Perft Test Suite:** Chess Programming Wiki — [Perft Results](https://www.chessprogramming.org/Perft_Results)
