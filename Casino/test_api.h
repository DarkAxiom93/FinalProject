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

/* Poker hand score thresholds (mirrors poker.c defines: CATEGORY_UNIT = RANK_BASE^5 = 15^5) */
#define T_CATEGORY_UNIT   759375
#define T_STRAIGHT_FLUSH  (8 * T_CATEGORY_UNIT)
#define T_FOUR_OF_A_KIND  (7 * T_CATEGORY_UNIT)
#define T_FULL_HOUSE      (6 * T_CATEGORY_UNIT)
#define T_FLUSH           (5 * T_CATEGORY_UNIT)
#define T_STRAIGHT        (4 * T_CATEGORY_UNIT)
#define T_THREE_OF_A_KIND (3 * T_CATEGORY_UNIT)
#define T_TWO_PAIR        (2 * T_CATEGORY_UNIT)
#define T_ONE_PAIR        (1 * T_CATEGORY_UNIT)

#endif
