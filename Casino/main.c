#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "casino.h"
#include "utils.h" 

typedef struct {
    char name[MAX_NAME_LEN];
    int score;
} Highscore;

void update_leaderboard(Player* p) {
    // תיקון אזהרה C6001: איפוס מלא של מערך השיאים כדי למנוע ערכי זבל
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
            if (p->balance > scores[i].score) {
                scores[i].score = p->balance;
            }
            found = 1;
            break;
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
        for (int i = 0; i < limit; i++) {
            fprintf(file, "%s %d\n", scores[i].name, scores[i].score);
        }
        fclose(file);
    }
}

void display_leaderboard() {
    system("cls");
    printf("\x1b[33m");
    printf("==================================================\n");
    printf("           C A S I N O   H A L L   O F   F A M E  \n");
    printf("==================================================\n\x1b[0m");

    FILE* file = fopen("highscores.txt", "r");
    if (file != NULL) {
        char name[MAX_NAME_LEN];
        int score;
        int rank = 1;

        printf("\n  RANK  |  PLAYER NAME          |  BALANCE \n");
        printf("--------------------------------------------------\n");
        while (fscanf(file, "%s %d", name, &score) == 2 && rank <= 5) {
            printf("  #%d    |  %-20s |  $%d\n", rank, name, score);
            rank++;
        }
        fclose(file);

        if (rank == 1) {
            printf("\n  No records yet. Play a game and be the first!\n");
        }
    }
    else {
        printf("\n  No records yet. Play a game and be the first!\n");
    }

    printf("\n\x1b[32mPress ENTER to return to the main menu...\x1b[0m");
    wait_for_enter();
}

/*
 * פונקציה: load_player
 * שודרגה משמעותית כדי למנוע קריסות קריאה (Garbage Values)
 * משתמשת בקריאת שורות בטוחה וחיפוש מילות מפתח במקום ניחוש מבנה קשיח.
 */
void load_player(Player* p) {
    // שלב 1: הצבת ערכי ברירת מחדל כדי למנוע הופעת מספרים שליליים עצומים במידה והקריאה נכשלת
    p->balance = 1000;
    p->total_winnings = 0;
    p->total_losses = 0;

    char filename[MAX_NAME_LEN + 5];
    sprintf(filename, "%s.txt", p->name);

    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        char buffer[256];
        // קורא את הקובץ שורה אחר שורה כדי להתגבר על בעיות של ווינדוס (CRLF)
        while (fgets(buffer, sizeof(buffer), file)) {
            // חיפוש מילות המפתח בתוך השורה וחילוץ המספרים (פותר את אזהרות ה-C6031)
            if (strstr(buffer, "Player_Name:")) {
                if (sscanf(buffer, "Player_Name: %s", p->name) == 1) {}
            }
            else if (strstr(buffer, "Current_Balance:")) {
                if (sscanf(buffer, "Current_Balance: $%d", &p->balance) == 1) {}
            }
            else if (strstr(buffer, "Total_Winnings:")) {
                if (sscanf(buffer, "Total_Winnings: $%d", &p->total_winnings) == 1) {}
            }
            else if (strstr(buffer, "Total_Losses:")) {
                if (sscanf(buffer, "Total_Losses: $%d", &p->total_losses) == 1) {}
            }
        }
        printf("\n\x1b[32mWelcome back, %s! Your profile was loaded.\x1b[0m\n", p->name);
        fclose(file);
    }
    else {
        printf("\n\x1b[36mNew account created for %s. Starting balance: $1000.\x1b[0m\n", p->name);
    }
}

void save_player(Player* p) {
    char filename[MAX_NAME_LEN + 5];
    sprintf(filename, "%s.txt", p->name);

    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fprintf(file, "=== CASINO PLAYER PROFILE ===\n");
        fprintf(file, "Player_Name: %s\n", p->name);
        fprintf(file, "Current_Balance: $%d\n", p->balance);
        fprintf(file, "-----------------------------\n");
        fprintf(file, "Total_Winnings: $%d\n", p->total_winnings);
        fprintf(file, "Total_Losses: $%d\n", p->total_losses);
        fprintf(file, "=============================\n");
        fclose(file);
    }
    else {
        printf("\x1b[31mError: Could not save player data!\x1b[0m\n");
    }
}

int main() {
    srand((unsigned int)time(NULL));

    // תיקון חובה: איפוס המשתנה באפסים כדי ששום "ערך זבל" לא יישאר בזיכרון הראשי
    Player current_player = { 0 };

    int choice;
    int session_start_balance = 0;

    print_animated_banner();

    printf("\nEnter your player name (no spaces): ");

    if (scanf("%49s", current_player.name) != 1) {
        strcpy(current_player.name, "Guest");
    }
    while (getchar() != '\n');

    load_player(&current_player);
    session_start_balance = current_player.balance;
    delay_ms(1500);

    while (1) {
        system("cls");

        printf("========================================\n");
        printf("          \x1b[36mCASINO - MAIN MENU\x1b[0m          \n");
        printf("========================================\n");
        printf("Player: %s | Current Balance: $%d\n", current_player.name, current_player.balance);
        printf("----------------------------------------\n");

        if (current_player.balance <= 0) {
            printf("\x1b[31mGAME OVER: You are bankrupt!\x1b[0m\n");
            update_leaderboard(&current_player);
            save_player(&current_player);
            break;
        }

        printf("Select an option:\n");
        printf("1. Roulette\n");
        printf("2. Blackjack\n");
        printf("3. Poker (vs Dealer)\n");
        printf("4. Slot Machine\n");
        printf("5. Football\n");
        printf("6. Exit Casino\n");
        printf("7. View Leaderboard (Hall of Fame)\n");
        printf("Your choice: ");

        choice = get_safe_int();

        switch (choice) {
        case 1:
            system("cls");
            play_roulette(&current_player);
            break;
        case 2:
            system("cls");
            play_blackjack(&current_player);
            break;
        case 3:
            printf("\n--- Entering Poker Table ---\n");
            // play_poker(&current_player); 
            break;
        case 4:
            printf("\n--- Entering Slot Machine Area ---\n");
            play_slots(&current_player); 
            break;
        case 5:
            system("cls");
            play_football(&current_player);
            break;
        case 6:
        {
            int session_net = current_player.balance - session_start_balance;
            system("cls");
            printf("\n========================================\n");
            printf("          \x1b[36mCASINO CHECKOUT RECEIPT\x1b[0m          \n");
            printf("========================================\n");
            printf(" Player Name      : %s\n", current_player.name);
            printf(" Starting Balance : $%d\n", session_start_balance);
            printf(" Final Balance    : $%d\n", current_player.balance);
            printf("----------------------------------------\n");

            if (session_net > 0) {
                printf(" Session Profit   : \x1b[32m+$%d\x1b[0m\n", session_net);
            }
            else if (session_net < 0) {
                printf(" Session Loss     : \x1b[31m-$%d\x1b[0m\n", -session_net);
            }
            else {
                printf(" Session Net      : $0 (Broke Even)\n");
            }

            printf(" Lifetime Wins    : $%d\n", current_player.total_winnings);
            printf(" Lifetime Losses  : $%d\n", current_player.total_losses);
            printf("========================================\n");
            printf("\x1b[33m Thank you for playing! See you next time.\x1b[0m\n");

            update_leaderboard(&current_player);
            save_player(&current_player);

            delay_ms(4000);
            return 0;
        }
        case 7:
            display_leaderboard();
            break;
        default:
            printf("\n\x1b[33mInvalid choice. Please select a valid option.\x1b[0m\n");
            delay_ms(1500);
        }
    }

    return 0;
}