#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "casino.h"
#include "utils.h"
#include "account.h"

#define MAX_BALANCE 50000 // הגדרת מגבלת ההפקדה בקזינו

void load_player(Player* p) {
    p->balance = 1000; p->bank_balance = 0; p->total_winnings = 0; p->total_losses = 0;
    long loaded_checksum = 0;
    int has_checksum = 0;

    char filename[MAX_NAME_LEN + 5]; 
    snprintf(filename, sizeof(filename), "%s.bin", p->name);
    FILE* file = fopen(filename, "rb");
    if (file != NULL) {
        // 1. קריאת כל מבנה השחקן במכה אחת ישר לתוך המצביע p
        fread(p, sizeof(Player), 1, file);

        // 2. קריאת חתימת ה-Anti Cheat שנשמרה
        long loaded_checksum = 0;
        fread(&loaded_checksum, sizeof(long), 1, file);
        fclose(file);

        // אימות חתימת ה-Anti Cheat
        long expected_checksum = (((long)p->balance * 7919) ^ ((long)p->bank_balance * 6841)) +
            (((long)p->total_winnings * 5039) ^ (long)p->total_losses) ^ 0xDEADBEEF;

        if (has_checksum && loaded_checksum != expected_checksum) {
            system("cls");
            printf("" C_RED "\n======================================================\n");
            printf("  ! ! ! ANTI-CHEAT SYSTEM TRIGGERED ! ! !  \n");
            printf("======================================================\n" C_RESET "");
            printf("Data tampering detected in your save file!\n");
            printf("Your account has been reset as a penalty.\n");
            p->balance = 1000; p->bank_balance = 0; p->total_winnings = 0; p->total_losses = 0;
            save_player(p); // דורס את הקובץ המזויף ושומר חתימה חדשה וחוקית
            delay_ms(5000);
        }
        else {
            printf("\n" C_GREEN "Welcome back, %s! Your profile was securely loaded." C_RESET "\n", p->name);
        }
    }
    else {
        printf("\n" C_CYAN "New account created for %s. Starting balance: $1000." C_RESET "\n", p->name);
        save_player(p); // שומר מיד כדי לייצר את החתימה ההתחלתית
    }
}

void save_player(Player* p) {
    char filename[MAX_NAME_LEN + 5]; 
    snprintf(filename, sizeof(filename), "%s.bin", p->name);
    FILE* file = fopen(filename, "wb");
    if (file != NULL) {
        fwrite(p, sizeof(Player), 1, file);

        // מנגנון ה-Anti Cheat: יצירת חתימה מתמטית סודית
        // הנוסחה: (יתרה * 7) + (בנק * 3) + (זכיות * 5) - הפסדים
        long checksum = (((long)p->balance * 7919) ^ ((long)p->bank_balance * 6841)) +
            (((long)p->total_winnings * 5039) ^ (long)p->total_losses) ^ 0xDEADBEEF;
        fprintf(file, "=============================\n");
        fwrite(&checksum, sizeof(long), 1, file);

        fclose(file);
    }
    else {
        printf("" C_RED "Error: Could not save player data!" C_RESET "\n");
    }
}

// פונקציית עזר חדשה למשיכת כסף מהבנק לקופה של המשחק
void handle_withdrawal(Player* p) {
    system("cls");
    print_table_header("CASHIER (WITHDRAW)", "" C_GREEN "", p->balance);
    printf("Current funds locked in Bank: $%d\n", p->bank_balance);

    if (p->bank_balance <= 0) {
        printf("\n" C_RED "Your bank account is empty! Nothing to withdraw." C_RESET "\n");
        delay_ms(2000);
        return;
    }

    printf("\nEnter amount to withdraw from Bank (0 to cancel): $");
    int amount = get_safe_int();

    if (amount <= 0) {
        printf("Withdrawal cancelled.\n");
        delay_ms(1000);
        return;
    }

    if (amount > p->bank_balance) {
        printf("" C_RED "Error: You cannot withdraw more than what is in your bank ($%d)!" C_RESET "\n", p->bank_balance);
        delay_ms(2500);
    }
    else {
        p->bank_balance -= amount;
        p->balance += amount;
        printf("" C_GREEN "Successfully withdrew $%d from Bank to your wallet!" C_RESET "\n", amount);
        save_player(p); // שמירה מיידית לקובץ
        delay_ms(2000);
    }
}

// פונקציית הקופאי המעודכנת - מנהלת גם הפקדות לקזינו וגם משיכות/הפקדות לבנק
void handle_deposit(Player* p) {
    system("cls");
    print_table_header("CASINO CASHIER", "" C_GREEN "", p->balance);
    printf("Money in Wallet : $%d\n", p->balance);
    printf("Money in Bank   : $%d\n", p->bank_balance);
    printf("----------------------------------------\n");
    printf("Select Action:\n");
    printf("1. Deposit Money into Wallet (Max $50,000)\n");
    printf("2. Move Money from Wallet into SAFE BANK\n");
    printf("3. Withdraw Money from SAFE BANK back to Wallet\n");
    printf("0. Return to Main Menu\n");
    printf("Choice: ");

    int sub_choice = get_safe_int();

    if (sub_choice == 1) {
        // לוגיקת ההפקדה הקיימת שלכם
        if (p->balance >= MAX_BALANCE) {
            printf("\n" C_YELLOW "Wallet is already full." C_RESET "\n");
            delay_ms(2000); return;
        }
        printf("\nEnter amount to deposit to wallet: $");
        int amount = get_safe_int();
        if (amount <= 0) return;
        if (p->balance + amount > MAX_BALANCE) {
            printf("" C_RED "Exceeds limit!" C_RESET "\n"); delay_ms(2000);
        }
        else {
            p->balance += amount; printf("" C_GREEN "Deposited successfully." C_RESET "\n"); save_player(p); delay_ms(1500);
        }
    }
    else if (sub_choice == 2) {
        // הפקדה לתוך הבנק המאובטח
        printf("\nEnter amount to move into the Safe Bank (0 to cancel): $");
        int amount = get_safe_int();
        if (amount <= 0) return;
        if (amount > p->balance) {
            printf("" C_RED "You don't have that much money in your wallet!" C_RESET "\n"); delay_ms(2000);
        }
        else {
            p->balance -= amount;
            p->bank_balance += amount;
            printf("" C_GREEN "$%d safely moved to your Bank account!" C_RESET "\n", amount);
            save_player(p); delay_ms(2000);
        }
    }
    else if (sub_choice == 3) {
        handle_withdrawal(p); // קריאה לפונקציית המשיכה החדשה
    }
}

// ==========================================
// פאנל ניהול סודי - גישת מפתחים בלבד
// ==========================================
void admin_panel(Player* p) {
    system("cls");
    printf("\x1b[41m\x1b[97m SYSTEM RESTRICTED AREA " C_RESET "\n");
    printf("Enter Admin Password: ");
    char pass[20] = { 0 };
    if (scanf("%19s", pass) == 1) {
        while (getchar() != '\n');
        if (strcmp(pass, "HIT2026") != 0) {
            printf("" C_RED "ACCESS DENIED. INCORRECT PASSWORD." C_RESET "\n");
            delay_ms(2000);
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
                    FILE* file = fopen("highscores.txt", "w");
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
                char fname[MAX_NAME_LEN + 5];

                // פונה לקובץ ה-.bin החדש
                snprintf(fname, sizeof(fname), "%s.bin", target);

                // פותח לקריאה בינארית
                FILE* f = fopen(fname, "rb");

                if (f) {
                    Player audited_player;
                    long audited_checksum;

                    // שואב את הנתונים מהבינארי
                    fread(&audited_player, sizeof(Player), 1, f);
                    fread(&audited_checksum, sizeof(long), 1, f);
                    fclose(f);

                    // מדפיס למסך
                    printf("\n" C_CYAN "--- SECURE AUDIT REPORT: %s ---" C_RESET "\n", audited_player.name);
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