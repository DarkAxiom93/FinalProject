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

/* Scans data\*.bin, shows a numbered profile list + [N] New Player,
   writes the chosen / typed name into dest (MAX_NAME_LEN). */
static void select_player_profile(char* dest) {
    char names[9][MAX_NAME_LEN];
    int  count = 0;

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA("data\\*.bin", &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            char tmp[MAX_NAME_LEN] = { 0 };
            strncpy(tmp, fd.cFileName, MAX_NAME_LEN - 1);
            char* dot = strrchr(tmp, '.');
            if (dot) *dot = '\0';
            if (is_valid_name(tmp) && count < 9)
                strcpy(names[count++], tmp);
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }

    int want_new = (count == 0);

    if (!want_new) {
        while (1) {
            printf("\n" C_CYAN "==========================================" C_RESET "\n");
            printf("         " C_YELLOW "SELECT PLAYER PROFILE" C_RESET "\n");
            printf(C_CYAN "==========================================" C_RESET "\n\n");
            for (int i = 0; i < count; i++)
                printf("  [%d] " C_GREEN "%s" C_RESET "\n", i + 1, names[i]);
            printf("  [N] " C_CYAN "New Player" C_RESET "\n\nChoice: ");

            char keys[12] = { 0 };
            for (int i = 0; i < count; i++) keys[i] = (char)('1' + i);
            keys[count]     = 'N';
            keys[count + 1] = 'n';

            char ch = get_menu_key(keys);
            if (ch == 'N' || ch == 'n') { want_new = 1; break; }
            strcpy(dest, names[(int)(ch - '1')]);
            return;
        }
    }

    while (1) {
        printf("\nEnter your player name (A-Z, 0-9, and _ only): ");
        if (scanf("%49s", dest) == 1) {
            while (getchar() != '\n');
            if (is_valid_name(dest)) return;
            printf(C_RED "SECURITY ERROR: Invalid characters detected. Do not use slashes or dots." C_RESET "\n");
        }
        else {
            while (getchar() != '\n');
            strcpy(dest, "Guest");
            return;
        }
    }
}

static void display_player_stats(const Player* p) {
    clear_screen();
    print_table_header("MY STATS", C_CYAN, p->balance);

    long long net_worth    = (long long)p->balance + p->bank_balance;
    long long lifetime_net = p->total_winnings - p->total_losses;

    printf(" Wallet Balance    : " C_YELLOW "$%d" C_RESET "\n", p->balance);
    printf(" Safe Bank         : " C_YELLOW "$%d" C_RESET "\n", p->bank_balance);
    printf(" Net Worth         : " C_YELLOW "$%lld" C_RESET "\n", net_worth);
    printf("----------------------------------------\n");
    printf(" Lifetime Winnings : " C_GREEN  "+$%lld" C_RESET "\n", p->total_winnings);
    printf(" Lifetime Losses   : " C_RED    "-$%lld" C_RESET "\n", p->total_losses);

    if (lifetime_net >= 0)
        printf(" Lifetime Net      : " C_GREEN  "+$%lld" C_RESET "\n", lifetime_net);
    else
        printf(" Lifetime Net      : " C_RED    "-$%lld" C_RESET "\n", -lifetime_net);

    printf("----------------------------------------\n");
    if (p->total_losses == 0)
        printf(" Win/Loss Ratio    : " C_CYAN "N/A (no losses recorded)" C_RESET "\n");
    else
        printf(" Win/Loss Ratio    : " C_CYAN "%.2f" C_RESET "\n",
               (double)p->total_winnings / (double)p->total_losses);

    printf("\n");
    prompt_continue(NULL);
}

int main() {
    ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
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

    select_player_profile(current_player.name);

    load_player(&current_player);

    if (current_player.is_banned) {
        clear_screen();
        printf("" C_RED "\n========================================\n");
        printf("           ACCOUNT BANNED          \n");
        printf("========================================" C_RESET "\n");
        printf("This account has been banned by casino management.\n");
        printf("Contact casino support if you believe this is a mistake.\n");
        delay_ms(3000);
        return 0;
    }

    session_start_balance = current_player.balance;
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
                save_player(&current_player);
                break;
            }
            continue;
        }

        print_table_header("CASINO - MAIN MENU", "" C_CYAN "", current_player.balance);
        printf("" C_GREEN "  [ Safe Bank Balance: $%d ]" C_RESET "\n\n", current_player.bank_balance);
        printf("Select an option:\n");
        printf("1. Roulette\n2. Blackjack\n3. Ultimate Texas Hold'em\n4. Slot Machine\n5. Sports Betting (Winner)\n");
        printf("-------------------\n6. Cashier (Deposit Funds)\n7. View Leaderboard (Hall of Fame)\n8. My Stats\n9. Exit Casino\n");
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
            display_player_stats(&current_player);
        }
        else if (choice == 9) {
            int session_net = current_player.balance - session_start_balance;
            clear_screen();
            printf("\n========================================\n          " C_CYAN "CASINO CHECKOUT RECEIPT" C_RESET "          \n========================================\n");
            printf(" Player Name      : %s\n Starting Balance : $%d\n Final Balance    : $%d\n----------------------------------------\n", current_player.name, session_start_balance, current_player.balance);
            if (session_net > 0) printf(" Session Profit   : " C_GREEN "+$%d" C_RESET "\n", session_net);
            else if (session_net < 0) printf(" Session Loss     : " C_RED "-$%d" C_RESET "\n", -session_net);
            else printf(" Session Net      : $0 (Broke Even)\n");
            printf(" Lifetime Wins    : $%lld\n Lifetime Losses  : $%lld\n========================================\n" C_YELLOW " Thank you for playing! See you next time." C_RESET "\n", current_player.total_winnings, current_player.total_losses);

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