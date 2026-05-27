#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "casino.h"
#include "utils.h" 

#define MAX_BALANCE 50000 // הגדרת מגבלת ההפקדה בקזינו

typedef struct {
    char name[MAX_NAME_LEN];
    int score;
} Highscore;

typedef void (*GameFunction)(Player*);

void update_leaderboard(Player* p) {
    Highscore scores[6] = { 0 };
    int count = 0;
    FILE* file = fopen("highscores.txt", "r");
    if (file != NULL) {
        while (count < 5 && fscanf(file, "%s %d", scores[count].name, &scores[count].score) == 2) {
            count++;
        }
        fclose(file);
    }
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(scores[i].name, p->name) == 0) {
            if (p->balance > scores[i].score) scores[i].score = p->balance;
            found = 1; break;
        }
    }
    if (!found) {
        strcpy(scores[count].name, p->name);
        scores[count].score = p->balance;
        count++;
    }
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (scores[j].score < scores[j + 1].score) {
                Highscore temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }
    int limit = (count > 5) ? 5 : count;
    file = fopen("highscores.txt", "w");
    if (file != NULL) {
        for (int i = 0; i < limit; i++) fprintf(file, "%s %d\n", scores[i].name, scores[i].score);
        fclose(file);
    }
}

void display_leaderboard() {
    system("cls");
    printf("\x1b[33m==================================================\n");
    printf("           C A S I N O   H A L L   O F   F A M E  \n");
    printf("==================================================\n\x1b[0m");
    FILE* file = fopen("highscores.txt", "r");
    if (file != NULL) {
        char name[MAX_NAME_LEN]; int score; int rank = 1;
        printf("\n  RANK  |  PLAYER NAME          |  BALANCE \n--------------------------------------------------\n");
        while (fscanf(file, "%s %d", name, &score) == 2 && rank <= 5) {
            printf("  #%d    |  %-20s |  $%d\n", rank, name, score);
            rank++;
        }
        fclose(file);
        if (rank == 1) printf("\n  No records yet. Play a game and be the first!\n");
    }
    else {
        printf("\n  No records yet. Play a game and be the first!\n");
    }
    printf("\n\x1b[32mPress ENTER to return to the main menu...\x1b[0m");
    wait_for_enter();
}

void load_player(Player* p) {
    p->balance = 1000; p->total_winnings = 0; p->total_losses = 0;
    char filename[MAX_NAME_LEN + 5]; sprintf(filename, "%s.txt", p->name);
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), file)) {
            if (strstr(buffer, "Player_Name:")) { if (sscanf(buffer, "Player_Name: %s", p->name) == 1) {} }
            else if (strstr(buffer, "Current_Balance:")) { if (sscanf(buffer, "Current_Balance: $%d", &p->balance) == 1) {} }
            else if (strstr(buffer, "Total_Winnings:")) { if (sscanf(buffer, "Total_Winnings: $%d", &p->total_winnings) == 1) {} }
            else if (strstr(buffer, "Total_Losses:")) { if (sscanf(buffer, "Total_Losses: $%d", &p->total_losses) == 1) {} }
        }
        printf("\n\x1b[32mWelcome back, %s! Your profile was loaded.\x1b[0m\n", p->name);
        fclose(file);
    }
    else {
        printf("\n\x1b[36mNew account created for %s. Starting balance: $1000.\x1b[0m\n", p->name);
    }
}

void save_player(Player* p) {
    char filename[MAX_NAME_LEN + 5]; sprintf(filename, "%s.txt", p->name);
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "=== CASINO PLAYER PROFILE ===\nPlayer_Name: %s\nCurrent_Balance: $%d\n-----------------------------\nTotal_Winnings: $%d\nTotal_Losses: $%d\n=============================\n", p->name, p->balance, p->total_winnings, p->total_losses);
        fclose(file);
    }
    else { printf("\x1b[31mError: Could not save player data!\x1b[0m\n"); }
}

// פונקציית הקופאי החדשה
void handle_deposit(Player* p) {
    system("cls");
    print_table_header("CASHIER (DEPOSIT)", "\x1b[32m", p->balance);
    printf("Maximum allowed balance: $%d\n", MAX_BALANCE);

    if (p->balance >= MAX_BALANCE) {
        printf("\n\x1b[33mYour account is already at or above the maximum limit ($%d).\x1b[0m\n", MAX_BALANCE);
        printf("No further deposits allowed at this time.\n");
        delay_ms(2500);
        return;
    }

    printf("\nEnter amount to deposit (0 to cancel): $");
    int amount = get_safe_int();

    if (amount <= 0) {
        printf("Deposit cancelled.\n");
        delay_ms(1500);
        return;
    }

    if (p->balance + amount > MAX_BALANCE) {
        printf("\x1b[31mDeposit rejected! This would exceed the $%d limit.\x1b[0m\n", MAX_BALANCE);
        printf("You can deposit up to $%d.\n", MAX_BALANCE - p->balance);
        delay_ms(2500);
    }
    else {
        p->balance += amount;
        printf("\x1b[32mSuccessfully deposited $%d. New balance: $%d.\x1b[0m\n", amount, p->balance);
        save_player(p); // שומר את המצב אחרי הפקדה
        delay_ms(2000);
    }
}

int main() {
    srand((unsigned int)time(NULL));
    Player current_player = { 0 };
    int session_start_balance = 0;

    print_animated_banner();

    printf("\nEnter your player name (no spaces): ");
    if (scanf("%49s", current_player.name) != 1) strcpy(current_player.name, "Guest");
    while (getchar() != '\n');

    load_player(&current_player);
    session_start_balance = current_player.balance;
    delay_ms(1500);

    // מערך הפונקציות העדכני כולל כל המשחקים שבנינו
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
        system("cls");

        // מנגנון פשיטת הרגל החדש - שולח לקופאי במקום לזרוק החוצה
        if (current_player.balance <= 0) {
            printf("\x1b[31m\n========================================\n");
            printf("          OUT OF FUNDS!          \n");
            printf("========================================\x1b[0m\n");
            printf("You have lost all your money! Redirecting to the Cashier...\n");
            delay_ms(2500);

            handle_deposit(&current_player);

            // אם השחקן ביטל את ההפקדה או ניסה להפקיד 0
            if (current_player.balance <= 0) {
                printf("\n\x1b[31mGAME OVER: You are bankrupt and chose not to deposit!\x1b[0m\n");
                update_leaderboard(&current_player);
                save_player(&current_player);
                break;
            }
            continue; // אם הפקיד בהצלחה, הלולאה מתחילה מחדש עם התפריט
        }

        print_table_header("CASINO - MAIN MENU", "\x1b[36m", current_player.balance);
        printf("Select an option:\n");
        printf("1. Roulette\n2. Blackjack\n3. Ultimate Texas Hold'em\n4. Slot Machine\n5. Sports Betting (Winner)\n");
        printf("-------------------\n6. Cashier (Deposit Funds)\n7. View Leaderboard (Hall of Fame)\n8. Exit Casino\n");
        printf("Your choice: ");

        int choice = get_safe_int();

        if (choice >= 1 && choice <= total_games) {
            system("cls");
            casino_games[choice](&current_player);
        }
        else if (choice == 6) {
            handle_deposit(&current_player); // הפעלה יזומה של הקופאי
        }
        else if (choice == 7) {
            display_leaderboard();
        }
        else if (choice == 8) {
            int session_net = current_player.balance - session_start_balance;
            system("cls");
            printf("\n========================================\n          \x1b[36mCASINO CHECKOUT RECEIPT\x1b[0m          \n========================================\n");
            printf(" Player Name      : %s\n Starting Balance : $%d\n Final Balance    : $%d\n----------------------------------------\n", current_player.name, session_start_balance, current_player.balance);
            if (session_net > 0) printf(" Session Profit   : \x1b[32m+$%d\x1b[0m\n", session_net);
            else if (session_net < 0) printf(" Session Loss     : \x1b[31m-$%d\x1b[0m\n", -session_net);
            else printf(" Session Net      : $0 (Broke Even)\n");
            printf(" Lifetime Wins    : $%d\n Lifetime Losses  : $%d\n========================================\n\x1b[33m Thank you for playing! See you next time.\x1b[0m\n", current_player.total_winnings, current_player.total_losses);

            update_leaderboard(&current_player);
            save_player(&current_player);
            delay_ms(4000);
            return 0;
        }
        else {
            printf("\n\x1b[31mInvalid choice. Please select a valid option.\x1b[0m\n");
            delay_ms(1500);
        }
    }
    return 0;
}