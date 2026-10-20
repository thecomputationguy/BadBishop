#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

int main(void) {
    AllInit();

    S_BOARD board[1];

    // Test 1: Standard Start Position
    printf("--- Test 1: Start Position ---");
    ParseFen(START_FEN, board);
    PrintBoard(board);

    // Test 2: Position with en-passant and partial castling
    char *fen2 = "r1bqk2r/pp2bppp/2n1pn2/3p4/2PP4/2N2N2/PP2BPPP/R1BQK2R w KQkq d6 0 8";
    printf("--- Test 2: Middlegame Position ---");
    ParseFen(fen2, board);
    PrintBoard(board);

    return EXIT_SUCCESS;
}