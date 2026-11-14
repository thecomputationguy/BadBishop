/**
 * @file perft.c
 * @brief Performance test (perft) functions for verifying move generation correctness.
 */

#include "defs.h"
#include <assert.h>
#include <stdio.h>

static long leafNodes; /**< Accumulated leaf node count during a perft traversal. */

/**
 * @brief Recursively counts legal leaf nodes to the given depth.
 * @param depth Remaining search depth in plies.
 * @param pos Pointer to the board state.
 */
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

/**
 * @brief Counts the number of leaf nodes at a given depth from the current position.
 * @param depth Search depth in plies.
 * @param pos Pointer to the board state.
 * @return Total number of leaf nodes visited.
 */
long Perft(int depth, S_BOARD *pos) {
    assert(CheckBoard(pos));

    leafNodes = 0;
    PerftRecursive(depth, pos);
    return leafNodes;
}

/**
 * @brief Runs a perft test to the given depth and prints per-move node counts.
 * @param depth Search depth in plies.
 * @param pos Pointer to the board state.
 */
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