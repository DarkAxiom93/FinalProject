#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Windows.h>
#include <conio.h>
#include "casino.h"
#include "utils.h" 
#include "account.h"
#include "admin.h"
#include "leaderboard.h" 

#ifdef ENABLE_ADMIN_PANEL
#ifndef ADMIN_ENTRY_CODE
#error "CRITICAL: ADMIN_ENTRY_CODE is missing! You must inject it via Visual Studio Preprocessor."
#endif
#endif

typedef void (*GameFunction)(Player*);

void save_player(Player* p); 

int main() {
    SetConsoleTitleA("Ultimate Casino Simulator - VIP Edition");
    hide_cursor();
    CreateDirectory("data", NULL);

    int entropy_variable = 0;
    unsigned int secure_seed = (unsigned int)time(NULL) ^ (unsigned int)clock() ^ (unsigned int)(unsigned long long) & entropy_variable;

    init_casino_rand(secure_seed);
    init_visual_rand(secure_seed ^ 0x55555555);

    init_security();
    Player current_player = { 0 };
    int session_start_balance = 0;

    print_animated_banner();

    while (1) {
        printf("\nEnter your player name (A-Z, 0-9, and _ only): ");
        if (scanf("%49s", current_player.name) == 1) {
            while (getchar() != '\n');

            if (is_valid_name(current_player.name)) {
                break;
            }
            else {
                printf("" C_RED "SECURITY ERROR: Invalid characters detected. Do not use slashes or dots." C_RESET "\n");
            }
        }
        else {
            while (getchar() != '\n');
            strcpy(current_player.name, "Guest");
            break;
        }
    }

    load_player(&current_player);
    session_start_balance = current_player.balance;
    update_leaderboard(&current_player); 
    delay_ms(1500);

    GameFunction casino_games[] = {
        NULL,              // 0
        play_roulette,     // 1
        play_blackjack,    // 2
        play_poker,        // 3 
        play_slots,        // 4 
        play_football      // 5
    };
    int total_games = 5;

    while (1) {
        clear_screen();

        if (current_player.balance <= 0) {
            printf("" C_RED "\n========================================\n");
            printf("          OUT OF FUNDS!          \n");
            printf("========================================" C_RESET "\n");

            if (current_player.bank_balance > 0) {
                printf("" C_YELLOW "Don't panic! You still have $%d safely locked in your Bank." C_RESET "\n", current_player.bank_balance);
                printf("Options: [1] Withdraw from Bank  [2] Open Cashier (Deposit) \nAction: ");

                clear_input_buffer();
                char key;
                while (1) {
                    key = (char)_getch();
                    if (key == '1' || key == '2') break;
                }
                printf("%c\n", key);

                if (key == '1') {
                    handle_withdrawal(&current_player);
                }
                else {
                    handle_deposit(&current_player);
                }
            }
            else {
                printf("You have lost all your money! Redirecting to the Cashier...\n");
                delay_ms(2500);
                handle_deposit(&current_player);
            }

            if (current_player.balance <= 0) {
                printf("\n" C_RED "GAME OVER: You are bankrupt and chose not to fund your wallet!" C_RESET "\n");
                update_leaderboard(&current_player);
                save_player(&current_player);
                break;
            }
            continue;
        }

        print_table_header("CASINO - MAIN MENU", "" C_CYAN "", current_player.balance);
        printf("" C_GREEN "  [ Safe Bank Balance: $%d ]" C_RESET "\n\n", current_player.bank_balance);
        printf("Select an option:\n");
        printf("1. Roulette\n2. Blackjack\n3. Ultimate Texas Hold'em\n4. Slot Machine\n5. Sports Betting (Winner)\n");
        printf("-------------------\n6. Cashier (Deposit Funds)\n7. View Leaderboard (Hall of Fame)\n8. Exit Casino\n");
        printf("Your choice: ");

        int choice = get_safe_int();

        if (choice >= 1 && choice <= total_games) {
            clear_screen();
            casino_games[choice](&current_player);
        }
        else if (choice == 6) {
            handle_deposit(&current_player);
        }
        else if (choice == 7) {
            display_leaderboard();
        }
        else if (choice == 8) {
            int session_net = current_player.balance - session_start_balance;
            clear_screen();
            printf("\n========================================\n          " C_CYAN "CASINO CHECKOUT RECEIPT" C_RESET "          \n========================================\n");
            printf(" Player Name      : %s\n Starting Balance : $%d\n Final Balance    : $%d\n----------------------------------------\n", current_player.name, session_start_balance, current_player.balance);
            if (session_net > 0) printf(" Session Profit   : " C_GREEN "+$%d" C_RESET "\n", session_net);
            else if (session_net < 0) printf(" Session Loss     : " C_RED "-$%d" C_RESET "\n", -session_net);
            else printf(" Session Net      : $0 (Broke Even)\n");
            printf(" Lifetime Wins    : $%lld\n Lifetime Losses  : $%lld\n========================================\n" C_YELLOW " Thank you for playing! See you next time." C_RESET "\n", current_player.total_winnings, current_player.total_losses);

            update_leaderboard(&current_player); 
            save_player(&current_player);
            delay_ms(3000);
            return 0;
        }
#ifdef ENABLE_ADMIN_PANEL
        else if (choice == ADMIN_ENTRY_CODE) {
            admin_panel(&current_player);
        }
#endif
        else {
            display_error(1500, "Invalid choice. Please select a valid option.");
        }
    }
    return 0;
}