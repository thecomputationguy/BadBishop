/**
 * @file pvtable.c
 * @brief Transposition table (PV table) management, probing, storage, and PV extraction.
 */

#include "defs.h"
#include <assert.h>
#include <stdio.h>

static S_HASHENTRY
    HashPool[MAX_HASH_ENTRIES]; /**< Static memory pool for transposition table entries. */
S_HASHTABLE HashTable[1];       /**< Global transposition table instance. */

/**
 * @brief Computes the transposition table bucket index for a Zobrist position key.
 * @param key 64-bit Zobrist position key.
 */
#define HASH_INDEX(key) ((key) & HashTable->mask)

/**
 * @brief Verifies whether a packed move is legal in the current board position.
 * @param pos Pointer to the board state.
 * @param move Bitpacked move integer to verify.
 * @return TRUE if the move is legal, FALSE otherwise.
 */
static int MoveExists(S_BOARD *pos, const int move) {
    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);

    int MoveNum = 0;
    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        if (!MakeMove(pos, list->moves[MoveNum].move)) {
            continue;
        }
        TakeMove(pos);
        if (list->moves[MoveNum].move == move) {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * @brief Extracts the principal variation line from the transposition table into pos->PvArray.
 * @param depth Maximum number of plies to extract.
 * @param pos Pointer to the board state.
 * @return Number of moves extracted into the principal variation line.
 */
int GetPvLine(const int depth, S_BOARD *pos) {
    assert(depth < MAXDEPTH);

    int move = ProbePvMove(pos);
    int count = 0;

    while (move != NOMOVE && count < depth) {
        assert(count < MAXDEPTH);

        if (MoveExists(pos, move)) {
            MakeMove(pos, move);
            pos->PvArray[count++] = move;
        } else {
            break;
        }
        move = ProbePvMove(pos);
    }

    while (pos->ply > 0) {
        TakeMove(pos);
    }

    return count;
}

/**
 * @brief Clears all entries and resets statistics in a transposition table.
 * @param table Pointer to the hash table to clear.
 */
void ClearHashTable(S_HASHTABLE *table) {
    for (int i = 0; i < table->numEntries; ++i) {
        table->pTable[i].posKey = 0ULL;
        table->pTable[i].move = NOMOVE;
        table->pTable[i].depth = 0;
        table->pTable[i].score = 0;
        table->pTable[i].flags = HFNONE;
    }
    table->newWrite = 0;
    table->overWrite = 0;
    table->hit = 0;
    table->cut = 0;
}

/**
 * @brief Initializes a transposition table and clears all entries.
 * @param table Pointer to the hash table to initialize.
 */
void InitHashTable(S_HASHTABLE *table, int mb) {
    if (mb < 1) mb = 1;

    U64 maxBytes = (U64)mb * 1024ULL * 1024ULL;
    int targetEntries = (int)(maxBytes / sizeof(S_HASHENTRY));

    if (targetEntries > MAX_HASH_ENTRIES) {
        targetEntries = MAX_HASH_ENTRIES;
    }

    /* Round down to the largest power of two <= targetEntries for O(1) bitmasking */
    int pow2Entries = 1;
    while ((pow2Entries << 1) <= targetEntries) {
        pow2Entries <<= 1;
    }

    table->pTable = HashPool;
    table->numEntries = pow2Entries;
    table->mask = (U64)(pow2Entries - 1);
    ClearHashTable(table);
}

/**
 * @brief Probes the transposition table for a usable score or best move hint.
 * @param pos Pointer to the current board state.
 * @param move Output pointer receiving the stored move if the key matches.
 * @param score Output pointer receiving the usable score on a cutoff.
 * @param alpha Current alpha bound.
 * @param beta Current beta bound.
 * @param depth Remaining search depth in plies.
 * @return TRUE if a usable cutoff score was found, FALSE otherwise.
 */
int ProbeHashEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth) {
    U64 index = HASH_INDEX(pos->posKey);

    if (HashTable->pTable[index].posKey == pos->posKey) {
        *move = HashTable->pTable[index].move;
        if (HashTable->pTable[index].depth >= depth) {
            HashTable->hit++;

            *score = HashTable->pTable[index].score;
            if (*score > ISMATE) {
                *score -= pos->ply;
            } else if (*score < -ISMATE) {
                *score += pos->ply;
            }

            switch (HashTable->pTable[index].flags) {
            case HFALPHA:
                if (*score <= alpha) {
                    *score = alpha;
                    return TRUE;
                }
                break;
            case HFBETA:
                if (*score >= beta) {
                    *score = beta;
                    return TRUE;
                }
                break;
            case HFEXACT:
                return TRUE;
            default:
                assert(FALSE);
                break;
            }
        }
    }

    return FALSE;
}

/**
 * @brief Stores a position evaluation, best move, and bound in the transposition table.
 * @param pos Pointer to the current board state.
 * @param move Best move found for the position.
 * @param score Evaluation score to store (mate scores adjusted relative to ply).
 * @param flags Bound flag (HFALPHA, HFBETA, or HFEXACT).
 * @param depth Remaining search depth in plies.
 */
void StoreHashEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth) {
    U64 index = HASH_INDEX(pos->posKey);

    if (HashTable->pTable[index].posKey == 0ULL) {
        HashTable->newWrite++;
    } else {
        HashTable->overWrite++;
    }

    if (score > ISMATE) {
        score += pos->ply;
    } else if (score < -ISMATE) {
        score -= pos->ply;
    }

    HashTable->pTable[index].move = move;
    HashTable->pTable[index].posKey = pos->posKey;
    HashTable->pTable[index].flags = flags;
    HashTable->pTable[index].score = score;
    HashTable->pTable[index].depth = depth;
}

/**
 * @brief Probes the transposition table for the stored move of the current position.
 * @param pos Pointer to the current board state.
 * @return Stored move integer, or NOMOVE if not found.
 */
int ProbePvMove(const S_BOARD *pos) {
    U64 index = HASH_INDEX(pos->posKey);

    if (HashTable->pTable[index].posKey == pos->posKey) {
        return HashTable->pTable[index].move;
    }

    return NOMOVE;
}