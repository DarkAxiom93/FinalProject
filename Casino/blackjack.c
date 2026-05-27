#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "casino.h"
#include "utils.h"
#include "graphics.h"
#include "cards.h"

// הלוגיקה מתרגמת את ערכי הפוקר האוניברסליים לערכי בלאק-ג'ק
static int calculate_hand_value(Card* hand, int count) {
    int total = 0;
    int aces = 0;

    for (int i = 0; i < count; i++) {
        if (hand[i].rank_val == 14) { // Ace
            total += 11;
            aces++;
        }
        else if (hand[i].rank_val >= 11 && hand[i].rank_val <= 13) { // J, Q, K
            total += 10;
        }
        else {
            total += hand[i].rank_val; // 2-10
        }
    }

    while (total > 21 && aces > 0) {
        total -= 10;
        aces--;
    }
    return total;
}

static void print_hands(Card* hand, int count, const char* owner, int hide_second) {
    printf("\n--- %s's Hand ---\n", owner);

    for (int i = 0; i < count; i++) printf("+-------+ ");
    printf("\n");

    for (int i = 0; i < count; i++) {
        if (hide_second && i == 1) printf("|#######| ");
        else printf("| %-2s    | ", hand[i].str);
    }
    printf("\n");

    for (int i = 0; i < count; i++) {
        if (hide_second && i == 1) printf("|#######| ");
        else {
            if (hand[i].suit == 'H' || hand[i].suit == 'D')
                printf("|   " C_RED "%c" C_RESET "   | ", hand[i].suit);
            else
                printf("|   \x1b[97m%c" C_RESET "   | ", hand[i].suit);
        }
    }
    printf("\n");

    for (int i = 0; i < count; i++) {
        if (hide_second && i == 1) printf("|#######| ");
        else printf("|    %2s | ", hand[i].str);
    }
    printf("\n");

    for (int i = 0; i < count; i++) printf("+-------+ ");
    printf("\n");

    if (!hide_second) {
        printf("Total Value: %d\n", calculate_hand_value(hand, count));
    }
}

static void play_hand(Card* hand, int* count, Card* deck, int* deck_idx, int* bet, Player* player, const char* hand_name, int* busted) {
    int val = calculate_hand_value(hand, *count);

    while (val < 21) {
        printf("\n[%s] Options: [1] Hit  [2] Stand  [3] Double Down: ", hand_name);
        int move = get_safe_int();

        if (move == 1) {
            hand[(*count)++] = deck[(*deck_idx)++];
            val = calculate_hand_value(hand, *count);
            print_hands(hand, *count, hand_name, 0);

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
            if (player->balance < *bet) {
                printf("" C_RED "Insufficient funds to double down!" C_RESET "\n");
                continue;
            }
            player->balance -= *bet;
            *bet *= 2;
            printf("[%s] Bet doubled to $%d.\n", hand_name, *bet);

            hand[(*count)++] = deck[(*deck_idx)++];
            val = calculate_hand_value(hand, *count);
            print_hands(hand, *count, hand_name, 0);

            if (val > 21) {
                printf("\n" C_RED "BUST!" C_RESET " %s went over 21.\n", hand_name);
                *busted = 1;
                player->total_losses += *bet;
            }
            break;
        }
        else {
            printf("" C_YELLOW "Invalid option." C_RESET "\n");
        }
    }
}

static void resolve_bets(int p_val, int p_busted, int d_val, int bet, Player* player, const char* hand_name) {
    if (p_busted) return;

    printf("\n--- Result for %s ---\n", hand_name);
    if (d_val > 21) {
        printf("" C_GREEN "Dealer Busts! YOU WIN $%d!" C_RESET "\n", bet * 2);
        player->balance += (bet * 2);
        player->total_winnings += bet;
    }
    else if (d_val > p_val) {
        printf("" C_RED "Dealer Wins %d to %d." C_RESET "\n", d_val, p_val);
        player->total_losses += bet;
    }
    else if (d_val < p_val) {
        printf("" C_GREEN "YOU WIN $%d! %d to %d." C_RESET "\n", bet * 2, p_val, d_val);
        player->balance += (bet * 2);
        player->total_winnings += bet;
    }
    else {
        printf("" C_YELLOW "PUSH! It's a tie." C_RESET "\n");
        player->balance += bet;
    }
}

void play_blackjack(Player* player) {
    int is_playing = 1;

    print_blackjack_welcome();

    system("cls");
    printf("\n" C_YELLOW "==================================================" C_RESET "\n");
    printf("  " C_CYAN "[Dealer]" C_RESET " Welcome to the VIP Blackjack Table, %s!\n", player->name);
    printf("  " C_CYAN "[Dealer]" C_RESET " Shuffling the 6-deck shoe...\n");
    printf("" C_YELLOW "==================================================" C_RESET "\n");
    delay_ms(2000);

    while (is_playing) {
        print_table_header("BLACKJACK TABLE", "" C_CYAN "", player->balance);

        printf("Options: [0] Leave Table  [1] Place Bet\nAction: ");
        int action = get_safe_int();
        if (action == 0) break;
        if (action != 1) continue;

        printf("Enter bet amount: $");
        int bet1 = get_safe_int();

        if (bet1 <= 0 || bet1 > player->balance) {
            printf("" C_RED "Invalid amount or insufficient funds!" C_RESET "\n");
            continue;
        }

        player->balance -= bet1;
        printf("Bet of $%d placed. Dealing cards...\n", bet1);

        // יצירת 6 חפיסות אמיתיות לבלאק ג'ק VIP מהמנוע הגנרי!
        Card* deck = create_deck(6);
        shuffle_deck(deck, 312); // 6 * 52 = 312 cards

        Card* p_hand1 = (Card*)safe_malloc(11 * sizeof(Card));
        Card* p_hand2 = NULL;
        Card* d_hand = (Card*)safe_malloc(11 * sizeof(Card));

        int p1_count = 0, p2_count = 0, d_count = 0, deck_idx = 0;
        int is_split = 0, bet2 = 0;

        p_hand1[p1_count++] = deck[deck_idx++];
        d_hand[d_count++] = deck[deck_idx++];
        p_hand1[p1_count++] = deck[deck_idx++];
        d_hand[d_count++] = deck[deck_idx++];

        print_hands(d_hand, d_count, "Dealer", 1);
        print_hands(p_hand1, p1_count, player->name, 0);

        int p1_val = calculate_hand_value(p_hand1, p1_count);
        int d_val = calculate_hand_value(d_hand, d_count);

        if (p1_val == 21) {
            printf("\n" C_GREEN "BLACKJACK!" C_RESET " You hit 21 instantly!\n");
            print_hands(d_hand, d_count, "Dealer", 0);

            if (d_val == 21) {
                printf("" C_YELLOW "Push (Tie)." C_RESET " Dealer also has Blackjack.\n");
                player->balance += bet1;
            }
            else {
                int win_amount = bet1 + (int)(bet1 * 1.5);
                printf("" C_GREEN "You won $%d!" C_RESET "\n", (int)(bet1 * 1.5));
                player->balance += win_amount;
                player->total_winnings += (win_amount - bet1);
            }
        }
        else {
            if (strcmp(p_hand1[0].str, p_hand1[1].str) == 0 && player->balance >= bet1) {
                printf("\n" C_CYAN "You have a pair! Do you want to SPLIT? (Costs an additional $%d)" C_RESET "\n", bet1);
                printf("[1] Yes, Split  [2] No, Play as one hand: ");
                if (get_safe_int() == 1) {
                    is_split = 1;
                    bet2 = bet1;
                    player->balance -= bet2;

                    p_hand2 = (Card*)safe_malloc(11 * sizeof(Card));

                    p_hand2[0] = p_hand1[1];
                    p1_count = 1;
                    p2_count = 1;

                    p_hand1[p1_count++] = deck[deck_idx++];
                    p_hand2[p2_count++] = deck[deck_idx++];

                    printf("\n" C_CYAN "--- HANDS SPLIT ---" C_RESET "\n");
                }
            }

            int p1_busted = 0;
            printf("\n--- PLAYING %s ---\n", is_split ? "HAND 1" : "HAND");
            print_hands(p_hand1, p1_count, is_split ? "Hand 1" : player->name, 0);
            play_hand(p_hand1, &p1_count, deck, &deck_idx, &bet1, player, is_split ? "Hand 1" : player->name, &p1_busted);

            int p2_busted = 0;
            if (is_split) {
                printf("\n--- PLAYING HAND 2 ---\n");
                print_hands(p_hand2, p2_count, "Hand 2", 0);
                play_hand(p_hand2, &p2_count, deck, &deck_idx, &bet2, player, "Hand 2", &p2_busted);
            }

            if (!p1_busted || (is_split && !p2_busted)) {
                printf("\n--- Dealer's Turn ---\n");
                print_hands(d_hand, d_count, "Dealer", 0);

                while (calculate_hand_value(d_hand, d_count) < 17) {
                    printf("Dealer hits...\n");
                    d_hand[d_count++] = deck[deck_idx++];
                    print_hands(d_hand, d_count, "Dealer", 0);
                }
                d_val = calculate_hand_value(d_hand, d_count);

                resolve_bets(calculate_hand_value(p_hand1, p1_count), p1_busted, d_val, bet1, player, is_split ? "Hand 1" : "Main Hand");
                if (is_split) {
                    resolve_bets(calculate_hand_value(p_hand2, p2_count), p2_busted, d_val, bet2, player, "Hand 2");
                }
            }
        }

        free(deck);
        free(p_hand1);
        free(d_hand);
        if (is_split) free(p_hand2);

        if (player->balance <= 0) {
            printf("\n" C_RED "You are bankrupt! Security is escorting you out." C_RESET "\n");
            is_playing = 0;
        }
    }
}