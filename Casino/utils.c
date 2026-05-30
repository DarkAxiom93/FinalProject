#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <conio.h>
#include "utils.h"

// פונקציה אגרסיבית לניקוי חוצץ המקלדת 
void clear_input_buffer() {
    // 1. מרוקן את חוצץ המערכת (Console Buffer) מכל המקשים שנלחצו באקראי
    while (_kbhit()) {
        (void)_getch();
    }
}

int get_safe_int() {
    clear_input_buffer();
    int value;
    // הלולאה הזו רצה כל עוד המשתמש לא הכניס מספר חוקי בכלל (למשל, הקליד רק אותיות)
    while (scanf("%d", &value) != 1 || value < 0) {
        // מנקים באגרסיביות את כל הזבל מהחוצץ עד שהמשתמש לחץ Enter
        while (getchar() != '\n');
        printf("" C_RED "Invalid input!" C_RESET " Please enter a positive numeric value: ");
    }

    // אם המשתמש הכניס מספר חוקי אבל הוסיף זבל אחריו (למשל "100abc"),
    // אנחנו לוקחים את ה-100, אבל מנקים את שאר הזבל כדי שלא יהרוס את הסיבוב הבא.
    while (getchar() != '\n');

    return value;
}

void delay_ms(int ms) {
    clock_t start_time = clock();
    while (clock() < start_time + (ms * CLOCKS_PER_SEC / 1000));
}

// פונקציה חדשה לעצירת התוכנית והמתנה לאישור (פותר את הדילוג המהיר ואזהרת C6031)
void wait_for_enter() {
    clear_input_buffer(); // ניקוי הקלדות אקראיות מזמן האנימציות

    // המתנה ללחיצת אנטר אמיתית בלבד! עוקף לחלוטין את stdin
    while (1) {
        int ch = _getch();
        if (ch == '\r' || ch == '\n') break;
    }
}

void print_animated_banner() {
    const char* colors[] = { "" C_RED "", "" C_GREEN "", "" C_YELLOW "", "\x1b[34m", "" C_MAGENTA "", "" C_CYAN "" };
    system("cls");

    for (int i = 0; i < 6; i++) {
        printf("%s\n", colors[i]);
        printf("  $$$$$$\\   $$$$$$\\   $$$$$$\\  $$$$$$\\ $$\\   $$\\  $$$$$$\\  \n");
        printf(" $$  __$$\\ $$  __$$\\ $$  __$$\\ \\_$$  _|$$$\\  $$ |$$  __$$\\ \n");
        printf(" $$ /  \\__|$$ /  $$ |$$ /  \\__|  $$ |  $$$$\\ $$ |$$ /  $$ |\n");
        printf(" $$ |      $$$$$$$$ |\\$$$$$$\\    $$ |  $$ $$\\$$ |$$ |  $$ |\n");
        printf(" $$ |      $$  __$$ | \\____$$\\   $$ |  $$ \\$$$$ |$$ |  $$ |\n");
        printf(" $$ |  $$\\ $$ |  $$ |$$\\   $$ |  $$ |  $$ |\\$$$ |$$ |  $$ |\n");
        printf(" \\$$$$$$  |$$ |  $$ |\\$$$$$$  |$$$$$$\\ $$ | \\$$ | $$$$$$  |\n");
        printf("  \\______/ \\__|  \\__| \\______/ \\______|\\__|  \\__| \\______/ \n\n");
        printf("             W E L C O M E   T O   T H E   G A M E         \n");
        printf("===============================================================\n");

        fflush(stdout);
        delay_ms(350);

        if (i < 5) printf("\x1b[12A");
    }
    printf("" C_RESET "");
}

/*
 * פונקציה: print_table_header
 * תפקיד: מרכזת את כל הדפסות הכותרת בפרויקט למקום אחד (DRY Principle)
 */
void print_table_header(const char* title, const char* color, int balance) {
    printf("\n========================================\n");
    printf("          %s%s" C_RESET "          \n", color, title);
    printf("Current Balance: $%d\n", balance);
    printf("========================================\n");
}

/*
 * פונקציה: safe_malloc
 * תפקיד: עוטפת את הפונקציה malloc בבדיקת שגיאות קריטית.
 * מונעת קריסות פתאומיות ומנקה את הקוד במשחקים עצמם.
 */
void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        // המערכת רושמת שגיאה אבל לא קורסת. האחריות לטפל בזה עוברת לפונקציה הקוראת.
        fprintf(stderr, "" C_RED "CRITICAL ERROR: Memory allocation failed!" C_RESET "\n");
        return NULL;
    }
    return ptr;
}
// משתנה סטטי השומר את המצב הפנימי של מנוע האנימציות בלבד
static unsigned int visual_prng_state = 0;

void init_visual_rand(unsigned int seed) {
    visual_prng_state = seed;
}

int visual_rand() {
    // נוסחת LCG בסיסית המיועדת לאנימציות בלבד ולא להימורים
    visual_prng_state = visual_prng_state * 1103515245 + 12345;
    return (unsigned int)(visual_prng_state / 65536) % 32768;
}
unsigned int secure_hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        // המרת כל תו למספר באמצעות: hash * 33 + c
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int is_valid_name(const char* name) {
    if (name == NULL || name[0] == '\0') return 0;

    for (int i = 0; name[i] != '\0'; i++) {
        // מתיר אך ורק אותיות באנגלית, מספרים, וקו תחתון
        if (!isalnum((unsigned char)name[i]) && name[i] != '_') {
            return 0; // נמצא תו אסור (כמו סלאש או נקודה)
        }
    }
    return 1;
}
// מנוע חכם להצגת שגיאות באדום עם תמיכה במשתנים והשהיה אוטומטית
void display_error(int delay_time_ms, const char* format, ...) {
    printf("" C_RED ""); // הפעלת צבע אדום

    va_list args;
    va_start(args, format);
    vprintf(format, args); // הדפסה דינמית של השגיאה והמשתנים (אם יש)
    va_end(args);

    printf("" C_RESET "\n"); // כיבוי צבע אדום וירידת שורה

    if (delay_time_ms > 0) {
        delay_ms(delay_time_ms);
    }
}

// מנוע אחיד לבקשת הקשת Enter מהמשתמש
void prompt_continue(const char* message) {
    if (message != NULL && message[0] != '\0') {
        printf("\n" C_GREEN "%s" C_RESET "", message);
    }
    else {
        printf("\n" C_GREEN "Press ENTER to continue..." C_RESET "");
    }
    wait_for_enter();
}

// משתני מפתח גלובליים (אך לא מקודדים קשיח - נטענים בזמן ריצה)
unsigned int casino_secret_key = 0;
unsigned int casino_salt_1 = 0;
unsigned int casino_salt_2 = 0;

void init_security() {
    FILE* file = fopen("data/server.key", "r");

    if (file != NULL) {
        // טעינת מפתחות משרת קיים
        if (fscanf(file, "%u\n%u\n%u", &casino_secret_key, &casino_salt_1, &casino_salt_2) != 3) {
            printf("" C_RED "CRITICAL SYSTEM ERROR: Security key corrupted! System halt." C_RESET "\n");
            exit(1);
        }
        fclose(file);
    }
    else {
        // הרצה ראשונה (First Boot): ייצור מפתחות ייחודיים לשרת

        // 1. שדרוג אבטחה: Seed משולב מכמה מקורות ליצירת אנטרופיה גבוהה (Time + Clock + ASLR Address)
        unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)clock() ^ (unsigned int)(size_t)&seed;
        srand(seed);

        // 2. שדרוג ייצור המפתח: שילוב 3 קריאות כדי לכסות 32 ביט מלאים (עוקף את מגבלת 15-הביט של Windows)
        casino_secret_key = ((unsigned int)rand() << 17) ^ ((unsigned int)rand() << 2) ^ rand();

        casino_salt_1 = (rand() % 90000) + 10000;
        casino_salt_2 = (rand() % 90000) + 10000;

        file = fopen("data/server.key", "w");
        if (file != NULL) {
            fprintf(file, "%u\n%u\n%u\n", casino_secret_key, casino_salt_1, casino_salt_2);
            fclose(file);
            // הסתרת קובץ המפתחות במערכת Windows
            system("attrib +h data\\server.key");
        }
        else {
            printf("" C_RED "CRITICAL SYSTEM ERROR: Could not create server key file." C_RESET "\n");
            exit(1);
        }
    }
}

// פונקציית צופן הזרם (עובדת גם כהצפנה וגם כפענוח בעזרת XOR)
void crypt_buffer(char* data, int length) {
    unsigned int current_key = casino_secret_key ^ casino_salt_1;
    for (int i = 0; i < length; i++) {
        data[i] ^= (char)(current_key & 0xFF);
        // סיבוב המפתח והוספת מלח כדי למנוע דפוסים חוזרים
        current_key = (current_key >> 1) | ((current_key & 1) << 31);
        current_key ^= casino_salt_2;
    }
}

// אלגוריתם מושהה נגד כוח גס (Key Stretching)
unsigned int hash_password(const char* password) {
    unsigned int hash = 5381;
    int c;

    // 1. גיבוב בסיסי
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c;
    }

    // 2. מתיחת מפתח - 100,000 איטרציות
    for (int i = 0; i < 100000; i++) {
        hash = ((hash << 5) + hash) ^ i;
    }

    return hash;
}