#include "defs.h"
#include <assert.h>
#include <stdio.h>


static long leafNodes;

static void PerftRecursive(int depth, S_BOARD *pos) {
    assert(CheckBoard(pos));

    if (depth == 0) {
        leafNodes++;
        return;
    }

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int MoveNum = 0;
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        if (!MakeMove(pos, list->moves[MoveNum].move)) {
            continue;
        }
        PerftRecursive(depth - 1, pos);
        TakeMove(pos);
    }
}

long Perft(int depth, S_BOARD *pos) {
    assert(CheckBoard(pos));

    leafNodes = 0;
    PerftRecursive(depth, pos);
    return leafNodes;
}

void PerftTest(int depth, S_BOARD *pos) {
    assert(CheckBoard(pos));

    PrintBoard(pos);
    printf("\nStarting Perft toberry Depth: %d\n", depth);
    leafNodes = 0;

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int move;
    int MoveNum = 0;
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        move = list->moves[MoveNum].move;
        if (!MakeMove(pos, move)) {
            continue;
        }
        long cumnodes = leafNodes;
        PerftRecursive(depth - 1, pos);
        TakeMove(pos);
        long oldnodes = leafNodes - cumnodes;
        printf("move %d : %s : %ld\n", MoveNum + 1, PrMove(move), oldnodes);
    }

    printf("\nTest Complete : %ld nodes visited\n", leafNodes);
}