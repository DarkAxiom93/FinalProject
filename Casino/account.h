#ifndef ACCOUNT_H
#define ACCOUNT_H
#include "casino.h" // כדי להכיר את המבנה של Player

// הצהרות הפונקציות של מערכת המשתמשים והבנק
void save_player(Player* p);
void load_player(Player* p);
void handle_withdrawal(Player* p);
void handle_deposit(Player* p);

#endif
