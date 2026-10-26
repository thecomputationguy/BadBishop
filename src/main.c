#include "defs.h"
#include <stdio.h>


int main(void) {
    AllInit();

    S_BOARD board[1];
    S_MOVELIST list[1];

    // Test 1: Start position (Expected: 20 moves: 16 pawn moves + 4 knight moves)
    ParseFen(START_FEN, board);
    GenerateAllMoves(board, list);
    printf("Start Position Move Count: %d (Expected: 20)\n", list->count);

    // Test 2: Kiwipete (Expected: 48 moves, including e1g1 and e1c1 castling)
    const char *kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ParseFen(kiwipete, board);
    PrintBoard(board);
    GenerateAllMoves(board, list);
    PrintMoveList(list);

    return 0;
}