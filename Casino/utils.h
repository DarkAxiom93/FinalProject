#ifndef UTILS_H
#define UTILS_H

// קודי צבע ANSI לשימוש בכל רחבי הפרויקט
#define BG_RED     "\x1b[41m"
#define BG_BLACK   "\x1b[40m"
#define BG_GREEN   "\x1b[42m"
#define RESET      "\x1b[0m"
#define TEXT_WHITE "\x1b[97m"

// הצהרות על פונקציות עזר מערכתיות
int get_safe_int();
void delay_ms(int ms);
void print_animated_banner();

void wait_for_enter();
// פונקציות עיצוב ושליטה
void print_table_header(const char* title, const char* color, int balance);
void* safe_malloc(size_t size);
#endif#
