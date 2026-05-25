#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "casino.h"
#include "utils.h" 

// מבנה נתונים ייעודי לניהול טבלת השיאים
typedef struct {
    char name[MAX_NAME_LEN];
    int score;
} Highscore;

/*
 * פונקציה: update_leaderboard
 * תפקיד: מעדכנת את טבלת השיאים בקובץ המרכזי עם יציאת השחקן
 * מדגים: קבצים, מערכים, ואלגוריתם מיון (Bubble Sort)
 */
void update_leaderboard(Player* p) {
    Highscore scores[6]; // מקום ל-5 הטובים ביותר + השחקן הנוכחי
    int count = 0;

    // קריאת השיאים הקיימים מקובץ
    FILE* file = fopen("highscores.txt", "r");
    if (file != NULL) {
        while (count < 5 && fscanf(file, "%s %d", scores[count].name, &scores[count].score) == 2) {
            count++;
        }
        fclose(file);
    }

    // בדיקה האם השחקן כבר קיים בטבלה
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(scores[i].name, p->name) == 0) {
            // אם הוא כבר שם, נעדכן את השיא שלו רק אם הוא הרוויח יותר
            if (p->balance > scores[i].score) {
                scores[i].score = p->balance;
            }
            found = 1;
            break;
        }
    }

    // אם הוא לא קיים בטבלה, נוסיף אותו לסוף המערך
    if (!found) {
        strcpy(scores[count].name, p->name);
        scores[count].score = p->balance;
        count++;
    }

    // אלגוריתם מיון (Bubble Sort) לסדור הרשימה מהגבוה לנמוך
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (scores[j].score < scores[j + 1].score) {
                Highscore temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }

    // כתיבת 5 הגדולים בחזרה לקובץ
    int limit = (count > 5) ? 5 : count;
    file = fopen("highscores.txt", "w");
    if (file != NULL) {
        for (int i = 0; i < limit; i++) {
            fprintf(file, "%s %d\n", scores[i].name, scores[i].score);
        }
        fclose(file);
    }
}

/*
 * פונקציה: display_leaderboard
 * תפקיד: מדפיסה את "היכל התהילה" של הקזינו (UX משופר)
 */
void display_leaderboard() {
    system("cls");
    printf("\x1b[33m"); // צבע צהוב/זהב
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

void load_player(Player* p) {
    char filename[MAX_NAME_LEN + 5];
    sprintf(filename, "%s.txt", p->name);

    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        fscanf(file, "=== CASINO PLAYER PROFILE ===\n");
        fscanf(file, "Player_Name: %s\n", p->name);
        fscanf(file, "Current_Balance: $%d\n", &p->balance);
        fscanf(file, "Total_Winnings: $%d\n", &p->total_winnings);
        fscanf(file, "Total_Losses: $%d\n", &p->total_losses);

        printf("\n\x1b[32mWelcome back, %s! Your profile was loaded.\x1b[0m\n", p->name);
        fclose(file);
    }
    else {
        p->balance = 1000;
        p->total_winnings = 0;
        p->total_losses = 0;
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
        printf("\x1b[32mPlayer data saved successfully to '%s'.\x1b[0m\n", filename);
    }
    else {
        printf("\x1b[31mError: Could not save player data!\x1b[0m\n");
    }
}

int main() {
    srand((unsigned int)time(NULL));
    Player current_player;
    int choice;

    print_animated_banner();

    printf("\nEnter your player name (no spaces): ");

    if (scanf("%49s", current_player.name) != 1) {
        strcpy(current_player.name, "Guest");
    }
    // ניקוי חוצץ המקלדת כדי למנוע קפיצה ישר לתוך התפריט
    while (getchar() != '\n');

    load_player(&current_player);
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
            update_leaderboard(&current_player); // עדכון הטבלה לפני היציאה
            save_player(&current_player);
            break;
        }

        printf("Select an option:\n");
        printf("1. Roulette\n");
        printf("2. Blackjack\n");
        printf("3. Poker (vs Dealer)\n");
        printf("4. Slot Machine\n");
        printf("5. Craps (Dice Game)\n");
        printf("6. Exit Casino\n");
        printf("7. View Leaderboard (Hall of Fame)\n"); // אופציה חדשה
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
            // play_slots(&current_player); 
            break;
        case 5:
            printf("\n--- Entering Craps Table ---\n");
            play_craps(&current_player); 
            break;
        case 6:
            printf("\nThank you for playing, %s! You left with $%d.\n", current_player.name, current_player.balance);
            update_leaderboard(&current_player); // עדכון טבלת השיאים
            save_player(&current_player);        // שמירת משתמש
            return 0;
        case 7:
            display_leaderboard(); // הצגת הטבלה
            break;
        default:
            printf("\n\x1b[33mInvalid choice. Please select a valid option.\x1b[0m\n");
            delay_ms(1500);
        }
    }

    return 0;
}