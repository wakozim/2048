#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#define BOARD_SIZE 4
#define ROWS (BOARD_SIZE)
#define COLUMNS (BOARD_SIZE)

#define CELL_SIZE 100
#define CELL_GAP 10
#define FIELD_WIDTH ((COLUMNS * CELL_SIZE) + ((COLUMNS - 1) * CELL_GAP))
#define FIELD_HEIGHT ((ROWS * CELL_SIZE) + ((ROWS - 1) * CELL_GAP))


int rand_int(int max)
{
    return rand() % max;
}


static const char *labels[] = {"1", "2", "4", "8", "16", "32", "64", "128", "256", "514", "1024", "2048"};
static int field[ROWS][COLUMNS] = {0};
static int prev_field[ROWS][COLUMNS] = {0};


void add_random_cell() {
    int y = rand_int(ROWS);    
    int x = rand_int(COLUMNS);    
    if (field[y][x] == 0) field[y][x] = 2;
}

void print_board()
{
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLUMNS; ++col) {
            printf("%d ", field[row][col]);
        }
        printf("\n");
    }
}

// Rotate the board 90 degrees clockwise
void rotate_board()
{
    for (int i = 0; i < BOARD_SIZE/2; ++i) {
        for (int j = i; j < BOARD_SIZE - i - 1; ++j) {
            int temp = field[i][j];
            field[i][j] = field[BOARD_SIZE - 1 - j][i];
            field[BOARD_SIZE - 1 - j][i] = field[BOARD_SIZE - 1 - i][BOARD_SIZE - 1 - j];
            field[BOARD_SIZE - 1 - i][BOARD_SIZE - 1 - j] = field[j][BOARD_SIZE - 1 - i];
            field[j][BOARD_SIZE - 1 - i] = temp;
        } 
    }
}


typedef enum {
    ANY = 0,
    NON_ZERO,
    VALUE,
} Target;

bool swipe_board_right()
{
    bool is_board_swiped = false;
    for (int y = 0; y < ROWS; ++y) {
        for (int x = COLUMNS - 1; x >= 0; --x) {
            bool use_value = field[y][x] == 0 ? false : true;
            int value = field[y][x];
            int nx = -1;
            for (int sx = x - 1; sx >= 0; --sx) {
                if (use_value) {
                  if (field[y][sx] == value) {
                      nx = sx; 
                  } else if (field[y][sx] != 0) {
                      break;
                  }
                } else {
                  if (field[y][sx] != 0) {
                      nx = sx;
                      break;
                  }
                }
            } 
            if (nx < 0) continue;
            is_board_swiped = true;
            if (use_value) {
                field[y][x] += value;
            } else {
                field[y][x] = field[y][nx];
            }
            field[y][nx] = 0;
            if (!use_value) ++x;
        }
    }
    return is_board_swiped;
}


bool swipe_board_left() {
    rotate_board(); 
    rotate_board(); 
    bool is_board_swiped = swipe_board_right();
    rotate_board(); 
    rotate_board(); 
    return is_board_swiped;
}


bool swipe_board_down() {
    rotate_board();
    rotate_board(); 
    rotate_board(); 
    bool is_board_swiped = swipe_board_right();
    rotate_board(); 
    return is_board_swiped;
}


bool swipe_board_up() {
    rotate_board(); 
    bool is_board_swiped = swipe_board_right();
    rotate_board(); 
    rotate_board(); 
    rotate_board(); 
    return is_board_swiped;

}


void save_prev_board() {
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLUMNS; ++x) {
            prev_field[y][x] = field[y][x];
        }
    }    
}


void flip_boards() {
    for (int y = 0; y < ROWS; ++y) {
        for (int x = 0; x < COLUMNS; ++x) {
            field[y][x] = prev_field[y][x];
        }
    }
}


int main() 
{
    Color cell_colors[] = {WHITE, BROWN, PURPLE, VIOLET, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN, LIME, DARKGREEN, SKYBLUE};
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(600, 600, "2048");
    SetTargetFPS(60);
    
    int sx = GetScreenWidth()/2 - FIELD_WIDTH/2;
    int sy = GetScreenHeight()/2 - FIELD_HEIGHT/2;    
    
    field[0][0] = 2; 
    field[1][0] = 2; 
    //field[2][0] = 2; 
    field[3][0] = 4; 
    //field[3][0] = 8; 
    //add_random_cell();
    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            for (int cy = 0; cy < ROWS; cy++) {
                for (int cx = 0; cx < COLUMNS; cx++) {
                    int x = sx + (cx * CELL_SIZE) + ((cx - 1) * CELL_GAP);
                    int y = sy + (cy * CELL_SIZE) + ((cy - 1) * CELL_GAP);
                    if (field[cy][cx] != 0) {
                        int value = field[cy][cx];
                        const char *text = labels[(int)log2(value)];
                        int font_size = 40;
                        int text_width = MeasureText(text, font_size);
                        int tx = x + CELL_SIZE/2 - text_width/2;
                        int ty = y + CELL_SIZE/2 - font_size/2;
                        DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, cell_colors[(int)log2(value)]);
                        DrawText(text, tx, ty, font_size, BLACK);
                    } else {
                        DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, (cx + cy) % 2 ? GRAY : GRAY );
                    }
                }
            }
             
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
                save_prev_board();
                if (swipe_board_right()) 
                    add_random_cell();
            }
            if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
                save_prev_board();
                if (swipe_board_down())
                    add_random_cell();
            }
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
                save_prev_board();
                if (swipe_board_left()) 
                    add_random_cell();
            }
            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
                save_prev_board();
                if (swipe_board_up()) 
                    add_random_cell(); 
            }
            if (IsKeyPressed(KEY_Z)) {
                flip_boards();
            }
        EndDrawing();
    }
    CloseWindow();
}
