/**
 * @file movegen.c
 * @brief Pseudo-legal move generation for all pieces, captures, castling, and en passant.
 */

#include "defs.h"
#include <stdio.h>

/**
 * @brief Checks if a 120-square index is off the 8x8 board.
 * @param sq Square index on the 120-square board.
 */
#define SQOFFBOARD(sq) (FilesBrd[(sq)] == OFFBOARD)

// clang-format off
const int LoopSlidePce[8] = { wB, wR, wQ, 0, bB, bR, bQ, 0 };    /**< Sliding piece types grouped by side (0-terminated). */
const int LoopNonSlidePce[6] = { wN, wK, 0, bN, bK, 0 };         /**< Non-sliding piece types grouped by side (0-terminated). */
const int LoopSlideIndex[2] = { 0, 4 };                          /**< Starting index into LoopSlidePce for WHITE and BLACK. */
const int LoopNonSlideIndex[2] = { 0, 3 };                       /**< Starting index into LoopNonSlidePce for WHITE and BLACK. */

/** @brief Direction offsets on the 120-square board for each piece type. */
const int PceDir[13][8] = {
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { -8, -19, -21, -12, 8, 19, 21, 12 },
    { -9, -11, 11, 9, 0, 0, 0, 0 },
    { -1, -10, 1, 10, 0, 0, 0, 0 },
    { -1, -10, 1, 10, -9, -11, 11, 9 },
    { -1, -10, 1, 10, -9, -11, 11, 9 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { -8, -19, -21, -12, 8, 19, 21, 12 },
    { -9, -11, 11, 9, 0, 0, 0, 0 },
    { -1, -10, 1, 10, 0, 0, 0, 0 },
    { -1, -10, 1, 10, -9, -11, 11, 9 },
    { -1, -10, 1, 10, -9, -11, 11, 9 }
};

/** @brief Number of valid direction offsets in PceDir for each piece type. */
const int NumDir[13] = {
    0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8
};
// clang-format on

/**
 * @brief Add a quiet move to the move list.
 * @param pos Pointer to the board state.
 * @param move Bitpacked move integer.
 * @param list Pointer to the move list.
 */
static void AddQuietMove(const S_BOARD *pos, int move, S_MOVELIST *list) {
    (void)pos;
    list->moves[list->count].move = move;
    list->moves[list->count].score = 0;
    list->count++;
}

/**
 * @brief Add a capture move to the move list.
 * @param pos Pointer to the board state.
 * @param move Bitpacked move integer.
 * @param list Pointer to the move list.
 */
static void AddCaptureMove(const S_BOARD *pos, int move, S_MOVELIST *list) {
    (void)pos;
    list->moves[list->count].move = move;
    list->moves[list->count].score = 0;
    list->count++;
}

/**
 * @brief Add an en passant move to the move list.
 * @param pos Pointer to the board state.
 * @param move Bitpacked move integer.
 * @param list Pointer to the move list.
 */
static void AddEnPassantMove(const S_BOARD *pos, int move, S_MOVELIST *list) {
    (void)pos;
    list->moves[list->count].move = move;
    list->moves[list->count].score = 0;
    list->count++;
}

/**
 * @brief Add a white pawn capture move to the move list, generating promotions if on rank 7.
 * @param pos Pointer to the board state.
 * @param from Origin square index.
 * @param to Destination square index.
 * @param cap Captured piece type.
 * @param list Pointer to the move list.
 */
static void AddWhitePawnCapMove(const S_BOARD *pos, const int from, const int to, const int cap,
                                S_MOVELIST *list) {
    if (RanksBrd[from] == RANK_7) {
        AddCaptureMove(pos, MOVE(from, to, cap, wQ, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wR, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wB, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, wN, 0), list);
    } else {
        AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
    }
}

/**
 * @brief Add a white pawn move to the move list, generating promotions if on rank 7.
 * @param pos Pointer to the board state.
 * @param from Origin square index.
 * @param to Destination square index.
 * @param list Pointer to the move list.
 */
static void AddWhitePawnMove(const S_BOARD *pos, const int from, const int to, S_MOVELIST *list) {
    if (RanksBrd[from] == RANK_7) {
        AddQuietMove(pos, MOVE(from, to, EMPTY, wQ, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wR, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wB, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, wN, 0), list);
    } else {
        AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
    }
}

/**
 * @brief Add a black pawn capture move to the move list, generating promotions if on rank 2.
 * @param pos Pointer to the board state.
 * @param from Origin square index.
 * @param to Destination square index.
 * @param cap Captured piece type.
 * @param list Pointer to the move list.
 */
static void AddBlackPawnCapMove(const S_BOARD *pos, const int from, const int to, const int cap,
                                S_MOVELIST *list) {
    if (RanksBrd[from] == RANK_2) {
        AddCaptureMove(pos, MOVE(from, to, cap, bQ, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bR, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bB, 0), list);
        AddCaptureMove(pos, MOVE(from, to, cap, bN, 0), list);
    } else {
        AddCaptureMove(pos, MOVE(from, to, cap, EMPTY, 0), list);
    }
}

/**
 * @brief Add a black pawn move to the move list, generating promotions if on rank 2.
 * @param pos Pointer to the board state.
 * @param from Origin square index.
 * @param to Destination square index.
 * @param list Pointer to the move list.
 */
static void AddBlackPawnMove(const S_BOARD *pos, const int from, const int to, S_MOVELIST *list) {
    if (RanksBrd[from] == RANK_2) {
        AddQuietMove(pos, MOVE(from, to, EMPTY, bQ, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bR, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bB, 0), list);
        AddQuietMove(pos, MOVE(from, to, EMPTY, bN, 0), list);
    } else {
        AddQuietMove(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
    }
}

/**
 * @brief Generates all possible pseudo-legal moves for the current position and stores them in a move list.
 * @param pos Pointer to the board state.
 * @param list Pointer to the move list to populate.
 */
void GenerateAllMoves(const S_BOARD *pos, S_MOVELIST *list) {
    list->count = 0;

    int pce = EMPTY;
    int side = pos->side;
    int sq = 0;
    int t_sq = 0;
    int pceNum = 0;
    int dir = 0;
    int index = 0;
    int pceIndex = 0;

    if (side == WHITE) {
        for (pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum) {
            sq = pos->pList[wP][pceNum];

            if (pos->pieces[sq + 10] == EMPTY) {
                AddWhitePawnMove(pos, sq, sq + 10, list);
                if (RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY) {
                    AddQuietMove(pos, MOVE(sq, (sq + 20), EMPTY, EMPTY, MFLAGPS), list);
                }
            }

            if (!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
                AddWhitePawnCapMove(pos, sq, sq + 9, pos->pieces[sq + 9], list);
            }
            if (!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
                AddWhitePawnCapMove(pos, sq, sq + 11, pos->pieces[sq + 11], list);
            }

            if (pos->enPas != NO_SQ) {
                if (sq + 9 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq, sq + 9, EMPTY, EMPTY, MFLAGEP), list);
                }
                if (sq + 11 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq, sq + 11, EMPTY, EMPTY, MFLAGEP), list);
                }
            }
        }

        // White Castling
        if (pos->castlePerm & WKCA) {
            if (pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY) {
                if (!SqAttacked(E1, BLACK, pos) && !SqAttacked(F1, BLACK, pos)) {
                    AddQuietMove(pos, MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }
        if (pos->castlePerm & WQCA) {
            if (pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY && pos->pieces[B1] == EMPTY) {
                if (!SqAttacked(E1, BLACK, pos) && !SqAttacked(D1, BLACK, pos)) {
                    AddQuietMove(pos, MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }

    } else {
        for (pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {
            sq = pos->pList[bP][pceNum];

            if (pos->pieces[sq - 10] == EMPTY) {
                AddBlackPawnMove(pos, sq, sq - 10, list);
                if (RanksBrd[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY) {
                    AddQuietMove(pos, MOVE(sq, (sq - 20), EMPTY, EMPTY, MFLAGPS), list);
                }
            }

            if (!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
                AddBlackPawnCapMove(pos, sq, sq - 9, pos->pieces[sq - 9], list);
            }
            if (!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
                AddBlackPawnCapMove(pos, sq, sq - 11, pos->pieces[sq - 11], list);
            }

            if (pos->enPas != NO_SQ) {
                if (sq - 9 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq, sq - 9, EMPTY, EMPTY, MFLAGEP), list);
                }
                if (sq - 11 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq, sq - 11, EMPTY, EMPTY, MFLAGEP), list);
                }
            }
        }

        // Black Castling
        if (pos->castlePerm & BKCA) {
            if (pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY) {
                if (!SqAttacked(E8, WHITE, pos) && !SqAttacked(F8, WHITE, pos)) {
                    AddQuietMove(pos, MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }
        if (pos->castlePerm & BQCA) {
            if (pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY && pos->pieces[B8] == EMPTY) {
                if (!SqAttacked(E8, WHITE, pos) && !SqAttacked(D8, WHITE, pos)) {
                    AddQuietMove(pos, MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }
    }

    /* Loop for Sliders (Bishops, Rooks, Queens) */
    pceIndex = LoopSlideIndex[side];
    pce = LoopSlidePce[pceIndex++];
    while (pce != 0) {
        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
            sq = pos->pList[pce][pceNum];

            for (index = 0; index < NumDir[pce]; ++index) {
                dir = PceDir[pce][index];
                t_sq = sq + dir;

                while (!SQOFFBOARD(t_sq)) {
                    // BLACK ^ 1 == WHITE, WHITE ^ 1 == BLACK
                    if (pos->pieces[t_sq] != EMPTY) {
                        if (PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                            AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                        }
                        break;
                    }
                    AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
                    t_sq += dir;
                }
            }
        }
        pce = LoopSlidePce[pceIndex++];
    }

    /* Loop for Non-Sliders (Knights, Kings) */
    pceIndex = LoopNonSlideIndex[side];
    pce = LoopNonSlidePce[pceIndex++];
    while (pce != 0) {
        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
            sq = pos->pList[pce][pceNum];

            for (index = 0; index < NumDir[pce]; ++index) {
                dir = PceDir[pce][index];
                t_sq = sq + dir;

                if (SQOFFBOARD(t_sq)) {
                    continue;
                }

                if (pos->pieces[t_sq] != EMPTY) {
                    if (PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                        AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                    }
                    continue;
                }
                AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
            }
        }
        pce = LoopNonSlidePce[pceIndex++];
    }
}

/**
 * @brief Generates all capture moves (including en passant) for the current position.
 * @param pos Pointer to the board state.
 * @param list Pointer to the move list to populate.
 */
void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list) {
    list->count = 0;

    int pce = EMPTY;
    int side = pos->side;
    int sq = 0;
    int t_sq = 0;
    int pceNum = 0;
    int dir = 0;
    int index = 0;
    int pceIndex = 0;

    if (side == WHITE) {
        for (pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum) {
            sq = pos->pList[wP][pceNum];

            // Diagonal captures (+9 and +11)
            if (!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
                AddWhitePawnCapMove(pos, sq, sq + 9, pos->pieces[sq + 9], list);
            }
            if (!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
                AddWhitePawnCapMove(pos, sq, sq + 11, pos->pieces[sq + 11], list);
            }

            // En Passant captures
            if (pos->enPas != NO_SQ) {
                if (sq + 9 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq, sq + 9, EMPTY, EMPTY, MFLAGEP), list);
                }
                if (sq + 11 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq, sq + 11, EMPTY, EMPTY, MFLAGEP), list);
                }
            }
        }
    } else {
        for (pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {
            sq = pos->pList[bP][pceNum];

            // Diagonal captures (-9 and -11)
            if (!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
                AddBlackPawnCapMove(pos, sq, sq - 9, pos->pieces[sq - 9], list);
            }
            if (!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
                AddBlackPawnCapMove(pos, sq, sq - 11, pos->pieces[sq - 11], list);
            }

            // En Passant captures
            if (pos->enPas != NO_SQ) {
                if (sq - 9 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq, sq - 9, EMPTY, EMPTY, MFLAGEP), list);
                }
                if (sq - 11 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq, sq - 11, EMPTY, EMPTY, MFLAGEP), list);
                }
            }
        }
    }

    /* Loop for Sliders (Bishops, Rooks, Queens) — Captures Only */
    pceIndex = LoopSlideIndex[side];
    pce = LoopSlidePce[pceIndex++];
    while (pce != 0) {
        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
            sq = pos->pList[pce][pceNum];

            for (index = 0; index < NumDir[pce]; ++index) {
                dir = PceDir[pce][index];
                t_sq = sq + dir;

                while (!SQOFFBOARD(t_sq)) {
                    if (pos->pieces[t_sq] != EMPTY) {
                        if (PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                            AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                        }
                        break;
                    }
                    t_sq += dir;
                }
            }
        }
        pce = LoopSlidePce[pceIndex++];
    }

    /* Loop for Non-Sliders (Knights, Kings) — Captures Only */
    pceIndex = LoopNonSlideIndex[side];
    pce = LoopNonSlidePce[pceIndex++];
    while (pce != 0) {
        for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
            sq = pos->pList[pce][pceNum];

            for (index = 0; index < NumDir[pce]; ++index) {
                dir = PceDir[pce][index];
                t_sq = sq + dir;

                if (SQOFFBOARD(t_sq)) {
                    continue;
                }

                if (pos->pieces[t_sq] != EMPTY) {
                    if (PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                        AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                    }
                    continue;
                }
            }
        }
        pce = LoopNonSlidePce[pceIndex++];
    }
}