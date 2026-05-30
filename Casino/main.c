#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "casino.h"
#include "utils.h" 
#include "account.h"
#include "admin.h"

typedef struct {
    char name[MAX_NAME_LEN];
    int score;
} Highscore;

typedef void (*GameFunction)(Player*);

void save_player(Player* p); // Prototype declaration

static void update_leaderboard(Player* p) {
    Highscore scores[6] = { 0 };
    int count = 0;

    // ניתוב לתיקיית הנתונים החדשה
    FILE* file = fopen("data/highscores.txt", "r");
    if (file != NULL) {
        unsigned int file_hash;
        // קריאה מאובטחת הדורשת 3 פרמטרים: שם, ניקוד, חתימה
        while (count < 5 && fscanf(file, "%49s %d %u", scores[count].name, &scores[count].score, &file_hash) == 3) {
            // Anti-Cheat: הוספת השורה למערך רק אם החתימה מאומתת
            if (file_hash == (secure_hash(scores[count].name) ^ scores[count].score)) {
                count++;
            }
        }
        fclose(file);
    }

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(scores[i].name, p->name) == 0) {
            // הדירוג נקבע כעת אך ורק לפי סך הזכיות (Total Winnings)
            if (p->total_winnings > scores[i].score) {
                scores[i].score = p->total_winnings;
            }
            found = 1; break;
        }
    }
    if (!found) {
        strcpy(scores[count].name, p->name);
        scores[count].score = p->total_winnings;
        count++;
    }

    // מיון בסיסי
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
    file = fopen("data/highscores.txt", "w");
    if (file != NULL) {
        for (int i = 0; i < limit; i++) {
            // יצירת חתימה דינמית וכתיבתה לקובץ כדי למנוע עריכה חיצונית
            unsigned int signature = secure_hash(scores[i].name) ^ scores[i].score;
            fprintf(file, "%s %d %u\n", scores[i].name, scores[i].score, signature);
        }
        fclose(file);
    }
}

static void display_leaderboard() {
    system("cls");
    printf("" C_YELLOW "==================================================\n");
    printf("           C A S I N O   H A L L   O F   F A M E  \n");
    printf("==================================================\n" C_RESET "");

    FILE* file = fopen("data/highscores.txt", "r");
    if (file != NULL) {
        char name[MAX_NAME_LEN];
        int score;
        unsigned int file_hash;
        int rank = 1;

        printf("\n  RANK  |  PLAYER NAME          |  TOTAL WINNINGS \n--------------------------------------------------\n");
        while (fscanf(file, "%49s %d %u", name, &score, &file_hash) == 3 && rank <= 5) {
            // מציג רק שורות שלא עברו השחתה
            if (file_hash == (secure_hash(name) ^ score)) {
                printf("  #%d    |  %-20s |  $%d\n", rank, name, score);
                rank++;
            }
        }
        fclose(file);
        if (rank == 1) printf("\n  No records found or file tampered. Play to be the first!\n");
    }
    else {
        printf("\n  No records yet. Play a game and be the first!\n");
    }

    prompt_continue("Press ENTER to return to the main menu...");
}



int main() {

    // ==========================================================
    // SECURE PRNG INITIALIZATION (שלב 3)
    // ==========================================================
    int entropy_variable = 0;
    unsigned int secure_seed = (unsigned int)time(NULL) ^ (unsigned int)clock() ^ (unsigned int)(unsigned long long) & entropy_variable;

    // 1. אתחול ליבת ההימורים של הכסף האמיתי (הקוד המעולה שלך)
    srand(secure_seed);

    // 2. אתחול מנוע האנימציות עם גרעין שעבר מיסוך נוסף כדי לנתק אותו מהכסף
    init_visual_rand(secure_seed ^ 0x55555555);
    // ==========================================================
    init_security();
    Player current_player = { 0 };
    int session_start_balance = 0;

    print_animated_banner();

    // לולאת קלט מאובטחת - לא מאפשרת מעבר עד לקבלת שם תקין
    while (1) {
        printf("\nEnter your player name (A-Z, 0-9, and _ only): ");
        if (scanf("%49s", current_player.name) == 1) {
            while (getchar() != '\n'); // ניקוי חוצץ

            if (is_valid_name(current_player.name)) {
                break; // השם חוקי ואושר
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
            printf("" C_RED "\n========================================\n");
            printf("          OUT OF FUNDS!          \n");
            printf("========================================" C_RESET "\n");
            printf("You have lost all your money! Redirecting to the Cashier...\n");
            delay_ms(2500);

            handle_deposit(&current_player);

            // אם השחקן ביטל את ההפקדה או ניסה להפקיד 0
            if (current_player.balance <= 0) {
                printf("\n" C_RED "GAME OVER: You are bankrupt and chose not to deposit!" C_RESET "\n");
                update_leaderboard(&current_player);
                save_player(&current_player);
                break;
            }
            continue; // אם הפקיד בהצלחה, הלולאה מתחילה מחדש עם התפריט
        }

        print_table_header("CASINO - MAIN MENU", "" C_CYAN "", current_player.balance);
        printf("" C_GREEN "  [ Safe Bank Balance: $%d ]" C_RESET "\n\n", current_player.bank_balance);
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
            printf("\n========================================\n          " C_CYAN "CASINO CHECKOUT RECEIPT" C_RESET "          \n========================================\n");
            printf(" Player Name      : %s\n Starting Balance : $%d\n Final Balance    : $%d\n----------------------------------------\n", current_player.name, session_start_balance, current_player.balance);
            if (session_net > 0) printf(" Session Profit   : " C_GREEN "+$%d" C_RESET "\n", session_net);
            else if (session_net < 0) printf(" Session Loss     : " C_RED "-$%d" C_RESET "\n", -session_net);
            else printf(" Session Net      : $0 (Broke Even)\n");
            printf(" Lifetime Wins    : $%d\n Lifetime Losses  : $%d\n========================================\n" C_YELLOW " Thank you for playing! See you next time." C_RESET "\n", current_player.total_winnings, current_player.total_losses);

            update_leaderboard(&current_player);
            save_player(&current_player);
            delay_ms(4000);
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