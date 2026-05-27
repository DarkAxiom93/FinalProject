#ifndef UTILS_H
#define UTILS_H

// ==========================================
// COLOR MACROS FOR TERMINAL UI
// ==========================================
#define C_RESET    "\x1b[0m"
#define C_RED      "\x1b[31m"
#define C_GREEN    "\x1b[32m"
#define C_YELLOW   "\x1b[33m"
#define C_BLUE     "\x1b[34m"
#define C_MAGENTA  "\x1b[35m"
#define C_CYAN     "\x1b[36m"
#define C_WHITE    "\x1b[97m"

#include <stdlib.h> // פותר את שגיאת size_t

// קודי צבע ANSI לשימוש בכל רחבי הפרויקט
#define BG_RED     "\x1b[41m"
#define BG_BLACK   "\x1b[40m"
#define BG_GREEN   "\x1b[42m"
#define RESET      "" C_RESET ""
#define TEXT_WHITE "\x1b[97m"

// הצהרות על פונקציות עזר מערכתיות
int get_safe_int();
void delay_ms(int ms);
void print_animated_banner();
void wait_for_enter();
void print_table_header(const char* title, const char* color, int balance);
void* safe_malloc(size_t size);

#endif