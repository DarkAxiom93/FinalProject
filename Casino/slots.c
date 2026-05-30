#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "casino.h"
#include "utils.h"
#include <conio.h> // מאפשרת שימוש ב-_getch() לקריאת מקשים מיידית
#include "graphics.h"
#include "account.h"

// מערך של סמלים למכונת המזל
const char* slot_symbols[] = {
    "" C_RED "[ 7 ]" C_RESET "", // 0: שבע אדום (ג'קפוט)
    "" C_YELLOW "[ $ ]" C_RESET "", // 1: דולר זהב
    "" C_GREEN "[ # ]" C_RESET "", // 2: סולמית ירוקה
    "" C_MAGENTA "[ @ ]" C_RESET "", // 3: שטרודל סגול
    "" C_CYAN "[ * ]" C_RESET ""  // 4: כוכב תכלת
};

// פונקציה שמציירת את המכונה עם האנימציה
static void draw_slot_machine(int s1, int s2, int s3) {
    printf("\n");
    printf("   .-----------------------.\n");
    printf("   |  " C_YELLOW "C A S I N O   S L O T" C_RESET "  |\n");
    printf("   |-----------------------|\n");
    printf("   |                       |\n");
    printf("   |   %s %s %s  |\n", slot_symbols[s1], slot_symbols[s2], slot_symbols[s3]);
    printf("   |                       |\n");
    printf("   '-----------------------'\n");
}

void play_slots(Player* player) {
    int is_playing = 1;
    print_slots_welcome();

    int bet = 0; // משתנה שיזכור את ההימור הנוכחי לאורך הסיבובים

    while (is_playing) {
        print_table_header("SLOT MACHINE", "" C_YELLOW "", player->balance);

        // שלב א': אם זה הסיבוב הראשון או שהחלטנו להחליף הימור
        if (bet == 0) {
            printf("Options: [0] Leave Machine  [1] Insert Coin (Bet)\nAction: ");
            int action = get_safe_int();

            if (action == 0) break;
            if (action != 1) {
                printf("" C_YELLOW "Invalid option." C_RESET "\n");
                delay_ms(1000);
                continue;
            }

            printf("Enter bet amount: $");
            bet = get_safe_int();
        }
        else {
            // אם המשחק ממשיך ברצף, נציג את ההימור הנוכחי שרץ
            printf("Current Bet: $%d (Running automatically)\n", bet);
        }

        // בדיקת תקינות ההימור והיתרה
        if (bet <= 0 || bet > player->balance) {
            display_error(1500, "Invalid amount or insufficient funds!");
            bet = 0;
            continue;
        }

        if (bet > MAX_BET) {
            display_error(1500, "Machine maximum bet is $%d!", MAX_BET);
            bet = 0;
            continue;
        }

        player->balance -= bet;
        save_player(player);
        printf("\n" C_CYAN "Spinning the reels..." C_RESET "\n");

        // אנימציית גלילה מבוססת מנוע ויזואלי - אינה פולטת נתונים על המחולל הראשי
        int r1, r2, r3;
        for (int i = 0; i < 15; i++) {
            r1 = visual_rand() % 5;
            r2 = visual_rand() % 5;
            r3 = visual_rand() % 5;
            printf("\r   [ %s %s %s ]   ", slot_symbols[r1], slot_symbols[r2], slot_symbols[r3]);
            fflush(stdout);
            delay_ms(80 + (i * 10));
        }
        printf("\n");

        // הגרלת התוצאה האמיתית מתוך הליבה המאובטחת
        r1 = rand() % 5;
        r2 = rand() % 5;
        r3 = rand() % 5;

        // הצגת התוצאה הסופית הקובעת
        draw_slot_machine(r1, r2, r3);

        // חישוב זכיות
        int payout = 0;
        if (r1 == 0 && r2 == 0 && r3 == 0) {
            payout = bet * 50;
            printf("\n" C_RED "*** J A C K P O T ***" C_RESET "\n");
            play_jackpot_sound();
            printf("" C_GREEN "UNBELIEVABLE! You hit three 7s and won $%d!" C_RESET "\n", payout);
        }
        else if (r1 == r2 && r2 == r3) {
            payout = bet * 10;
            play_win_sound();
            printf("\n" C_GREEN "BIG WIN! Three of a kind! You won $%d!" C_RESET "\n", payout);
        }
        else if ((r1 == 0 && r2 == 0) || (r1 == 0 && r3 == 0) || (r2 == 0 && r3 == 0)) {
            payout = bet * 5;
            printf("\n" C_GREEN "NICE! Two 7s! You won $%d!" C_RESET "\n", payout);
        }
        else if (r1 == r2 || r1 == r3 || r2 == r3) {
            payout = bet * 2;
            printf("\n" C_GREEN "Small Win! Two of a kind! You won $%d!" C_RESET "\n", payout);
        }
        else {
            printf("\n" C_RED "No match. Better luck next pull!" C_RESET "\n");
            player->total_losses += bet;
        }

        // קוד חדש ומבוקר:
        if (payout > 0) {
            player->total_winnings += ((long long)payout - bet); // עדכון הסטטיסטיקה נשאר כרגיל
            add_balance_safe(player, payout);         // הזרמה מבוקרת של הכסף
        }

        save_player(player);

        // בדיקת פשיטת רגל (אם היתרה התאפסה, המנוע של main יטפל בזה)
        if (player->balance <= 0) {
            is_playing = 0;
            break;
        }

        // שלב ב': תפריט המקשים המהיר ללא לחיצה על Enter
        printf("\n--------------------------------------------------\n");
        printf(" -> Press [" C_GREEN "SPACE" C_RESET "] to spin again with the same bet ($%d)\n", bet);
        printf(" -> Press [" C_YELLOW "C" C_RESET "] to change bet amount\n");
        printf(" -> Press [" C_RED "0" C_RESET "] to exit to main menu\n");
        printf("--------------------------------------------------\n");

        clear_input_buffer();

        char key = ' ';
        while (1) {
            key = (char)_getch(); // קליטת מקש בודד מיידית
            if (key == ' ' || key == 'c' || key == 'C' || key == '0') {
                break; // יציאה מלולאת ההמתנה למקש חוקי
            }
        }

        if (key == '0') {
            is_playing = 0; // יוצא לתפריט הראשי
        }
        else if (key == 'c' || key == 'C') {
            bet = 0; // מאפס את ההימור כדי שהלולאה הבאה תבקש הימור חדש
        }
        // אם נלחץ ' ', הלולאה פשוט תמשיך לסיבוב הבא עם אותו סכום בדיוק!
    }
}