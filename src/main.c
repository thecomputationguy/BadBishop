#include <stdio.h>
#include "defs.h"

void ShowSqAtBySide(const int side, const S_BOARD *pos) {
    int rank = 0;
    int file = 0;
    int sq = 0;

    printf("\nSquares attacked by %c:\n\n", SideChar[side]);
    for (rank = RANK_8; rank >= RANK_1; --rank) {
        printf("%d  ", rank + 1);
        for (file = FILE_A; file <= FILE_H; ++file) {
            sq = FR2SQ(file, rank);
            if (SqAttacked(sq, side, pos) == TRUE) {
                printf(" X ");
            } else {
                printf(" - ");
            }
        }
        printf("\n");
    }
    printf("\n    a  b  c  d  e  f  g  h\n\n");
}

int main(void) {
    AllInit();

    S_BOARD board[1];

    // Open tactical position: White Queen on d4, Black Rook on e8, Knights & Pawns
    char *fen = "4r1k1/ppp2ppp/2n5/8/3Q4/5N2/PPP2PPP/4R1K1 w - - 0 1";
    ParseFen(fen, board);
    PrintBoard(board);

    ShowSqAtBySide(WHITE, board);
    ShowSqAtBySide(BLACK, board);

    return 0;
}