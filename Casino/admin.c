#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "casino.h"
#include "utils.h"
#include "account.h"
#include "admin.h"

// ==========================================
// פאנל ניהול סודי - גישת מפתחים בלבד
// ==========================================
#ifdef ENABLE_ADMIN_PANEL

void admin_panel(Player* p) {
    system("cls");
    printf("\x1b[41m\x1b[97m SYSTEM RESTRICTED AREA " C_RESET "\n");
    printf("Enter Admin Password: ");

    char pass[50] = { 0 }; // תמיכה בסיסמה ארוכה באמת
    if (scanf("%49s", pass) == 1) {
        while (getchar() != '\n');

        unsigned int pass_hash = hash_password(pass); // שימוש באלגוריתם המאובטח

        // כרגע מכוון ל-0 כדי לזרוק אותך החוצה ולהדפיס את ההאש החדש שלך!
        unsigned int expected_hash = 4168410452U;

        if (pass_hash != expected_hash) {
            printf("" C_RED "ACCESS DENIED. INCORRECT PASSWORD." C_RESET "\n");

         

            delay_ms(5000);
            return;
        }
    }

    else {
        while (getchar() != '\n'); return;
    }

    while (1) {
        system("cls");
        printf("" C_RED "========================================\n");
        printf("       CASINO ADMIN CONTROL PANEL       \n");
        printf("========================================" C_RESET "\n");
        printf("1. Grant House Bonus to Current Player\n");
        printf("2. Reset Leaderboard (Wipe Highscores)\n");
        printf("3. " C_YELLOW "Spy on Player Account (Audit)" C_RESET "\n");
        printf("4. " C_RED "Confiscate Funds from Current Player" C_RESET "\n");
        printf("0. Exit Admin Panel\n");
        printf("Select command: ");

        int cmd = get_safe_int();
        if (cmd == 0) break;
        else if (cmd == 1) {
            printf("Enter bonus amount: $");
            int bonus = get_safe_int();
            if (bonus > 0) {
                p->balance += bonus;
                printf("" C_GREEN "Bonus of $%d granted to %s." C_RESET "\n", bonus, p->name);
                save_player(p);
                delay_ms(1500);
            }
        }
        else if (cmd == 2) {
            printf("" C_RED "Are you sure you want to wipe the leaderboard? (Y/N): " C_RESET "");
            char confirm;
            if (scanf(" %c", &confirm) == 1) {
                while (getchar() != '\n');
                if (confirm == 'Y' || confirm == 'y') {
                    FILE* file = fopen("data/highscores.txt", "w");
                    if (file) { fclose(file); printf("" C_GREEN "Leaderboard successfully wiped." C_RESET "\n"); }
                    delay_ms(1500);
                }
            }
        }
        else if (cmd == 3) {
            printf("Enter exact player name to audit: ");
            char target[MAX_NAME_LEN];
            if (scanf("%49s", target) == 1) {
                while (getchar() != '\n');
                if (!is_valid_name(target)) {
                    printf("" C_RED "Invalid player name format." C_RESET "\n");
                    delay_ms(2000);
                    continue;
                }
                char fname[MAX_NAME_LEN + 15];
                snprintf(fname, sizeof(fname), "data/%s.bin", target);

                FILE* f = fopen(fname, "rb"); // קריאה בינארית מהקובץ המוצפן
                if (f) {
                    char buffer[1024] = { 0 };
                    int len = (int)fread(buffer, 1, sizeof(buffer) - 1, f);
                    fclose(f);

                    if (len > 0) {
                        crypt_buffer(buffer, len); // פענוח סודי לטובת מנהל המערכת
                        buffer[len] = '\0';

                        Player audited_player = { 0 };
                        long audited_checksum = 0;

                        int read_count = sscanf(buffer, "%49s\n%d\n%d\n%d\n%d\n%ld",
                            audited_player.name, &audited_player.balance, &audited_player.bank_balance,
                            &audited_player.total_winnings, &audited_player.total_losses, &audited_checksum);

                        if (read_count == 6) {
                            printf("\n" C_CYAN "--- SECURE DECRYPTED AUDIT REPORT: %s ---" C_RESET "\n", audited_player.name);
                            printf("  Player_Name:     %s\n", audited_player.name);
                            printf("  Current_Balance: $%d\n", audited_player.balance);
                            printf("  Bank_Balance:    $%d\n", audited_player.bank_balance);
                            printf("  -----------------------------\n");
                            printf("  Total_Winnings:  $%d\n", audited_player.total_winnings);
                            printf("  Total_Losses:    $%d\n", audited_player.total_losses);
                            printf("  =============================\n");
                            printf("  Checksum:        %ld\n", audited_checksum);
                            printf(C_CYAN "---------------------------------------" C_RESET "\n");
                        }
                        else {
                            printf("" C_RED "Error: Player record is corrupted or unreadable." C_RESET "\n");
                        }
                    }
                }
                else {
                    printf("" C_RED "Record not found for player: %s" C_RESET "\n", target);
                }
                printf("\nPress ENTER to continue...");
                wait_for_enter();
            }
        }
        else if (cmd == 4) {
            printf("Enter amount to confiscate: $");
            int penalty = get_safe_int();
            if (penalty > 0 && penalty <= p->balance) {
                p->balance -= penalty;
                printf("" C_RED "Confiscated $%d! New balance: $%d" C_RESET "\n", penalty, p->balance);
                save_player(p);
                delay_ms(2000);
            }
            else {
                printf("" C_RED "Invalid amount or player doesn't have enough funds." C_RESET "\n"); delay_ms(1500);
            }
        }
    }
}
#endif // ENABLE_ADMIN_PANEL