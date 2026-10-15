#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

int main(int argc, char** argv) {
    AllInit(); // Initialize all necessary data structures for the chess engine

    printf("A1: File=%d, Rank=%d\n", FilesBrd[A1], RanksBrd[A1]);
    printf("E4: File=%d, Rank=%d\n", FilesBrd[E4], RanksBrd[E4]);
    printf("Index 0 (Offboard): File=%d, Rank=%d\n", FilesBrd[0], RanksBrd[0]);

    return (EXIT_SUCCESS);
}

