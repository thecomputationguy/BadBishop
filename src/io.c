/**
 * @file io.c
 * @brief Input/output formatting helpers for squares, moves, and move lists.
 */

#include "defs.h"
#include <stdio.h>

/**
 * @brief Prints a square in algebraic notation.
 * @param sq Square index on the 120-square board.
 * @return Pointer to a static null-terminated algebraic square string.
 */
char *PrSq(const int sq) {
    static char SqStr[3];

    int file = FilesBrd[sq];
    int rank = RanksBrd[sq];

    sprintf(SqStr, "%c%c", ('a' + file), ('1' + rank));
    return SqStr;
}

/**
 * @brief Print a move in algebraic notation.
 * @param move Bitpacked move integer.
 * @return Pointer to a static null-terminated algebraic move string.
 */
char *PrMove(const int move) {
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

/**
 * @brief Prints a list of moves in algebraic notation.
 * @param list Pointer to the move list to print.
 */
void PrintMoveList(const S_MOVELIST *list) {
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