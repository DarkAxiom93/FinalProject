#ifndef ACCOUNT_H
#define ACCOUNT_H
#include "casino.h" // כדי להכיר את המבנה של Player

// הצהרות הפונקציות של מערכת המשתמשים והבנק
void save_player(Player* p);
void load_player(Player* p);
void handle_withdrawal(Player* p);
void handle_deposit(Player* p);
void add_balance_safe(Player* p, int amount);

// טוען את data/<name>.bin לתוך *out (מאופס ומכיל את name תחילה).
// מחזיר: 1 = נטען ונפענח בהצלחה, -1 = הקובץ קיים אך פגום/לא קריא, 0 = הקובץ לא קיים.
// *out_checksum_ok מציין אם החתימה הדיגיטלית תואמת (False = נחשד בזיוף/שיבוש).
// *out_file_version (אופציונלי, אפשר NULL) מקבל את גרסת פורמט השמירה שבה הקובץ נכתב.
int read_player_file(const char* name, Player* out, int* out_checksum_ok, int* out_file_version);

#endif
