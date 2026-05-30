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
// פונקציית עזר לזיהוי יד של "17 רך" (אס וקלפים שסכומם 6)
static int is_soft_17(Card* hand, int count) {
    int total = 0, hard_total = 0;

    for (int i = 0; i < count; i++) {
        // חישוב מקסימלי (אס נחשב כ-11, פנים נחשבים כ-10)
        int v = (hand[i].rank_val == 14) ? 11 :
            (hand[i].rank_val >= 11) ? 10 : hand[i].rank_val;
        total += v;

        // חישוב קשיח (אס נחשב כ-1 תמיד)
        int hard_v = (hand[i].rank_val == 14) ? 1 :
            (hand[i].rank_val >= 11) ? 10 : hand[i].rank_val;
        hard_total += hard_v;
    }

    // אם היד סוכמת ל-17, אבל הסכום הקשיח שונה - זהו 17 רך!
    return (total == 17 && total != hard_total);
}


static void play_hand(Card* hand, int* count, Card* deck, int* deck_idx, int* bet, Player* player, const char* hand_name, int* busted) {
    int val = calculate_hand_value(hand, *count);

    while (val < 21) {
        printf("\n[%s] Options: [1] Hit  [2] Stand  [3] Double Down: ", hand_name);

        clear_input_buffer(); // מוודאים שאין זבל בחוצץ
        int move = 0;
        char key;

        // לולאת קליטה מיידית של מקש
        while (1) {
            key = (char)_getch();
            if (key == '1') { move = 1; break; }
            else if (key == '2') { move = 2; break; }
            else if (key == '3') { move = 3; break; }
        }

        printf("%c\n", key); // מדפיסים את מה שהמשתמש לחץ כדי לתת פידבק ויזואלי

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
            // 1. חישוב סכום ההוספה המקסימלי המותר ללא גלישה מתקרת השולחן
            int allowed_addition = *bet;
            int was_capped = 0;

            if (*bet + allowed_addition > MAX_BET) {
                allowed_addition = MAX_BET - *bet;
                was_capped = 1;
            }

            // 2. חסימה אם ההימור ההתחלתי כבר היה במקסימום המותר
            if (allowed_addition <= 0) {
                display_error(1500, "Bet is already at table maximum ($%d). Cannot double down!", MAX_BET);
                continue;
            }

            // 3. בדיקת יתרה עבור סכום ההוספה בלבד (החלקי או המלא)
            if (player->balance < allowed_addition) {
                display_error(1500, "Insufficient funds to double down!");
                continue;
            }

            // 4. עריכת הפוינטר והיתרה באופן מסונכרן ובטוח
            player->balance -= allowed_addition;
            *bet += allowed_addition;
            save_player(player);

            if (was_capped) {
                printf("\n" C_YELLOW "Notice: Double Down capped at table maximum ($%d)!" C_RESET "\n", MAX_BET);
            }
            printf("[%s] Bet increased to $%d.\n", hand_name, *bet);

            // המשך משיכת קלף בודד כנדרש בחוקי Double Down
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
        player->total_winnings += bet;
        add_balance_safe(player, bet * 2); // החלפה מאובטחת
    }
    else if (d_val > p_val) {
        printf("" C_RED "Dealer Wins %d to %d." C_RESET "\n", d_val, p_val);
        player->total_losses += bet;
    }
    else if (d_val < p_val) {
        printf("" C_GREEN "YOU WIN $%d! %d to %d." C_RESET "\n", bet * 2, p_val, d_val);
        player->total_winnings += bet;
        add_balance_safe(player, bet * 2); // החלפה מאובטחת
    }
    else {
        printf("" C_YELLOW "PUSH! It's a tie." C_RESET "\n");
        add_balance_safe(player, bet); // החזרת ההימור באופו מאובטח
    }
}

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
        print_table_header("BLACKJACK TABLE", "" C_CYAN "", player->balance);

        printf("Options: [0] Leave Table  [1] Place Bet\nAction: ");
        int action = get_safe_int();
        if (action == 0) break;
        if (action != 1) continue;

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
        player->balance -= bet1;
        save_player(player);
        printf("Bet of $%d placed. Dealing cards...\n", bet1);

        // יצירת 6 חפיסות אמיתיות לבלאק ג'ק VIP מהמנוע הגנרי!
        Card deck[312] = { 0 };
        init_deck(deck, 6);
        shuffle_deck(deck, 312);

        // הקצאת הידיים מראש על ה-Stack ללא תלות במערכת ההפעלה
        Card p_hand1[12] = { 0 };
        Card p_hand2[12] = { 0 };
        Card d_hand[12] = { 0 };

        int p1_count = 0, p2_count = 0, d_count = 0, deck_idx = 0;
        int is_split = 0, bet2 = 0;

        p_hand1[p1_count++] = deck[deck_idx++];
        d_hand[d_count++] = deck[deck_idx++];
        p_hand1[p1_count++] = deck[deck_idx++];
        d_hand[d_count++] = deck[deck_idx++];

        print_cards_ascii(d_hand, d_count, "Dealer", 1);
        printf("Total Value: %d\n", calculate_hand_value(p_hand1, p1_count));
        print_cards_ascii(p_hand1, p1_count, player->name, 0);
        printf("Total Value: %d\n", calculate_hand_value(p_hand1, p1_count));

        int p1_val = calculate_hand_value(p_hand1, p1_count);
        int d_val = calculate_hand_value(d_hand, d_count);

        if (p1_val == 21) {
            printf("\n" C_GREEN "BLACKJACK!" C_RESET " You hit 21 instantly!\n");
            print_cards_ascii(d_hand, d_count, "Dealer", 0);
            printf("Total Value: %d\n", calculate_hand_value(d_hand, d_count));

            if (d_val == 21) {
                printf("" C_YELLOW "Push (Tie)." C_RESET " Dealer also has Blackjack.\n");
                add_balance_safe(player, bet1);
            }
            else {
                int win_amount = bet1 + (int)(bet1 * 1.5);
                printf("" C_GREEN "You won $%d!" C_RESET "\n", (int)(bet1 * 1.5));
                player->total_winnings += ((long long)win_amount - bet1);
                add_balance_safe(player, win_amount);
            }
        }
        else {
            if (p_hand1[0].rank_val == p_hand1[1].rank_val && player->balance >= bet1) {
                printf("\n" C_CYAN "You have a pair! Do you want to SPLIT? (Costs an additional $%d)" C_RESET "\n", bet1);
                printf("[1] Yes, Split  [2] No, Play as one hand: ");

                if (get_safe_int() == 1) {
                    is_split = 1;
                    bet2 = bet1;
                    player->balance -= bet2;
                    save_player(player);
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
            print_cards_ascii(p_hand1, p1_count, is_split ? "Hand 1" : player->name, 0);
            printf("Total Value: %d\n", calculate_hand_value(p_hand1, p1_count));
            play_hand(p_hand1, &p1_count, deck, &deck_idx, &bet1, player, is_split ? "Hand 1" : player->name, &p1_busted);

            int p2_busted = 0;
            if (is_split) {
                printf("\n--- PLAYING HAND 2 ---\n");
                print_cards_ascii(p_hand2, p2_count, "Hand 2", 0);
                printf("Total Value: %d\n", calculate_hand_value(p_hand2, p2_count));
                play_hand(p_hand2, &p2_count, deck, &deck_idx, &bet2, player, "Hand 2", &p2_busted);
            }

            if (!p1_busted || (is_split && !p2_busted)) {
                printf("\n--- Dealer's Turn ---\n");
                print_cards_ascii(d_hand, d_count, "Dealer", 0);
                printf("Total Value: %d\n", calculate_hand_value(d_hand, d_count));

                // הדילר מושך קלף אם יש לו פחות מ-17, או אם יש לו בדיוק 17 רך (אס ו-6)
                while (calculate_hand_value(d_hand, d_count) < 17 || is_soft_17(d_hand, d_count)) {
                    if (is_soft_17(d_hand, d_count)) {
                        printf("\n" C_YELLOW "Dealer has a Soft 17. Dealer must hit!" C_RESET "\n");
                    }

                    delay_ms(1500);
                    printf("Dealer hits...\n");
                    d_hand[d_count++] = deck[deck_idx++];
                    print_cards_ascii(d_hand, d_count, "Dealer", 0);
                    printf("Total Value: %d\n", calculate_hand_value(d_hand, d_count));
                }
                d_val = calculate_hand_value(d_hand, d_count);

                resolve_bets(calculate_hand_value(p_hand1, p1_count), p1_busted, d_val, bet1, player, is_split ? "Hand 1" : "Main Hand");
                if (is_split) {
                    resolve_bets(calculate_hand_value(p_hand2, p2_count), p2_busted, d_val, bet2, player, "Hand 2");
                }
            }
        }
        save_player(player);

        if (player->balance <= 0) {
            printf("\n" C_RED "You are bankrupt! Security is escorting you out." C_RESET "\n");
            is_playing = 0;
        }
    }
}
