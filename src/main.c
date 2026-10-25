#include <stdio.h>
#include "defs.h"

int main(void) {
    AllInit();

    S_BOARD board[1];

    printf("--- Loading Start Position ---\n");
    ParseFen(START_FEN, board);
    PrintBoard(board);
    printf("CheckBoard result: %s\n\n", CheckBoard(board) ? "PASSED" : "FAILED");

    printf("--- Loading Middlegame Position ---\n");
    char *fen = "r1bqk2r/pp2bppp/2n1pn2/3p4/2PP4/2N2N2/PP2BPPP/R1BQK2R w KQkq - 0 8";
    ParseFen(fen, board);
    PrintBoard(board);
    printf("CheckBoard result: %s\n\n", CheckBoard(board) ? "PASSED" : "FAILED");

    return 0;
}