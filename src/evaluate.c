/**
 * @file evaluate.c
 * @brief Static position evaluation, piece-square tables, and personality-based scoring.
 */

#include "defs.h"
#include <stdlib.h>

/** @brief Personality configurations indexed by ENGINE_STYLE. */
const S_PERSONALITY Personalities[3] = {
    // STYLE_BALANCED: Objective 100% weights, 0 contempt
    {100, 100, 100, 100, 100, 100, 0  },

    // STYLE_POSITIONAL: High pawn structure & defense, conservative attack
    {105, 110, 160, 110, 140, 85,  0  },

    // STYLE_SHARP: Discounts material slightly, loves mobility & enemy king attacks, hates draws
    {92,  110, 75,  145, 80,  185, -50}
};

static const int PawnIsolated = -10; /**< Penalty for an isolated pawn. */
const int PawnDoubled = -10;         /**< Penalty for a doubled pawn. */
static const int PawnPassed[8] = {0,  5,  10,  20,
                                  35, 60, 100, 200}; /**< Passed pawn bonus indexed by rank. */
static const int RookOpenFile = 10;                  /**< Bonus for a rook on an open file. */
static const int RookSemiOpenFile = 5;               /**< Bonus for a rook on a semi-open file. */
static const int QueenOpenFile = 5;                  /**< Bonus for a queen on an open file. */
static const int QueenSemiOpenFile = 3;              /**< Bonus for a queen on a semi-open file. */
static const int BishopPair = 30;                    /**< Bonus for having the bishop pair. */
static const int KingOpenFilePenalty =
    25; /**< Penalty for a king on an open/semi-open file in middlegame. */

/** @brief Endgame material threshold (1 Rook + 2 Minors + 2 Pawns = 1350). */
#define ENDGAME_MAT 1350

// clang-format off
/** @brief Piece-square table for pawns (White's perspective). */
static const int PawnTable[64] = {
    0,   0,   0,   0,   0,   0,   0,   0,
   10,  10,   0, -10, -10,   0,  10,  10,
    5,   0,   0,   5,   5,   0,   0,   5,
    0,   0,  10,  20,  20,  10,   0,   0,
    5,   5,   5,  10,  10,   5,   5,   5,
   10,  10,  10,  20,  20,  10,  10,  10,
   20,  20,  20,  30,  30,  20,  20,  20,
    0,   0,   0,   0,   0,   0,   0,   0
};

/** @brief Piece-square table for knights (White's perspective). */
static const int KnightTable[64] = {
    0, -10,   0,   0,   0,   0, -10,   0,
    0,   0,   0,   5,   5,   0,   0,   0,
    0,   0,  10,  10,  10,  10,   0,   0,
    0,   0,  10,  20,  20,  10,   5,   0,
    5,  10,  15,  20,  20,  15,  10,   5,
    5,  10,  10,  20,  20,  10,  10,   5,
    0,   0,   5,  10,  10,   5,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

/** @brief Piece-square table for bishops (White's perspective). */
static const int BishopTable[64] = {
    0,   0, -10,   0,   0, -10,   0,   0,
    0,   0,   0,  10,  10,   0,   0,   0,
    0,   0,  10,  15,  15,  10,   0,   0,
    0,  10,  15,  20,  20,  15,  10,   0,
    0,  10,  15,  20,  20,  15,  10,   0,
    0,   0,  10,  15,  15,  10,   0,   0,
    0,   0,   0,  10,  10,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0
};

/** @brief Piece-square table for rooks (White's perspective). */
static const int RookTable[64] = {
    0,   0,   5,  10,  10,   5,   0,   0,
    0,   0,   5,  10,  10,   5,   0,   0,
    0,   0,   5,  10,  10,   5,   0,   0,
    0,   0,   5,  10,  10,   5,   0,   0,
    0,   0,   5,  10,  10,   5,   0,   0,
    0,   0,   5,  10,  10,   5,   0,   0,
   25,  25,  25,  25,  25,  25,  25,  25,
    0,   0,   5,  10,  10,   5,   0,   0
};

/** @brief Piece-square table for kings in the endgame (White's perspective). */
static const int KingE[64] = {
  -50, -10,   0,   0,   0,   0, -10, -50,
  -10,   0,  10,  10,  10,  10,   0, -10,
    0,  10,  20,  20,  20,  20,  10,   0,
    0,  10,  20,  40,  40,  20,  10,   0,
    0,  10,  20,  40,  40,  20,  10,   0,
    0,  10,  20,  20,  20,  20,  10,   0,
  -10,   0,  10,  10,  10,  10,   0, -10,
  -50, -10,   0,   0,   0,   0, -10, -50
};

/** @brief Piece-square table for kings in the opening/middlegame (White's perspective). */
static const int KingO[64] = {
    0,   5,   5, -10, -10,   0,  10,   5,
  -30, -30, -30, -30, -30, -30, -30, -30,
  -50, -50, -50, -50, -50, -50, -50, -50,
  -70, -70, -70, -70, -70, -70, -70, -70,
  -70, -70, -70, -70, -70, -70, -70, -70,
  -70, -70, -70, -70, -70, -70, -70, -70,
  -70, -70, -70, -70, -70, -70, -70, -70,
  -70, -70, -70, -70, -70, -70, -70, -70
};

/** @brief Lookup table to vertically mirror a 64-square index for Black's perspective. */
const int Mirror64[64] = {
   56,  57,  58,  59,  60,  61,  62,  63,
   48,  49,  50,  51,  52,  53,  54,  55,
   40,  41,  42,  43,  44,  45,  46,  47,
   32,  33,  34,  35,  36,  37,  38,  39,
   24,  25,  26,  27,  28,  29,  30,  31,
   16,  17,  18,  19,  20,  21,  22,  23,
    8,   9,  10,  11,  12,  13,  14,  15,
    0,   1,   2,   3,   4,   5,   6,   7
};
// clang-format on

/**
 * @brief Mirrors a 64-square index vertically between White and Black perspectives.
 * @param sq 64-square index (0-63).
 */
#define MIRROR64(sq) (Mirror64[(sq)])

/**
 * @brief Fast material draw detection (K vs K, KN vs K, KB vs K, KNN vs K).
 * @param pos Pointer to the board state.
 * @return TRUE if the position is a draw due to insufficient material, FALSE otherwise.
 */
static int MaterialDraw(const S_BOARD *pos) {
    if (!pos->pceNum[wR] && !pos->pceNum[bR] && !pos->pceNum[wQ] && !pos->pceNum[bQ]) {
        if (!pos->pceNum[bB] && !pos->pceNum[wB]) {
            if (pos->pceNum[wN] < 3 && pos->pceNum[bN] < 3) {
                return TRUE;
            }
        } else if (!pos->pceNum[wN] && !pos->pceNum[bN]) {
            if (abs(pos->pceNum[wB] - pos->pceNum[bB]) < 2) {
                return TRUE;
            }
        } else if ((pos->pceNum[wN] < 3 && !pos->pceNum[wB]) ||
                   (pos->pceNum[wB] == 1 && !pos->pceNum[wN])) {
            if ((pos->pceNum[bN] < 3 && !pos->pceNum[bB]) ||
                (pos->pceNum[bB] == 1 && !pos->pceNum[bN])) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/**
 * @brief Returns the contempt value based on the current position's style.
 * @param pos Pointer to the board state.
 * @return Contempt score in centipawns.
 */
int GetContempt(const S_BOARD *pos) {
    int style = pos->style;
    if (style < STYLE_BALANCED || style > STYLE_SHARP) {
        style = STYLE_BALANCED;
    }
    return Personalities[style].contempt;
}

/**
 * @brief Evaluates the current position and returns a score from the side-to-move's perspective.
 * @param pos Pointer to the board state.
 * @return Static evaluation score in centipawns.
 */
int EvalPosition(const S_BOARD *pos) {
    if (!pos->pceNum[wP] && !pos->pceNum[bP] && MaterialDraw(pos) == TRUE) {
        return 0;
    }

    int pce;
    int pceNum;
    int sq;

    // 5 Distinct Evaluation Feature Buckets (from White's perspective)
    int matScore = pos->material[WHITE] - pos->material[BLACK];
    int pstScore = 0;
    int pawnScore = 0;
    int mobScore = 0;
    int whiteKingSafety = 0;
    int blackKingSafety = 0;

    // Bishop Pair Bonus
    if (pos->pceNum[wB] >= 2) matScore += BishopPair;
    if (pos->pceNum[bB] >= 2) matScore -= BishopPair;

    // 1. White Pawns (PST + Isolated + Passed Bitboards)
    pce = wP;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        pstScore += PawnTable[SQ64(sq)];

        if ((IsolatedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
            pawnScore += PawnIsolated;
        }
        if ((WhitePassedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
            pawnScore += PawnPassed[RanksBrd[sq]];
        }
    }

    // 2. Black Pawns (PST + Isolated + Passed Bitboards)
    pce = bP;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        pstScore -= PawnTable[MIRROR64(SQ64(sq))];

        if ((IsolatedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
            pawnScore -= PawnIsolated;
        }
        if ((BlackPassedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
            pawnScore -= PawnPassed[7 - RanksBrd[sq]];
        }
    }

    // Doubled & Tripled Pawn Penalties (O(1) per file via FileBBMask)
    for (int file = FILE_A; file <= FILE_H; ++file) {
        U64 wFilePawns = pos->pawns[WHITE] & FileBBMask[file];
        int wCount = CountBits(wFilePawns);
        if (wCount > 1) {
            pawnScore += (wCount - 1) * PawnDoubled;
        }

        U64 bFilePawns = pos->pawns[BLACK] & FileBBMask[file];
        int bCount = CountBits(bFilePawns);
        if (bCount > 1) {
            pawnScore -= (bCount - 1) * PawnDoubled;
        }
    }

    // 3. Knights
    pce = wN;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        pstScore += KnightTable[SQ64(sq)];
    }
    pce = bN;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        pstScore -= KnightTable[MIRROR64(SQ64(sq))];
    }

    // 4. Bishops
    pce = wB;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        pstScore += BishopTable[SQ64(sq)];
    }
    pce = bB;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        pstScore -= BishopTable[MIRROR64(SQ64(sq))];
    }

    // 5. Rooks (PST + Open / Semi-Open File Mobility)
    pce = wR;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        pstScore += RookTable[SQ64(sq)];

        if ((pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]]) == 0) {
            mobScore += RookOpenFile;
        } else if ((pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]]) == 0) {
            mobScore += RookSemiOpenFile;
        }
    }
    pce = bR;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        pstScore -= RookTable[MIRROR64(SQ64(sq))];

        if ((pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]]) == 0) {
            mobScore -= RookOpenFile;
        } else if ((pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]]) == 0) {
            mobScore -= RookSemiOpenFile;
        }
    }

    // 6. Queens (Open / Semi-Open File Activity)
    pce = wQ;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        if ((pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]]) == 0) {
            mobScore += QueenOpenFile;
        } else if ((pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]]) == 0) {
            mobScore += QueenSemiOpenFile;
        }
    }
    pce = bQ;
    for (pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
        sq = pos->pList[pce][pceNum];
        if ((pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]]) == 0) {
            mobScore -= QueenOpenFile;
        } else if ((pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]]) == 0) {
            mobScore -= QueenSemiOpenFile;
        }
    }

    // 7. Kings (Phase-Tapered PST + Middlegame Pawn Shield / Open File Danger)
    sq = pos->KingSq[WHITE];
    if ((pos->material[BLACK] - PieceVal[bK]) <= ENDGAME_MAT) {
        pstScore += KingE[SQ64(sq)];
    } else {
        pstScore += KingO[SQ64(sq)];
        int kFile = FilesBrd[sq];
        if ((pos->pawns[WHITE] & FileBBMask[kFile]) == 0) {
            whiteKingSafety -= KingOpenFilePenalty;
        }
    }

    sq = pos->KingSq[BLACK];
    if ((pos->material[WHITE] - PieceVal[wK]) <= ENDGAME_MAT) {
        pstScore -= KingE[MIRROR64(SQ64(sq))];
    } else {
        pstScore -= KingO[MIRROR64(SQ64(sq))];
        int kFile = FilesBrd[sq];
        if ((pos->pawns[BLACK] & FileBBMask[kFile]) == 0) {
            blackKingSafety -= KingOpenFilePenalty;
        }
    }

    // 8. Apply Personality Weights
    int style = pos->style;
    if (style < STYLE_BALANCED || style > STYLE_SHARP) {
        style = STYLE_BALANCED;
    }
    const S_PERSONALITY *p = &Personalities[style];

    int kingSafetyTerm = 0;
    if (pos->side == WHITE) {
        kingSafetyTerm = (whiteKingSafety * p->ownKingSafety - blackKingSafety * p->oppKingAttack);
    } else {
        kingSafetyTerm = (whiteKingSafety * p->oppKingAttack - blackKingSafety * p->ownKingSafety);
    }

    int finalScore =
        (matScore * p->materialWeight + pstScore * p->pstWeight + pawnScore * p->pawnStructWeight +
         mobScore * p->mobilityWeight + kingSafetyTerm) /
        100;

    return (pos->side == WHITE) ? finalScore : -finalScore;
}