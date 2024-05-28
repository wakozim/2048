#ifndef GAME_H_
#define GAME_H_

#define BOARD_SIZE 4
#define BOARD_CAP (BOARD_SIZE * BOARD_SIZE)
#define ROWS (BOARD_SIZE)
#define COLUMNS (BOARD_SIZE)

int cell_at();
void set_cell_at();
int movement_at();
void save_back_board();
bool swipe_board_right();
bool swipe_board_left();
bool swipe_board_down();
bool swipe_board_up();
void print_board();
void add_random_cell();


#endif // GAME_H_
