#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "casino.h"
#include "utils.h"

// מבנה קלף ייעודי לפוקר
typedef struct {
    int rank_val;
    char suit;
    char str[4];
} P_Card;

void print_poker_welcome() {
    system("cls");
    printf("\x1b[35m");
    printf("  _    _ _   _   _                 _       \n");
    printf(" | |  | | | | | (_)               | |      \n");
    printf(" | |  | | | |_| |_ _ __ ___   __ _| |_ ___ \n");
    printf(" | |  | | | __| | | '_ ` _ \\ / _` | __/ _ \\\n");
    printf(" | |__| | | |_| | | | | | | | (_| | ||  __/\n");
    printf("  \\____/|_|\\__|_|_|_| |_| |_|\\__,_|\\__\\___|\n");
    printf("        T E X A S   H O L D ' E M          \n\x1b[0m");

    printf("\x1b[36m=========================================================================\x1b[0m\n");
    printf("                       \x1b[33mULTIMATE RULES & PAYOUTS\x1b[0m\n");
    printf("\x1b[36m=========================================================================\x1b[0m\n");
    printf(" * Place equal ANTE and BLIND bets to receive cards.\n");
    printf(" * Optional TRIPS bet: Pays on 3-of-a-Kind or better, win or lose!\n");
    printf(" * PRE-FLOP : Check, or Play 3x/4x Ante.\n");
    printf(" * FLOP     : Check, or Play 2x Ante.\n");
    printf(" * RIVER    : Play 1x Ante, or FOLD (lose Ante & Blind).\n");
    printf("\x1b[36m=========================================================================\x1b[0m\n\n");

    printf("\x1b[32mPress ENTER to sit at the table...\x1b[0m");
    wait_for_enter();
    system("cls");
}

int evaluate_poker_hand(P_Card hand[], int count) {
    int ranks[15] = { 0 };
    int suits[4] = { 0 };
    int max_rank = 0;

    for (int i = 0; i < count; i++) {
        ranks[hand[i].rank_val]++;
        if (hand[i].suit == 'H') suits[0]++;
        else if (hand[i].suit == 'D') suits[1]++;
        else if (hand[i].suit == 'C') suits[2]++;
        else if (hand[i].suit == 'S') suits[3]++;
        if (hand[i].rank_val > max_rank) max_rank = hand[i].rank_val;
    }

    int is_flush = 0;
    for (int i = 0; i < 4; i++) {
        if (suits[i] >= 5) is_flush = 1;
    }

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

    int pairs = 0, trips = 0, quads = 0;
    int highest_pair = 0, highest_trip = 0;

    for (int i = 2; i <= 14; i++) {
        if (ranks[i] == 2) { pairs++; if (i > highest_pair) highest_pair = i; }
        if (ranks[i] == 3) { trips++; if (i > highest_trip) highest_trip = i; }
        if (ranks[i] == 4) { quads++; }
    }

    if (is_flush && straight_high > 0) return 800000 + straight_high;
    if (quads > 0) return 700000 + max_rank;
    if (trips > 0 && pairs > 0) return 600000 + highest_trip;
    if (trips > 1) return 600000 + highest_trip;
    if (is_flush) return 500000 + max_rank;
    if (straight_high > 0) return 400000 + straight_high;
    if (trips > 0) return 300000 + highest_trip;
    if (pairs >= 2) return 200000 + highest_pair;
    if (pairs == 1) return 100000 + highest_pair;

    return max_rank;
}

const char* get_hand_name(int score) {
    if (score >= 800000) return "Straight Flush";
    if (score >= 700000) return "Four of a Kind";
    if (score >= 600000) return "Full House";
    if (score >= 500000) return "Flush";
    if (score >= 400000) return "Straight";
    if (score >= 300000) return "Three of a Kind";
    if (score >= 200000) return "Two Pair";
    if (score >= 100000) return "One Pair";
    return "High Card";
}

void print_poker_hand_ascii(P_Card* hand, int count, const char* title, int is_hidden) {
    if (title != NULL) printf("\n--- %s ---\n", title);
    for (int i = 0; i < count; i++) printf("+-------+ ");
    printf("\n");
    for (int i = 0; i < count; i++) {
        if (is_hidden) printf("|#######| ");
        else printf("| %-2s    | ", hand[i].str);
    }
    printf("\n");
    for (int i = 0; i < count; i++) {
        if (is_hidden) printf("|#######| ");
        else {
            if (hand[i].suit == 'H' || hand[i].suit == 'D') printf("|   \x1b[31m%c\x1b[0m   | ", hand[i].suit);
            else printf("|   \x1b[97m%c\x1b[0m   | ", hand[i].suit);
        }
    }
    printf("\n");
    for (int i = 0; i < count; i++) {
        if (is_hidden) printf("|#######| ");
        else printf("|    %2s | ", hand[i].str);
    }
    printf("\n");
    for (int i = 0; i < count; i++) printf("+-------+ ");
    printf("\n");
}

void play_poker(Player* player) {
    int is_playing = 1;
    print_poker_welcome();

    while (is_playing) {
        print_table_header("ULTIMATE TEXAS HOLD'EM", "\x1b[35m", player->balance);

        printf("Options: [0] Leave Table  [1] Place Bets\nAction: ");
        int action = get_safe_int();
        if (action == 0) break;
        if (action != 1) continue;

        printf("Enter ANTE amount (BLIND will be identical): $");
        int ante = get_safe_int();
        int blind = ante;

        printf("Enter optional TRIPS bet (Enter 0 to skip): $");
        int trips = get_safe_int();

        int total_initial_bet = ante + blind + trips;

        if (total_initial_bet <= 0 || total_initial_bet > player->balance) {
            printf("\x1b[31mInvalid amounts or insufficient funds!\x1b[0m\n");
            continue;
        }

        player->balance -= total_initial_bet;
        int play_bet = 0; // הימור ה-Play מתחיל כ-0
        int has_folded = 0;

        // יצירת חפיסה וערבוב
        P_Card deck[52];
        char suits[] = { 'H', 'D', 'C', 'S' };
        char* ranks_str[] = { "2","3","4","5","6","7","8","9","10","J","Q","K","A" };
        int idx = 0;
        for (int s = 0; s < 4; s++) {
            for (int r = 0; r < 13; r++) {
                deck[idx].rank_val = r + 2; deck[idx].suit = suits[s]; strcpy(deck[idx].str, ranks_str[r]); idx++;
            }
        }
        for (int i = 0; i < 52; i++) {
            int r = i + rand() % (52 - i);
            P_Card temp = deck[i]; deck[i] = deck[r]; deck[r] = temp;
        }

        int d_idx = 0;
        P_Card player_cards[2], dealer_cards[2], community[5];

        player_cards[0] = deck[d_idx++]; player_cards[1] = deck[d_idx++];
        dealer_cards[0] = deck[d_idx++]; dealer_cards[1] = deck[d_idx++];
        community[0] = deck[d_idx++]; community[1] = deck[d_idx++]; community[2] = deck[d_idx++];
        community[3] = deck[d_idx++]; community[4] = deck[d_idx++];

        // ==========================================
        // שלב 1: טרום פלופ (Pre-Flop)
        // ==========================================
        printf("\n\x1b[36mDealing hole cards...\x1b[0m\n");
        print_poker_hand_ascii(player_cards, 2, "Your Hand", 0);

        printf("Current Bets -> Ante: $%d | Blind: $%d | Trips: $%d\n", ante, blind, trips);
        printf("\n\x1b[33m--- PRE-FLOP ACTION ---\x1b[0m\n");
        printf("Options: [1] CHECK  [2] PLAY (3x = $%d)  [3] PLAY (4x = $%d)\nAction: ", ante * 3, ante * 4);

        int pre_flop_action = get_safe_int();
        if (pre_flop_action == 2 || pre_flop_action == 3) {
            int mult = (pre_flop_action == 2) ? 3 : 4;
            if (player->balance < ante * mult) {
                printf("\x1b[31mInsufficient funds for Play bet. Auto-checking...\x1b[0m\n");
            }
            else {
                play_bet = ante * mult;
                player->balance -= play_bet;
                printf("\x1b[32mPlay bet of $%d placed.\x1b[0m\n", play_bet);
            }
        }

        // ==========================================
        // שלב 2: פלופ (The Flop)
        // ==========================================
        printf("\n\x1b[36mDealing the FLOP...\x1b[0m\n");
        delay_ms(1000);
        print_poker_hand_ascii(community, 3, "Community Cards (FLOP)", 0);

        if (play_bet == 0) {
            printf("\n\x1b[33m--- FLOP ACTION ---\x1b[0m\n");
            printf("Options: [1] CHECK  [2] PLAY (2x = $%d)\nAction: ", ante * 2);
            int flop_action = get_safe_int();

            if (flop_action == 2) {
                if (player->balance < ante * 2) {
                    printf("\x1b[31mInsufficient funds for Play bet. Auto-checking...\x1b[0m\n");
                }
                else {
                    play_bet = ante * 2;
                    player->balance -= play_bet;
                    printf("\x1b[32mPlay bet of $%d placed.\x1b[0m\n", play_bet);
                }
            }
        }

        // ==========================================
        // שלב 3: טרן וריבר (Turn & River)
        // ==========================================
        printf("\n\x1b[36mDealing Turn & River...\x1b[0m\n");
        delay_ms(1000);
        print_poker_hand_ascii(community, 5, "Final Community Cards", 0);

        if (play_bet == 0) {
            printf("\n\x1b[33m--- FINAL ACTION ---\x1b[0m\n");
            printf("Options: [1] PLAY (1x = $%d)  [2] FOLD\nAction: ", ante);
            int river_action = get_safe_int();

            if (river_action == 2) {
                printf("\x1b[31mYou FOLDED. Ante ($%d) and Blind ($%d) are lost.\x1b[0m\n", ante, blind);
                player->total_losses += (ante + blind);
                has_folded = 1;
                // הימור ה-Trips עדיין פעיל (יחושב בהמשך)
            }
            else {
                if (player->balance < ante) {
                    printf("\x1b[31mInsufficient funds to call! You are forced to fold.\x1b[0m\n");
                    player->total_losses += (ante + blind);
                    has_folded = 1;
                }
                else {
                    play_bet = ante;
                    player->balance -= play_bet;
                    printf("\x1b[32mPlay bet of $%d placed.\x1b[0m\n", play_bet);
                }
            }
        }

        // ==========================================
        // שלב 4: חשיפה והערכת ידיים (Showdown)
        // ==========================================
        print_poker_hand_ascii(dealer_cards, 2, "Dealer Reveals Hand", 0);

        P_Card p_eval[7], d_eval[7];
        for (int i = 0; i < 5; i++) { p_eval[i] = community[i]; d_eval[i] = community[i]; }
        p_eval[5] = player_cards[0]; p_eval[6] = player_cards[1];
        d_eval[5] = dealer_cards[0]; d_eval[6] = dealer_cards[1];

        int p_score = evaluate_poker_hand(p_eval, 7);
        int d_score = evaluate_poker_hand(d_eval, 7);

        printf("\nYour Best Hand  : \x1b[32m%s\x1b[0m\n", get_hand_name(p_score));
        printf("Dealer Best Hand: \x1b[31m%s\x1b[0m\n", get_hand_name(d_score));

        // חישוב זכיית Trips (קורה בכל מקרה, גם אם קיפלנו)
        if (trips > 0) {
            int trips_win = 0;
            if (p_score >= 800000) trips_win = trips * 50;      // Straight Flush
            else if (p_score >= 700000) trips_win = trips * 30; // Four of a Kind
            else if (p_score >= 600000) trips_win = trips * 8;  // Full House
            else if (p_score >= 500000) trips_win = trips * 7;  // Flush
            else if (p_score >= 400000) trips_win = trips * 4;  // Straight
            else if (p_score >= 300000) trips_win = trips * 3;  // Three of a Kind

            if (trips_win > 0) {
                printf("\n\x1b[32mTRIPS BET WON! Payout: $%d\x1b[0m\n", trips_win);
                player->balance += (trips + trips_win);
                player->total_winnings += trips_win;
            }
            else {
                printf("\n\x1b[31mTrips bet lost.\x1b[0m\n");
                player->total_losses += trips;
            }
        }

        // חישוב מנצח ליד המרכזית (רק אם לא קיפלנו)
        if (!has_folded) {
            if (p_score > d_score) {
                int total_win = (ante * 2) + (play_bet * 2); // אנטה ופליי משלמים 1:1
                int blind_win = blind; // במציאות Blind תלוי בטבלת תשלום, נפשט ל-Push במקרה של ניצחון רגיל

                // תשלום נוסף על Blind אם יש לנו רצף ומעלה (חוקי UTH סטנדרטיים)
                if (p_score >= 400000) {
                    if (p_score >= 800000) blind_win += blind * 50;
                    else if (p_score >= 700000) blind_win += blind * 10;
                    else if (p_score >= 600000) blind_win += blind * 3;
                    else if (p_score >= 500000) blind_win += blind * 1;
                    else blind_win += blind * 1; // Straight
                }

                int total_payout = total_win + blind_win;
                printf("\n\x1b[32mYOU WIN THE HAND! Collected: $%d\x1b[0m\n", total_payout);
                player->balance += total_payout;
                player->total_winnings += (total_payout - (ante + blind + play_bet));

            }
            else if (d_score > p_score) {
                printf("\n\x1b[31mDEALER WINS THE HAND!\x1b[0m\n");
                player->total_losses += (ante + blind + play_bet);
            }
            else {
                printf("\n\x1b[33mPUSH (TIE)! Main bets returned.\x1b[0m\n");
                player->balance += (ante + blind + play_bet);
            }
        }

        printf("\n\x1b[32mPress ENTER to continue...\x1b[0m");
        wait_for_enter();

        if (player->balance <= 0) {
            printf("\n\x1b[31mYou are bankrupt! Security is escorting you out.\x1b[0m\n");
            is_playing = 0;
        }
    }
}