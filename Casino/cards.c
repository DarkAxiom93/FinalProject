#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>
#include "cards.h"
#include "utils.h"

// ייצור חפיסות קלפים דינמי (מקבל כמה חפיסות שרוצים - 1 לפוקר, 6 לבלאק ג'ק)
Card* create_deck(int num_decks) {
    size_t total_cards = (size_t)num_decks * 52;
    Card* deck = (Card*)safe_malloc(total_cards * sizeof(Card));
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
    return deck;
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