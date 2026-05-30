#ifndef CASINO_H
#define CASINO_H

#define MAX_NAME_LEN 50
#define MAX_BET 10000
#define MAX_BALANCE 50000
// אזהרת תחזוקה (Future-Proofing):
// אם סכום זה יוגדל אי פעם מעבר ל-2.14 מיליארד (INT_MAX), 
// חובה לשנות את bank_balance במבנה Player ל-long long ולעדכן את כל ההדפסות!
#define MAX_BANK_BALANCE 500000
#define SAVE_FILE_VERSION 2  // הגרסה הנוכחית של פורמט השמירה

// מבנה נתונים 1: ניהול שחקן 
typedef struct {
    char name[MAX_NAME_LEN];
    int balance;         // יתרה נוכחית של השחקן
    int bank_balance;
    long long total_winnings;  // מעקב אחרי זכיות 
    long long total_losses;    // מעקב אחרי הפסדים
} Player;

// מבנה נתונים 2: ניהול הימור מעודכן 
typedef struct {
    int amount;          // סכום ההימור
    int bet_type;        // סוג ההימור (1=Straight, 2=Split, 3=Street, 4=Color)
    int numbers[4];      // מערך לשמירת עד 3 מספרים (עבור רחוב או פיצול)
    int num_count;       // כמות המספרים השמורים במערך
} Bet;

// הצהרות על פונקציות מערכת מרכזיות
void print_roulette_board();
void play_roulette(Player* p);
void play_blackjack(Player* p); // הוסף את השורה הזו
void play_football(Player* p); // המחליף של קראפס
void play_poker(Player* p);    // לשותף
void play_slots(Player* p);    // לשותף

#endif