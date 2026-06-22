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
void draw_slot_machine_top();
void draw_slot_machine_row(int s1, int s2, int s3);
void draw_slot_machine_bottom();
void draw_slot_machine_cursor_to_row();
void draw_slot_machine_cursor_below();

#endif
