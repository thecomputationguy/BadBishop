#include <stdio.h>
#include "defs.h"

const char PceChar[] = ".PNBRQKpnbrqk";
const char SideChar[] = "wb-";

void PrintBoard(const S_BOARD *pos) {
    /*
    Prints the current board state in a human-readable format.
    Displays the board with ranks and files, along with the side to move, en passant square, castling rights, and the position key.
    */
    int sq, file, rank, piece;

    printf("\nGame Board:\n\n");

    for (rank = RANK_8; rank >= RANK_1; rank--) {
        printf("%d  ", rank + 1);
        for (file = FILE_A; file <= FILE_H; file++) {
            sq = FR2SQ(file, rank);
            piece = pos->pieces[sq];
            printf("%3c", PceChar[piece]);
        }
        printf("\n");
    }

    printf("\n   ");
    for (file = FILE_A; file <= FILE_H; file++) {
        printf("%3c", 'a' + file);
    }
    printf("\n\n");

    printf("side: %c\n", SideChar[pos->side]);
    printf("enPas: %d\n", pos->enPas);
    printf("castle: %c%c%c%c\n",
           pos->castlePerm & WKCA ? 'K' : '-',
           pos->castlePerm & WQCA ? 'Q' : '-',
           pos->castlePerm & BKCA ? 'k' : '-',
           pos->castlePerm & BQCA ? 'q' : '-');
    printf("posKey: %llX\n\n", pos->posKey);

}

void ResetBoard(S_BOARD *pos) {
    /*
    Resets the board to an empty state.
    */
    int index = 0;

    // 1. Set all 120 squares to OFFBOARD
    for (index = 0; index < BRD_SQ_NUM; index++) {
        pos->pieces[index] = OFFBOARD;
    }

    // 2. Set the 64 squares on the board to EMPTY
    for (index = 0; index < 64; index++) {
        int sq120 = Sq64ToSq120[index];
        pos->pieces[sq120] = EMPTY;
    }

    // 3. Reset piece counters
    for (index = 0; index < 3; ++index) {
        pos->bigPce[index] = 0;
        pos->majPce[index] = 0;
        pos->minPce[index] = 0;
        pos->pawns[index] = 0ULL; // Reset pawn bitboards
    }

    for (index = 0; index < 13; ++index) {
        pos->pceNum[index] = 0;
    }

    // 4. Reset king coordinates
    pos->KingSq[WHITE] = NO_SQ;
    pos->KingSq[BLACK] = NO_SQ;

    // 5. Reset board state flags
    pos->side = BOTH; // No side to move
    pos->enPas = NO_SQ; // No en passant square
    pos->fiftyMove = 0; // Reset fifty-move counter
    pos->ply = 0; // Reset search depth
    pos->hisPly = 0; // Reset total half-moves played
    pos->castlePerm = 0; // No castling rights
    pos->posKey = 0ULL; // Reset position key
}