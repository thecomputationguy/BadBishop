/**
 * @file bitboards.c
 * @brief Bitboard manipulation and display utilities.
 */

#include "defs.h"
#include <stdio.h>

/**
 * @brief Prints the bitboard in a human-readable format.
 * @param bb The 64-bit bitboard to display.
 */
void PrintBitBoard(U64 bb) {
    int rank = 0;
    int file = 0;
    int sq = 0;
    int sq64 = 0;

    printf("\n");
    for (rank = RANK_8; rank >= RANK_1; --rank) {
        printf("%d  ", rank + 1); // Print rank number (1-8)
        for (file = FILE_A; file <= FILE_H; ++file) {
            sq = (21 + file) + (rank * 10); // 120-square coordinate
            sq64 = SQ64(sq);                // Convert to 64-square coordinate

            if ((1ULL << sq64) & bb) {
                printf("X ");
            } else {
                printf("- ");
            }
        }
        printf("\n");
    }
    printf("\n   a b c d e f g h\n\n");
}

/**
 * @brief Counts the number of set bits in the bitboard using Kernighan's Algorithm.
 * @param bb The 64-bit bitboard to count.
 * @return Number of bits set to 1.
 */
int CountBits(U64 bb) {
    int count = 0;
    while (bb) {
        bb &= (bb - 1); // Clear the least significant bit
        count++;
    }
    return count;
}

/**
 * @brief Pops the least significant bit from the bitboard and returns its index.
 * @param bb Pointer to the 64-bit bitboard to modify.
 * @return 0-based index (0-63) of the popped least significant bit.
 */
int PopBit(U64 *bb) {
    U64 b = *bb ^ (*bb - 1);
    unsigned int fold = (unsigned int)((b & 0xffffffff) ^ (b >> 32));
    *bb &= (*bb - 1);

    // TODO: De Bruijn-like or table lookup mapping, but standard builtin / count-trailing-zeros is
    // fast and portable
    return CountBits(b - 1);
}