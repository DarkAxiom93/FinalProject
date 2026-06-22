#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "casino.h"
#include "utils.h"
#include "graphics.h"
#include "cards.h"
#include "account.h"

#define RANK_BASE     15
#define CATEGORY_UNIT 759375 /* RANK_BASE^5: tiebreak space reserved per category */

#define SCORE_HIGH_CARD       (0 * CATEGORY_UNIT)
#define SCORE_ONE_PAIR        (1 * CATEGORY_UNIT)
#define SCORE_TWO_PAIR        (2 * CATEGORY_UNIT)
#define SCORE_THREE_OF_A_KIND (3 * CATEGORY_UNIT)
#define SCORE_STRAIGHT        (4 * CATEGORY_UNIT)
#define SCORE_FLUSH           (5 * CATEGORY_UNIT)
#define SCORE_FULL_HOUSE      (6 * CATEGORY_UNIT)
#define SCORE_FOUR_OF_A_KIND  (7 * CATEGORY_UNIT)
#define SCORE_STRAIGHT_FLUSH  (8 * CATEGORY_UNIT)

/* Packs up to 5 ranks (highest priority first) into one tiebreak number,
   so two hands of the same category compare correctly card-by-card, not just by one rank. */
static int pack_ranks(int r0, int r1, int r2, int r3, int r4) {
    return ((((r0 * RANK_BASE) + r1) * RANK_BASE + r2) * RANK_BASE + r3) * RANK_BASE + r4;
}

/* Highest `n` ranks present in ranks[2..14], skipping exclude_a/exclude_b. Missing slots are 0. */
static void top_ranks(const int ranks[15], int exclude_a, int exclude_b, int n, int out[]) {
    int idx = 0;
    for (int i = 14; i >= 2 && idx < n; i--) {
        if (ranks[i] > 0 && i != exclude_a && i != exclude_b) out[idx++] = i;
    }
    while (idx < n) out[idx++] = 0;
}

int evaluate_poker_hand(Card hand[], int count) {
    int ranks[15] = { 0 };
    int suits[4] = { 0 };
    int suit_ranks[4][15] = { 0 };

    for (int i = 0; i < count; i++) {
        ranks[hand[i].rank_val]++;
        int s_idx = -1;
        if (hand[i].suit == 'H') s_idx = 0;
        else if (hand[i].suit == 'D') s_idx = 1;
        else if (hand[i].suit == 'C') s_idx = 2;
        else if (hand[i].suit == 'S') s_idx = 3;

        if (s_idx != -1) {
            suits[s_idx]++;
            suit_ranks[s_idx][hand[i].rank_val]++;
        }
    }

    int straight_flush_high = 0;
    for (int s = 0; s < 4; s++) {
        if (suits[s] >= 5) {
            int cons = 0;
            for (int i = 14; i >= 2; i--) {
                if (suit_ranks[s][i] > 0) {
                    cons++;
                    if (cons >= 5 && straight_flush_high == 0) straight_flush_high = i + 4;
                }
                else {
                    cons = 0;
                }
            }
            if (suit_ranks[s][14] > 0 && suit_ranks[s][2] > 0 && suit_ranks[s][3] > 0 &&
                suit_ranks[s][4] > 0 && suit_ranks[s][5] > 0) {
                if (straight_flush_high == 0) straight_flush_high = 5;
            }
        }
    }
    if (straight_flush_high > 0) return SCORE_STRAIGHT_FLUSH + pack_ranks(straight_flush_high, 0, 0, 0, 0);

    // רביעיה: דרגת הרביעיה עצמה + קלף קיקר אחד
    int quad_rank = 0;
    for (int i = 14; i >= 2; i--) {
        if (ranks[i] == 4) { quad_rank = i; break; }
    }
    if (quad_rank > 0) {
        int kicker[1];
        top_ranks(ranks, quad_rank, -1, 1, kicker);
        return SCORE_FOUR_OF_A_KIND + pack_ranks(quad_rank, kicker[0], 0, 0, 0);
    }

    // השלשה הגבוהה ביותר, וה"זוג" הגבוה ביותר מדרגה אחרת
    // (יכול להיות שארית של שלשה שנייה, למשל 7-7-7 + K-K-K => פול האוס מלכים מלא בשבעות)
    int trip_rank = 0;
    for (int i = 14; i >= 2; i--) {
        if (ranks[i] >= 3) { trip_rank = i; break; }
    }
    int pair_rank = 0;
    if (trip_rank > 0) {
        for (int i = 14; i >= 2; i--) {
            if (i != trip_rank && ranks[i] >= 2) { pair_rank = i; break; }
        }
    }
    if (trip_rank > 0 && pair_rank > 0) {
        return SCORE_FULL_HOUSE + pack_ranks(trip_rank, pair_rank, 0, 0, 0);
    }

    // פלאש: חמשת הקלפים הגבוהים מתוך צבע הפלאש בלבד (לא מהיד כולה)
    int flush_suit = -1;
    for (int s = 0; s < 4; s++) {
        if (suits[s] >= 5) { flush_suit = s; break; }
    }
    if (flush_suit != -1) {
        int top5[5];
        top_ranks(suit_ranks[flush_suit], -1, -1, 5, top5);
        return SCORE_FLUSH + pack_ranks(top5[0], top5[1], top5[2], top5[3], top5[4]);
    }

    // סטרייט רגיל
    int straight_high = 0;
    int cons = 0;
    for (int i = 14; i >= 2; i--) {
        if (ranks[i] > 0) {
            cons++;
            if (cons >= 5 && straight_high == 0) straight_high = i + 4;
        }
        else {
            cons = 0;
        }
    }
    if (ranks[14] > 0 && ranks[2] > 0 && ranks[3] > 0 && ranks[4] > 0 && ranks[5] > 0) {
        if (straight_high == 0) straight_high = 5;
    }
    if (straight_high > 0) return SCORE_STRAIGHT + pack_ranks(straight_high, 0, 0, 0, 0);

    if (trip_rank > 0) {
        int kickers[2];
        top_ranks(ranks, trip_rank, -1, 2, kickers);
        return SCORE_THREE_OF_A_KIND + pack_ranks(trip_rank, kickers[0], kickers[1], 0, 0);
    }

    int pair1 = 0, pair2 = 0;
    for (int i = 14; i >= 2 && pair2 == 0; i--) {
        if (ranks[i] == 2) {
            if (pair1 == 0) pair1 = i;
            else pair2 = i;
        }
    }
    if (pair1 > 0 && pair2 > 0) {
        int kicker[1];
        top_ranks(ranks, pair1, pair2, 1, kicker);
        return SCORE_TWO_PAIR + pack_ranks(pair1, pair2, kicker[0], 0, 0);
    }
    if (pair1 > 0) {
        int kickers[3];
        top_ranks(ranks, pair1, -1, 3, kickers);
        return SCORE_ONE_PAIR + pack_ranks(pair1, kickers[0], kickers[1], kickers[2], 0);
    }

    int top5[5];
    top_ranks(ranks, -1, -1, 5, top5);
    return SCORE_HIGH_CARD + pack_ranks(top5[0], top5[1], top5[2], top5[3], top5[4]);
}

static const char* get_hand_name(int score) {
    if (score >= SCORE_STRAIGHT_FLUSH) return "Straight Flush";
    if (score >= SCORE_FOUR_OF_A_KIND) return "Four of a Kind";
    if (score >= SCORE_FULL_HOUSE) return "Full House";
    if (score >= SCORE_FLUSH) return "Flush";
    if (score >= SCORE_STRAIGHT) return "Straight";
    if (score >= SCORE_THREE_OF_A_KIND) return "Three of a Kind";
    if (score >= SCORE_TWO_PAIR) return "Two Pair";
    if (score >= SCORE_ONE_PAIR) return "One Pair";
    return "High Card";
}

void play_poker(Player* player) {
    int is_playing = 1;
    print_poker_welcome();

    while (is_playing) {
        clear_screen();
        print_table_header("ULTIMATE TEXAS HOLD'EM", "" C_MAGENTA "", player->balance);

        printf("Options: [0] Leave Table  [1] Place Bets\nAction: ");
        int action = get_menu_key("01") - '0';
        if (action == 0) break;

        printf("Enter ANTE amount (BLIND will be identical): $");
        int ante = get_safe_int();
        int blind = ante;

        printf("Enter optional TRIPS bet (Enter 0 to skip): $");
        int trips = get_safe_int();

        int total_initial_bet = ante + blind + trips;

        if (total_initial_bet <= 0 || total_initial_bet > player->balance) {
            display_error(1500, "Invalid amounts or insufficient funds!");
            continue;
        }
        if (total_initial_bet > MAX_BET) {
            display_error(1500, "Table maximum total initial bet is $%d!", MAX_BET);
            continue;
        }
        player->balance -= total_initial_bet;
        save_player(player);
        int play_bet = 0;
        int has_folded = 0;

        // שימוש במנוע הקלפים הגנרי - יצירת חפיסה אחת וערבוב
        Card deck[52];
        init_deck(deck, 1);
        shuffle_deck(deck, 52);

        int d_idx = 0;
        Card player_cards[2] = { 0 }, dealer_cards[2] = { 0 }, community[5] = { 0 };

        player_cards[0] = deck[d_idx++]; player_cards[1] = deck[d_idx++];
        dealer_cards[0] = deck[d_idx++]; dealer_cards[1] = deck[d_idx++];
        community[0] = deck[d_idx++]; community[1] = deck[d_idx++]; community[2] = deck[d_idx++];
        community[3] = deck[d_idx++]; community[4] = deck[d_idx++];

        // ===========================
        // שלב 1: טרום פלופ (Pre-Flop)
        // ===========================
        printf("\n" C_CYAN "Dealing hole cards..." C_RESET "\n");

        printf(" [ Legend: " C_RED "H" C_RESET "=Hearts | " C_RED "D" C_RESET "=Diamonds | \x1b[97mC" C_RESET "=Clubs | \x1b[97mS" C_RESET "=Spades ]\n");

        print_cards_ascii(player_cards, 2, "Your Hand", 0);

        // חישוב והצגת היד הנוכחית 
        int current_score = evaluate_poker_hand(player_cards, 2);
        printf("Current Hand: " C_GREEN "%s" C_RESET "\n", get_hand_name(current_score));

        printf("Current Bets -> Ante: $%d | Blind: $%d | Trips: $%d\n", ante, blind, trips);
        printf("\n" C_YELLOW "--- PRE-FLOP ACTION ---" C_RESET "\n");
        printf("Options: [1] CHECK  [2] PLAY (3x = $%d)  [3] PLAY (4x = $%d)\nAction: ", ante * 3, ante * 4);
        int pre_flop_action = get_menu_key("123") - '0';
        if (pre_flop_action == 2 || pre_flop_action == 3) {
            int mult = (pre_flop_action == 2) ? 3 : 4;

            if (ante * mult > MAX_BET) {
                display_error(2500, "Play bet of $%d exceeds MAX_BET ($%d)! Auto-checking...", ante * mult, MAX_BET);
            }
            else if (player->balance < ante * mult) {
                display_error(1500, "Insufficient funds for Play bet. Auto-checking...");
            }
            else {
                play_bet = ante * mult;
                player->balance -= play_bet;
                save_player(player);
                printf("" C_GREEN "Play bet of $%d placed." C_RESET "\n", play_bet);
            }
        }

        // ======================
        // שלב 2: פלופ (The Flop)
        // ======================
        printf("\n" C_CYAN "Dealing the FLOP..." C_RESET "\n");
        delay_ms(1000);

        print_cards_ascii(player_cards, 2, "Your Hand", 0);
        print_cards_ascii(community, 3, "Community Cards (FLOP)", 0);

        // חישוב והצגת היד בשילוב הקלפים הקהילתיים 
        Card flop_eval[5] = { 0 };
        flop_eval[0] = player_cards[0]; flop_eval[1] = player_cards[1];
        flop_eval[2] = community[0]; flop_eval[3] = community[1]; flop_eval[4] = community[2];
        current_score = evaluate_poker_hand(flop_eval, 5);
        printf("Current Hand: " C_GREEN "%s" C_RESET "\n", get_hand_name(current_score));

        if (play_bet == 0) {
            printf("\n" C_YELLOW "--- FLOP ACTION ---" C_RESET "\n");
            printf("Options: [1] CHECK  [2] PLAY (2x = $%d)\nAction: ", ante * 2);
            int flop_action = get_menu_key("12") - '0';

            if (flop_action == 2) {
                if (ante * 2 > MAX_BET) {
                    display_error(2500, "Play bet of $%d exceeds MAX_BET ($%d)! Auto-checking...", ante * 2, MAX_BET);
                }
                else if (player->balance < ante * 2) {
                    display_error(1500, "Insufficient funds for Play bet. Auto-checking...");
                }
                else {
                    play_bet = ante * 2;
                    player->balance -= play_bet;
                    save_player(player);
                    printf("" C_GREEN "Play bet of $%d placed." C_RESET "\n", play_bet);
                }
            }
        }

        // ===============================
        // שלב 3: טרן וריבר (Turn & River)
        // ===============================
        printf("\n" C_CYAN "Dealing Turn & River..." C_RESET "\n");
        delay_ms(1000);

        print_cards_ascii(player_cards, 2, "Your Hand", 0);
        print_cards_ascii(community, 5, "Final Community Cards", 0);

        // חישוב והצגת היד בשילוב כל הקלפים 
        Card river_eval[7] = { 0 };
        river_eval[0] = player_cards[0]; river_eval[1] = player_cards[1];
        for (int i = 0; i < 5; i++) river_eval[i + 2] = community[i];
        current_score = evaluate_poker_hand(river_eval, 7);
        printf("Current Hand: " C_GREEN "%s" C_RESET "\n", get_hand_name(current_score));

        if (play_bet == 0) {
            printf("\n" C_YELLOW "--- FINAL ACTION ---" C_RESET "\n");
            printf("Options: [1] PLAY (1x = $%d)  [2] FOLD\nAction: ", ante);
            int river_action = get_menu_key("12") - '0';

            if (river_action == 2) {
                printf("" C_RED "You FOLDED. Ante ($%d) and Blind ($%d) are lost." C_RESET "\n", ante, blind);
                player->total_losses += ((long long)ante + blind);
                has_folded = 1;
            }
            else {
                if (player->balance < ante) {
                    display_error(2000, "Insufficient funds to call! You are forced to fold.");
                    player->total_losses += ((long long)ante + blind);
                    has_folded = 1;
                }
                else {
                    play_bet = ante;
                    player->balance -= play_bet;
                    save_player(player);
                    printf("" C_GREEN "Play bet of $%d placed." C_RESET "\n", play_bet);
                }
            }
        }

        // ==========================================
        // שלב 4: חשיפה והערכת ידיים (Showdown)
        // ==========================================

        // 1. הערכת היד של השחקן (מתבצעת תמיד, גם אם קופלנו, בשביל ה-Trips)
        Card p_eval[7] = { 0 };
        for (int i = 0; i < 5; i++) { p_eval[i] = community[i]; }
        p_eval[5] = player_cards[0]; p_eval[6] = player_cards[1];
        int p_score = evaluate_poker_hand(p_eval, 7);

        // 2. בדיקת זכייה ב-Trips (מתבצעת תמיד, win or lose or fold)
        if (trips > 0) {
            int trips_win = 0;
            if (p_score >= SCORE_STRAIGHT_FLUSH) trips_win = trips * 50;
            else if (p_score >= SCORE_FOUR_OF_A_KIND) trips_win = trips * 30;
            else if (p_score >= SCORE_FULL_HOUSE) trips_win = trips * 8;
            else if (p_score >= SCORE_FLUSH) trips_win = trips * 7;
            else if (p_score >= SCORE_STRAIGHT) trips_win = trips * 4;
            else if (p_score >= SCORE_THREE_OF_A_KIND) trips_win = trips * 3;

            if (trips_win > 0) {
                printf("\n" C_GREEN "TRIPS BET WON! Your final hand was %s! Payout: $%d" C_RESET "\n", get_hand_name(p_score), trips_win);
                player->total_winnings += trips_win;
                add_balance_safe(player, trips + trips_win);
            }
            else {
                printf("\n" C_RED "Trips bet lost." C_RESET "\n");
                player->total_losses += trips;
            }
        }

        if (!has_folded) {
            printf("\n" C_YELLOW "Dealer is flipping their hole cards..." C_RESET "\n");

            // מותחים את השחקן
            for (int i = 0; i < 3; i++) {
                printf(". ");
                fflush(stdout);
                delay_ms(500);
            }
            printf("\n");

            print_cards_ascii(dealer_cards, 2, "Dealer Reveals Hand", 0);
            delay_ms(1000); 

            Card d_eval[7] = { 0 };
            for (int i = 0; i < 5; i++) { d_eval[i] = community[i]; }
            d_eval[5] = dealer_cards[0]; d_eval[6] = dealer_cards[1];
            int d_score = evaluate_poker_hand(d_eval, 7);

            printf("\nYour Best Hand  : " C_GREEN "%s" C_RESET "\n", get_hand_name(p_score));
            printf("Dealer Best Hand: " C_RED "%s" C_RESET "\n", get_hand_name(d_score));

            if (p_score > d_score) {
                int total_win = (ante * 2) + (play_bet * 2);
                int blind_win = blind;

                if (p_score >= SCORE_STRAIGHT) {
                    if (p_score >= SCORE_STRAIGHT_FLUSH) blind_win += blind * 50;
                    else if (p_score >= SCORE_FOUR_OF_A_KIND) blind_win += blind * 10;
                    else if (p_score >= SCORE_FULL_HOUSE) blind_win += blind * 3;
                    else if (p_score >= SCORE_FLUSH) blind_win += blind * 1;
                    else blind_win += blind * 1;
                }

                int total_payout = total_win + blind_win;
                printf("\n" C_GREEN "YOU WIN THE HAND! Collected: $%d" C_RESET "\n", total_payout);
                player->total_winnings += ((long long)total_payout - ((long long)ante + blind + play_bet));
                add_balance_safe(player, total_payout);
            }
            else if (d_score > p_score) {
                printf("\n" C_RED "DEALER WINS THE HAND!" C_RESET "\n");
                player->total_losses += ((long long)ante + blind + play_bet);
            }
            else {
                printf("\n" C_YELLOW "PUSH (TIE)! Main bets returned." C_RESET "\n");
                add_balance_safe(player, ante + blind + play_bet);
            }
        }
        else {
            // התראה קצרה ומהירה שחוסכת לשחקן את זמן האנימציה של הדילר
            printf("\n" C_YELLOW "Main hand skipped because you folded pre-river." C_RESET "\n");
        }
        save_player(player);
        prompt_continue(NULL);

        if (player->balance <= 0) {
            printf("\n" C_RED "You are bankrupt! Security is escorting you out." C_RESET "\n");
            is_playing = 0;
        }
    }
}