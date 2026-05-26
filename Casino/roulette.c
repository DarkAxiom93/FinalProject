#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "casino.h"
#include "utils.h"

#define MAX_BETS_PER_SPIN 10

int get_number_color(int number) {
    if (number == 0 || number == 37) return 0;
    int red_numbers[] = { 1, 3, 5, 7, 9, 12, 14, 16, 18, 21, 23, 25, 27, 28, 30, 32, 34, 36 };
    for (int i = 0; i < 18; i++) {
        if (number == red_numbers[i]) return 1;
    }
    return 2;
}

// פונקציה חדשה: הדפסת 5 התוצאות האחרונות (UX)
void print_spin_history(int history[]) {
    printf("\n\x1b[36m--- LAST 5 SPINS ---\x1b[0m\n[ ");
    for (int i = 0; i < 5; i++) {
        if (history[i] != -1) {
            int color = get_number_color(history[i]);
            if (history[i] == 37) printf(BG_GREEN TEXT_WHITE " 00 " RESET " ");
            else if (color == 1) printf(BG_RED TEXT_WHITE " %02d " RESET " ", history[i]);
            else if (color == 2) printf(BG_BLACK TEXT_WHITE " %02d " RESET " ", history[i]);
            else printf(BG_GREEN TEXT_WHITE "  0 " RESET " ");
        }
        else {
            printf(" --  ");
        }
    }
    printf("]\n");
}

void print_roulette_board() {
    printf("\n");
    printf(BG_GREEN TEXT_WHITE "   00    " RESET);
    for (int i = 3; i <= 36; i += 3) {
        if (get_number_color(i) == 1) printf(BG_RED TEXT_WHITE "  %2d   " RESET, i);
        else printf(BG_BLACK TEXT_WHITE "  %2d   " RESET, i);
    }
    printf("\n");

    printf(BG_GREEN TEXT_WHITE "         " RESET);
    for (int i = 2; i <= 35; i += 3) {
        if (get_number_color(i) == 1) printf(BG_RED TEXT_WHITE "  %2d   " RESET, i);
        else printf(BG_BLACK TEXT_WHITE "  %2d   " RESET, i);
    }
    printf("\n");

    printf(BG_GREEN TEXT_WHITE "    0    " RESET);
    for (int i = 1; i <= 34; i += 3) {
        if (get_number_color(i) == 1) printf(BG_RED TEXT_WHITE "  %2d   " RESET, i);
        else printf(BG_BLACK TEXT_WHITE "  %2d   " RESET, i);
    }
    printf("\n");

    printf("         ");
    printf(BG_GREEN TEXT_WHITE "           1st 12           " RESET);
    printf(BG_GREEN TEXT_WHITE "           2nd 12           " RESET);
    printf(BG_GREEN TEXT_WHITE "           3rd 12           " RESET);
    printf("\n");

    printf("         ");
    printf(BG_GREEN TEXT_WHITE "     1-18     " RESET);
    printf(BG_GREEN TEXT_WHITE "     Even     " RESET);
    printf(BG_RED TEXT_WHITE   "     Red      " RESET);
    printf(BG_BLACK TEXT_WHITE "    Black     " RESET);
    printf(BG_GREEN TEXT_WHITE "     Odd      " RESET);
    printf(BG_GREEN TEXT_WHITE "     19-36    " RESET);
    printf("\n\n");
}

void print_active_bets(Bet active_bets[], int count) {
    if (count == 0) {
        printf("\n\x1b[33mNo active bets on the table right now.\x1b[0m\n");
        return;
    }

    printf("\n--- CURRENT ACTIVE BETS ---\n");
    for (int i = 0; i < count; i++) {
        printf("[%d] $%d placed on: ", i + 1, active_bets[i].amount);
        switch (active_bets[i].bet_type) {
        case 1:
            if (active_bets[i].numbers[0] == 37) printf("Number 00");
            else printf("Number %d", active_bets[i].numbers[0]);
            break;
        case 2:
            printf("Split [%d, %d]", active_bets[i].numbers[0], active_bets[i].numbers[1]);
            break;
        case 3:
            if (active_bets[i].numbers[0] == 1) printf("1st 12 (1-12)");
            else if (active_bets[i].numbers[0] == 2) printf("2nd 12 (13-24)");
            else printf("3rd 12 (25-36)");
            break;
        case 4:
            printf("Color %s", active_bets[i].numbers[0] == 1 ? "RED" : "BLACK");
            break;
        case 5:
            printf("%s", active_bets[i].numbers[0] == 1 ? "EVEN" : "ODD");
            break;
        case 6:
            printf("Half %s", active_bets[i].numbers[0] == 1 ? "1-18" : "19-36");
            break;
        }
        printf("\n");
    }
    printf("---------------------------\n");
}

Bet place_bet() {
    Bet new_bet = { 0 };
    new_bet.bet_type = 0;

    printf("\n--- SELECT BET TYPE ---\n");
    printf("1. Straight Up (1 Number)  [Pays 35:1]\n");
    printf("2. Split (2 Numbers)       [Pays 17:1]\n");
    printf("3. Dozens (1st/2nd/3rd 12) [Pays  2:1]\n");
    printf("4. Color (Red/Black)       [Pays  1:1]\n");
    printf("5. Even/Odd                [Pays  1:1]\n");
    printf("6. Halves (1-18 / 19-36)   [Pays  1:1]\n");
    printf("Choose option: ");

    int bet_choice = get_safe_int();

    switch (bet_choice) {
    case 1:
        printf("Enter a number (0-36, or 37 for '00'): ");
        int num = get_safe_int();
        if (num < 0 || num > 37) {
            printf("\x1b[31mError: Invalid roulette number! Must be 0-36, or 37 for 00.\x1b[0m\n");
            return new_bet;
        }
        new_bet.numbers[0] = num;
        new_bet.num_count = 1;
        new_bet.bet_type = 1;
        break;

    case 2:
        printf("Enter FIRST number for the split (1-36): ");
        int first_num = get_safe_int();
        if (first_num < 1 || first_num > 36) {
            printf("\x1b[31mError: Split must start with a number 1-36.\x1b[0m\n");
            return new_bet;
        }
        new_bet.numbers[0] = first_num;

        int valid_options[4];
        int valid_count = 0;
        if (first_num - 3 >= 1) valid_options[valid_count++] = first_num - 3;
        if (first_num + 3 <= 36) valid_options[valid_count++] = first_num + 3;
        if (first_num % 3 != 1) valid_options[valid_count++] = first_num - 1;
        if (first_num % 3 != 0) valid_options[valid_count++] = first_num + 1;

        printf("Valid adjacent numbers for %d are: ", first_num);
        for (int i = 0; i < valid_count; i++) printf("[%d] ", valid_options[i]);
        printf("\nSelect the SECOND number from the list above: ");
        int second_num = get_safe_int();

        int is_valid = 0;
        for (int i = 0; i < valid_count; i++) {
            if (second_num == valid_options[i]) {
                is_valid = 1;
                break;
            }
        }
        if (!is_valid) {
            printf("\x1b[31mError: %d is not adjacent to %d on the board!\x1b[0m\n", second_num, first_num);
            return new_bet;
        }
        new_bet.numbers[1] = second_num;
        new_bet.num_count = 2;
        new_bet.bet_type = 2;
        break;

    case 3:
        printf("Choose Dozen (1 for 1-12, 2 for 13-24, 3 for 25-36): ");
        int dozen = get_safe_int();
        if (dozen < 1 || dozen > 3) {
            printf("\x1b[31mError: Invalid Dozen choice.\x1b[0m\n");
            return new_bet;
        }
        new_bet.numbers[0] = dozen;
        new_bet.num_count = 1;
        new_bet.bet_type = 3;
        break;

    case 4:
        printf("Choose color (\x1b[31mR\x1b[0m for Red, \x1b[97mB\x1b[0m for Black): ");
        char color_choice;

        if (scanf(" %c", &color_choice) != 1) {
            printf("\x1b[31mInvalid input format.\x1b[0m\n");
            while (getchar() != '\n');
            return new_bet;
        }
        while (getchar() != '\n');

        if (color_choice == 'R' || color_choice == 'r') {
            new_bet.numbers[0] = 1;
        }
        else if (color_choice == 'B' || color_choice == 'b') {
            new_bet.numbers[0] = 2;
        }
        else {
            printf("\x1b[31mError: Invalid choice! You must enter R or B.\x1b[0m\n");
            return new_bet;
        }
        new_bet.num_count = 1;
        new_bet.bet_type = 4;
        break;

    case 5:
        printf("Choose (1 for Even, 2 for Odd): ");
        new_bet.numbers[0] = get_safe_int();
        new_bet.num_count = 1;
        new_bet.bet_type = 5;
        break;

    case 6:
        printf("Choose Half (1 for 1-18, 2 for 19-36): ");
        new_bet.numbers[0] = get_safe_int();
        new_bet.num_count = 1;
        new_bet.bet_type = 6;
        break;

    default:
        printf("\x1b[31mInvalid bet type!\x1b[0m\n");
        return new_bet;
    }

    printf("Enter bet amount: $");
    new_bet.amount = get_safe_int();
    return new_bet;
}

int check_win(Bet b, int spin_result) {
    int spin_color = get_number_color(spin_result);
    int is_winner = 0;
    int payout_multiplier = 0;

    switch (b.bet_type) {
    case 1:
        if (spin_result == b.numbers[0]) {
            is_winner = 1; payout_multiplier = 35;
        }
        break;
    case 2:
        if (spin_result == b.numbers[0] || spin_result == b.numbers[1]) {
            is_winner = 1; payout_multiplier = 17;
        }
        break;
    case 3:
        if (b.numbers[0] == 1 && spin_result >= 1 && spin_result <= 12) { is_winner = 1; payout_multiplier = 2; }
        else if (b.numbers[0] == 2 && spin_result >= 13 && spin_result <= 24) { is_winner = 1; payout_multiplier = 2; }
        else if (b.numbers[0] == 3 && spin_result >= 25 && spin_result <= 36) { is_winner = 1; payout_multiplier = 2; }
        break;
    case 4:
        if (spin_color == b.numbers[0]) {
            is_winner = 1; payout_multiplier = 1;
        }
        break;
    case 5:
        if (spin_result != 0 && spin_result != 37) {
            if (b.numbers[0] == 1 && spin_result % 2 == 0) { is_winner = 1; payout_multiplier = 1; }
            else if (b.numbers[0] == 2 && spin_result % 2 != 0) { is_winner = 1; payout_multiplier = 1; }
        }
        break;
    case 6:
        if (spin_result != 0 && spin_result != 37) {
            if (b.numbers[0] == 1 && spin_result >= 1 && spin_result <= 18) { is_winner = 1; payout_multiplier = 1; }
            else if (b.numbers[0] == 2 && spin_result >= 19 && spin_result <= 36) { is_winner = 1; payout_multiplier = 1; }
        }
        break;
    }

    if (is_winner) return b.amount + (b.amount * payout_multiplier);
    return 0;
}

void print_roulette_welcome() {
    system("cls");
    printf("\x1b[31m");
    printf("  _______  _______  __   __  ___      _______  _______  _______  _______ \n");
    printf(" |       ||       ||  | |  ||   |    |       ||       ||       ||       |\n");
    printf(" |    _  ||   _   ||  | |  ||   |    |    ___||_     _||_     _||    ___|\n");
    printf(" |   |_| ||  | |  ||  |_|  ||   |    |   |___   |   |    |   |  |   |___ \n");
    printf(" |    __ <|  |_|  ||       ||   |___ |    ___|  |   |    |   |  |    ___|\n");
    printf(" |   |  | |       ||       ||       ||   |___   |   |    |   |  |   |___ \n");
    printf(" |___|  |_|_______||_______||_______||_______|  |___|    |___|  |_______|\n");
    printf("\x1b[0m\n");

    printf("\x1b[36m=========================================================================\x1b[0m\n");
    printf("                       \x1b[33mTABLE RULES & PAYOUTS\x1b[0m\n");
    printf("\x1b[36m=========================================================================\x1b[0m\n");
    printf(" * Straight Up (1 Number)  : Pays 35 to 1\n");
    printf(" * Split (2 Numbers)       : Pays 17 to 1\n");
    printf(" * Street (3 Numbers)      : Pays 11 to 1\n");
    printf(" * Dozens (12 Numbers)     : Pays 2 to 1\n");
    printf(" * Color / Even / Odd      : Pays 1 to 1\n");
    printf("\x1b[36m=========================================================================\x1b[0m\n\n");

    printf("\x1b[32mPress ENTER to acknowledge rules and join the table...\x1b[0m");
    wait_for_enter();
    system("cls");
}

void play_roulette(Player* player) {
    int is_playing = 1;
    Bet active_bets[MAX_BETS_PER_SPIN];
    int num_active_bets = 0;

    // מערך ההיסטוריה החדש (-1 מייצג שאין עדיין תוצאה)
    int history[5] = { -1, -1, -1, -1, -1 };

    print_roulette_welcome();

    while (is_playing) {
        print_table_header("ROULETTE TABLE", "\x1b[36m", player->balance);
        print_spin_history(history); // קריאה להדפסת ההיסטוריה
        print_roulette_board();
        printf("Active Bets on table: %d\n", num_active_bets);

        printf("\nOptions:\n");
        printf("0. Leave Table\n");
        printf("1. Place a Bet\n");
        printf("2. View Active Bets\n");
        if (num_active_bets > 0) {
            printf("3. \x1b[33mSPIN THE WHEEL!\x1b[0m\n");
        }
        printf("Action: ");

        int action = get_safe_int();

        if (action == 0) {
            if (num_active_bets > 0) {
                printf("\x1b[33mWarning: You have active bets on the table. You must spin first!\x1b[0m\n");
                continue;
            }
            is_playing = 0;
            break;
        }
        else if (action == 1) {
            if (num_active_bets >= MAX_BETS_PER_SPIN) {
                printf("\x1b[31mMaximum bets reached for this spin.\x1b[0m\n");
                continue;
            }

            Bet current_bet = place_bet();
            if (current_bet.bet_type == 0) {
                printf("Bet cancelled due to invalid selection.\n");
                continue;
            }
            if (current_bet.amount > player->balance || current_bet.amount <= 0) {
                printf("\x1b[31mInvalid amount or insufficient funds!\x1b[0m\n");
                continue;
            }

            player->balance -= current_bet.amount;
            active_bets[num_active_bets] = current_bet;
            num_active_bets++;
            printf("\x1b[32mBet placed successfully!\x1b[0m New balance: $%d\n", player->balance);
        }
        else if (action == 2) {
            print_active_bets(active_bets, num_active_bets);
        }
        else if (action == 3 && num_active_bets > 0) {
            printf("\n\x1b[33mNO MORE BETS!\x1b[0m\n");

            for (int i = 3; i > 0; i--) {
                printf("Spinning in %d...\n", i);
                delay_ms(800);
            }

            printf("\n");
            for (int spin_anim = 0; spin_anim < 25; spin_anim++) {
                int temp_res = rand() % 38;
                printf("\r[ \x1b[36m*\x1b[0m ] Ball rolling... %2d ", temp_res == 37 ? 00 : temp_res);
                fflush(stdout);
                delay_ms(50 + (spin_anim * 10));
            }
            printf("\r                                      \r");

            int spin_result = rand() % 38;
            int spin_color = get_number_color(spin_result);

            printf("The ball landed on: ");
            if (spin_result == 37) printf(BG_GREEN TEXT_WHITE "  00  " RESET "\n\n");
            else if (spin_color == 1) printf(BG_RED TEXT_WHITE "  %d (RED)  " RESET "\n\n", spin_result);
            else if (spin_color == 2) printf(BG_BLACK TEXT_WHITE "  %d (BLACK)  " RESET "\n\n", spin_result);
            else printf(BG_GREEN TEXT_WHITE "  0 (GREEN)  " RESET "\n\n");

            // עדכון היסטוריית המספרים - מזיזים הכל שמאלה ודוחפים את החדש
            for (int i = 4; i > 0; i--) {
                history[i] = history[i - 1];
            }
            history[0] = spin_result;

            int total_round_winnings = 0;
            for (int i = 0; i < num_active_bets; i++) {
                int winnings = check_win(active_bets[i], spin_result);
                if (winnings > 0) {
                    total_round_winnings += winnings;
                    player->total_winnings += (winnings - active_bets[i].amount);
                }
                else {
                    player->total_losses += active_bets[i].amount;
                }
            }

            if (total_round_winnings > 0) {
                printf("\x1b[32mWINNER!\x1b[0m You won a total of $%d in this spin!\n", total_round_winnings);
                player->balance += total_round_winnings;
            }
            else {
                printf("\x1b[31mAll bets lost this round.\x1b[0m\n");
            }

            num_active_bets = 0;

            if (player->balance <= 0) {
                printf("\n\x1b[31mYou are bankrupt! Security is escorting you out.\x1b[0m\n");
                is_playing = 0;
            }
        }
        else {
            printf("\x1b[33mInvalid action.\x1b[0m\n");
        }
    }
}