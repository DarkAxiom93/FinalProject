#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "casino.h"
#include "utils.h" // ייבוא תשתיות העזר שלנו

Card* create_deck() {
    // שימוש ב-safe_malloc במקום malloc רגיל עם בדיקת שגיאות
    Card* deck = (Card*)safe_malloc(52 * sizeof(Card));

    char suits[] = { 'H', 'D', 'C', 'S' };
    char* ranks[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A" };
    int values[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11 };

    int index = 0;
    for (int s = 0; s < 4; s++) {
        for (int r = 0; r < 13; r++) {
            strcpy(deck[index].rank, ranks[r]);
            deck[index].suit = suits[s];
            deck[index].value = values[r];
            index++;
        }
    }
    return deck;
}

void shuffle_deck(Card* deck) {
    for (int i = 0; i < 52; i++) {
        int r = i + rand() % (52 - i);
        Card temp = deck[i];
        deck[i] = deck[r];
        deck[r] = temp;
    }
}

int calculate_hand_value(Card* hand, int count) {
    int total = 0;
    int aces = 0;

    for (int i = 0; i < count; i++) {
        total += hand[i].value;
        if (strcmp(hand[i].rank, "A") == 0) aces++;
    }

    while (total > 21 && aces > 0) {
        total -= 10;
        aces--;
    }
    return total;
}

void print_hands(Card* hand, int count, const char* owner, int hide_second) {
    printf("\n--- %s's Hand ---\n", owner);

    for (int i = 0; i < count; i++) printf("+-------+ ");
    printf("\n");

    for (int i = 0; i < count; i++) {
        if (hide_second && i == 1) printf("|#######| ");
        else printf("| %-2s    | ", hand[i].rank);
    }
    printf("\n");

    for (int i = 0; i < count; i++) {
        if (hide_second && i == 1) printf("|#######| ");
        else {
            if (hand[i].suit == 'H' || hand[i].suit == 'D')
                printf("|   \x1b[31m%c\x1b[0m   | ", hand[i].suit);
            else
                printf("|   \x1b[97m%c\x1b[0m   | ", hand[i].suit);
        }
    }
    printf("\n");

    for (int i = 0; i < count; i++) {
        if (hide_second && i == 1) printf("|#######| ");
        else printf("|    %2s | ", hand[i].rank);
    }
    printf("\n");

    for (int i = 0; i < count; i++) printf("+-------+ ");
    printf("\n");

    if (!hide_second) {
        printf("Total Value: %d\n", calculate_hand_value(hand, count));
    }
}

void print_blackjack_welcome() {
    system("cls");
    printf("\x1b[97m"); // צבע לבן
    printf("  ____  ___      _______  _______  ___   _  _______  _______  _______  ___   _ \n");
    printf(" |    ||   |    |       ||       ||   | | ||       ||       ||       ||   | | |\n");
    printf(" |   _||   |    |   _   ||       ||   |_| ||_     _||   _   ||       ||   |_| |\n");
    printf(" |  |  |   |    |  |_|  ||       ||      _|  |   |  |  |_|  ||       ||      _|\n");
    printf(" |  |_ |   |___ |       ||      _||     |_   |   |  |       ||      _||     |_ \n");
    printf(" |    ||       ||   _   ||     |_ |    _  |  |   |  |   _   ||     |_ |    _  |\n");
    printf(" |____||_______||__| |__||_______||___| |_|  |___|  |__| |__||_______||___| |_|\n");
    printf("\x1b[0m\n");

    printf("\x1b[36m=========================================================================\x1b[0m\n");
    printf("                       \x1b[33mTABLE RULES & PAYOUTS\x1b[0m\n");
    printf("\x1b[36m=========================================================================\x1b[0m\n");
    printf(" * Blackjack (21 on first 2 cards) : Pays 3 to 2\n");
    printf(" * Standard Win                    : Pays 1 to 1\n");
    printf(" * Dealer Rules                    : Dealer must hit on soft 17\n");
    printf(" * Splitting                       : Allowed on matching pairs\n");
    printf(" * Double Down                     : Allowed on initial hand\n");
    printf("\x1b[36m=========================================================================\x1b[0m\n\n");

    printf("\x1b[32mPress ENTER to acknowledge rules and join the table...\x1b[0m");
    wait_for_enter();
    system("cls");
}

void play_hand(Card* hand, int* count, Card* deck, int* deck_idx, int* bet, Player* player, const char* hand_name, int* busted) {
    int val = calculate_hand_value(hand, *count);

    while (val < 21) {
        printf("\n[%s] Options: [1] Hit  [2] Stand  [3] Double Down: ", hand_name);
        int move = get_safe_int();

        if (move == 1) {
            hand[(*count)++] = deck[(*deck_idx)++];
            val = calculate_hand_value(hand, *count);
            print_hands(hand, *count, hand_name, 0);

            if (val > 21) {
                printf("\n\x1b[31mBUST!\x1b[0m %s went over 21.\n", hand_name);
                *busted = 1;
                player->total_losses += *bet;
            }
        }
        else if (move == 2) {
            break;
        }
        else if (move == 3) {
            if (player->balance < *bet) {
                printf("\x1b[31mInsufficient funds to double down!\x1b[0m\n");
                continue;
            }
            player->balance -= *bet;
            *bet *= 2;
            printf("[%s] Bet doubled to $%d.\n", hand_name, *bet);

            hand[(*count)++] = deck[(*deck_idx)++];
            val = calculate_hand_value(hand, *count);
            print_hands(hand, *count, hand_name, 0);

            if (val > 21) {
                printf("\n\x1b[31mBUST!\x1b[0m %s went over 21.\n", hand_name);
                *busted = 1;
                player->total_losses += *bet;
            }
            break;
        }
        else {
            printf("\x1b[33mInvalid option.\x1b[0m\n");
        }
    }
}

void resolve_bets(int p_val, int p_busted, int d_val, int bet, Player* player, const char* hand_name) {
    if (p_busted) return;

    printf("\n--- Result for %s ---\n", hand_name);
    if (d_val > 21) {
        printf("\x1b[32mDealer Busts! YOU WIN $%d!\x1b[0m\n", bet * 2);
        player->balance += (bet * 2);
        player->total_winnings += bet;
    }
    else if (d_val > p_val) {
        printf("\x1b[31mDealer Wins %d to %d.\x1b[0m\n", d_val, p_val);
        player->total_losses += bet;
    }
    else if (d_val < p_val) {
        printf("\x1b[32mYOU WIN $%d! %d to %d.\x1b[0m\n", bet * 2, p_val, d_val);
        player->balance += (bet * 2);
        player->total_winnings += bet;
    }
    else {
        printf("\x1b[33mPUSH! It's a tie.\x1b[0m\n");
        player->balance += bet;
    }
}

void play_blackjack(Player* player) {
    int is_playing = 1;
    print_blackjack_welcome();

    while (is_playing) {
        // שימוש בתשתית הכותרת האחידה שלנו (DRY)
        print_table_header("BLACKJACK TABLE", "\x1b[36m", player->balance);

        printf("Options: [0] Leave Table  [1] Place Bet\nAction: ");
        int action = get_safe_int();
        if (action == 0) break;
        if (action != 1) continue;

        printf("Enter bet amount: $");
        int bet1 = get_safe_int();

        if (bet1 <= 0 || bet1 > player->balance) {
            printf("\x1b[31mInvalid amount or insufficient funds!\x1b[0m\n");
            continue;
        }

        player->balance -= bet1;
        printf("Bet of $%d placed. Dealing cards...\n", bet1);

        Card* deck = create_deck();
        shuffle_deck(deck);

        // שימוש ב-safe_malloc לידיים
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
            printf("\n\x1b[32mBLACKJACK!\x1b[0m You hit 21 instantly!\n");
            print_hands(d_hand, d_count, "Dealer", 0);

            if (d_val == 21) {
                printf("\x1b[33mPush (Tie).\x1b[0m Dealer also has Blackjack.\n");
                player->balance += bet1;
            }
            else {
                int win_amount = bet1 + (int)(bet1 * 1.5);
                printf("\x1b[32mYou won $%d!\x1b[0m\n", (int)(bet1 * 1.5));
                player->balance += win_amount;
                player->total_winnings += (win_amount - bet1);
            }
        }
        else {
            if (strcmp(p_hand1[0].rank, p_hand1[1].rank) == 0 && player->balance >= bet1) {
                printf("\n\x1b[36mYou have a pair! Do you want to SPLIT? (Costs an additional $%d)\x1b[0m\n", bet1);
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

                    printf("\n\x1b[36m--- HANDS SPLIT ---\x1b[0m\n");
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
            printf("\n\x1b[31mYou are bankrupt! Security is escorting you out.\x1b[0m\n");
            is_playing = 0;
        }
    }
}