#include "defs.h"

// Direction offset arrays on the 120-square mailbox
const int KnDir[8] = { -8, -19, -21, -12, 8, 19, 21, 12 };
const int RkDir[4] = { -1, -10, 1, 10 };
const int BiDir[4] = { -9, -11, 11, 9 };
const int KiDir[8] = { -1, -10, 1, 10, -9, -11, 11, 9 };

int SqAttacked(const int sq, const int side, const S_BOARD *pos) {
    /* Check if a square is attacked by the specified side */
    int pce, index, t_sq, dir;

    // 1. Pawns
    if (side == WHITE) {
        if (pos->pieces[sq - 11] == wP || pos->pieces[sq - 9] == wP) {
            return TRUE;
        }
    } else {
        if (pos->pieces[sq + 11] == bP || pos->pieces[sq + 9] == bP) {
            return TRUE;
        }
    }

    // 2. Knights
    for (index = 0; index < 8; ++index) {
        pce = pos->pieces[sq + KnDir[index]];
        if (pce != OFFBOARD && PieceKnight[pce] == TRUE && PieceCol[pce] == side) {
            return TRUE;
        }
    }

    // 3. Rooks and Queens (Orthogonal rays)
    for (index = 0; index < 4; ++index) {
        dir = RkDir[index];
        t_sq = sq + dir;
        pce = pos->pieces[t_sq];
        while (pce != OFFBOARD) {
            if (pce != EMPTY) {
                if (PieceRookQueen[pce] == TRUE && PieceCol[pce] == side) {
                    return TRUE;
                }
                break; // Blocked by another piece
            }
            t_sq += dir;
            pce = pos->pieces[t_sq];
        }
    }

    // 4. Bishops and Queens (Diagonal rays)
    for (index = 0; index < 4; ++index) {
        dir = BiDir[index];
        t_sq = sq + dir;
        pce = pos->pieces[t_sq];
        while (pce != OFFBOARD) {
            if (pce != EMPTY) {
                if (PieceBishopQueen[pce] == TRUE && PieceCol[pce] == side) {
                    return TRUE;
                }
                break; // Blocked by another piece
            }
            t_sq += dir;
            pce = pos->pieces[t_sq];
        }
    }

    // 5. Kings
    for (index = 0; index < 8; ++index) {
        pce = pos->pieces[sq + KiDir[index]];
        if (pce != OFFBOARD && PieceKing[pce] == TRUE && PieceCol[pce] == side) {
            return TRUE;
        }
    }

    return FALSE;
}