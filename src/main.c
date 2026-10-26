#include "defs.h"
#include <stdio.h>

int main(void) {
    AllInit();

    // Test 1: Quiet pawn push e2 -> e4 (Pawn Start flag)
    int move1 = MOVE(E2, E4, EMPTY, EMPTY, MFLAGPS);
    printf("Move 1: %s (From: %s, To: %s, PawnStart: %d)\n", PrMove(move1), PrSq(FROMSQ(move1)),
           PrSq(TOSQ(move1)), (move1 & MFLAGPS) != 0);

    // Test 2: Knight promotion with capture on a8 (b7 -> a8=N, capturing Black Rook)
    int move2 = MOVE(B7, A8, bR, wN, 0);
    printf("Move 2: %s (Captured piece ID: %d, Promoted ID: %d)\n", PrMove(move2), CAPTURED(move2),
           PROMOTED(move2));

    return 0;
}