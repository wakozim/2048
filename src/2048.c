#include <stdbool.h>
#include "2048.h"
#include <stdlib.h>

#ifndef PLATFORM_WEB
    #include <stdio.h>
#endif // PLATFORM_WEB


static int board[ROWS][COLUMNS]          = {0};
static int back_board[ROWS][COLUMNS]     = {0};
static int movement_board[ROWS][COLUMNS] = {0};
static int score                         = 0;

int get_score(void)
{
    return score;
}

void reset_score(void)
{
    score = 0;
}

int cell_at(int x, int y) {
    return back_board[y][x];
}

void set_cell_at(int x, int y, int value) {
    board[y][x] = value;
}

int movement_at(int x, int y) {
    return movement_board[y][x];
}

void save_back_board(void)
{
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            back_board[y][x] = board[y][x];
        }
    }
}

void clear_movement_board(void) {
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            movement_board[y][x] = 0;
        }
    }
}

void clear_board(void)
{
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            board[y][x] = 0; 
        }
    }
    clear_movement_board();
}


void add_random_cell(void) {
    bool indexes[BOARD_CAP] = {0};
    int i = -1;
    int tryes = 0;

    do {
        i = rand() % BOARD_CAP;
        if (!indexes[i]) {
            ++tryes;
            indexes[i] = true;
            if (tryes >= BOARD_CAP) {
                i = -1;
                break;
            } 
        }
    } while (board[i/BOARD_SIZE][i%BOARD_SIZE] != 0);

    if (i < 0 || i >= BOARD_CAP) return;
    board[i/BOARD_SIZE][i%BOARD_SIZE] = 2;
}

#ifndef PLATFORM_WEB
void print_movement_board(void)
{
    printf("-------------------\n");
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLUMNS; ++col) {
            printf("%d ", movement_board[row][col]);
        }
        printf("\n");
    }
    printf("-------------------\n");
}

void print_board(void)
{
    printf("-----[ BOARD ]-------\n");
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLUMNS; ++col) {
            printf("%d ", board[row][col]);
        }
        printf("\n");
    }
    printf("-------------------\n");
}
#endif // PLATFORM_WEB

// Rotate the board 90 degrees clockwise
void rotate_board(void)
{
    for (int i = 0; i < BOARD_SIZE/2; ++i) {
        for (int j = i; j < BOARD_SIZE - i - 1; ++j) {
            int temp = board[i][j];
            board[i][j] = board[BOARD_SIZE - 1 - j][i];
            board[BOARD_SIZE - 1 - j][i] = board[BOARD_SIZE - 1 - i][BOARD_SIZE - 1 - j];
            board[BOARD_SIZE - 1 - i][BOARD_SIZE - 1 - j] = board[j][BOARD_SIZE - 1 - i];
            board[j][BOARD_SIZE - 1 - i] = temp;

            int movement_temp = movement_board[i][j];
            movement_board[i][j] = movement_board[BOARD_SIZE - 1 - j][i];
            movement_board[BOARD_SIZE - 1 - j][i] = movement_board[BOARD_SIZE - 1 - i][BOARD_SIZE - 1 - j];
            movement_board[BOARD_SIZE - 1 - i][BOARD_SIZE - 1 - j] = movement_board[j][BOARD_SIZE - 1 - i];
            movement_board[j][BOARD_SIZE - 1 - i] = movement_temp;
        } 
    }
}


bool swipe_board_right(void)
{
    clear_movement_board();
    bool is_board_swiped = false;
    for (int y = 0; y < ROWS; ++y) {
        for (int x = COLUMNS - 1; x >= 0; --x) {
            bool use_value = board[y][x] == 0 ? false : true;
            int value = board[y][x];
            int nx = -1;
            for (int sx = x - 1; sx >= 0; --sx) {
                if (use_value) {
                  if (board[y][sx] == value) {
                      nx = sx; 
                      break;
                  } else if (board[y][sx] != 0) {
                      break;
                  }
                } else {
                  if (board[y][sx] != 0) {
                      nx = sx;
                      break;
                  }
                }
            } 
            if (nx < 0) {
                continue;
            }
            is_board_swiped = true;
            if (use_value) {
                board[y][x] += value;
                score += value*2;
            } else {
                board[y][x] = board[y][nx];
            }
            movement_board[y][nx] = x - nx;
            board[y][nx] = 0;
            if (!use_value) ++x;
        }
    }
    return is_board_swiped;
}


bool swipe_board_left(void) {
    rotate_board(); 
    rotate_board(); 
    bool is_board_swiped = swipe_board_right();
    rotate_board(); 
    rotate_board(); 
    return is_board_swiped;
}


bool swipe_board_down(void) {
    rotate_board();
    rotate_board(); 
    rotate_board(); 
    bool is_board_swiped = swipe_board_right();
    rotate_board(); 
    return is_board_swiped;
}


bool swipe_board_up(void) {
    rotate_board(); 
    bool is_board_swiped = swipe_board_right();
    rotate_board(); 
    rotate_board(); 
    rotate_board(); 
    return is_board_swiped;
}
