#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "casino.h"
#include "utils.h"
#include "graphics.h"
#include "account.h"

#define NUM_MATCHES 3

// מבנה נתונים משודרג שמייצג משחק בטופס
typedef struct {
    char home_team[30];
    char away_team[30];
    double odds_1;
    double odds_X;
    double odds_2;
    int user_prediction;  // 1 = בית, 2 = תיקו, 3 = חוץ, 0 = דילוג (Skipped)
    int actual_outcome;
} Match;

// מאגר קבוצות שממנו המערכת תגריל משחקים שונים בכל סבב
const char* team_pool[] = {
    "Real Madrid", "FC Barcelona", "Manchester City", "Liverpool",
    "Bayern Munich", "Dortmund", "Paris SG", "Juventus",
    "Arsenal", "Chelsea", "AC Milan", "Atletico Madrid"
};
#define POOL_SIZE 12

static int get_match_prediction(int match_num) {
    char choice;
    while (1) {
        clear_input_buffer();
        printf("Enter prediction for Match %d (1, X, 2, or S to skip): ", match_num);
        if (scanf(" %c", &choice) == 1) {
            while (getchar() != '\n');
            if (choice == '1') return 1;
            if (choice == 'X' || choice == 'x') return 2;
            if (choice == '2') return 3;
            if (choice == 'S' || choice == 's') return 0; // 0 מייצג דילוג
        }
        else {
            while (getchar() != '\n');
        }
        printf("" C_RED "Invalid choice!" C_RESET " Please enter '1', 'X', '2', or 'S'.\n");
    }
}

static const char* outcome_to_str(int outcome) {
    if (outcome == 1) return "1 (Home Win)";
    if (outcome == 2) return "X (Draw)";
    return "2 (Away Win)";
}

void play_football(Player* player) {
    print_football_welcome();

    Match slip[NUM_MATCHES] = { 0 };

    // מערך עזר למניעת בחירה כפולה של אותה קבוצה באותו טופס
    int used_indices[POOL_SIZE] = { 0 };

    // הגרלת משחקים וקבוצות דינמיות לחלוטין לסיבוב הנוכחי
    for (int i = 0; i < NUM_MATCHES; i++) {
        int home_idx, away_idx;

        // הגרלת קבוצת בית ייחודית
        do {
            home_idx = rand() % POOL_SIZE;
        } while (used_indices[home_idx]);
        used_indices[home_idx] = 1;

        // הגרלת קבוצת חוץ ייחודית
        do {
            away_idx = rand() % POOL_SIZE;
        } while (used_indices[away_idx]);
        used_indices[away_idx] = 1;

        strcpy(slip[i].home_team, team_pool[home_idx]);
        strcpy(slip[i].away_team, team_pool[away_idx]);

        // הגרלת יחסי זכייה אקראיים והגיוניים
        slip[i].odds_1 = 1.6f + ((double)rand() / 4294967295.0) * 1.2;
        slip[i].odds_X = 2.8f + ((double)rand() / 4294967295.0) * 1.0;
        slip[i].odds_2 = 1.9f + ((double)rand() / 4294967295.0) * 1.5;
    }

    print_table_header("WINNER SPORTSBOOK", "" C_GREEN "", player->balance);

    // הצגת טבלת המשחקים שהוגרלו
    printf("\n==================== TODAY'S WINNER SLIP ====================\n");
    printf(" # | MATCH                             |  (1)  |  (X)  |  (2)  \n");
    printf("-------------------------------------------------------------\n");
    for (int i = 0; i < NUM_MATCHES; i++) {
        char match_line[100];
        snprintf(match_line, sizeof(match_line), "%s vs %s", slip[i].home_team, slip[i].away_team);
        printf(" %d | %-33s | %.2f  | %.2f  | %.2f \n", i + 1, match_line, slip[i].odds_1, slip[i].odds_X, slip[i].odds_2);
    }
    printf("=============================================================\n\n");

    // קליטת הניחושים של המשתמש לטופס
    double total_slip_odds = 1.0;
    int active_bets_count = 0;

    for (int i = 0; i < NUM_MATCHES; i++) {
        slip[i].user_prediction = get_match_prediction(i + 1);

        if (slip[i].user_prediction == 1) {
            total_slip_odds *= slip[i].odds_1;
            active_bets_count++;
        }
        else if (slip[i].user_prediction == 2) {
            total_slip_odds *= slip[i].odds_X;
            active_bets_count++;
        }
        else if (slip[i].user_prediction == 3) {
            total_slip_odds *= slip[i].odds_2;
            active_bets_count++;
        }
        // אם המשתמש בחר ב-0 (S) - אנחנו לא מכפילים את היחס ולא מעלים את מונה ההימורים הפעילים
    }

    // הגנה מפני מצב שבו המשתמש דילג על כל שלושת המשחקים
    if (active_bets_count == 0) {
        printf("\n" C_YELLOW "You skipped all matches on the slip. Returning to Main Menu..." C_RESET "\n");
        delay_ms(2500);
        return; // יציאה חזרה לתפריט הראשי
    }

    printf("\nTotal Slip Odds: " C_YELLOW "x%.2f" C_RESET "\n", total_slip_odds);
    printf("Enter your total bet on this slip: $");
    int bet = get_safe_int();

    if (bet <= 0 || bet > player->balance) {
        display_error(2500, "Invalid amount or insufficient funds! Ticket cancelled.");
        return;
    }

    if (bet > MAX_BET) {
        display_error(2500, "Sportsbook maximum bet is $%d! Ticket cancelled.", MAX_BET);
        return;
    }

    player->balance -= bet;
    save_player(player);
    printf("\n" C_CYAN "Matches are live! Simulating scores..." C_RESET "\n");
    for (int i = 0; i < 4; i++) {
        printf("Goal updates coming in... %d'\n", (i + 1) * 20);
        delay_ms(700);
    }

    // הגרלת תוצאות המשחקים ובדיקת הטופס
    int hit_all = 1;
    printf("\n" C_YELLOW "=================== SLIP RESULTS ===================" C_RESET "\n");
    for (int i = 0; i < NUM_MATCHES; i++) {
        // הגרלת תוצאת אמת: 1=בית (45%), 2=תיקו (25%), 3=חוץ (30%)
        int r = rand() % 100;
        if (r < 45) slip[i].actual_outcome = 1;
        else if (r < 70) slip[i].actual_outcome = 2;
        else slip[i].actual_outcome = 3;

        // ייצור תוצאת סיום (שערים) ריאליסטית שתואמת להכרעה
        int home_goals = 0, away_goals = 0;
        if (slip[i].actual_outcome == 1) { // ניצחון בית
            home_goals = (rand() % 4) + 1; // 1 עד 4 שערים
            away_goals = rand() % home_goals; // תמיד פחות משערי הבית
        }
        else if (slip[i].actual_outcome == 2) { // תיקו
            home_goals = rand() % 4; // 0 עד 3 שערים
            away_goals = home_goals; // זהה
        }
        else { // ניצחון חוץ
            away_goals = (rand() % 4) + 1; // 1 עד 4 שערים
            home_goals = rand() % away_goals; // תמיד פחות משערי החוץ
        }

        // תצוגת UI משודרגת לכל משחק
        printf("\n" C_CYAN "[ MATCH %d ]" C_RESET " %s vs %s\n", i + 1, slip[i].home_team, slip[i].away_team);
        printf("FINAL SCORE: " C_WHITE "%d - %d" C_RESET " (%s)\n", home_goals, away_goals, outcome_to_str(slip[i].actual_outcome));

        if (slip[i].user_prediction == 0) {
            printf("Your Pick  : SKIPPED\n");
            printf("Status     : " C_CYAN "[ - ] NEUTRAL" C_RESET "\n");
        }
        else {
            printf("Your Pick  : %s\n", outcome_to_str(slip[i].user_prediction));
            if (slip[i].user_prediction == slip[i].actual_outcome) {
                printf("Status     : " C_GREEN "[ V ] HIT!" C_RESET "\n");
            }
            else {
                printf("Status     : " C_RED "[ X ] MISS" C_RESET "\n");
                hit_all = 0; // פגיעה שנכשלה פוסלת את הטופס
            }
        }
        printf("----------------------------------------------------\n");
    }

    // חישוב זכיות/הפסדים
    if (hit_all) {
        int winnings = (int)(bet * total_slip_odds);
        printf("\n" C_GREEN "!!! WOW !!! YOU HIT THE WINNER SLIP !!!" C_RESET "\n");
        printf("You won a total of " C_GREEN "$%d" C_RESET " from odds of x%.2f!\n", winnings, total_slip_odds);
        player->balance += winnings;
        player->total_winnings += ((long long)winnings - bet);
    }
    else {
        printf("\n" C_RED "Slip busted! You missed one or more games. Better luck next week!" C_RESET "\n");
        player->total_losses += bet;
    }

    save_player(player);

    // בסיום הצגת התוצאות, לחיצה על Enter תסיים את הפונקציה ותחזיר את השחקנים אוטומטית ל-main menu
    prompt_continue("Press ENTER to return to the Casino Main Menu...");
}