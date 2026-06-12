#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "casino.h"
#include "utils.h"
#include "account.h"
#include "admin.h"

// ==========================================
// פאנל ניהול סודי - גישת מפתחים בלבד
// ==========================================
#ifdef ENABLE_ADMIN_PANEL

void admin_panel(Player* p) {
    clear_screen();
    printf("\x1b[41m\x1b[97m SYSTEM RESTRICTED AREA " C_RESET "\n");
    printf("Enter Admin Password: ");

    char pass[50] = { 0 };
    int p_idx = 0;
    char ch;

    // לולאת קליטת סיסמה מוסתרת עם תמיכה במחיקה 
    while (1) {
        ch = (char)_getch(); // קליטת תו ללא הדפסה למסך

        if (ch == '\r' || ch == '\n') {
            pass[p_idx] = '\0';
            printf("\n");
            break;
        }
        else if (ch == '\b') {
            if (p_idx > 0) {
                p_idx--;
                printf("\b \b");
            }
        }
        else if (p_idx < 49) {
            pass[p_idx++] = ch;
            printf("*");
        }
    }

    unsigned int pass_hash = hash_password(pass);

    unsigned int expected_hash = get_admin_hash();

    if (pass_hash != expected_hash) {
        printf("" C_RED "ACCESS DENIED. INCORRECT PASSWORD." C_RESET "\n");
        delay_ms(5000);
        return;
    }

    while (1) {
        clear_screen();
        printf("" C_RED "========================================\n");
        printf("       CASINO ADMIN CONTROL PANEL       \n");
        printf("========================================" C_RESET "\n");
        printf("1. Grant House Bonus to Current Player\n");
        printf("2. Reset Leaderboard (Wipe Highscores)\n");
        printf("3. " C_YELLOW "Spy on Player Account (Audit)" C_RESET "\n");
        printf("4. " C_RED "Confiscate Funds from Current Player" C_RESET "\n");
        printf("0. Exit Admin Panel\n");
        printf("Select command: ");

        int cmd = get_menu_key("01234") - '0';
        if (cmd == 0) break;
        else if (cmd == 1) {
            printf("Enter bonus amount: $");
            int bonus = get_safe_int();

            if (bonus > 0) {
                if (bonus > MAX_BALANCE) {
                    display_error(2000, "Error: Bonus amount cannot exceed MAX_BALANCE ($%d).", MAX_BALANCE);
                }
                else if ((long long)p->balance + bonus > MAX_BALANCE) {
                    printf("\n" C_YELLOW "Notice: Bonus capped at wallet limit ($%d)." C_RESET "\n", MAX_BALANCE);
                    p->balance = MAX_BALANCE;
                    printf("" C_GREEN "Bonus granted to %s. New balance: $%d" C_RESET "\n", p->name, p->balance);
                    save_player(p);
                    delay_ms(2000);
                }
                // מצב תקין: הבונוס נכנס בשלמותו
                else {
                    p->balance += bonus;
                    printf("" C_GREEN "Bonus of $%d granted to %s. New balance: $%d" C_RESET "\n", bonus, p->name, p->balance);
                    save_player(p);
                    delay_ms(1500);
                }
            }
        }
        else if (cmd == 2) {
            printf("" C_RED "Are you sure you want to wipe the leaderboard? (Y/N): " C_RESET "");
            char confirm = get_menu_key("YyNn");
            if (confirm == 'Y' || confirm == 'y') {
                FILE* file = fopen("data/highscores.txt", "w");
                if (file) { fclose(file); printf("" C_GREEN "Leaderboard successfully wiped." C_RESET "\n"); }
                delay_ms(1500);
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

                FILE* f = fopen(fname, "rb");
                if (f) {
                    char buffer[1024] = { 0 };
                    int len = (int)fread(buffer, 1, sizeof(buffer) - 1, f);
                    fclose(f);

                    if (len > 0) {
                        crypt_buffer(buffer, len); // פענוח סודי לטובת מנהל המערכת
                        buffer[len] = '\0';

                        Player audited_player = { 0 };
                        long long audited_checksum = 0;
                        int file_version = 0;

                        int read_count = sscanf(buffer, "%d\n%49s\n%d\n%d\n%lld\n%lld\n%lld",
                            &file_version, audited_player.name, &audited_player.balance, &audited_player.bank_balance,
                            &audited_player.total_winnings, &audited_player.total_losses, &audited_checksum);

                        if (read_count == 0) {
                            read_count = sscanf(buffer, "%49s\n%d\n%d\n%lld\n%lld\n%lld",
                                audited_player.name, &audited_player.balance, &audited_player.bank_balance,
                                &audited_player.total_winnings, &audited_player.total_losses, &audited_checksum);
                        }

                        if (read_count >= 6) { 
                            printf("\n" C_CYAN "--- SECURE DECRYPTED AUDIT REPORT: %s ---" C_RESET "\n", audited_player.name);
                            printf("  Player_Name:     %s\n", audited_player.name);
                            printf("  Current_Balance: $%d\n", audited_player.balance);
                            printf("  Bank_Balance:    $%d\n", audited_player.bank_balance);
                            printf("  -----------------------------\n");
                            printf("  Total_Winnings:  $%lld\n", audited_player.total_winnings);
                            printf("  Total_Losses:    $%lld\n", audited_player.total_losses);
                            printf("  =============================\n");
                            printf("  Checksum:        %lld\n", audited_checksum);
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