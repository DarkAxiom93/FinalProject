#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "casino.h"
#include "utils.h"

/*
 * פונקציה: print_craps_welcome
 * תפקיד: מדפיסה באנר כניסה ברור למשחק הקוביות וממתינה לאישור המשתמש
 */
void print_craps_welcome() {
    system("cls");
    printf("\x1b[35m"); // צבע סגול/מג'נטה
    printf("   ____  ____      _    ____  ____  \n");
    printf("  / ___||  _ \\    / \\  |  _ \\/ ___| \n");
    printf(" | |    | |_) |  / _ \\ | |_) \\___ \\ \n");
    printf(" | |___ |  _ <  / ___ \\|  __/ ___) |\n");
    printf("  \\____||_| \\_\\/_/   \\_\\_|   |____/ \n");
    printf("\x1b[0m\n");

    printf("\x1b[36m=========================================================================\x1b[0m\n");
    printf("                       \x1b[33mTABLE RULES (PASS LINE)\x1b[0m\n");
    printf("\x1b[36m=========================================================================\x1b[0m\n");
    printf(" * Come Out Roll: 7 or 11 wins instantly.\n");
    printf(" * Come Out Roll: 2, 3, or 12 loses instantly (Craps).\n");
    printf(" * Any other number (4,5,6,8,9,10) becomes the 'Point'.\n");
    printf(" * Point Phase  : Roll the Point number again to win.\n");
    printf(" * Point Phase  : Roll a 7 before the Point and you lose.\n");
    printf("\x1b[36m=========================================================================\x1b[0m\n\n");

    printf("\x1b[32mPress ENTER to acknowledge rules and join the table...\x1b[0m");
    wait_for_enter();
    system("cls");
}

/*
 * פונקציה: print_dice
 * תפקיד: חווית משתמש (UX) - הדפסת קוביות פשוטה לקונסולה
 */
void print_dice(int d1, int d2) {
    printf("\n\x1b[97m"); // צבע לבן
    printf("  _______     _______\n");
    printf(" |       |   |       |\n");
    printf(" |   %d   | + |   %d   | = \x1b[33m%d\x1b[97m\n", d1, d2, d1 + d2);
    printf(" |_______|   |_______|\n");
    printf("\x1b[0m\n");
}

/*
 * פונקציה: roll_dice
 * תפקיד: מגרילה 2 קוביות, מפעילה אנימציית השהייה ומדפיסה את התוצאה
 */
int roll_dice() {
    printf("Rolling the dice...\n");

    // אנימציית הטלה קצרה
    for (int i = 0; i < 3; i++) {
        printf(". ");
        fflush(stdout);
        delay_ms(400);
    }

    int d1 = (rand() % 6) + 1;
    int d2 = (rand() % 6) + 1;

    print_dice(d1, d2);
    return d1 + d2;
}

/*
 * פונקציה: handle_point_phase
 * תפקיד: מנהלת את השלב השני של הקראפס, חולצה מהפונקציה הראשית לסדר וקריאות.
 */
void handle_point_phase(int point, Player* player, int bet) {
    printf("\n\x1b[36mThe Point is now %d.\x1b[0m\n", point);
    printf("You must roll a %d to win, but if you roll a 7 you lose.\n", point);

    while (1) {
        printf("\nPress ENTER to roll the dice...");
        wait_for_enter();

        int sum = roll_dice();

        if (sum == point) {
            printf("\x1b[32mYOU HIT THE POINT! You win $%d!\x1b[0m\n", bet);
            player->balance += (bet * 2);
            player->total_winnings += bet;
            break; // יציאה מהלולאה הפנימית
        }
        else if (sum == 7) {
            printf("\x1b[31mSEVEN OUT! You lose.\x1b[0m\n");
            player->total_losses += bet;
            break;
        }
        else {
            printf("Roll again. Trying to hit %d...\n", point);
        }
    }
}

/*
 * פונקציה: play_craps
 * תפקיד: מנהלת את שלב הכניסה וההימור הראשון (קוד קצר וקריא משמעותית)
 */
void play_craps(Player* player) {
    int is_playing = 1;
    print_craps_welcome();

    while (is_playing) {
        print_table_header("CRAPS TABLE (Dice)", "\x1b[35m", player->balance); // שימוש בתשתית החדשה!

        printf("Options: [0] Leave Table  [1] Place Pass Line Bet\nAction: ");
        int action = get_safe_int();

        if (action == 0) break;
        if (action != 1) { printf("\x1b[33mInvalid option.\x1b[0m\n"); continue; }

        printf("Enter bet amount: $");
        int bet = get_safe_int();
        if (bet <= 0 || bet > player->balance) {
            printf("\x1b[31mInvalid amount or insufficient funds!\x1b[0m\n");
            continue;
        }

        player->balance -= bet;
        printf("Bet placed. \x1b[36mCome Out Roll\x1b[0m coming up...\n");
        delay_ms(1000);

        int sum = roll_dice();

        if (sum == 7 || sum == 11) {
            printf("\x1b[32mNATURAL! You win $%d!\x1b[0m\n", bet);
            player->balance += (bet * 2);
            player->total_winnings += bet;
        }
        else if (sum == 2 || sum == 3 || sum == 12) {
            printf("\x1b[31mCRAPS! You lose.\x1b[0m\n");
            player->total_losses += bet;
        }
        else {
            handle_point_phase(sum, player, bet); // קריאה לפונקציית העזר
        }

        if (player->balance <= 0) {
            printf("\n\x1b[31mYou are bankrupt! Security is escorting you out.\x1b[0m\n");
            is_playing = 0;
        }
    }
}