#include <stdio.h>
#include <stdlib.h>
#include "utils.h" // כדי להשתמש בצבעים שלנו
#include "graphics.h"

// ==========================================
// ROULETTE GRAPHICS
// ==========================================
// הדביקו כאן את פונקציית print_roulette_welcome במלואה
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
    printf(" * Street (3 Numbers)      : Pays 11 to 1\n");
    printf(" * Dozens (12 Numbers)     : Pays 2 to 1\n");
    printf(" * Color / Even / Odd      : Pays 1 to 1\n");
    printf(C_CYAN "=========================================================================" C_RESET "\n\n");

    printf(C_GREEN "Press ENTER to acknowledge rules and join the table..." C_RESET "");
    wait_for_enter();
    clear_screen();
}

// ==========================================
// BLACKJACK GRAPHICS
// ==========================================
// הדביקו כאן את פונקציית print_blackjack_welcome במלואה
 void print_blackjack_welcome() {
    clear_screen();
    printf("\x1b[97m"); // צבע לבן
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
// הדביקו כאן את פונקציית print_poker_welcome במלואה
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
// הדביקו כאן את פונקציית print_slots_welcome במלואה
 void print_slots_welcome() {
     clear_screen();
     printf("" C_YELLOW ""); // זהב
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

// ==========================================
// FOOTBALL GRAPHICS
// ==========================================
// הדביקו כאן את פונקציית print_football_welcome במלואה
 void print_football_welcome() {
     clear_screen();
     printf("" C_GREEN ""); // ירוק דשא
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