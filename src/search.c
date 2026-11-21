/**
 * @file search.c
 * @brief Iterative deepening alpha-beta search, quiescence search, and move ordering.
 */

#include "defs.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>

static const int VictimScore[13] = {0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600}; /**< Base victim values for MVV-LVA capture ordering. */
static int MvvLvaScores[13][13]; /**< Precomputed MVV-LVA scores indexed by [victim][attacker]. */

/**
 * @brief Initializes the MVV-LVA (Most Valuable Victim - Least Valuable Attacker) capture score table.
 */
void InitMvvLva(void) {
    int Attacker;
    int Victim;
    for (Attacker = wP; Attacker <= bK; ++Attacker) {
        for (Victim = wP; Victim <= bK; ++Victim) {
            MvvLvaScores[Victim][Attacker] =
                VictimScore[Victim] + 6 - (VictimScore[Attacker] / 100);
        }
    }
}

/**
 * @brief Returns the current processor time in milliseconds.
 * @return Elapsed time in milliseconds.
 */
long GetTimeMs(void) {
    return (long)((clock() * 1000ULL) / CLOCKS_PER_SEC);
}

/**
 * @brief Checks whether the allocated search time has expired and sets info->stopped if so.
 * @param info Pointer to the search state and timing configuration.
 */
static void CheckUp(S_SEARCHINFO *info) {
    if (info->timeset == TRUE && GetTimeMs() > info->stoptime) {
        info->stopped = TRUE;
    }
}

/**
 * @brief Performs selection sort step to swap the highest-scored remaining move into moveNum.
 * @param moveNum Current index in the move list.
 * @param list Pointer to the move list to reorder in place.
 */
static void PickNextMove(int moveNum, S_MOVELIST *list) {
    S_MOVE temp;
    int index = 0;
    int bestScore = -1;
    int bestNum = moveNum;

    for (index = moveNum; index < list->count; ++index) {
        if (list->moves[index].score > bestScore) {
            bestScore = list->moves[index].score;
            bestNum = index;
        }
    }

    temp = list->moves[moveNum];
    list->moves[moveNum] = list->moves[bestNum];
    list->moves[bestNum] = temp;
}

/**
 * @brief Assigns move-ordering scores to all moves in a move list (PV, MVV-LVA captures, killers, history).
 * @param pos Pointer to the board state.
 * @param list Pointer to the move list whose entries will be scored.
 * @param pvMove Principal variation move from the transposition table (or NOMOVE).
 */
static void ScoreMoves(const S_BOARD *pos, S_MOVELIST *list, int pvMove) {
    for (int i = 0; i < list->count; ++i) {
        int move = list->moves[i].move;

        if (move == pvMove) {
            list->moves[i].score = 2000000;
        } else if (CAPTURED(move) != EMPTY) {
            int victim = CAPTURED(move);
            int attacker = pos->pieces[FROMSQ(move)];
            list->moves[i].score = MvvLvaScores[victim][attacker] + 1000000;
        } else if (move & MFLAGEP) {
            list->moves[i].score = 105 + 1000000;
        } else {
            if (pos->searchKillers[0][pos->ply] == move) {
                list->moves[i].score = 900000;
            } else if (pos->searchKillers[1][pos->ply] == move) {
                list->moves[i].score = 800000;
            } else {
                int pce = pos->pieces[FROMSQ(move)];
                list->moves[i].score = pos->searchHistory[pce][TOSQ(move)];
            }
        }
    }
}

/**
 * @brief Checks if the current board position has occurred previously within the 50-move window.
 * @param pos Pointer to the board state.
 * @return TRUE if a repetition is detected, FALSE otherwise.
 */
int IsRepetition(const S_BOARD *pos) {
    int index = 0;

    for (index = pos->hisPly - pos->fiftyMove; index < pos->hisPly - 1; ++index) {
        assert(index >= 0 && index < MAXGAMEMOVES);
        if (pos->posKey == pos->history[index].posKey) {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * @brief Resets search heuristics, transposition table counters, and search info prior to starting a search.
 * @param pos Pointer to the board state.
 * @param info Pointer to the search info structure to reset.
 */
static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info) {
    int index = 0;
    int index2 = 0;

    for (index = 0; index < 13; ++index) {
        for (index2 = 0; index2 < BRD_SQ_NUM; ++index2) {
            pos->searchHistory[index][index2] = 0;
        }
    }

    for (index = 0; index < 2; ++index) {
        for (index2 = 0; index2 < MAXDEPTH; ++index2) {
            pos->searchKillers[index][index2] = 0;
        }
    }

    HashTable->overWrite = 0;
    HashTable->hit = 0;
    HashTable->cut = 0;
    pos->ply = 0;

    info->stopped = 0;
    info->nodes = 0;
    info->fh = 0.0f;
    info->fhf = 0.0f;
}

/**
 * @brief Searches capture sequences at leaf nodes to reach a quiet position and mitigate the horizon effect.
 * @param alpha Lower bound score.
 * @param beta Upper bound score.
 * @param pos Pointer to the board state.
 * @param info Pointer to search parameters and statistics.
 * @return Quiescence evaluation score from the side-to-move's perspective.
 */
static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info) {
    assert(CheckBoard(pos));

    if ((info->nodes & 2047) == 0) {
        CheckUp(info);
    }

    info->nodes++;

    if ((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
        return GetContempt(pos);
    }

    if (pos->ply > MAXDEPTH - 1) {
        return EvalPosition(pos);
    }

    // Stand-Pat Score (Baseline if we make no more captures)
    int Score = EvalPosition(pos);

    if (Score >= beta) {
        return beta;
    }

    if (Score > alpha) {
        alpha = Score;
    }

    S_MOVELIST list[1];
    GenerateAllCaps(pos, list);
    ScoreMoves(pos, list, NOMOVE);

    int MoveNum = 0;
    int Legal = 0;
    Score = -INF_BOUND;

    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        PickNextMove(MoveNum, list);

        // Delta Pruning: Skip captures that cannot possibly raise score to alpha
        int captured = CAPTURED(list->moves[MoveNum].move);
        if (captured != EMPTY && PROMOTED(list->moves[MoveNum].move) == EMPTY) {
            int standPat = EvalPosition(pos);
            if (standPat + PieceVal[captured] + 200 < alpha) {
                continue;
            }
        }

        if (!MakeMove(pos, list->moves[MoveNum].move)) {
            continue;
        }

        Legal++;
        Score = -Quiescence(-beta, -alpha, pos, info);
        TakeMove(pos);

        if (info->stopped == TRUE) {
            return 0;
        }

        if (Score > alpha) {
            if (Score >= beta) {
                if (Legal == 1) {
                    info->fhf++;
                }
                info->fh++;
                return beta;
            }
            alpha = Score;
        }
    }

    return alpha;
}

/**
 * @brief Performs negamax alpha-beta search with transposition table probing, killers, and history heuristics.
 * @param alpha Lower bound score.
 * @param beta Upper bound score.
 * @param depth Remaining search depth in plies.
 * @param pos Pointer to the board state.
 * @param info Pointer to search parameters and statistics.
 * @param DoNull Whether null-move pruning is permitted in this subtree.
 * @return Best score found within [alpha, beta] from the side-to-move's perspective.
 */
static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull) {
    assert(CheckBoard(pos));
    (void)DoNull; // Will be activated in Step 19B

    if (depth <= 0) {
        return Quiescence(alpha, beta, pos, info);
    }

    if ((info->nodes & 2047) == 0) {
        CheckUp(info);
    }

    info->nodes++;

    if ((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
        return GetContempt(pos);
    }

    if (pos->ply > MAXDEPTH - 1) {
        return EvalPosition(pos);
    }

    // Probe Transposition Table
    int Score = -INF_BOUND;
    int PvMove = NOMOVE;

    if (ProbeHashEntry(pos, &PvMove, &Score, alpha, beta, depth) == TRUE) {
        HashTable->cut++;
        return Score;
    }

    S_MOVELIST list[1];
    GenerateAllMoves(pos, list);
    ScoreMoves(pos, list, PvMove);

    int MoveNum = 0;
    int Legal = 0;
    int OldAlpha = alpha;
    int BestMove = NOMOVE;
    int BestScore = -INF_BOUND;
    Score = -INF_BOUND;

    for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        PickNextMove(MoveNum, list);

        if (!MakeMove(pos, list->moves[MoveNum].move)) {
            continue;
        }

        Legal++;
        Score = -AlphaBeta(-beta, -alpha, depth - 1, pos, info, TRUE);
        TakeMove(pos);

        if (info->stopped == TRUE) {
            return 0;
        }

        if (Score > BestScore) {
            BestScore = Score;
            BestMove = list->moves[MoveNum].move;
            if (Score > alpha) {
                if (Score >= beta) {
                    if (Legal == 1) {
                        info->fhf++;
                    }
                    info->fh++;

                    // Update Killer Moves & History Heuristic on quiet move Beta-Cutoff
                    if (!(list->moves[MoveNum].move & MFLAGCAP)) {
                        pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
                        pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
                        pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] +=
                            depth * depth;
                    }

                    StoreHashEntry(pos, BestMove, beta, HFBETA, depth);
                    return beta;
                }
                alpha = Score;

                // Reward quiet moves that improve Alpha
                if (!(list->moves[MoveNum].move & MFLAGCAP)) {
                    pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
                }
            }
        }
    }

    // Checkmate or Stalemate Detection
    if (Legal == 0) {
        if (SqAttacked(pos->KingSq[pos->side], pos->side ^ 1, pos)) {
            return -INF_BOUND + pos->ply; // Checkmate (shorter mate = higher score)
        } else {
            return 0; // Stalemate
        }
    }

    if (alpha != OldAlpha) {
        StoreHashEntry(pos, BestMove, BestScore, HFEXACT, depth);
    } else {
        StoreHashEntry(pos, BestMove, alpha, HFALPHA, depth);
    }

    return alpha;
}

/**
 * @brief Searches the current position using iterative deepening alpha-beta and prints UCI info.
 * @param pos Pointer to the board state.
 * @param info Pointer to search parameters and statistics.
 */
void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info) {
    int bestMove = NOMOVE;
    int bestScore = -INF_BOUND;
    int currentDepth = 0;
    int pvMoves = 0;
    int pvNum = 0;

    ClearForSearch(pos, info);

    // Iterative Deepening Loop
    for (currentDepth = 1; currentDepth <= info->depth; ++currentDepth) {
        bestScore = AlphaBeta(-INF_BOUND, INF_BOUND, currentDepth, pos, info, TRUE);

        if (info->stopped == TRUE) {
            break;
        }

        pvMoves = GetPvLine(currentDepth, pos);
        bestMove = pos->PvArray[0];

        printf("info score cp %d depth %d nodes %ld time %ld pv", bestScore, currentDepth,
               info->nodes, GetTimeMs() - info->starttime);
        for (pvNum = 0; pvNum < pvMoves; ++pvNum) {
            printf(" %s", PrMove(pos->PvArray[pvNum]));
        }
        printf("\n");
    }

    printf("bestmove %s\n", PrMove(bestMove));
}