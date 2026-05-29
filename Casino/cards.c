#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cards.h"
#include "utils.h"

// ייצור חפיסות קלפים דינמי (מקבל כמה חפיסות שרוצים - 1 לפוקר, 6 לבלאק ג'ק)
void init_deck(Card* deck, int num_decks) {
    char suits[] = { 'H', 'D', 'C', 'S' };
    char* ranks_str[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A" };

    int index = 0;
    for (int d = 0; d < num_decks; d++) {
        for (int s = 0; s < 4; s++) {
            for (int r = 0; r < 13; r++) {
                deck[index].rank_val = r + 2;
                deck[index].suit = suits[s];
                strcpy(deck[index].str, ranks_str[r]);
                index++;
            }
        }
    }
}

// אלגוריתם ערבוב מקצועי (Fisher-Yates Shuffle)
void shuffle_deck(Card* deck, int total_cards) {
    for (int i = 0; i < total_cards; i++) {
        int r = i + rand() % (total_cards - i);
        Card temp = deck[i];
        deck[i] = deck[r];
        deck[r] = temp;
    }
}
void print_cards_ascii(Card* hand, int count, const char* title, int hide_second) {
    if (title != NULL) printf("\n--- %s ---\n", title);

    for (int i = 0; i < count; i++) printf("+-------+ ");
    printf("\n");

    for (int i = 0; i < count; i++) {
        if (hide_second && i == 1) printf("|#######| ");
        else printf("| %-2s    | ", hand[i].str);
    }
    printf("\n");

    for (int i = 0; i < count; i++) {
        if (hide_second && i == 1) printf("|#######| ");
        else {
            if (hand[i].suit == 'H' || hand[i].suit == 'D')
                printf("|   \x1b[31m%c\x1b[0m   | ", hand[i].suit);
            else
                printf("|   \x1b[97m%c\x1b[0m   | ", hand[i].suit);
        }
    }
    printf("\n");

    for (int i = 0; i < count; i++) {
        if (hide_second && i == 1) printf("|#######| ");
        else printf("|    %2s | ", hand[i].str);
    }
    printf("\n");

    for (int i = 0; i < count; i++) printf("+-------+ ");
    printf("\n");
}