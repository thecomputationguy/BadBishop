#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

int main(int argc, char** argv) {
    AllInit(); // Initialize all necessary data structures for the chess engine

    U64 testBoard = 0ULL; // Initialize an empty bitboard

    SETBIT(testBoard, SQ64(A1)); // 64-index: 0
    SETBIT(testBoard, SQ64(C3)); // 64-index: 18
    SETBIT(testBoard, SQ64(H8)); // 64-index: 63

    printf("Initial bitboard count: %d\n", CountBits(testBoard));

    while (testBoard) {
        int sq64 = PopBit(&testBoard);
        printf("Popped square 64: %d (which is 120-sq: %d)\n", sq64, SQ120(sq64));
    }

    printf("Count after clearing all bits: %d\n", CountBits(testBoard));
    return (EXIT_SUCCESS);
}

