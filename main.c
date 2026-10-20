#include <stdio.h>
#include "defs.h"

int main(void) {
    AllInit();

    S_BOARD board[1];
    ResetBoard(board);

    // Place test pieces
    board->pieces[E1] = wK;
    board->pieces[E8] = bK;
    board->pieces[D4] = wP;
    board->pieces[E4] = bP;
    board->side = WHITE;
    board->castlePerm = WKCA | WQCA | BKCA | BQCA;
    board->posKey = GeneratePosKey(board);

    PrintBoard(board);

    return 0;
}