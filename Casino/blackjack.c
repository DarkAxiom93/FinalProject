#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include "casino.h"
#include "utils.h"
#include "graphics.h"
#include "cards.h"
#include "account.h"

// ==========================================
// פונקציות עזר - לוגיקת קלפים
// ==========================================
static int calculate_hand_value(Card* hand, int count) {
    int total = 0, aces = 0;
    for (int i = 0; i < count; i++) {
        if (hand[i].rank_val == 14) { total += 11; aces++; }
        else if (hand[i].rank_val >= 11 && hand[i].rank_val <= 13) { total += 10; }
        else { total += hand[i].rank_val; }
    }
    while (total > 21 && aces > 0) { total -= 10; aces--; }
    return total;
}

static int is_soft_17(Card* hand, int count) {
    if (calculate_hand_value(hand, count) != 17) {
        return 0;
    }

    int hard_total = 0;
    for (int i = 0; i < count; i++) {
        hard_total += (hand[i].rank_val == 14) ? 1 : (hand[i].rank_val >= 11) ? 10 : hand[i].rank_val;
    }

    return (hard_total == 7);
}

// ==========================================
// פונקציות עזר - ניהול מהלכים
// ==========================================
static void play_hand(Card* hand, int* count, Card* deck, int* deck_idx, int* bet, Player* player, const char* hand_name, int* busted) {
    int val = calculate_hand_value(hand, *count);

    while (val < 21) {
        if (*count >= 11) { 
            printf("\n" C_RED "Hand size limit reached! Forced Stand." C_RESET "\n");
            break;
        }
        printf("\n[%s] Options: [1] Hit  [2] Stand  [3] Double Down: ", hand_name);

        clear_input_buffer();
        int move = 0;
        char key;
        while (1) {
            key = (char)_getch();
            if (key == '1') { move = 1; break; }
            else if (key == '2') { move = 2; break; }
            else if (key == '3') { move = 3; break; }
        }
        printf("%c\n", key);

        if (move == 1) {
            hand[(*count)++] = deck[(*deck_idx)++];
            val = calculate_hand_value(hand, *count);
            print_cards_ascii(hand, *count, hand_name, 0);
            printf("Total Value: %d\n", val);

            if (val > 21) {
                printf("\n" C_RED "BUST!" C_RESET " %s went over 21.\n", hand_name);
                *busted = 1;
                player->total_losses += *bet;
            }
        }
        else if (move == 2) {
            break;
        }
        else if (move == 3) {
            int allowed_addition = *bet;
            int was_capped = 0;

            if (*bet + allowed_addition > MAX_BET) {
                allowed_addition = MAX_BET - *bet;
                was_capped = 1;
            }
            if (allowed_addition <= 0) {
                display_error(1500, "Bet is already at table maximum ($%d). Cannot double down!", MAX_BET);
                continue;
            }
            if (player->balance < allowed_addition) {
                display_error(1500, "Insufficient funds to double down!");
                continue;
            }

            player->balance -= allowed_addition;
            *bet += allowed_addition;

            if (was_capped) printf("\n" C_YELLOW "Notice: Double Down capped at table maximum ($%d)!" C_RESET "\n", MAX_BET);
            printf("[%s] Bet increased to $%d.\n", hand_name, *bet);

            hand[(*count)++] = deck[(*deck_idx)++];
            val = calculate_hand_value(hand, *count);
            print_cards_ascii(hand, *count, hand_name, 0);
            printf("Total Value: %d\n", val);

            if (val > 21) {
                printf("\n" C_RED "BUST!" C_RESET " %s went over 21.\n", hand_name);
                *busted = 1;
                player->total_losses += *bet;
            }
            break;
        }
    }
}

static void resolve_bets(int p_val, int p_busted, int d_val, int bet, Player* player, const char* hand_name) {
    if (p_busted) return;

    printf("\n--- Result for %s ---\n", hand_name);
    if (d_val > 21) {
        printf("" C_GREEN "Dealer Busts! YOU WIN $%d!" C_RESET "\n", bet * 2);
        player->total_winnings += bet;
        add_balance_safe(player, bet * 2);
    }
    else if (d_val > p_val) {
        printf("" C_RED "Dealer Wins %d to %d." C_RESET "\n", d_val, p_val);
        player->total_losses += bet;
    }
    else if (d_val < p_val) {
        printf("" C_GREEN "YOU WIN $%d! %d to %d." C_RESET "\n", bet * 2, p_val, d_val);
        player->total_winnings += bet;
        add_balance_safe(player, bet * 2);
    }
    else {
        printf("" C_YELLOW "PUSH! It's a tie." C_RESET "\n");
        add_balance_safe(player, bet);
    }
}

// ==========================================
// פונקציות המודולים החדשות (Refactored Sub-Routines)
// ==========================================
static void handle_natural_blackjack(Player* player, int bet, int d_val, Card* d_hand, int d_count) {
    printf("\n" C_GREEN "BLACKJACK!" C_RESET " You hit 21 instantly!\n");
    print_cards_ascii(d_hand, d_count, "Dealer", 0);
    printf("Total Value: %d\n", calculate_hand_value(d_hand, d_count));

    if (d_val == 21) {
        printf("" C_YELLOW "Push (Tie)." C_RESET " Dealer also has Blackjack.\n");
        add_balance_safe(player, bet);
    }
    else {
        int win_amount = bet + (int)((bet * 3) / 2);
        printf("" C_GREEN "You won $%d!" C_RESET "\n", (int)(bet * 1.5));
        player->total_winnings += ((long long)win_amount - bet);
        add_balance_safe(player, win_amount);
    }
}

static void run_dealer_turn(Card* d_hand, int* d_count, Card* deck, int* deck_idx) {
    printf("\n--- Dealer's Turn ---\n");
    print_cards_ascii(d_hand, *d_count, "Dealer", 0);
    printf("Total Value: %d\n", calculate_hand_value(d_hand, *d_count));

    while ((calculate_hand_value(d_hand, *d_count) < 17 || is_soft_17(d_hand, *d_count)) && *d_count < 11) {
        if (is_soft_17(d_hand, *d_count)) {
            printf("\n" C_CYAN "Dealer has a Soft 17. Dealer must hit!" C_RESET "\n");
        }
        delay_ms(1500);
        printf("Dealer hits...\n");
        d_hand[(*d_count)++] = deck[(*deck_idx)++];
        print_cards_ascii(d_hand, *d_count, "Dealer", 0);
        printf("Total Value: %d\n", calculate_hand_value(d_hand, *d_count));
    }
}

static int offer_and_handle_split(Player* player, int* bet1, int* bet2, Card* p_hand1, int* p1_count, Card* p_hand2, int* p2_count, Card* deck, int* deck_idx) {
    if (p_hand1[0].rank_val == p_hand1[1].rank_val && player->balance >= *bet1) {
        printf("\n" C_CYAN "You have a pair! Do you want to SPLIT? (Costs an additional $%d)" C_RESET "\n", *bet1);
        printf("[1] Yes, Split  [2] No, Play as one hand: ");

        clear_input_buffer();
        char key;
        while (1) {
            key = (char)_getch();
            if (key == '1' || key == '2') break;
        }
        printf("%c\n", key); 

        if (key == '1') {
            *bet2 = *bet1;
            player->balance -= *bet2;
            p_hand2[0] = p_hand1[1];
            *p1_count = 1;
            *p2_count = 1;
            p_hand1[(*p1_count)++] = deck[(*deck_idx)++];
            p_hand2[(*p2_count)++] = deck[(*deck_idx)++];
            printf("\n" C_CYAN "--- HANDS SPLIT ---" C_RESET "\n");
            return 1;
        }
    }
    return 0;
}

// ===============
// הלולאה הראשית 
// ===============
void play_blackjack(Player* player) {
    int is_playing = 1;
    print_blackjack_welcome();

    clear_screen();
    printf("\n" C_YELLOW "==================================================" C_RESET "\n");
    printf("  " C_CYAN "[Dealer]" C_RESET " Welcome to the VIP Blackjack Table, %s!\n", player->name);
    printf("  " C_CYAN "[Dealer]" C_RESET " Shuffling the 6-deck shoe...\n");
    printf("" C_YELLOW "==================================================" C_RESET "\n");
    delay_ms(2000);

    while (is_playing) {
        clear_screen();
        print_table_header("BLACKJACK TABLE", "" C_CYAN "", player->balance);

        printf("Options: [0] Leave Table  [1] Place Bet\nAction: ");

        clear_input_buffer();
        char action_key;
        while (1) {
            action_key = (char)_getch();
            if (action_key == '0' || action_key == '1') break;
        }
        printf("%c\n", action_key); 

        if (action_key == '0') break;

        printf("Enter bet amount: $");
        int bet1 = get_safe_int();

        if (bet1 <= 0 || bet1 > player->balance) {
            display_error(1500, "Invalid amount or insufficient funds!");
            continue;
        }
        if (bet1 > MAX_BET) {
            display_error(1500, "Table maximum bet is $%d!", MAX_BET);
            continue;
        }

        int round_start_funds = player->balance + player->bank_balance;

        player->balance -= bet1;
        printf("Bet of $%d placed. Dealing cards...\n", bet1);

        Card deck[312] = { 0 };
        init_deck(deck, 6);
        shuffle_deck(deck, 312);

        Card p_hand1[12] = { 0 }, p_hand2[12] = { 0 }, d_hand[12] = { 0 };
        int p1_count = 0, p2_count = 0, d_count = 0, deck_idx = 0;
        int is_split = 0, bet2 = 0;

        // חלוקה ראשונית (Initial Deal)
        p_hand1[p1_count++] = deck[deck_idx++];
        d_hand[d_count++] = deck[deck_idx++];
        p_hand1[p1_count++] = deck[deck_idx++];
        d_hand[d_count++] = deck[deck_idx++];

        print_cards_ascii(d_hand, d_count, "Dealer", 1);
        print_cards_ascii(p_hand1, p1_count, player->name, 0);
        printf("Total Value: %d\n", calculate_hand_value(p_hand1, p1_count));

        int p1_val = calculate_hand_value(p_hand1, p1_count);
        int d_val = calculate_hand_value(d_hand, d_count);

        if (p1_val == 21) {
            handle_natural_blackjack(player, bet1, d_val, d_hand, d_count);
        }
        else {
            is_split = offer_and_handle_split(player, &bet1, &bet2, p_hand1, &p1_count, p_hand2, &p2_count, deck, &deck_idx);

            int p1_busted = 0, p2_busted = 0;
            int is_ace_split = (is_split && p_hand1[0].rank_val == 14);

            if (is_ace_split) {
                printf("\n" C_YELLOW "--- CASINO RULE: SPLIT ACES ---" C_RESET "\n");
                printf("You receive exactly ONE card per hand. Forced Stand.\n");
                delay_ms(1500);

                p_hand1[p1_count++] = deck[deck_idx++];
                p_hand2[p2_count++] = deck[deck_idx++];

                printf("\n--- PLAYING HAND 1 ---\n");
                print_cards_ascii(p_hand1, p1_count, "Hand 1", 0);
                printf("Total Value: %d\n", calculate_hand_value(p_hand1, p1_count));

                printf("\n--- PLAYING HAND 2 ---\n");
                print_cards_ascii(p_hand2, p2_count, "Hand 2", 0);
                printf("Total Value: %d\n", calculate_hand_value(p_hand2, p2_count));
                delay_ms(2000);
            }
            else {
                printf("\n--- PLAYING %s ---\n", is_split ? "HAND 1" : "HAND");
                print_cards_ascii(p_hand1, p1_count, is_split ? "Hand 1" : player->name, 0);
                printf("Total Value: %d\n", calculate_hand_value(p_hand1, p1_count));
                play_hand(p_hand1, &p1_count, deck, &deck_idx, &bet1, player, is_split ? "Hand 1" : player->name, &p1_busted);

                if (is_split) {
                    printf("\n--- PLAYING HAND 2 ---\n");
                    print_cards_ascii(p_hand2, p2_count, "Hand 2", 0);
                    printf("Total Value: %d\n", calculate_hand_value(p_hand2, p2_count));
                    play_hand(p_hand2, &p2_count, deck, &deck_idx, &bet2, player, "Hand 2", &p2_busted);
                }
            }

            // תור הדילר רץ רק אם השחקן לא נשרף לחלוטין
            if (!p1_busted || (is_split && !p2_busted)) {
                run_dealer_turn(d_hand, &d_count, deck, &deck_idx);
                d_val = calculate_hand_value(d_hand, d_count);

                resolve_bets(calculate_hand_value(p_hand1, p1_count), p1_busted, d_val, bet1, player, is_split ? "Hand 1" : "Main Hand");
                if (is_split) resolve_bets(calculate_hand_value(p_hand2, p2_count), p2_busted, d_val, bet2, player, "Hand 2");
            }
        }

        int round_end_funds = player->balance + player->bank_balance;
        int net_change = round_end_funds - round_start_funds;

        printf("\n========================================\n");
        if (net_change > 0) {
            printf("" C_GREEN "  ROUND SUMMARY: You won a net total of +$%d!" C_RESET "\n", net_change);
        }
        else if (net_change < 0) {
            printf("" C_RED "  ROUND SUMMARY: You lost a net total of -$%d." C_RESET "\n", -net_change);
        }
        else {
            printf("" C_YELLOW "  ROUND SUMMARY: You broke even ($0)." C_RESET "\n");
        }
        printf("========================================\n");
        save_player(player); // שמירה אחת בלבד בסוף הסיבוב!
        prompt_continue(NULL);

        if (player->balance <= 0) {
            printf("\n" C_RED "You are bankrupt! Security is escorting you out." C_RESET "\n");
            is_playing = 0;
        }
    }
}