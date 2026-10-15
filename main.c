#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

int main(int argc, char** argv) {
    AllInit(); // Initialize all necessary data structures for the chess engine

    S_BOARD board;
    int i;

    for (i = 0; i < BRD_SQ_NUM; ++i) {
        board.pieces[i] = OFFBOARD;
    }
    
    for (i = 0; i < 64; ++i) {
        board.pieces[SQ120(i)] = EMPTY;
    }

    board.side = WHITE;
    board.enPas = NO_SQ;
    board.castlePerm = 0;

    // Place a White King on E1 and Black King on E8
    board.pieces[E1] = wK;
    board.pieces[E8] = bK;

    U64 key1 = GeneratePosKey(&board);
    printf("Initial Hash Key: %llx\n", key1);

    // Change side to BLACK and observe key difference
    board.side = BLACK;
    U64 key2 = GeneratePosKey(&board);
    printf("Hash Key after side change: %llx\n", key2);

    return (EXIT_SUCCESS);
}

