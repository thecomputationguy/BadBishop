/**
 * @file hashkeys.c
 * @brief Zobrist hashing initialization and position key generation.
 */

#include "defs.h"
#include <stdlib.h>

U64 PieceKeys[13][120]; /**< Zobrist keys for each piece type on each square. */
U64 SideKey;            /**< Zobrist key for the side to move. */
U64 CastleKeys[16];     /**< Zobrist keys for each possible castling rights combination. */

/**
 * @brief Generates a random 64-bit number by combining multiple calls to rand() and shifting the bits appropriately.
 * @return A pseudo-random 64-bit unsigned integer.
 */
static U64 Rand64(void) {
    U64 r = 0ULL;
    r |= ((U64)rand() & 0x7FFF);
    r |= (((U64)rand() & 0x7FFF) << 15);
    r |= (((U64)rand() & 0x7FFF) << 30);
    r |= (((U64)rand() & 0x7FFF) << 45);
    r |= (((U64)rand() & 0xF) << 60);
    return r;
}

/**
 * @brief Initializes the Zobrist hash keys for pieces, side, and castling rights.
 */
void InitHashKeys(void) {
    int i = 0;
    int j = 0;

    for (i = 0; i < 13; ++i) {
        for (j = 0; j < 120; ++j) {
            PieceKeys[i][j] = Rand64();
        }
    }

    SideKey = Rand64();

    for (i = 0; i < 16; ++i) {
        CastleKeys[i] = Rand64();
    }
}

/**
 * @brief Generates a unique 64-bit hash key for the current board position.
 * @param pos Pointer to the board state.
 * @return 64-bit Zobrist position key.
 */
U64 GeneratePosKey(const S_BOARD *pos) {
    int sq = 0;
    U64 finalKey = 0ULL;
    int piece = EMPTY;

    // Pieces
    for (sq = 0; sq < BRD_SQ_NUM; ++sq) {
        piece = pos->pieces[sq];
        if (piece != NO_SQ && piece != EMPTY && piece != OFFBOARD) {
            finalKey ^= PieceKeys[piece][sq];
        }
    }

    // Side to move
    if (pos->side == WHITE) {
        finalKey ^= SideKey;
    }

    // En passant square
    if (pos->enPas != NO_SQ) {
        finalKey ^= PieceKeys[EMPTY][pos->enPas];
    }

    // Castling permissions (0 to 15)
    finalKey ^= CastleKeys[pos->castlePerm];

    return finalKey;
}