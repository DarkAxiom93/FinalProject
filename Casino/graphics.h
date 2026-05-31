#ifndef GRAPHICS_H
#define GRAPHICS_H

// פונקציות תצוגה בלבד - ללא לוגיקה
void print_roulette_welcome();
void print_blackjack_welcome();
void print_poker_welcome();
void print_slots_welcome();
void print_football_welcome();
void print_roulette_board();
extern const char* slot_symbols[];
void draw_slot_machine(int s1, int s2, int s3);

#endif
