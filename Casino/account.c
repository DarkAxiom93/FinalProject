#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "casino.h"
#include "utils.h"
#include "account.h"

// פונקציית עזר לחישוב חתימת אבטחה מנקודה אחת בלבד
static long calculate_checksum(Player* p) {
    return (((long)p->balance * 7919) ^ ((long)p->bank_balance * 6841)) +
        (((long)p->total_winnings * 5039) ^ (long)p->total_losses) ^ 0xDEADBEEF;
}

#define MAX_BALANCE 50000 // הגדרת מגבלת ההפקדה בקזינו

void load_player(Player* p) {
    char temp_name[MAX_NAME_LEN];
    strcpy(temp_name, p->name);

    memset(p, 0, sizeof(Player));
    strcpy(p->name, temp_name);

    p->balance = 1000; p->bank_balance = 0; p->total_winnings = 0; p->total_losses = 0;

    char filename[MAX_NAME_LEN + 15];
    snprintf(filename, sizeof(filename), "data/%s.bin", p->name);

    // פתיחה לקריאת טקסט (r)
    FILE* file = fopen(filename, "r");

    if (file != NULL) {
        long loaded_checksum = 0;

        // קריאה מפורשת של הנתונים שורות-שורות
        int read_count = fscanf(file, "%49s\n%d\n%d\n%d\n%d\n%ld",
            p->name, &p->balance, &p->bank_balance, &p->total_winnings, &p->total_losses, &loaded_checksum);

        // סגירת משאב מיידית
        fclose(file);

        if (read_count == 6) {
            long expected_checksum = calculate_checksum(p);

            if (loaded_checksum != expected_checksum) {
                system("cls");
                printf("" C_RED "\n======================================================\n");
                printf("  ! ! ! ANTI-CHEAT SYSTEM TRIGGERED ! ! !  \n");
                printf("======================================================\n" C_RESET "");
                printf("Data tampering detected in your save file!\n");
                printf("Your account has been reset as a penalty.\n");

                memset(p, 0, sizeof(Player));
                strcpy(p->name, temp_name);
                p->balance = 0; p->bank_balance = 0; p->total_winnings = 0; p->total_losses = 0;

                save_player(p);
                delay_ms(5000);
            }
            else {
                printf("\n" C_GREEN "Welcome back, %s! Your profile was securely loaded." C_RESET "\n", p->name);
            }
        }
        else {
            printf("\n" C_RED "SYSTEM ERROR: Save file is corrupted! Resetting account." C_RESET "\n");
            memset(p, 0, sizeof(Player));
            strcpy(p->name, temp_name);
            save_player(p);
            delay_ms(3000);
        }
    }
    else {
        printf("\n" C_CYAN "New account created for %s. Starting balance: $1000." C_RESET "\n", p->name);
        save_player(p);
    }
}

void save_player(Player* p) {
    char filename[MAX_NAME_LEN + 15];
    char temp_filename[MAX_NAME_LEN + 15];
    snprintf(filename, sizeof(filename), "data/%s.bin", p->name);
    snprintf(temp_filename, sizeof(temp_filename), "data/%s.tmp", p->name);

    // פתיחה לכתיבת טקסט (w)
    FILE* file = fopen(temp_filename, "w");
    if (file != NULL) {
        long checksum = calculate_checksum(p);

        // כתיבת הנתונים בפורמט טקסטואלי ברור שורה אחר שורה
        fprintf(file, "%s\n%d\n%d\n%d\n%d\n%ld\n",
            p->name, p->balance, p->bank_balance, p->total_winnings, p->total_losses, checksum);

        fclose(file);

        (void)remove(filename);
        (void)rename(temp_filename, filename);
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

