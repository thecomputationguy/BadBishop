#include "defs.h"
#include <stdio.h>

char *PrSq(const int sq) {
    /*
    Print a square in algebraic notation.
    */
    static char SqStr[3];

    int file = FilesBrd[sq];
    int rank = RanksBrd[sq];

    sprintf(SqStr, "%c%c", ('a' + file), ('1' + rank));
    return SqStr;
}

char *PrMove(const int move) {
    /*
    Print a move in algebraic notation.
    */
    static char MvStr[6];

    int ff = FilesBrd[FROMSQ(move)];
    int rf = RanksBrd[FROMSQ(move)];
    int ft = FilesBrd[TOSQ(move)];
    int rt = RanksBrd[TOSQ(move)];

    int promoted = PROMOTED(move);

    if (promoted) {
        char pchar = 'q';

        // Determine the promotion piece
        if (PieceKnight[promoted] == TRUE) {
            pchar = 'n';
        } else if (PieceRookQueen[promoted] == TRUE && PieceBishopQueen[promoted] == FALSE) {
            pchar = 'r';
        } else if (PieceRookQueen[promoted] == FALSE && PieceBishopQueen[promoted] == TRUE) {
            pchar = 'b';
        }
        sprintf(MvStr, "%c%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt), pchar);
    } else {
        sprintf(MvStr, "%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt));
    }

    return MvStr;
}

void PrintMoveList(const S_MOVELIST *list) {
    /*
    Prints a list of moves in algebraic notation.
    */
    int index = 0;
    int score = 0;
    int move = 0;
    printf("MoveList (%d moves):\n", list->count);

    for (index = 0; index < list->count; ++index) {
        move = list->moves[index].move;
        score = list->moves[index].score;
        printf("Move:%d > %s (score:%d)\n", index + 1, PrMove(move), score);
    }
    printf("MoveList Total %d Moves:\n\n", list->count);
}