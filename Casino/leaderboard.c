#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "casino.h"
#include "utils.h"
#include "account.h"
#include "leaderboard.h"

// טבלת המובילים נקראת בכל פעם ישירות מקבצי השחקנים (data/*.bin) - אין קובץ ביניים
// שיכול להתיישן, כך שהדירוג תמיד משקף את total_winnings האמיתי והעדכני של כל שחקן.
void display_leaderboard() {
    clear_screen();
    printf("" C_YELLOW "==================================================\n");
    printf("           C A S I N O   H A L L   O F   F A M E  \n");
    printf("==================================================\n" C_RESET "");

    Highscore top[MAX_SCORES] = { 0 };
    int top_count = 0;

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA("data\\*.bin", &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            char name[MAX_NAME_LEN] = { 0 };
            strncpy(name, fd.cFileName, MAX_NAME_LEN - 1);
            char* dot = strrchr(name, '.');
            if (dot) *dot = '\0';
            if (!is_valid_name(name)) continue;

            Player pl = { 0 };
            if (read_player_file(name, &pl, NULL, NULL) != 1) continue;

            // מאתרים את מקום ההכנסה הנכון בטבלת ה-Top, ושומרים אותה ממוינת מהגבוה לנמוך
            int insert_at = top_count;
            for (int i = 0; i < top_count; i++) {
                if (pl.total_winnings > top[i].score) { insert_at = i; break; }
            }
            if (insert_at >= MAX_SCORES) continue; // לא מספיק גבוה כדי להיכנס לטבלה

            int last = (top_count < MAX_SCORES) ? top_count : MAX_SCORES - 1;
            for (int j = last; j > insert_at; j--) top[j] = top[j - 1];

            strcpy(top[insert_at].name, pl.name);
            top[insert_at].score = pl.total_winnings;
            if (top_count < MAX_SCORES) top_count++;
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }

    if (top_count == 0) {
        printf("\n  No records yet. Play a game and be the first!\n");
    }
    else {
        printf("\n  RANK  |  PLAYER NAME          |  TOTAL WINNINGS \n--------------------------------------------------\n");
        for (int i = 0; i < top_count; i++) {
            printf("  #%d    |  %-20s |  $%lld\n", i + 1, top[i].name, top[i].score);
        }
    }

    prompt_continue("Press ENTER to return to the main menu...");
}
