#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "casino.h"
#include "utils.h"
#include <conio.h> 
#include "graphics.h"
#include "account.h"

static int get_weighted_symbol() {
    int r = rand() % 100;
    if (r < 5) return 0;
    else if (r < 20) return 1;
    else if (r < 45) return 2;
    else if (r < 75) return 3;
    else return 4;
}

void play_slots(Player* player) {
    int is_playing = 1;
    print_slots_welcome();

    int bet = 0;

    // מערך דינאמי ומונה עבור היסטוריית הרווחים/הפסדים פר סיבוב
    int* session_history = NULL;
    int history_count = 0;

    while (is_playing) {
        clear_screen();
        print_table_header("SLOT MACHINE", "" C_YELLOW "", player->balance);

        if (bet == 0) {
            printf("Options: [0] Leave Machine  [1] Insert Coin (Bet)\nAction: ");
            int action = get_safe_int();

            if (action == 0) break;
            if (action != 1) {
                printf("" C_YELLOW "Invalid option." C_RESET "\n");
                delay_ms(1000);
                continue;
            }

            printf("Enter bet amount: $");
            bet = get_safe_int();
        }
        else {
            printf("Current Bet: $%d (Running automatically)\n", bet);
        }

        if (bet <= 0 || bet > player->balance) {
            display_error(1500, "Invalid amount or insufficient funds!");
            bet = 0;
            continue;
        }

        if (bet > MAX_BET) {
            display_error(1500, "Machine maximum bet is $%d!", MAX_BET);
            bet = 0;
            continue;
        }

        player->balance -= bet;
        save_player(player);
        printf("\n" C_CYAN "Spinning the reels..." C_RESET "\n");

        // הקצאה דינאמית לתוצאות הגלגלים
        int* reels = (int*)malloc(3 * sizeof(int));
        if (reels == NULL) {
            display_error(2000, "Memory allocation failed for reels!");
            break;
        }

        // מציירים את כל המסגרת (כולל התחתית) מראש, ואז זזים חזרה לשורת הגלגלים כדי שהאנימציה תתבצע בתוכה
        draw_slot_machine_top();
        draw_slot_machine_row(visual_rand() % 5, visual_rand() % 5, visual_rand() % 5);
        draw_slot_machine_bottom();
        draw_slot_machine_cursor_to_row();

        for (int i = 0; i < 15; i++) {
            int v1 = visual_rand() % 5;
            int v2 = visual_rand() % 5;
            int v3 = visual_rand() % 5;
            draw_slot_machine_row(v1, v2, v3);
            delay_ms(80 + (i * 10));
        }

        reels[0] = rand() % 5;
        reels[1] = rand() % 5;
        reels[2] = rand() % 5;

        draw_slot_machine_row(reels[0], reels[1], reels[2]);
        draw_slot_machine_cursor_below();

        int payout = 0;
        if (reels[0] == 0 && reels[1] == 0 && reels[2] == 0) {
            payout = bet * 50;
            printf("\n" C_RED "*** J A C K P O T ***" C_RESET "\n");
            play_jackpot_sound();
            printf("" C_GREEN "UNBELIEVABLE! You hit three 7s and won $%d!" C_RESET "\n", payout);
        }
        else if (reels[0] == reels[1] && reels[1] == reels[2]) {
            payout = bet * 10;
            play_win_sound();
            printf("\n" C_GREEN "BIG WIN! Three of a kind! You won $%d!" C_RESET "\n", payout);
        }
        else if ((reels[0] == 0 && reels[1] == 0) || (reels[0] == 0 && reels[2] == 0) || (reels[1] == 0 && reels[2] == 0)) {
            payout = bet * 5;
            printf("\n" C_GREEN "NICE! Two 7s! You won $%d!" C_RESET "\n", payout);
        }
        else if (reels[0] == reels[1] || reels[0] == reels[2] || reels[1] == reels[2]) {
            payout = bet * 2;
            printf("\n" C_GREEN "Small Win! Two of a kind! You won $%d!" C_RESET "\n", payout);
        }
        else {
            printf("\n" C_RED "No match. Better luck next pull!" C_RESET "\n");
            player->total_losses += bet;
        }

        // חישוב נטו לסיבוב ועדכון ההיסטוריה הדינמית ב-realloc
        int net_profit = (payout > 0) ? (payout - bet) : -bet;
        int* temp = (int*)realloc(session_history, (history_count + 1) * sizeof(int));
        if (temp != NULL) {
            session_history = temp;
            session_history[history_count++] = net_profit;
        }

        if (payout > 0) {
            player->total_winnings += ((long long)payout - bet);
            add_balance_safe(player, payout);
        }

        save_player(player);
        free(reels); // שחרור מערך הגלגלים הדינמי בכל סיבוב

        if (player->balance <= 0) {
            is_playing = 0;
            break;
        }

        printf("\n--------------------------------------------------\n");
        printf(" -> Press [" C_GREEN "SPACE" C_RESET "] to spin again with the same bet ($%d)\n", bet);
        printf(" -> Press [" C_YELLOW "C" C_RESET "] to change bet amount\n");
        printf(" -> Press [" C_RED "0" C_RESET "] to exit to main menu\n");
        printf("--------------------------------------------------\n");

        clear_input_buffer();
        char key = ' ';
        while (1) {
            key = (char)_getch();
            if (key == ' ' || key == 'c' || key == 'C' || key == '0') {
                break;
            }
        }

        if (key == '0') {
            is_playing = 0;
        }
        else if (key == 'c' || key == 'C') {
            bet = 0;
        }
    }

    // הדפסת סיכום מהמערך הדינמי לפני היציאה
    if (session_history != NULL && history_count > 0) {
        clear_screen();
        printf("\n" C_CYAN "========================================" C_RESET "\n");
        printf("       SLOTS SESSION HISTORY LOG        \n");
        printf("" C_CYAN "========================================" C_RESET "\n");
        int total_net = 0;
        for (int i = 0; i < history_count; i++) {
            total_net += session_history[i];
            if (i < 10) { // מציג עד 10 סיבובים אחרונים כדי לא להציף את המסך
                if (session_history[i] > 0)
                    printf(" Spin %d: " C_GREEN "+$%d" C_RESET "\n", i + 1, session_history[i]);
                else
                    printf(" Spin %d: " C_RED "-$%d" C_RESET "\n", i + 1, -session_history[i]);
            }
        }
        if (history_count > 10) printf(" ... and %d more spins.\n", history_count - 10);
        printf("----------------------------------------\n");
        if (total_net > 0) printf(" Session Total: " C_GREEN "+$%d Profit!" C_RESET "\n", total_net);
        else printf(" Session Total: " C_RED "-$%d Loss" C_RESET "\n", -total_net);

        free(session_history); // שחרור המערך הדינמי של ההיסטוריה לפני החזרה לתפריט
        prompt_continue(NULL);
    }
}