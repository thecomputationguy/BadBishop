#include "defs.h"
#include <stdio.h>

void PrintBitBoard(U64 bb) {
    /* Print the bitboard in a human-readable format */
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

int CountBits(U64 bb) {
    /*
    Counts the number of set bits in the bitboard using Kernighan's Algorithm.
    */
    int count = 0;
    while (bb) {
        bb &= (bb - 1); // Clear the least significant bit
        count++;
    }
    return count;
}

int PopBit(U64 *bb) {
    /*
    Pops the least significant bit from the bitboard and returns its index.
    */
    U64 b = *bb ^ (*bb - 1);
    unsigned int fold = (unsigned int)((b & 0xffffffff) ^ (b >> 32));
    *bb &= (*bb - 1);

    // TODO: De Bruijn-like or table lookup mapping, but standard builtin / count-trailing-zeros is
    // fast and portable
    return CountBits(b - 1);
}