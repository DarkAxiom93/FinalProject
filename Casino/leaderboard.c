#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "casino.h"
#include "utils.h"
#include "leaderboard.h"

// מאקרו לחישוב חתימת אבטחה לטבלת המובילים (מועבר לכאן מ-main.c)
#define HIGHSCORE_SIG(name, score) \
    (secure_hash(name) ^ (unsigned int)((score) & 0xFFFFFFFF) ^ (unsigned int)((score) >> 32))

void update_leaderboard(Player* p) {
    Highscore scores[MAX_SCORES + 1] = { 0 };
    int count = 0;

    FILE* file = fopen("data/highscores.txt", "r");
    if (file != NULL) {
        unsigned int file_hash;
        while (count < MAX_SCORES && fscanf(file, "%49s %lld %u", scores[count].name, &scores[count].score, &file_hash) == 3) {
            // Anti-Cheat: הוספת השורה למערך רק אם החתימה מאומתת
            if (file_hash == HIGHSCORE_SIG(scores[count].name, scores[count].score)) {
                count++;
            }
        }
        fclose(file);
    }

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(scores[i].name, p->name) == 0) {
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

    // מיון הטבלה מהגבוה לנמוך
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (scores[j].score < scores[j + 1].score) {
                Highscore temp = scores[j];
                scores[j] = scores[j + 1];
                scores[j + 1] = temp;
            }
        }
    }

    int limit = (count > MAX_SCORES) ? MAX_SCORES : count;

    file = fopen("data/highscores.txt", "w");
    if (file != NULL) {
        for (int i = 0; i < limit; i++) {
            unsigned int signature = HIGHSCORE_SIG(scores[i].name, scores[i].score);
            fprintf(file, "%s %lld %u\n", scores[i].name, scores[i].score, signature);
        }
        fclose(file);
    }
}

void display_leaderboard() {
    clear_screen();
    printf("" C_YELLOW "==================================================\n");
    printf("           C A S I N O   H A L L   O F   F A M E  \n");
    printf("==================================================\n" C_RESET "");

    FILE* file = fopen("data/highscores.txt", "r");
    if (file != NULL) {
        char name[MAX_NAME_LEN];
        long long score;
        unsigned int file_hash;
        int rank = 1;

        printf("\n  RANK  |  PLAYER NAME          |  TOTAL WINNINGS \n--------------------------------------------------\n");
        while (fscanf(file, "%49s %lld %u", name, &score, &file_hash) == 3 && rank <= MAX_SCORES) {
            // מציג רק שורות שלא עברו השחתה
            if (file_hash == HIGHSCORE_SIG(name, score)) {
                printf("  #%d    |  %-20s |  $%lld\n", rank, name, score);
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