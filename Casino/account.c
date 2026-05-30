#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "casino.h"
#include "utils.h"
#include "account.h"

// חתימת אבטחה ב-64-ביט המונעת חיתוך נתונים והתנגשויות
static long long calculate_checksum(Player* p) {
    return (((long long)p->balance * casino_salt_1) ^ ((long long)p->bank_balance * casino_salt_2)) +
        (((long long)p->total_winnings * 5039LL) ^ (long long)p->total_losses) ^ casino_secret_key;
}

void load_player(Player* p) {
    char temp_name[MAX_NAME_LEN];
    strcpy(temp_name, p->name);
    memset(p, 0, sizeof(Player));
    strcpy(p->name, temp_name);
    p->balance = 1000; p->bank_balance = 0; p->total_winnings = 0; p->total_losses = 0;
    char filename[MAX_NAME_LEN + 15];
    snprintf(filename, sizeof(filename), "data/%s.bin", p->name);

    // קריאה בינארית לתוך חוצץ (Buffer) כי המידע מוצפן
    FILE* file = fopen(filename, "rb");

    if (file != NULL) {
        // חישוב תפוסת זיכרון לשמירת עתיד (Future-Proofing):
        // מקסימום תווים צפוי: VERSION(2) + NAME(49) + 5 נתונים של 64-ביט (עד 20 תווים כ"א) + ירידות שורה.
        // סך הכל כ-160 תווים בלחץ. הקצאה של 1024 תווים מספקת מרווח ביטחון נדיב מאוד.
        char buffer[1024] = { 0 };
        int len = (int)fread(buffer, 1, sizeof(buffer) - 1, file);
        fclose(file);

        if (len > 0) {
            // פענוח צופן הזרם
            crypt_buffer(buffer, len);
            buffer[len] = '\0'; // אבטחת סיום מחרוזת

            long long loaded_checksum = 0;
            // שאיבת הנתונים מהטקסט המפוענח שבתוך החוצץ
            int file_version = 0; // משתנה זמני לבדיקת הגרסה

            // שימוש ב-sscanf עם הפרמטר החדש (הגרסה)
            int read_count = sscanf(buffer, "%d\n%49s\n%d\n%d\n%lld\n%lld\n%lld",
                &file_version, p->name, &p->balance, &p->bank_balance,
                &p->total_winnings, &p->total_losses, &loaded_checksum);

            if (read_count >= 6) { // אם קראנו לפחות את הנתונים הבסיסיים
                if (file_version < SAVE_FILE_VERSION) {
                    printf(C_YELLOW "Detected legacy save file! Upgrading to v%d..." C_RESET "\n", SAVE_FILE_VERSION);
                    // כאן אפשר להוסיף לוגיקה להשלמת נתונים חסרים אם נרצה בעתיד
                }
                long long expected_checksum = calculate_checksum(p);
                if (loaded_checksum != expected_checksum) {
                    clear_screen();
                    printf("" C_RED "\n======================================================\n");
                    printf("  ! ! ! ANTI-CHEAT SYSTEM TRIGGERED ! ! !  \n");
                    printf("======================================================\n" C_RESET "");
                    printf("Data tampering detected in your save file!\n");
                    printf("Your account has been reset as a penalty.\n");
                    memset(p, 0, sizeof(Player));
                    strcpy(p->name, temp_name);
                    save_player(p);
                    delay_ms(5000);
                }
                else {
                    printf("\n" C_GREEN "Welcome back, %s! Your profile was securely loaded and decrypted." C_RESET "\n", p->name);
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

    FILE* file = fopen(temp_filename, "wb");
    if (file != NULL) {
        long long checksum = calculate_checksum(p);
        char buffer[1024] = { 0 };

        // כתיבת הטקסט הברור לתוך חוצץ (Buffer)
        int len = snprintf(buffer, sizeof(buffer), "%d\n%s\n%d\n%d\n%lld\n%lld\n%lld\n",
            SAVE_FILE_VERSION, p->name, p->balance, p->bank_balance,
            p->total_winnings, p->total_losses, checksum);
        // הצפנת כל תוכן החוצץ
        crypt_buffer(buffer, len);

        // כתיבת הבינארי המוצפן לקובץ
        fwrite(buffer, 1, len, file);
        fclose(file);

        (void)remove(filename);
        (void)rename(temp_filename, filename);
    }
    else {
        printf("" C_RED "Error: Could not save encrypted player data!" C_RESET "\n");
    }
}

// פונקציית עזר למשיכת כסף מהבנק לקופה של המשחק
void handle_withdrawal(Player* p) {
    clear_screen();
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

    // הגנה 1: משיכה מעבר למה שיש בבנק
    if (amount > p->bank_balance) {
        printf("" C_RED "Error: You cannot withdraw more than what is in your bank ($%d)!" C_RESET "\n", p->bank_balance);
        delay_ms(2500);
    }
    // הגנה 2: מניעת עקיפת מגבלת הארנק (Wallet Overflow Bypass)
    else if (p->balance + amount > MAX_BALANCE) {
        int max_withdraw = MAX_BALANCE - p->balance;
        if (max_withdraw <= 0) {
            display_error(2500, "Your wallet is completely full! You cannot withdraw any more funds.");
        }
        else {
            display_error(2500, "Wallet limit reached! You can withdraw at most $%d right now.", max_withdraw);
        }
    }
    else {
        // הכל תקין - מבצעים את ההעברה
        p->bank_balance -= amount;
        p->balance += amount;
        printf("" C_GREEN "Successfully withdrew $%d from Bank to your wallet!" C_RESET "\n", amount);
        save_player(p); // שמירה מיידית לקובץ
        delay_ms(2000);
    }
}

// פונקציית הקופאי המעודכנת - מנהלת גם הפקדות לקזינו וגם משיכות/הפקדות לבנק
void handle_deposit(Player* p) {
    clear_screen();
    print_table_header("CASINO CASHIER", "" C_GREEN "", p->balance);
    printf("Money in Wallet : $%d\n", p->balance);
    printf("Money in Bank   : $%d\n", p->bank_balance);
    printf("----------------------------------------\n");
    printf("Select Action:\n");
    printf("1. Deposit Money into Wallet (Max $%d)\n", MAX_BALANCE);
    printf("2. Move Money from Wallet into SAFE BANK (Max $%d)\n", MAX_BANK_BALANCE);
    printf("3. Withdraw Money from SAFE BANK back to Wallet\n");
    printf("0. Return to Main Menu\n");
    printf("Choice: ");

    int sub_choice = get_safe_int();

    if (sub_choice == 1) {
        // פעולה 1: הפקדת כסף מבחוץ לתוך הארנק (ATM)
        if (p->balance >= MAX_BALANCE) {
            printf("\n" C_YELLOW "Wallet is already full." C_RESET "\n");
            delay_ms(2000);
            return;
        }

        printf("\nEnter amount to deposit to wallet: $");
        int amount = get_safe_int();
        if (amount <= 0) return;

        if (p->balance + amount > MAX_BALANCE) {
            display_error(2000, "Exceeds wallet limit! You can only hold up to $%d in your wallet.", MAX_BALANCE);
        }
        else {
            // כאן רק מוסיפים לארנק, אין קשר לבנק
            p->balance += amount;
            printf("" C_GREEN "Successfully deposited $%d into your wallet!" C_RESET "\n", amount);
            save_player(p);
            delay_ms(1500);
        }
    }
    else if (sub_choice == 2) {
        // פעולה 2: העברת כסף מהארנק לתוך הכספת בבנק
        printf("\nEnter amount to move into the Safe Bank (0 to cancel): $");
        int amount = get_safe_int();
        if (amount <= 0) return;

        // הגנה 1: אי אפשר להעביר לבנק כסף שאין לך בארנק
        if (amount > p->balance) {
            display_error(2000, "You don't have that much money in your wallet!");
        }
        // הגנה 2: חסימת הצפת הכספת (Bank Overflow)
        else if (p->bank_balance + amount > MAX_BANK_BALANCE) {
            display_error(2500, "Bank vault capacity reached! Maximum allowed in safe is $%d.", MAX_BANK_BALANCE);
        }
        else {
            // הכל תקין - העברה מהארנק לבנק
            p->balance -= amount;
            p->bank_balance += amount;
            printf("" C_GREEN "$%d safely moved to your Bank account!" C_RESET "\n", amount);
            save_player(p);
            delay_ms(2000);
        }
    }
    else if (sub_choice == 3) {
        // פעולה 3: משיכה מהבנק לארנק
        handle_withdrawal(p);
    }
}

void add_balance_safe(Player* p, int amount) {
    if (amount <= 0) return;

    // שימוש ב-long long למניעת גלישה נומרית בזמן הבדיקה
    long long potential_balance = (long long)p->balance + amount;

    if (potential_balance > MAX_BALANCE) {
        int overflow = (int)(potential_balance - MAX_BALANCE);
        p->balance = MAX_BALANCE;

        // בדיקה האם חשבון הבנק יכול לקלוט את העודף
        if ((long long)p->bank_balance + overflow > MAX_BANK_BALANCE) {
            int allowed_to_bank = MAX_BANK_BALANCE - p->bank_balance;
            p->bank_balance = MAX_BANK_BALANCE;

            printf("\n" C_RED "CRITICAL: Casino Vault limits exceeded!" C_RESET "\n");
            printf("Wallet reached MAX ($%d). Bank reached MAX ($%d).\n", MAX_BALANCE, MAX_BANK_BALANCE);
            printf("$%d evaporated due to banking regulations.\n", overflow - allowed_to_bank);
        }
        else {
            p->bank_balance += overflow;
            printf("\n" C_YELLOW "Wallet limit ($%d) reached!" C_RESET "\n", MAX_BALANCE);
            printf("$%d automatically redirected to your SAFE BANK account.\n", overflow);
        }
    }
    else {
        p->balance += amount;
    }
    save_player(p); // שמירה מיידית ומאובטחת של ה-State החדש
}

