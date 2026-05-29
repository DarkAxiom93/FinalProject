#ifndef CARDS_H
#define CARDS_H

// מבנה קלף גנרי שמשותף לכל משחקי הקלפים בקזינו
typedef struct {
    int rank_val;   // 2 to 14 (11=J, 12=Q, 13=K, 14=A)
    char suit;      // 'H', 'D', 'C', 'S'
    char str[4];    // "2" to "10", "J", "Q", "K", "A"
} Card;

// פונקציות מנוע הקלפים האוניברסלי
void init_deck(Card* deck, int num_decks);
void shuffle_deck(Card* deck, int total_cards);

void print_cards_ascii(Card* hand, int count, const char* title, int hide_second);

#endif