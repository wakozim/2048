#ifndef GAME_H_
#define GAME_H_

#define BOARD_SIZE 4
#define BOARD_CAP (BOARD_SIZE * BOARD_SIZE)
#define ROWS (BOARD_SIZE)
#define COLUMNS (BOARD_SIZE)

int cell_at(int x, int y);
void set_cell_at(int x, int y, int value);
int movement_at(int x, int y);
int get_score(void);
void reset_score(void);
void save_back_board(void);
void clear_board(void);
bool swipe_board_right(void);
bool swipe_board_left(void);
bool swipe_board_down(void);
bool swipe_board_up(void);
void print_board(void);
void add_random_cell(void);

#endif // GAME_H_
