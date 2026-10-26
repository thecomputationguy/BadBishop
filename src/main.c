#include "defs.h"
#include <stdio.h>


int main(void) {
    AllInit();

    S_BOARD board[1];
    S_MOVELIST list[1];

    // White pawn test FEN: promotions on rank 7, en-passant on c6, double pushes on rank 2
    char *pawn_white_fen = "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1";
    ParseFen(pawn_white_fen, board);
    PrintBoard(board);

    GenerateAllMoves(board, list);
    PrintMoveList(list);

    return 0;
}