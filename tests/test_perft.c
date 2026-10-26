#include <stdio.h>
#include "defs.h"

int main(void) {
    AllInit();

    S_BOARD board[1];

    // Test 1: Standard Start Position
    if (ParseFen(START_FEN, board) != 0) return 1;
    if (!CheckBoard(board)) return 1;

    // Test 2: Kiwipete Position
    const char *kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    if (ParseFen(kiwipete, board) != 0) return 1;
    if (!CheckBoard(board)) return 1;

    printf("All board integrity tests PASSED!\n");
    return 0;
}