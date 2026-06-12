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

// מבנה נתונים שמייצג משחק בטופס
typedef struct {
    char home_team[30];
    char away_team[30];
    double odds_1;
    double odds_X;
    double odds_2;
    int user_prediction;  
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
    printf("Enter prediction for Match %d (1, X, 2, or S to skip): ", match_num);
    char choice = get_menu_key("1Xx2Ss");
    if (choice == '1') return 1;
    if (choice == 'X' || choice == 'x') return 2;
    if (choice == '2') return 3;
    return 0; // S or s — דילוג
}

static const char* outcome_to_str(int outcome) {
    if (outcome == 1) return "1 (Home Win)";
    if (outcome == 2) return "X (Draw)";
    return "2 (Away Win)";
}

void play_football(Player* player) {
    print_football_welcome();

    int is_playing = 1;

    while (is_playing) {
        clear_screen();
        // הקצאה דינמית לטופס המשחקים במקום מערך סטטי
        Match* slip = (Match*)calloc(NUM_MATCHES, sizeof(Match));
        // הקצאה דינמית למערך האינדקסים למניעת בחירה כפולה
        int* used_indices = (int*)calloc(POOL_SIZE, sizeof(int));

        // בדיקת בטיחות להקצאת הזיכרון
        if (slip == NULL || used_indices == NULL) {
            display_error(2000, "CRITICAL ERROR: Memory allocation failed for football slip!");
            if (slip) free(slip);
            if (used_indices) free(used_indices);
            return;
        }

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

        int same_slip = 1; // לולאה פנימית: שליטה על משחק עם אותו טופס

        while (same_slip) {
            clear_screen();
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
            }

            if (active_bets_count == 0) {
                printf("\n" C_YELLOW "You skipped all matches on the slip. Ticket cancelled." C_RESET "\n");
                delay_ms(2500);
                same_slip = 0;
                continue;
            }

            printf("\nTotal Slip Odds: " C_YELLOW "x%.2f" C_RESET "\n", total_slip_odds);
            printf("Enter your total bet on this slip: $");
            int bet = get_safe_int();

            if (bet <= 0 || bet > player->balance) {
                display_error(2500, "Invalid amount or insufficient funds! Ticket cancelled.");
                continue;
            }

            if (bet > MAX_BET) {
                display_error(2500, "Sportsbook maximum bet is $%d! Ticket cancelled.", MAX_BET);
                continue;
            }

            player->balance -= bet;
            save_player(player);

            printf("\n" C_CYAN "Matches are live! Simulating scores..." C_RESET "\n");
            for (int i = 0; i < 4; i++) {
                printf("Goal updates coming in... %d'\n", (i + 1) * 20);
                delay_ms(400 + (visual_rand() % 800));
            }

            int hit_all = 1;
            printf("\n" C_YELLOW "=================== SLIP RESULTS ===================" C_RESET "\n");
            for (int i = 0; i < NUM_MATCHES; i++) {
                int r = rand() % 100;
                if (r < 45) slip[i].actual_outcome = 1;
                else if (r < 70) slip[i].actual_outcome = 2;
                else slip[i].actual_outcome = 3;

                int home_goals = 0, away_goals = 0;
                if (slip[i].actual_outcome == 1) {
                    home_goals = (rand() % 4) + 1;
                    away_goals = rand() % home_goals;
                }
                else if (slip[i].actual_outcome == 2) {
                    home_goals = rand() % 4;
                    away_goals = home_goals;
                }
                else {
                    away_goals = (rand() % 4) + 1;
                    home_goals = rand() % away_goals;
                }

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
                        hit_all = 0;
                    }
                }
                printf("----------------------------------------------------\n");
            }

            if (hit_all) {
                int winnings = (int)(bet * total_slip_odds);
                printf("\n" C_GREEN "!!! WOW !!! YOU HIT THE WINNER SLIP !!!" C_RESET "\n");
                printf("You won a total of " C_GREEN "$%d" C_RESET " from odds of x%.2f!\n", winnings, total_slip_odds);
                player->total_winnings += ((long long)winnings - bet);
                add_balance_safe(player, winnings);
            }
            else {
                printf("\n" C_RED "Slip busted! You missed one or more games. Better luck next week!" C_RESET "\n");
                player->total_losses += bet;
            }

            save_player(player);

            printf("\n====================================================\n");
            printf("Options:\n");
            printf("[1] Play this exact slip again (Same Teams & Odds)\n");
            printf("[2] Generate a NEW Winner slip\n");
            printf("[0] Return to Casino Main Menu\n");
            printf("Action: ");

            int post_game_action = get_menu_key("012") - '0';

            if (post_game_action == 0) {
                same_slip = 0;
                is_playing = 0;
            }
            else if (post_game_action == 2) {
                same_slip = 0;
            }

            if (player->balance <= 0 && is_playing) {
                printf("\n" C_RED "You are out of funds in your wallet!" C_RESET "\n");
                delay_ms(2000);
                same_slip = 0;
                is_playing = 0;
            }
        }

        // שחרור הזיכרון הדינמי החיוני למניעת דליפות (Memory Leaks)
        free(slip);
        free(used_indices);
    }
}