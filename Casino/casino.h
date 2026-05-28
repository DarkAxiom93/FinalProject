#ifndef CASINO_H
#define CASINO_H

#define MAX_NAME_LEN 50
#define MAX_BET 10000

// מבנה נתונים 1: ניהול שחקן (עונה על דרישת חובה ל-structs)
typedef struct {
    char name[MAX_NAME_LEN];
    int balance;         // יתרה נוכחית של השחקן
    int bank_balance;
    int total_winnings;  // מעקב אחרי זכיות 
    int total_losses;    // מעקב אחרי הפסדים
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