//Known:
// //Limitation : Evaluation engine currently resolves ties based on primary hand value and ignores kicker cards(leads to slightly higher Push rate).

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

#define SCORE_STRAIGHT_FLUSH  800000
#define SCORE_FOUR_OF_A_KIND  700000
#define SCORE_FULL_HOUSE      600000
#define SCORE_FLUSH           500000
#define SCORE_STRAIGHT        400000
#define SCORE_THREE_OF_A_KIND 300000
#define SCORE_TWO_PAIR        200000
#define SCORE_ONE_PAIR        100000

static int evaluate_poker_hand(Card hand[], int count) {
    int ranks[15] = { 0 };
    int suits[4] = { 0 };
    int suit_ranks[4][15] = { 0 }; // מעקב חדש: ערכים מחולקים לפי צורה
    int max_rank = 0;

    for (int i = 0; i < count; i++) {
        ranks[hand[i].rank_val]++;
        int s_idx = -1;
        if (hand[i].suit == 'H') s_idx = 0;
        else if (hand[i].suit == 'D') s_idx = 1;
        else if (hand[i].suit == 'C') s_idx = 2;
        else if (hand[i].suit == 'S') s_idx = 3;

        if (s_idx != -1) {
            suits[s_idx]++;
            suit_ranks[s_idx][hand[i].rank_val]++; // תיעוד הערך בתוך הצורה הספציפית
        }

        if (hand[i].rank_val > max_rank) max_rank = hand[i].rank_val;
    }

    // שלב חדש: חיפוש Straight Flush חוקי (5 עוקבים מאותה צורה בדיוק)
    int straight_flush_high = 0;
    for (int s = 0; s < 4; s++) {
        if (suits[s] >= 5) { // בודקים רצף רק אם יש פלאש בצורה הזו
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
            // סטרייט פלאש תחתון (A-2-3-4-5) באותה הצורה
            if (suit_ranks[s][14] > 0 && suit_ranks[s][2] > 0 && suit_ranks[s][3] > 0 &&
                suit_ranks[s][4] > 0 && suit_ranks[s][5] > 0) {
                if (straight_flush_high == 0) straight_flush_high = 5;
            }
        }
    }

    // חישוב Flush רגיל
    int is_flush = 0;
    for (int i = 0; i < 4; i++) {
        if (suits[i] >= 5) is_flush = 1;
    }

    // חישוב Straight רגיל
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

    // חישוב זוגות ושלשות
    int pairs = 0, trips = 0, quads = 0;
    int highest_pair = 0, highest_trip = 0;

    for (int i = 2; i <= 14; i++) {
        if (ranks[i] == 2) { pairs++; if (i > highest_pair) highest_pair = i; }
        if (ranks[i] == 3) { trips++; if (i > highest_trip) highest_trip = i; }
        if (ranks[i] == 4) { quads++; }
    }

    // עץ הדירוג המעודכן - מונע את ה-False Positive
    if (straight_flush_high > 0) return SCORE_STRAIGHT_FLUSH + straight_flush_high; // שימוש במשתנה החדש!
    if (quads > 0) return SCORE_FOUR_OF_A_KIND + max_rank;
    if (trips > 0 && pairs > 0) return SCORE_FULL_HOUSE + highest_trip;
    if (is_flush) return SCORE_FLUSH + max_rank;
    if (straight_high > 0) return SCORE_STRAIGHT + straight_high;
    if (trips > 0) return SCORE_THREE_OF_A_KIND + highest_trip;
    if (pairs >= 2) return SCORE_TWO_PAIR + highest_pair;
    if (pairs == 1) return SCORE_ONE_PAIR + highest_pair;

    return max_rank;
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
        print_table_header("ULTIMATE TEXAS HOLD'EM", "" C_MAGENTA "", player->balance);

        int action;
        while (1) {
            printf("Options: [0] Leave Table  [1] Place Bets\nAction: ");
            action = get_safe_int();
            if (action == 0 || action == 1) break;
            display_error(1500, "Invalid option. Please select 0 or 1.");
        }
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

        // ==========================================
        // שלב 1: טרום פלופ (Pre-Flop)
        // ==========================================
        printf("\n" C_CYAN "Dealing hole cards..." C_RESET "\n");

        printf(" [ Legend: " C_RED "H" C_RESET "=Hearts | " C_RED "D" C_RESET "=Diamonds | \x1b[97mC" C_RESET "=Clubs | \x1b[97mS" C_RESET "=Spades ]\n");

        print_cards_ascii(player_cards, 2, "Your Hand", 0);

        // חישוב והצגת היד הנוכחית (טרום פלופ)
        int current_score = evaluate_poker_hand(player_cards, 2);
        printf("Current Hand: " C_GREEN "%s" C_RESET "\n", get_hand_name(current_score));

        printf("Current Bets -> Ante: $%d | Blind: $%d | Trips: $%d\n", ante, blind, trips);
        printf("\n" C_YELLOW "--- PRE-FLOP ACTION ---" C_RESET "\n");
        int pre_flop_action;
        while (1) {
            printf("Options: [1] CHECK  [2] PLAY (3x = $%d)  [3] PLAY (4x = $%d)\nAction: ", ante * 3, ante * 4);
            pre_flop_action = get_safe_int();
            if (pre_flop_action >= 1 && pre_flop_action <= 3) break;
            display_error(1500, "Invalid option. Please select 1, 2, or 3.");
        }
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

        // ==========================================
        // שלב 2: פלופ (The Flop)
        // ==========================================
        printf("\n" C_CYAN "Dealing the FLOP..." C_RESET "\n");
        delay_ms(1000);

        print_cards_ascii(player_cards, 2, "Your Hand", 0);
        print_cards_ascii(community, 3, "Community Cards (FLOP)", 0);

        // חישוב והצגת היד בשילוב הקלפים הקהילתיים (פלופ)
        Card flop_eval[5] = { 0 };
        flop_eval[0] = player_cards[0]; flop_eval[1] = player_cards[1];
        flop_eval[2] = community[0]; flop_eval[3] = community[1]; flop_eval[4] = community[2];
        current_score = evaluate_poker_hand(flop_eval, 5);
        printf("Current Hand: " C_GREEN "%s" C_RESET "\n", get_hand_name(current_score));

        if (play_bet == 0) {
            printf("\n" C_YELLOW "--- FLOP ACTION ---" C_RESET "\n");
            int flop_action;
            while (1) {
                printf("Options: [1] CHECK  [2] PLAY (2x = $%d)\nAction: ", ante * 2);
                flop_action = get_safe_int();
                if (flop_action == 1 || flop_action == 2) break;
                display_error(1500, "Invalid option. Please select 1 or 2.");
            }

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

        // ==========================================
        // שלב 3: טרן וריבר (Turn & River)
        // ==========================================
        printf("\n" C_CYAN "Dealing Turn & River..." C_RESET "\n");
        delay_ms(1000);

        print_cards_ascii(player_cards, 2, "Your Hand", 0);
        print_cards_ascii(community, 5, "Final Community Cards", 0);

        // חישוב והצגת היד בשילוב כל הקלפים (ריבר)
        Card river_eval[7] = { 0 };
        river_eval[0] = player_cards[0]; river_eval[1] = player_cards[1];
        for (int i = 0; i < 5; i++) river_eval[i + 2] = community[i];
        current_score = evaluate_poker_hand(river_eval, 7);
        printf("Current Hand: " C_GREEN "%s" C_RESET "\n", get_hand_name(current_score));

        if (play_bet == 0) {
            printf("\n" C_YELLOW "--- FINAL ACTION ---" C_RESET "\n");
            int river_action;
            while (1) {
                printf("Options: [1] PLAY (1x = $%d)  [2] FOLD\nAction: ", ante);
                river_action = get_safe_int();
                if (river_action == 1 || river_action == 2) break;
                display_error(1500, "Invalid option. Please select 1 or 2.");
            }

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

        // 3. חשיפת הדילר והשוואת ידיים - רק אם השחקן נשאר במשחק!
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
            delay_ms(1000); // נותנים לשחקן שנייה לעכל את הקלפים

            Card d_eval[7] = { 0 };
            for (int i = 0; i < 5; i++) { d_eval[i] = community[i]; }
            d_eval[5] = dealer_cards[0]; d_eval[6] = dealer_cards[1];
            int d_score = evaluate_poker_hand(d_eval, 7);

            printf("\nYour Best Hand  : " C_GREEN "%s" C_RESET "\n", get_hand_name(p_score));
            printf("Dealer Best Hand: " C_RED "%s" C_RESET "\n", get_hand_name(d_score));

            if (p_score > d_score) {
                int total_win = (ante * 2) + (play_bet * 2);
                int blind_win = blind;

                if (p_score >= 400000) {
                    if (p_score >= 800000) blind_win += blind * 50;
                    else if (p_score >= 700000) blind_win += blind * 10;
                    else if (p_score >= 600000) blind_win += blind * 3;
                    else if (p_score >= 500000) blind_win += blind * 1;
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
        // חשוב! מניעת דליפת זיכרון בסוף הסיבוב
        prompt_continue(NULL);

        if (player->balance <= 0) {
            printf("\n" C_RED "You are bankrupt! Security is escorting you out." C_RESET "\n");
            is_playing = 0;
        }
    }
}