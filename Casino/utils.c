#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

int get_safe_int() {
    int value;
    // הלולאה הזו רצה כל עוד המשתמש לא הכניס מספר חוקי בכלל (למשל, הקליד רק אותיות)
    while (scanf("%d", &value) != 1) {
        // מנקים באגרסיביות את כל הזבל מהחוצץ עד שהמשתמש לחץ Enter
        while (getchar() != '\n');
        printf("" C_RED "Invalid input!" C_RESET " Please enter a numeric value: ");
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
    int discard = getchar(); // קליטת התו למשתנה כדי להשתיק את האזהרה של ה-Compiler
    (void)discard;           // ציון מפורש שאנחנו מתעלמים מהערך בכוונה
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
        printf("" C_RED "CRITICAL ERROR: Memory allocation failed!" C_RESET "\n");
        exit(1); // סיום מאולץ של התוכנית במקרה של חוסר זיכרון
    }
    return ptr;
}