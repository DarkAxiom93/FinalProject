#ifndef TEST_API_H
#define TEST_API_H
/*
 * Declarations for internal functions exposed for testing (Option B: static removed).
 * Do NOT include this header in production code.
 */
#include "cards.h"
#include "casino.h"

/* blackjack.c */
int calculate_hand_value(Card* hand, int count);
int is_soft_17(Card* hand, int count);

/* poker.c */
int evaluate_poker_hand(Card hand[], int count);

/* roulette.c */
int check_win(Bet b, int spin_result);

/* Poker hand score thresholds (mirrors poker.c defines) */
#define T_STRAIGHT_FLUSH  800000
#define T_FOUR_OF_A_KIND  700000
#define T_FULL_HOUSE      600000
#define T_FLUSH           500000
#define T_STRAIGHT        400000
#define T_THREE_OF_A_KIND 300000
#define T_TWO_PAIR        200000
#define T_ONE_PAIR        100000

#endif
