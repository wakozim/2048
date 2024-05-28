#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include <math.h>
#include "2048.h"
#include "raymath.h"
#include <time.h>


#define CELL_SIZE 100
#define CELL_GAP 10
#define FIELD_WIDTH ((COLUMNS * CELL_SIZE) + ((COLUMNS - 1) * CELL_GAP))
#define FIELD_HEIGHT ((ROWS * CELL_SIZE) + ((ROWS - 1) * CELL_GAP))
#define FIELD_GAP 50


typedef enum {
    GAME_PLAY,
    GAME_MOVE_CELLS_LEFT,
    GAME_MOVE_CELLS_RIGHT,
    GAME_MOVE_CELLS_DOWN,
    GAME_MOVE_CELLS_UP,
} State;

static State game_state = GAME_PLAY;
static float game_time = 0.0f;
static float max_time = 0.1f;

float in_out_cubic(float x)
{
    return x < 0.5 ? 4 * x * x * x : 1 - pow(-2 * x + 2, 3) / 2;
}

void draw_move_cells()
{
    Color cell_colors[] = {WHITE, BROWN, PURPLE, VIOLET, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN, LIME, DARKGREEN, SKYBLUE};
    int sx = GetScreenWidth()/2 - FIELD_WIDTH/2;
    int sy = GetScreenHeight()/2 - FIELD_HEIGHT/2;   
    for (int cy = 0; cy < ROWS; cy++) {
        for (int cx = 0; cx < COLUMNS; cx++) {
            int offset = movement_at(cx, cy);
            int cell_value = cell_at(cx, cy);
            if (offset == 0) {
                continue;
            }
            int start_x = sx + (cx * CELL_SIZE) + (cx * CELL_GAP);
            int start_y = sy + (cy * CELL_SIZE) + (cy * CELL_GAP);
            int x = start_x;
            int y = start_y;

            if (game_state == GAME_MOVE_CELLS_RIGHT) {
                int end_x = sx + ((cx + offset) * CELL_SIZE) + ((cx+offset) * CELL_GAP);
                x = Lerp(start_x, end_x, in_out_cubic(game_time/max_time));            
            } else if (game_state == GAME_MOVE_CELLS_LEFT) {
                int end_x = sx + ((cx-offset) * CELL_SIZE) + ((cx-offset) * CELL_GAP);
                x = Lerp(start_x, end_x, in_out_cubic(game_time/max_time)); 
            } else if (game_state == GAME_MOVE_CELLS_UP) {
                int end_y = sy + ((cy-offset) * CELL_SIZE) + ((cy-offset) * CELL_GAP);
                y = Lerp(start_y, end_y, in_out_cubic(game_time/max_time));
            } else if (game_state == GAME_MOVE_CELLS_DOWN) {
                int end_y = sy + ((cy+offset) * CELL_SIZE) + ((cy+offset) * CELL_GAP);
                y = Lerp(start_y, end_y, in_out_cubic(game_time/max_time));
            }
            
            DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, cell_colors[(int)log2(cell_value)]);
            if (cell_value != 0) {
                int font_size = 40;
                const char *text = TextFormat("%d", cell_value); 
                int text_width = MeasureText(text, font_size);
                int tx = x + CELL_SIZE/2 - text_width/2;
                int ty = y + CELL_SIZE/2 - font_size/2;
                DrawText(text, tx, ty, font_size, BLACK);
            }
        }
    }
}

void draw_board()
{
    Color cell_colors[] = {WHITE, BROWN, PURPLE, VIOLET, YELLOW, GOLD, ORANGE, PINK, RED, MAROON, GREEN, LIME, DARKGREEN, SKYBLUE};
    int sx = GetScreenWidth()/2 - FIELD_WIDTH/2;
    int sy = GetScreenHeight()/2 - FIELD_HEIGHT/2;

    for (int cy = 0; cy < ROWS; cy++) {
        for (int cx = 0; cx < COLUMNS; cx++) {
            int cell_value = cell_at(cx, cy);
            int start_x = sx + (cx * CELL_SIZE) + (cx * CELL_GAP);
            int start_y = sy + (cy * CELL_SIZE) + (cy * CELL_GAP);
            int x = start_x;
            int y = start_y;
            if (cell_value != 0) {
                int font_size = 40;
                const char *text = TextFormat("%d", cell_value); 
                int text_width = MeasureText(text, font_size);
                int tx = x + CELL_SIZE/2 - text_width/2;
                int ty = y + CELL_SIZE/2 - font_size/2;
                if (game_state != GAME_PLAY) {
                    int x_offset = movement_at(cx, cy);
                    if (x_offset > 0) { 
                        DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, (cx + cy) % 2 ? GRAY : GRAY );
                        continue;
                    };
                }

                DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, cell_colors[(int)log2(cell_value)]);
                DrawText(text, tx, ty, font_size, BLACK);

            } else {
                DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, (cx + cy) % 2 ? GRAY : GRAY );
            }
        }
    }
    if (game_state != GAME_PLAY) draw_move_cells();
}


int main() 
{
    srand(time(0));
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(FIELD_WIDTH + FIELD_GAP*2, FIELD_HEIGHT + FIELD_GAP*2, "2048");
    SetTargetFPS(60);    
    
    add_random_cell();
    save_back_board();

    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            draw_board();           
             
            if (game_state == GAME_PLAY) {
            if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
                save_back_board();
                if (swipe_board_right()) { 
                    game_state = GAME_MOVE_CELLS_RIGHT;
                }
            }
            if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
                save_back_board();
                if (swipe_board_down()) {
                    game_state = GAME_MOVE_CELLS_DOWN;
                }
            }
            if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
                save_back_board();
                if (swipe_board_left()) { 
                    game_state = GAME_MOVE_CELLS_LEFT;
                }
            }
            if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
                save_back_board();
                if (swipe_board_up()) { 
                    game_state = GAME_MOVE_CELLS_UP;
                }
            }

           } else {
                game_time += GetFrameTime();
                if (game_time > max_time) {
                    add_random_cell();
                    save_back_board();
                    game_state = GAME_PLAY;
                    game_time = 0.0f;
                }
            }
        EndDrawing();
    }
    CloseWindow();
}
