#include <stdio.h>
#include <stdlib.h>
#include "utils.h" // כדי להשתמש בצבעים שלנו
#include "graphics.h"
#include "casino.h"

// ==========================================
// ROULETTE GRAPHICS
// ==========================================
void print_roulette_welcome() {
    clear_screen();
    printf("%s\n", C_RED);
    printf("  _______  _______  __   __  ___      _______  _______  _______  _______ \n");
    printf(" |       ||       ||  | |  ||   |    |       ||       ||       ||       |\n");
    printf(" |    _  ||   _   ||  | |  ||   |    |    ___||_     _||_     _||    ___|\n");
    printf(" |   |_| ||  | |  ||  |_|  ||   |    |   |___   |   |    |   |  |   |___ \n");
    printf(" |    __ <|  |_|  ||       ||   |___ |    ___|  |   |    |   |  |    ___|\n");
    printf(" |   |  | |       ||       ||       ||   |___   |   |    |   |  |   |___ \n");
    printf(" |___|  |_|_______||_______||_______||_______|  |___|    |___|  |_______|\n");
    printf("%s\n", C_RESET);

    printf(C_CYAN "=========================================================================" C_RESET "\n");
    printf("                       " C_YELLOW "TABLE RULES & PAYOUTS" C_RESET "\n");
    printf(C_CYAN "=========================================================================" C_RESET "\n");
    printf(" * Straight Up (1 Number)  : Pays 35 to 1\n");
    printf(" * Split (2 Numbers)       : Pays 17 to 1\n");
    printf(" * Corner (4 Numbers)      : Pays  8 to 1\n");
    printf(" * Dozens (12 Numbers)     : Pays  2 to 1\n");
    printf(" * Color / Even / Odd      : Pays  1 to 1\n");
    printf(" * Halves (1-18 / 19-36)   : Pays  1 to 1\n");
    printf(C_CYAN "=========================================================================" C_RESET "\n\n");

    printf(C_GREEN "Press ENTER to acknowledge rules and join the table..." C_RESET "");
    wait_for_enter();
    clear_screen();
}


void print_roulette_board() {
    printf("\n");
    printf(BG_GREEN TEXT_WHITE "   00    " C_RESET);
    for (int i = 3; i <= 36; i += 3) {
        if (get_number_color(i) == 1) printf(BG_RED TEXT_WHITE "  %2d   " C_RESET, i);
        else printf(BG_BLACK TEXT_WHITE "  %2d   " C_RESET, i);
    }
    printf("\n");

    printf(BG_GREEN TEXT_WHITE "         " C_RESET);
    for (int i = 2; i <= 35; i += 3) {
        if (get_number_color(i) == 1) printf(BG_RED TEXT_WHITE "  %2d   " C_RESET, i);
        else printf(BG_BLACK TEXT_WHITE "  %2d   " C_RESET, i);
    }
    printf("\n");

    printf(BG_GREEN TEXT_WHITE "    0    " C_RESET);
    for (int i = 1; i <= 34; i += 3) {
        if (get_number_color(i) == 1) printf(BG_RED TEXT_WHITE "  %2d   " C_RESET, i);
        else printf(BG_BLACK TEXT_WHITE "  %2d   " C_RESET, i);
    }
    printf("\n");

    printf("         ");
    printf(BG_GREEN TEXT_WHITE "           1st 12           " C_RESET);
    printf(BG_GREEN TEXT_WHITE "           2nd 12           " C_RESET);
    printf(BG_GREEN TEXT_WHITE "           3rd 12           " C_RESET);
    printf("\n");

    printf("         ");
    printf(BG_GREEN TEXT_WHITE "     1-18     " C_RESET);
    printf(BG_GREEN TEXT_WHITE "     Even     " C_RESET);
    printf(BG_RED TEXT_WHITE   "     Red      " C_RESET);
    printf(BG_BLACK TEXT_WHITE "    Black     " C_RESET);
    printf(BG_GREEN TEXT_WHITE "     Odd      " C_RESET);
    printf(BG_GREEN TEXT_WHITE "     19-36    " C_RESET);
    printf("\n\n");
}

// ==========================================
// BLACKJACK GRAPHICS
// ==========================================
 void print_blackjack_welcome() {
    clear_screen();
    printf("\x1b[97m"); 
    printf("  ______  ___      _______  _______  ___   _    ____  _______  _______  ___   _ \n");
    printf(" |  __  \\|   |    |       ||       ||   | | |  |_   ||       ||       ||   | | |\n");
    printf(" | |__| ||   |    |   _   ||       ||   |_| |    |  ||   _   ||       ||   |_| |\n");
    printf(" |      /|   |    |  |_|  ||       ||      _|    |  ||  |_|  ||       ||      _|\n");
    printf(" |  __  \\|   |___ |       ||      _||     |_  _  |  ||       ||      _||     |_ \n");
    printf(" | |__| ||       ||   _   ||     |_ |    _  || |_|  ||   _   ||     |_ |    _  |\n");
    printf(" |______/|_______||__| |__||_______||___| |_||______||__| |__||_______||___| |_|\n");
    printf("" C_RESET "\n");

    printf("" C_CYAN "=========================================================================" C_RESET "\n");
    printf("                       " C_YELLOW "TABLE RULES & PAYOUTS" C_RESET "\n");
    printf("" C_CYAN "=========================================================================" C_RESET "\n");
    printf(" * Blackjack (21 on first 2 cards) : Pays 3 to 2\n");
    printf(" * Standard Win                    : Pays 1 to 1\n");
    printf(" * Dealer Rules                    : Dealer must hit on soft 17\n");
    printf(" * Splitting                       : Allowed on matching pairs\n");
    printf(" * Double Down                     : Allowed on initial hand\n");
    printf("" C_CYAN "=========================================================================" C_RESET "\n\n");

    printf("" C_GREEN "Press ENTER to acknowledge rules and join the table..." C_RESET "");
    wait_for_enter();
    clear_screen();
}

// ==========================================
// POKER GRAPHICS
// ==========================================
 void print_poker_welcome() {
     clear_screen();
     printf("" C_MAGENTA "");
     printf("  _    _ _   _   _                 _       \n");
     printf(" | |  | | | | | (_)               | |      \n");
     printf(" | |  | | | |_| |_ _ __ ___   __ _| |_ ___ \n");
     printf(" | |  | | | __| | | '_ ` _ \\ / _` | __/ _ \\\n");
     printf(" | |__| | | |_| | | | | | | | (_| | ||  __/\n");
     printf("  \\____/|_|\\__|_|_|_| |_| |_|\\__,_|\\__\\___|\n");
     printf("        T E X A S   H O L D ' E M          \n" C_RESET "");

     printf("" C_CYAN "=========================================================================" C_RESET "\n");
     printf("                       " C_YELLOW "ULTIMATE RULES & PAYOUTS" C_RESET "\n");
     printf("" C_CYAN "=========================================================================" C_RESET "\n");
     printf(" * Place equal ANTE and BLIND bets to receive cards.\n");
     printf(" * Optional TRIPS bet: Pays on 3-of-a-Kind or better, win or lose!\n");
     printf(" * PRE-FLOP : Check, or Play 3x/4x Ante.\n");
     printf(" * FLOP     : Check, or Play 2x Ante.\n");
     printf(" * RIVER    : Play 1x Ante, or FOLD (lose Ante & Blind).\n");
     printf("" C_CYAN "=========================================================================" C_RESET "\n\n");

     printf("" C_GREEN "Press ENTER to sit at the table..." C_RESET "");
     wait_for_enter();
     clear_screen();
 }

// ==========================================
// SLOTS GRAPHICS
// ==========================================
 void print_slots_welcome() {
     clear_screen();
     printf("" C_YELLOW "");
     printf("  ____  _      ___ _____  ____  \n");
     printf(" / ___|| |    / _ \\_   _|/ ___| \n");
     printf(" \\___ \\| |   | | | || |  \\___ \\ \n");
     printf("  ___) | |___| |_| || |   ___) |\n");
     printf(" |____/|_____|\\___/ |_|  |____/ \n");
     printf("" C_RESET "\n");

     printf("" C_CYAN "=========================================================================" C_RESET "\n");
     printf("                       " C_YELLOW "PAYTABLE & RULES" C_RESET "\n");
     printf("" C_CYAN "=========================================================================" C_RESET "\n");
     printf(" * THREE 7s [ 7 ][ 7 ][ 7 ] : JACKPOT! Pays 50 to 1\n");
     printf(" * THREE of a kind          : BIG WIN! Pays 10 to 1\n");
     printf(" * TWO 7s anywhere          : Pays 5 to 1\n");
     printf(" * TWO of a kind            : Pays 2 to 1\n");
     printf("" C_CYAN "=========================================================================" C_RESET "\n\n");

     printf("" C_GREEN "Press ENTER to pull the lever..." C_RESET "");
     wait_for_enter();
     clear_screen();
 }
 // פונקציה שמציירת את המכונה עם האנימציה
  void draw_slot_machine(int s1, int s2, int s3) {
     printf("\n");
     printf("   .-----------------------.\n");
     printf("   |  " C_YELLOW "C A S I N O   S L O T" C_RESET "  |\n");
     printf("   |-----------------------|\n");
     printf("   |                       |\n");
     printf("   |   %s %s %s  |\n", slot_symbols[s1], slot_symbols[s2], slot_symbols[s3]);
     printf("   |                       |\n");
     printf("   '-----------------------'\n");
 }

 // מערך של סמלים למכונת המזל
 const char* slot_symbols[] = {
     "" C_RED "[ 7 ]" C_RESET "", // 0: שבע אדום (ג'קפוט)
     "" C_YELLOW "[ $ ]" C_RESET "", // 1: דולר זהב
     "" C_GREEN "[ # ]" C_RESET "", // 2: סולמית ירוקה
     "" C_MAGENTA "[ @ ]" C_RESET "", // 3: שטרודל סגול
     "" C_CYAN "[ * ]" C_RESET ""  // 4: כוכב תכלת
 };

// ==========================================
// FOOTBALL GRAPHICS
// ==========================================
 void print_football_welcome() {
     clear_screen();
     printf("" C_GREEN "");
     printf("  _    _    _ _____ _   _ _   _ ______ _____  \n");
     printf(" | |  | |  | |_   _| \\ | | \\ | |  ____|  __ \\ \n");
     printf(" | |  | |  | | | | |  \\| |  \\| | |__  | |__) |\n");
     printf(" | |  | |  | | | | | . ` | . ` |  __| |  _  / \n");
     printf(" | |__| |__| |_| |_| |\\  | |\\  | |____| | \\ \\ \n");
     printf("  \\_________/|_____|_| \\_|_| \\_|______|_|  \\_\\\n");
     printf("\n             W I N N E R   S L I P         \n" C_RESET "");

     printf("" C_CYAN "=========================================================================" C_RESET "\n");
     printf("                       " C_YELLOW "TABLE RULES & HOW TO WIN" C_RESET "\n");
     printf("" C_CYAN "=========================================================================" C_RESET "\n");
     printf(" * You are filling a 3-match betting slip.\n");
     printf(" * For each match, guess the outcome:\n");
     printf("   - Press [1] for Home Team Win\n");
     printf("   - Press [X] for a Draw\n");
     printf("   - Press [2] for Away Team Win\n");
     printf("   - Press [S] to SKIP this match (it won't count toward your slip)\n");
     printf(" * Total odds multiply across all active matches on the slip!\n");
     printf(" * To win, you must guess ALL active match outcomes correctly.\n");
     printf("" C_CYAN "=========================================================================" C_RESET "\n\n");

     printf("" C_GREEN "Press ENTER to get your betting slip..." C_RESET "");
     wait_for_enter();
     clear_screen();
 }