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

// ==========================================
// VISUAL ENGINE (ISOLATED PRNG)
// ==========================================
void init_visual_rand(unsigned int seed);
int visual_rand();
unsigned int secure_hash(const char* str);// פונקציית גיבוב מאובטחת לאימות סיסמאות
unsigned int hash_password(const char* password);// פונקציית גיבוב מושהית (Key Stretching) נגד מתקפות כוח גס
int is_valid_name(const char* name);// בדיקת תקינות מחרוזות למניעת Path Traversal
void clear_input_buffer();// פונקציה לניקוי אגרסיבי של אגירת קלט (מונעת באגים בזמן אנימציות)

// הצהרות על פונקציות עזר מערכתיות
int get_safe_int();
void delay_ms(int ms);
void print_animated_banner();
void wait_for_enter();

// פונקציות מעטפת לשכבת התצוגה (UI Layer)
void display_error(int delay_time_ms, const char* format, ...);
void prompt_continue(const char* message);
void print_table_header(const char* title, const char* color, int balance);
void* safe_malloc(size_t size);

// ==========================================
// SYSTEM SECURITY & ENCRYPTION ENGINE
// ==========================================
extern unsigned int casino_secret_key;
extern unsigned int casino_salt_1;
extern unsigned int casino_salt_2;

void init_security();
void crypt_buffer(char* data, int length);

// ==========================================
// ADVANCED CASINO PRNG (Xoroshiro128+)
// ==========================================
void init_casino_rand(unsigned long long seed);
unsigned int casino_rand(void);

// טריק אדריכלי: דורס את פונקציית rand הסטנדרטית בכל הפרויקט כדי שתשתמש במנוע שלנו!
#define rand casino_rand

// ==========================================
// AUDIO ENGINE (WAV FILES)
// ==========================================
void play_error_sound();
void play_win_sound();
void play_jackpot_sound();
void play_spin_sound(); 
void stop_sound();

void clear_screen();
#endif