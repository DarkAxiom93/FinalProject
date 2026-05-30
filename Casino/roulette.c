#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "casino.h"
#include "utils.h"
#include "graphics.h"
#include "account.h"

#define MAX_BETS_PER_SPIN 10

// הגדרות צבעים מקומיות ללוח הרולטה (כדי למנוע שגיאות קומפילציה)
#define BG_GREEN   "\x1b[42m"
#define BG_RED     "\x1b[41m"
#define BG_BLACK   "\x1b[40m"
#define TEXT_WHITE "\x1b[97m"

static int get_number_color(int number) {
    if (number == 0 || number == 37) return 0;
    // תוקן המספר 19 במקום 28 לפי חוקי הקזינו המקוריים
    int red_numbers[] = { 1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36 };
    for (int i = 0; i < 18; i++) {
        if (number == red_numbers[i]) return 1;
    }
    return 2;
}

// פונקציה: הדפסת 5 התוצאות האחרונות (UX)
static void print_spin_history(int history[]) {
    printf("\n" C_CYAN "--- LAST 5 SPINS ---" C_RESET "\n[ ");
    for (int i = 0; i < 5; i++) {
        if (history[i] != -1) {
            int color = get_number_color(history[i]);
            if (history[i] == 37) printf(BG_GREEN TEXT_WHITE " 00 " C_RESET " ");
            else if (color == 1) printf(BG_RED TEXT_WHITE " %02d " C_RESET " ", history[i]);
            else if (color == 2) printf(BG_BLACK TEXT_WHITE " %02d " C_RESET " ", history[i]);
            else printf(BG_GREEN TEXT_WHITE "  0 " C_RESET " ");
        }
        else {
            printf(" --  ");
        }
    }
    printf("]\n");
}

void print_roulette_board() {
    printf("\n");
    printf(BG_GREEN TEXT_WHITE "   00    " C_RESET);
    for (int i = 3; i <= 36; i += 3) {
        if (get_number_color(i) == 1) printf(BG_RED TEXT_WHITE "  %2d   " C_RESET, i);
        else printf(BG_BLACK TEXT_WHITE "  %2d   " C_RESET, i);
    }
    printf("\n");

    printf(BG_GREEN TEXT_WHITE "         " C_RESET);
    for (int i = 2; i <= 35; i += 3) {
        if (get_number_color(i) == 1) printf(BG_RED TEXT_WHITE "  %2d   " C_RESET, i);
        else printf(BG_BLACK TEXT_WHITE "  %2d   " C_RESET, i);
    }
    printf("\n");

    printf(BG_GREEN TEXT_WHITE "    0    " C_RESET);
    for (int i = 1; i <= 34; i += 3) {
        if (get_number_color(i) == 1) printf(BG_RED TEXT_WHITE "  %2d   " C_RESET, i);
        else printf(BG_BLACK TEXT_WHITE "  %2d   " C_RESET, i);
    }
    printf("\n");

    printf("         ");
    printf(BG_GREEN TEXT_WHITE "           1st 12           " C_RESET);
    printf(BG_GREEN TEXT_WHITE "           2nd 12           " C_RESET);
    printf(BG_GREEN TEXT_WHITE "           3rd 12           " C_RESET);
    printf("\n");

    printf("         ");
    printf(BG_GREEN TEXT_WHITE "     1-18     " C_RESET);
    printf(BG_GREEN TEXT_WHITE "     Even     " C_RESET);
    printf(BG_RED TEXT_WHITE   "     Red      " C_RESET);
    printf(BG_BLACK TEXT_WHITE "    Black     " C_RESET);
    printf(BG_GREEN TEXT_WHITE "     Odd      " C_RESET);
    printf(BG_GREEN TEXT_WHITE "     19-36    " C_RESET);
    printf("\n\n");
}

static void print_active_bets(Bet active_bets[], int count) {
    if (count == 0) {
        printf("\n" C_YELLOW "No active bets on the table right now." C_RESET "\n");
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
            printf("Split [");
            if (active_bets[i].numbers[0] == 37) printf("00");
            else printf("%d", active_bets[i].numbers[0]);
            printf(", ");
            if (active_bets[i].numbers[1] == 37) printf("00");
            else printf("%d", active_bets[i].numbers[1]);
            printf("]");
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
        case 7:
            printf("Corner/Square [%d, %d, %d, %d]", active_bets[i].numbers[0], active_bets[i].numbers[1], active_bets[i].numbers[2], active_bets[i].numbers[3]);
            break;
        }
        printf("\n");
    }
    printf("---------------------------\n");
}

/*
 * פונקציה לקליטת מספר רולטה כטקסט
 * מפרידה בצורה מוחלטת בין "0" ל-"00" (שמיוצג במערכת כ-37)
 */
static int get_roulette_number() {
    clear_input_buffer();
    char input[50] = { 0 };
    if (scanf("%9s", input) == 1) {
        while (getchar() != '\n');

        if (strcmp(input, "00") == 0) return 37;

        int num = atoi(input);
        if (num == 0 && input[0] != '0') return -1;
        if (num >= 0 && num <= 36) return num;
    }
    else {
        while (getchar() != '\n');
    }
    return -1;
}

static Bet handle_straight_bet() {
    Bet new_bet = { 0 };
    printf("Enter a number (0-36, or 00): ");
    int num = get_roulette_number();
    if (num == -1) {
        printf("" C_RED "Error: Invalid roulette number!" C_RESET "\n");
        return new_bet;
    }
    new_bet.numbers[0] = num;
    new_bet.num_count = 1;
    new_bet.bet_type = 1;
    return new_bet;
}

static Bet handle_split_corner_bet() {
    Bet new_bet = { 0 };
    printf("Enter FIRST number for the split (0-36, or 00): ");
    int first_num = get_roulette_number();
    if (first_num == -1) {
        printf("" C_RED "Error: Invalid roulette number." C_RESET "\n");
        return new_bet;
    }
    new_bet.numbers[0] = first_num;

    int valid_options[8] = { 0 };
    int valid_count = 0;

    if (first_num == 0) {
        valid_options[valid_count++] = 1; valid_options[valid_count++] = 2;
        valid_options[valid_count++] = 3; valid_options[valid_count++] = 37;
    }
    else if (first_num == 37) {
        valid_options[valid_count++] = 1; valid_options[valid_count++] = 2;
        valid_options[valid_count++] = 3; valid_options[valid_count++] = 0;
    }
    else {
        if (first_num - 3 >= 1) valid_options[valid_count++] = first_num - 3;
        if (first_num + 3 <= 36) valid_options[valid_count++] = first_num + 3;
        if (first_num % 3 != 1) valid_options[valid_count++] = first_num - 1;
        if (first_num % 3 != 0) valid_options[valid_count++] = first_num + 1;
        if (first_num <= 3) { valid_options[valid_count++] = 0; valid_options[valid_count++] = 37; }
    }

    if (first_num == 37) printf("Valid adjacent numbers for 00 are: ");
    else printf("Valid adjacent numbers for %d are: ", first_num);

    for (int i = 0; i < valid_count; i++) {
        if (valid_options[i] == 37) printf("[00] ");
        else printf("[%d] ", valid_options[i]);
    }

    printf("\nSelect the SECOND number from the list above: ");
    int second_num = get_roulette_number();

    int is_valid = 0;
    for (int i = 0; i < valid_count; i++) {
        if (second_num == valid_options[i]) { is_valid = 1; break; }
    }
    if (!is_valid) {
        printf("" C_RED "Error: Number not adjacent on the board!" C_RESET "\n");
        return new_bet;
    }

    new_bet.numbers[1] = second_num;
    new_bet.num_count = 2;
    new_bet.bet_type = 2;

    int min_n = first_num < second_num ? first_num : second_num;
    int max_n = first_num > second_num ? first_num : second_num;

    if (min_n != 0 && min_n != 37 && max_n != 0 && max_n != 37) {
        char expand_choice = ' ';
        while (1) {
            printf("\n" C_CYAN "Expand this split into a 4-number Corner bet? (Y/N):" C_RESET " ");
            if (scanf(" %c", &expand_choice) == 1) {
                while (getchar() != '\n');
                if (expand_choice == 'Y' || expand_choice == 'y' || expand_choice == 'N' || expand_choice == 'n') {
                    break;
                }
            }
            else {
                while (getchar() != '\n');
            }
            printf("" C_RED "Invalid input!" C_RESET " Please press Only Y for Yes or N for No.\n");
        }

        if (expand_choice == 'Y' || expand_choice == 'y') {
            int valid_corners[2][4] = { 0 };
            int corner_count = 0;

            if (max_n - min_n == 1) {
                if (min_n > 3) {
                    valid_corners[corner_count][0] = min_n - 3; valid_corners[corner_count][1] = max_n - 3;
                    valid_corners[corner_count][2] = min_n;     valid_corners[corner_count][3] = max_n;
                    corner_count++;
                }
                if (max_n <= 33) {
                    valid_corners[corner_count][0] = min_n;     valid_corners[corner_count][1] = max_n;
                    valid_corners[corner_count][2] = min_n + 3; valid_corners[corner_count][3] = max_n + 3;
                    corner_count++;
                }
            }
            else if (max_n - min_n == 3) {
                if (min_n % 3 != 1) {
                    valid_corners[corner_count][0] = min_n - 1; valid_corners[corner_count][1] = max_n - 1;
                    valid_corners[corner_count][2] = min_n;     valid_corners[corner_count][3] = max_n;
                    corner_count++;
                }
                if (max_n % 3 != 0) {
                    valid_corners[corner_count][0] = min_n;     valid_corners[corner_count][1] = max_n;
                    valid_corners[corner_count][2] = min_n + 1; valid_corners[corner_count][3] = max_n + 1;
                    corner_count++;
                }
            }

            if (corner_count > 0) {
                printf("Available Corners based on your split:\n");
                for (int c = 0; c < corner_count; c++) {
                    printf("[%d]: [%d, %d, %d, %d]\n", c + 1, valid_corners[c][0], valid_corners[c][1], valid_corners[c][2], valid_corners[c][3]);
                }
                while (1) {
                    printf("Select corner option (1");
                    if (corner_count == 2) printf(" or 2");
                    printf(", or 0 to cancel expansion and keep original split): ");
                    int corner_choice = get_safe_int();

                    if (corner_choice == 0) {
                        printf("Expansion cancelled. Keeping original Split bet.\n");
                        break;
                    }
                    if (corner_choice > 0 && corner_choice <= corner_count) {
                        int sel = corner_choice - 1;
                        new_bet.numbers[0] = valid_corners[sel][0];
                        new_bet.numbers[1] = valid_corners[sel][1];
                        new_bet.numbers[2] = valid_corners[sel][2];
                        new_bet.numbers[3] = valid_corners[sel][3];
                        new_bet.num_count = 4;
                        new_bet.bet_type = 7;
                        printf("" C_GREEN "Upgraded to Corner Bet!" C_RESET "\n");
                        break;
                    }
                    printf("" C_RED "Invalid option!" C_RESET " Please try again.\n");
                }
            }
        }
    }
    return new_bet;
}

static Bet handle_dozen_bet() {
    Bet new_bet = { 0 };
    printf("Choose Dozen (" C_WHITE "A" C_RESET " = 1-12, " C_WHITE "B" C_RESET " = 13-24, " C_WHITE "C" C_RESET " = 25-36): ");
    clear_input_buffer();
    char dozen_choice;
    if (scanf(" %c", &dozen_choice) != 1) {
        printf("" C_RED "Invalid input format." C_RESET "\n");
        while (getchar() != '\n'); return new_bet;
    }
    while (getchar() != '\n');
    if (dozen_choice == 'A' || dozen_choice == 'a') new_bet.numbers[0] = 1;
    else if (dozen_choice == 'B' || dozen_choice == 'b') new_bet.numbers[0] = 2;
    else if (dozen_choice == 'C' || dozen_choice == 'c') new_bet.numbers[0] = 3;
    else { printf("" C_RED "Error: Invalid choice!" C_RESET "\n"); return new_bet; }
    new_bet.num_count = 1; new_bet.bet_type = 3;
    return new_bet;
}

static Bet handle_color_bet() {
    Bet new_bet = { 0 };
    printf("Choose color (" C_RED "R" C_RESET " for Red, " C_WHITE "B" C_RESET " for Black): ");
    clear_input_buffer();
    char color_choice;
    if (scanf(" %c", &color_choice) != 1) {
        printf("" C_RED "Invalid input format." C_RESET "\n");
        while (getchar() != '\n'); return new_bet;
    }
    while (getchar() != '\n');
    if (color_choice == 'R' || color_choice == 'r') new_bet.numbers[0] = 1;
    else if (color_choice == 'B' || color_choice == 'b') new_bet.numbers[0] = 2;
    else { printf("" C_RED "Error: Invalid choice!" C_RESET "\n"); return new_bet; }
    new_bet.num_count = 1; new_bet.bet_type = 4;
    return new_bet;
}

static Bet handle_parity_bet() {
    Bet new_bet = { 0 };
    printf("Choose (" C_WHITE "E" C_RESET " for Even, " C_WHITE "O" C_RESET " for Odd): ");
    clear_input_buffer();
    char parity_choice;
    if (scanf(" %c", &parity_choice) != 1) {
        printf("" C_RED "Invalid input format." C_RESET "\n");
        while (getchar() != '\n'); return new_bet;
    }
    while (getchar() != '\n');
    if (parity_choice == 'E' || parity_choice == 'e') new_bet.numbers[0] = 1;
    else if (parity_choice == 'O' || parity_choice == 'o') new_bet.numbers[0] = 2;
    else { printf("" C_RED "Error: Invalid choice!" C_RESET "\n"); return new_bet; }
    new_bet.num_count = 1; new_bet.bet_type = 5;
    return new_bet;
}

static Bet place_bet() {
    Bet new_bet = { 0 };

    printf("\n--- SELECT BET TYPE ---\n");
    printf("1. Straight Up (1 Number)       [Pays 35:1]\n");
    printf("2. Split / Corner (2 or 4 Nums) [Pays 17:1 or 8:1]\n");
    printf("3. Dozens (A=1st, B=2nd, C=3rd) [Pays  2:1]\n");
    printf("4. Color (R=Red, B=Black)       [Pays  1:1]\n");
    printf("5. Even/Odd (E=Even, O=Odd)     [Pays  1:1]\n");
    printf("6. Halves (1-18 / 19-36)        [Pays  1:1]\n");
    printf("Choose option: ");

    int bet_choice = get_safe_int();

    switch (bet_choice) {
    case 1:
        new_bet = handle_straight_bet();
        break;
    case 2:
        new_bet = handle_split_corner_bet();
        break;
    case 3:
        new_bet = handle_dozen_bet();
        break;
    case 4:
        new_bet = handle_color_bet();
        break;
    case 5:
        new_bet = handle_parity_bet();
        break;
    case 6:
        printf("Choose Half (1 for 1-18, 2 for 19-36): ");
        new_bet.numbers[0] = get_safe_int();
        new_bet.num_count = 1;
        new_bet.bet_type = 6;
        break;
    default:
        printf("" C_RED "Invalid bet type!" C_RESET "\n");
        return new_bet;
    }

    // אימות שהלוגיקה הפנימית לא נכשלה לפני בקשת סכום
    if (new_bet.bet_type == 0) {
        return new_bet;
    }

    printf("Enter bet amount: $");
    new_bet.amount = get_safe_int();
    return new_bet;
}

static int check_win(Bet b, int spin_result) {
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
    case 7:
        if (spin_result == b.numbers[0] || spin_result == b.numbers[1] ||
            spin_result == b.numbers[2] || spin_result == b.numbers[3]) {
            is_winner = 1; 
            payout_multiplier = 8;
        }
        break;
    }

    if (is_winner) return b.amount + (b.amount * payout_multiplier);
    return 0;
}

void play_roulette(Player* player) {
    int is_playing = 1;
    Bet active_bets[MAX_BETS_PER_SPIN] = { 0 };
    int num_active_bets = 0;

    // מערך ההיסטוריה החדש (-1 מייצג שאין עדיין תוצאה)
    static int history[5] = { -1, -1, -1, -1, -1 };

    print_roulette_welcome();

    while (is_playing) {
        clear_screen();
        print_table_header("ROULETTE TABLE", C_CYAN, player->balance);
        print_spin_history(history); // קריאה להדפסת ההיסטוריה
        print_roulette_board();
        printf("Active Bets on table: %d\n", num_active_bets);

        printf("\nOptions:\n");
        printf("0. Leave Table\n");
        printf("1. Place a Bet\n");
        printf("2. View Active Bets\n");
        if (num_active_bets > 0) {
            printf("3. " C_YELLOW "SPIN THE WHEEL!" C_RESET "\n");
            printf("4. " C_RED "Cancel Last Bet (Undo)" C_RESET "\n");
        }
        printf("Action: ");

        int action = get_safe_int();

        if (action == 0) {
            if (num_active_bets > 0) {
                printf(C_YELLOW "Warning: You have active bets on the table. You must spin first!" C_RESET "\n");
                continue;
            }
            is_playing = 0;
            break;
        }
        else if (action == 1) {
            if (num_active_bets >= MAX_BETS_PER_SPIN) {
                printf(C_RED "Maximum bets reached for this spin." C_RESET "\n");
                continue;
            }

            Bet current_bet = place_bet();
            if (current_bet.bet_type == 0) {
                printf("Bet cancelled due to invalid selection.\n");
                continue;
            }
            if (current_bet.amount > player->balance || current_bet.amount <= 0) {
                display_error(1500, "Invalid amount or insufficient funds!");
                continue;
            }
            if (current_bet.amount > MAX_BET) {
                display_error(1500, "Table maximum bet is $%d!", MAX_BET);
                continue;
            }
            player->balance -= current_bet.amount;
            save_player(player);
            active_bets[num_active_bets] = current_bet;
            num_active_bets++;
            printf(C_GREEN "Bet placed successfully!" C_RESET " New balance: $%d\n", player->balance);
            delay_ms(1500);
        }
        else if (action == 2) {
            print_active_bets(active_bets, num_active_bets);
            prompt_continue(NULL);
        }
        else if (action == 4 && num_active_bets > 0) {
            num_active_bets--;
            player->balance += active_bets[num_active_bets].amount;
            save_player(player);
            printf("\n" C_GREEN "Last bet cancelled successfully. $%d refunded." C_RESET "\n", active_bets[num_active_bets].amount);
            delay_ms(1500);
        }
        else if (action == 3 && num_active_bets > 0) {
            printf("\n" C_YELLOW "NO MORE BETS!" C_RESET "\n");

            for (int i = 3; i > 0; i--) {
                printf("Spinning in %d...\n", i);
                delay_ms(800);
            }

            printf("\n");
            play_spin_sound(); // מתחיל לנגן את צליל הגלגול ברקע
            for (int spin_anim = 0; spin_anim < 25; spin_anim++) {
                // שימוש במנוע הויזואלי בלבד להצגת הכדור המתגלגל
                int temp_res = visual_rand() % 38;
                printf("\r[ " C_CYAN "*" C_RESET " ] Ball rolling... %2d ", temp_res == 37 ? 00 : temp_res);
                fflush(stdout);
                delay_ms(50 + (spin_anim * 10));
            }
            printf("\r                                      \r");
            stop_sound(); // עוצר את צליל הגלגול
            // התוצאה האמיתית מוגרלת מהמחולל הראשי המאובטח
            int spin_result = rand() % 38;
            int spin_color = get_number_color(spin_result);

            printf("The ball landed on: ");
            if (spin_result == 37) printf(BG_GREEN TEXT_WHITE "  00  " C_RESET "\n\n");
            else if (spin_color == 1) printf(BG_RED TEXT_WHITE "  %d (RED)  " C_RESET "\n\n", spin_result);
            else if (spin_color == 2) printf(BG_BLACK TEXT_WHITE "  %d (BLACK)  " C_RESET "\n\n", spin_result);
            else printf(BG_GREEN TEXT_WHITE "  0 (GREEN)  " C_RESET "\n\n");

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
                    player->total_winnings += ((long long)winnings - active_bets[i].amount);
                }
                else {
                    player->total_losses += active_bets[i].amount;
                }
            }

            // קוד חדש:
            if (total_round_winnings > 0) {
                play_win_sound();
                printf(C_GREEN "WINNER!" C_RESET " You won a total of $%d in this spin!\n", total_round_winnings);
                add_balance_safe(player, total_round_winnings);
            }
            else {
                printf(C_RED "All bets lost this round." C_RESET "\n");
            }

            save_player(player);
            num_active_bets = 0;

            prompt_continue("Press ENTER to clear table for the next spin...");

            if (player->balance <= 0) {
                printf("\n" C_RED "You are bankrupt! Security is escorting you out." C_RESET "\n");
                is_playing = 0;
            }
        }
        else {
            printf(C_YELLOW "Invalid action." C_RESET "\n");
        }
    }
}