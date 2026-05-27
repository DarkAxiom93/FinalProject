#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "casino.h"
#include "utils.h"
#include <conio.h> // מאפשרת שימוש ב-_getch() לקריאת מקשים מיידית

// מערך של סמלים למכונת המזל
const char* slot_symbols[] = {
    "\x1b[31m[ 7 ]\x1b[0m", // 0: שבע אדום (ג'קפוט)
    "\x1b[33m[ $ ]\x1b[0m", // 1: דולר זהב
    "\x1b[32m[ # ]\x1b[0m", // 2: סולמית ירוקה
    "\x1b[35m[ @ ]\x1b[0m", // 3: שטרודל סגול
    "\x1b[36m[ * ]\x1b[0m"  // 4: כוכב תכלת
};

void print_slots_welcome() {
    system("cls");
    printf("\x1b[33m"); // זהב
    printf("  ____  _      ___ _____  ____  \n");
    printf(" / ___|| |    / _ \\_   _|/ ___| \n");
    printf(" \\___ \\| |   | | | || |  \\___ \\ \n");
    printf("  ___) | |___| |_| || |   ___) |\n");
    printf(" |____/|_____|\\___/ |_|  |____/ \n");
    printf("\x1b[0m\n");

    printf("\x1b[36m=========================================================================\x1b[0m\n");
    printf("                       \x1b[33mPAYTABLE & RULES\x1b[0m\n");
    printf("\x1b[36m=========================================================================\x1b[0m\n");
    printf(" * THREE 7s [ 7 ][ 7 ][ 7 ] : JACKPOT! Pays 50 to 1\n");
    printf(" * THREE of a kind          : BIG WIN! Pays 10 to 1\n");
    printf(" * TWO 7s anywhere          : Pays 5 to 1\n");
    printf(" * TWO of a kind            : Pays 2 to 1\n");
    printf("\x1b[36m=========================================================================\x1b[0m\n\n");

    printf("\x1b[32mPress ENTER to pull the lever...\x1b[0m");
    wait_for_enter();
    system("cls");
}

// פונקציה שמציירת את המכונה עם האנימציה
void draw_slot_machine(int s1, int s2, int s3) {
    printf("\n");
    printf("   .-----------------------.\n");
    printf("   |  \x1b[33mC A S I N O   S L O T\x1b[0m  |\n");
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
        print_table_header("SLOT MACHINE", "\x1b[33m", player->balance);

        // שלב א': אם זה הסיבוב הראשון או שהחלטנו להחליף הימור
        if (bet == 0) {
            printf("Options: [0] Leave Machine  [1] Insert Coin (Bet)\nAction: ");
            int action = get_safe_int();

            if (action == 0) break;
            if (action != 1) {
                printf("\x1b[33mInvalid option.\x1b[0m\n");
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
            printf("\x1b[31mInvalid amount or insufficient funds!\x1b[0m\n");
            bet = 0; // מאפסים כדי לאלץ בחירת הימור מחדש
            delay_ms(1500);
            continue;
        }

        player->balance -= bet;
        printf("\n\x1b[36mSpinning the reels...\x1b[0m\n");

        // אנימציית גלילה של מכונת המזל
        int r1, r2, r3;
        for (int i = 0; i < 15; i++) {
            r1 = rand() % 5;
            r2 = rand() % 5;
            r3 = rand() % 5;
            printf("\r   [ %s %s %s ]   ", slot_symbols[r1], slot_symbols[r2], slot_symbols[r3]);
            fflush(stdout);
            delay_ms(80 + (i * 10));
        }
        printf("\n");

        // הצגת התוצאה הסופית
        draw_slot_machine(r1, r2, r3);

        // חישוב זכיות
        int payout = 0;
        if (r1 == 0 && r2 == 0 && r3 == 0) {
            payout = bet * 50;
            printf("\n\x1b[31m*** J A C K P O T ***\x1b[0m\n");
            printf("\x1b[32mUNBELIEVABLE! You hit three 7s and won $%d!\x1b[0m\n", payout);
        }
        else if (r1 == r2 && r2 == r3) {
            payout = bet * 10;
            printf("\n\x1b[32mBIG WIN! Three of a kind! You won $%d!\x1b[0m\n", payout);
        }
        else if ((r1 == 0 && r2 == 0) || (r1 == 0 && r3 == 0) || (r2 == 0 && r3 == 0)) {
            payout = bet * 5;
            printf("\n\x1b[32mNICE! Two 7s! You won $%d!\x1b[0m\n", payout);
        }
        else if (r1 == r2 || r1 == r3 || r2 == r3) {
            payout = bet * 2;
            printf("\n\x1b[32mSmall Win! Two of a kind! You won $%d!\x1b[0m\n", payout);
        }
        else {
            printf("\n\x1b[31mNo match. Better luck next pull!\x1b[0m\n");
            player->total_losses += bet;
        }

        if (payout > 0) {
            player->balance += payout;
            player->total_winnings += (payout - bet);
        }

        // בדיקת פשיטת רגל (אם היתרה התאפסה, המנוע של main יטפל בזה)
        if (player->balance <= 0) {
            is_playing = 0;
            break;
        }

        // שלב ב': תפריט המקשים המהיר ללא לחיצה על Enter
        printf("\n--------------------------------------------------\n");
        printf(" -> Press [\x1b[32mSPACE\x1b[0m] to spin again with the same bet ($%d)\n", bet);
        printf(" -> Press [\x1b[33mC\x1b[0m] to change bet amount\n");
        printf(" -> Press [\x1b[31m0\x1b[0m] to exit to main menu\n");
        printf("--------------------------------------------------\n");

        char key = ' ';
        while (1) {
            key = _getch(); // קליטת מקש בודד מיידית
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