#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
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
        printf("2. " C_YELLOW "Spy on Player Account (Audit)" C_RESET "\n");
        printf("3. " C_RED "Confiscate Funds from Current Player" C_RESET "\n");
        printf("4. " C_RED "Ban a Player" C_RESET "\n");
        printf("5. " C_GREEN "View Banned Players / Unban" C_RESET "\n");
        printf("6. " C_CYAN "Casino-Wide Statistics (House Profit Report)" C_RESET "\n");
        printf("0. Exit Admin Panel\n");
        printf("Select command: ");

        int cmd = get_menu_key("0123456") - '0';
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
            printf("Enter exact player name to audit: ");
            char target[MAX_NAME_LEN];
            if (scanf("%49s", target) == 1) {
                while (getchar() != '\n');
                if (!is_valid_name(target)) {
                    printf("" C_RED "Invalid player name format." C_RESET "\n");
                    delay_ms(2000);
                    continue;
                }

                Player audited_player = { 0 };
                int checksum_ok = 0;
                int status = read_player_file(target, &audited_player, &checksum_ok, NULL);

                if (status == 1) {
                    printf("\n" C_CYAN "--- SECURE DECRYPTED AUDIT REPORT: %s ---" C_RESET "\n", audited_player.name);
                    printf("  Player_Name:     %s\n", audited_player.name);
                    printf("  Current_Balance: $%d\n", audited_player.balance);
                    printf("  Bank_Balance:    $%d\n", audited_player.bank_balance);
                    printf("  -----------------------------\n");
                    printf("  Total_Winnings:  $%lld\n", audited_player.total_winnings);
                    printf("  Total_Losses:    $%lld\n", audited_player.total_losses);
                    printf("  Banned:          %s\n", audited_player.is_banned ? "YES" : "No");
                    printf("  =============================\n");
                    if (checksum_ok) printf("  " C_GREEN "Checksum: VALID (no tampering detected)" C_RESET "\n");
                    else printf("  " C_RED "Checksum: MISMATCH (possible tampering!)" C_RESET "\n");
                    printf(C_CYAN "---------------------------------------" C_RESET "\n");
                }
                else if (status == -1) {
                    printf("" C_RED "Error: Player record is corrupted or unreadable." C_RESET "\n");
                }
                else {
                    printf("" C_RED "Record not found for player: %s" C_RESET "\n", target);
                }
                printf("\nPress ENTER to continue...");
                wait_for_enter();
            }
        }
        else if (cmd == 3) {
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
        else if (cmd == 4) {
            printf("Enter exact player name to ban: ");
            char target[MAX_NAME_LEN];
            if (scanf("%49s", target) == 1) {
                while (getchar() != '\n');
                if (!is_valid_name(target)) {
                    printf("" C_RED "Invalid player name format." C_RESET "\n");
                    delay_ms(2000);
                    continue;
                }

                Player target_player = { 0 };
                int checksum_ok = 0;
                int status = read_player_file(target, &target_player, &checksum_ok, NULL);

                if (status == 0) {
                    printf("" C_RED "No account found for player: %s" C_RESET "\n", target);
                    delay_ms(2000);
                }
                else if (status == -1) {
                    printf("" C_RED "That player's save file is corrupted and cannot be modified safely." C_RESET "\n");
                    delay_ms(2000);
                }
                else if (target_player.is_banned) {
                    printf("" C_YELLOW "Player '%s' is already banned." C_RESET "\n", target_player.name);
                    delay_ms(1500);
                }
                else {
                    if (!checksum_ok) {
                        printf(C_YELLOW "Warning: this save file's checksum does not match (possible tampering). Proceeding anyway." C_RESET "\n");
                    }

                    target_player.is_banned = 1;

                    // אם המנהל חוסם את עצמו (השחקן המחובר כרגע), מסנכרנים גם את הסשן הפעיל
                    if (strcmp(target_player.name, p->name) == 0) {
                        p->is_banned = 1;
                    }

                    save_player(&target_player);
                    printf("" C_RED "Player '%s' has been BANNED." C_RESET "\n", target_player.name);
                    delay_ms(2000);
                }
            }
        }
        else if (cmd == 5) {
            clear_screen();
            printf("" C_GREEN "========================================\n");
            printf("       BANNED PLAYERS - SELECT TO UNBAN       \n");
            printf("========================================" C_RESET "\n");

            char banned_names[9][MAX_NAME_LEN];
            int banned_count = 0;
            int total_banned_found = 0;

            WIN32_FIND_DATAA fd;
            HANDLE hFind = FindFirstFileA("data\\*.bin", &fd);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    char name[MAX_NAME_LEN] = { 0 };
                    strncpy(name, fd.cFileName, MAX_NAME_LEN - 1);
                    char* dot = strrchr(name, '.');
                    if (dot) *dot = '\0';
                    if (!is_valid_name(name)) continue;

                    Player pl = { 0 };
                    int status = read_player_file(name, &pl, NULL, NULL);
                    if (status != 1 || !pl.is_banned) continue;

                    total_banned_found++;
                    if (banned_count < 9) strcpy(banned_names[banned_count++], pl.name);
                } while (FindNextFileA(hFind, &fd));
                FindClose(hFind);
            }

            if (banned_count == 0) {
                printf("\n" C_GREEN "No banned players found." C_RESET "\n");
                delay_ms(1500);
                continue;
            }

            printf("\n");
            for (int i = 0; i < banned_count; i++) {
                printf("  [%d] %s\n", i + 1, banned_names[i]);
            }
            if (total_banned_found > banned_count) {
                printf(C_YELLOW "  (showing first 9 of %d banned players)" C_RESET "\n", total_banned_found);
            }
            printf("  [0] Cancel / Back\n\nSelect player to UNBAN: ");

            char keys[11] = { 0 };
            keys[0] = '0';
            for (int i = 0; i < banned_count; i++) keys[i + 1] = (char)('1' + i);
            char sel = get_menu_key(keys);

            if (sel != '0') {
                int idx = sel - '1';
                Player target_player = { 0 };
                int status = read_player_file(banned_names[idx], &target_player, NULL, NULL);

                if (status == 1) {
                    target_player.is_banned = 0;
                    if (strcmp(target_player.name, p->name) == 0) {
                        p->is_banned = 0;
                    }
                    save_player(&target_player);
                    printf("" C_GREEN "Player '%s' has been UNBANNED." C_RESET "\n", target_player.name);
                }
                else {
                    printf("" C_RED "Error: could not reload that player's record." C_RESET "\n");
                }
                delay_ms(2000);
            }
        }
        else if (cmd == 6) {
            clear_screen();
            printf("" C_CYAN "========================================\n");
            printf("        CASINO-WIDE STATISTICS         \n");
            printf("========================================" C_RESET "\n");

            int player_count = 0, banned_count = 0, corrupted_count = 0;
            long long sum_winnings = 0, sum_losses = 0, sum_wallet = 0, sum_bank = 0;

            WIN32_FIND_DATAA fd;
            HANDLE hFind = FindFirstFileA("data\\*.bin", &fd);
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    char name[MAX_NAME_LEN] = { 0 };
                    strncpy(name, fd.cFileName, MAX_NAME_LEN - 1);
                    char* dot = strrchr(name, '.');
                    if (dot) *dot = '\0';
                    if (!is_valid_name(name)) continue;

                    Player pl = { 0 };
                    int status = read_player_file(name, &pl, NULL, NULL);
                    if (status == -1) { corrupted_count++; continue; }
                    if (status == 0) continue;

                    player_count++;
                    if (pl.is_banned) banned_count++;
                    sum_winnings += pl.total_winnings;
                    sum_losses += pl.total_losses;
                    sum_wallet += pl.balance;
                    sum_bank += pl.bank_balance;
                } while (FindNextFileA(hFind, &fd));
                FindClose(hFind);
            }

            long long house_profit = sum_losses - sum_winnings;
            long long outstanding = sum_wallet + sum_bank;

            printf("\n Registered Players      : %d  (" C_RED "%d banned" C_RESET ")\n", player_count, banned_count);
            if (corrupted_count > 0) {
                printf(" " C_YELLOW "Corrupted Records       : %d (skipped)" C_RESET "\n", corrupted_count);
            }
            printf("----------------------------------------\n");
            printf(" Total Paid to Players   : " C_RED "-$%lld" C_RESET "  (lifetime player winnings)\n", sum_winnings);
            printf(" Total Collected         : " C_GREEN "+$%lld" C_RESET "  (lifetime player losses)\n", sum_losses);
            printf("----------------------------------------\n");
            if (house_profit >= 0)
                printf(" HOUSE NET PROFIT        : " C_GREEN "+$%lld" C_RESET "\n", house_profit);
            else
                printf(" HOUSE NET LOSS          : " C_RED "-$%lld" C_RESET "\n", -house_profit);
            printf("----------------------------------------\n");
            printf(" Outstanding in Wallets  : $%lld\n", sum_wallet);
            printf(" Outstanding in Banks    : $%lld\n", sum_bank);
            printf(" Total Player Holdings   : $%lld  (casino's outstanding liability)\n", outstanding);
            printf("========================================\n");

            printf("\nPress ENTER to continue...");
            wait_for_enter();
        }
    }
}
#endif // ENABLE_ADMIN_PANEL