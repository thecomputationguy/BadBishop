#include "defs.h"

int main(void) {
    AllInit();

    S_BOARD board[1];
    const char *kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    ParseFen(kiwipete, board);
    PerftTest(3, board);

    return 0;
}