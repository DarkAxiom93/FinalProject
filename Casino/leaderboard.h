#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include "casino.h" // כדי להכיר את MAX_NAME_LEN ואת מבנה ה-Player

#define MAX_SCORES 5 // גודל טבלת המובילים (Hall of Fame)

// מבנה השורה בטבלת השיאים
typedef struct {
    char name[MAX_NAME_LEN];
    long long score;
} Highscore;

// הצהרת הפונקציות כדי ש-main יוכל להשתמש בהן
void update_leaderboard(Player* p);
void display_leaderboard();

#endif // LEADERBOARD_H
