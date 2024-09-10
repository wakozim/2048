#include <stdlib.h>
#include <stddef.h>
#include <math.h>
#include "raylib.h"
#include "raymath.h"
#include "2048.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#ifndef PLATFORM_WEB
    #include <stdio.h>
    #include <time.h>
    #include <string.h>
#endif

#define CELL_VALUE_DEFAULT_FONT_SIZE 50
#define CELL_SIZE 100
#define CELL_GAP 10
#define FIELD_WIDTH ((COLUMNS * CELL_SIZE) + ((COLUMNS - 1) * CELL_GAP))
#define FIELD_HEIGHT ((ROWS * CELL_SIZE) + ((ROWS - 1) * CELL_GAP))
#define FIELD_GAP 50

#ifdef PLATFORM_WEB
    #define ROUNDNESS 0.05f
#else
    #define ROUNDNESS 0.15f
#endif

#define EMPTY_CELL_COLOR ColorFromHSV(0, 0.0f, 0.45f) 
#define BACKGROUND_COLOR ColorFromHSV(0, 0.0f, 0.20f) 
#define BOARD_COLOR      ColorFromHSV(0, 0.0f, 0.30f) 
#define TEXT_COLOR       ColorFromHSV(0, 0.0f, 0.90f)

void raylib_js_set_entry(void (*entry)(void));


const char *font_path = "./assets/fonts/Roboto-Bold.ttf";
Font font;


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

typedef enum {
    MOVE_LEFT,
    MOVE_DOWN,
    MOVE_RIGHT,
    MOVE_UP,
} Move;

static Move move_buffer[100] = {0};
static int move_buffer_size = 0;
static int move_buffer_start = 0;


void draw_cell_value(int cell_value, int x, int y)
{
    static char text_buffer[4096] = {0};
    stbsp_snprintf(text_buffer, sizeof(text_buffer), "%d", cell_value);
    Vector2 text_size = MeasureTextEx(font, text_buffer, CELL_VALUE_DEFAULT_FONT_SIZE, 1);
    Vector2 pos = {
        .x = x + CELL_SIZE/2 - text_size.x/2,
        .y = y + CELL_SIZE/2 - text_size.y/2,
    };
    DrawTextEx(font, text_buffer, pos, CELL_VALUE_DEFAULT_FONT_SIZE, 1, TEXT_COLOR);
}


Color get_cell_color(int cell_value)
{
    int hue = (int)(0 + log2(cell_value) * 10) % 360; 
    Color color = ColorFromHSV(hue, 0.45f, 0.85f);
    //Color color = cell_colors[(int)log2(cell_value)];
    return color;
}


void draw_move_cells(void)
{
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
            
            Rectangle cell_rect = {x, y, CELL_SIZE, CELL_SIZE};
            DrawRectangleRounded(cell_rect, ROUNDNESS, 0, get_cell_color(cell_value));
            if (cell_value != 0) {
                draw_cell_value(cell_value, x, y);
            }
        }
    }
}

void draw_board(void)
{
    int sx = GetScreenWidth()/2 - FIELD_WIDTH/2;
    int sy = GetScreenHeight()/2 - FIELD_HEIGHT/2;
    
    int margin = 20;
    Rectangle board_rec = {sx - margin, sy - margin, FIELD_WIDTH + margin*2, FIELD_HEIGHT + margin*2};
    //DrawRectangleRounded(board_rec, 0.05, 0, BLACK);
    DrawRectangleRec(board_rec, BOARD_COLOR);

    for (int cy = 0; cy < ROWS; cy++) {
        for (int cx = 0; cx < COLUMNS; cx++) {
            int cell_value = cell_at(cx, cy);
            int x = sx + (cx * CELL_SIZE) + (cx * CELL_GAP);
            int y = sy + (cy * CELL_SIZE) + (cy * CELL_GAP);
            Rectangle cell_rect = {x, y, CELL_SIZE, CELL_SIZE};
            if (cell_value != 0) { 
                if (game_state != GAME_PLAY) {
                    int x_offset = movement_at(cx, cy);
                    if (x_offset > 0) { 
                        DrawRectangleRounded(cell_rect, ROUNDNESS, 0, EMPTY_CELL_COLOR);
                        continue;
                    };
                }
                
                DrawRectangleRounded(cell_rect, ROUNDNESS, 0, get_cell_color(cell_value));
                draw_cell_value(cell_value, x, y);

            } else {
                DrawRectangleRounded(cell_rect, ROUNDNESS, 0, EMPTY_CELL_COLOR);
            }
        }
    }
    if (game_state != GAME_PLAY) draw_move_cells();
}


void queue_move(Move move)
{
    move_buffer[(move_buffer_start + move_buffer_size) % 100] = move;
    if (move_buffer_size < 100)
        move_buffer_size++;
    else
        move_buffer_start = (move_buffer_start + 1) % 100;
}

bool dequeue_move(Move *move)
{
    if (move_buffer_size == 0) return false;
    
    *move = move_buffer[move_buffer_start]; 
    move_buffer_size--;
    move_buffer_start = (move_buffer_start + 1) % 100;
    return true;
}


void capture_input(void)
{
    if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
        queue_move(MOVE_RIGHT); 
    }

    if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
        queue_move(MOVE_DOWN); 
    }

    if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
        queue_move(MOVE_LEFT); 
    }

    if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
        queue_move(MOVE_UP); 
    }
        
    if (IsKeyPressed(KEY_R)) {
        clear_board();
        add_random_cell();
        save_back_board();
        game_state = GAME_PLAY;
    } 
}


void game_frame(void)
{
    BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
    
        draw_board();           
        capture_input(); 

        if (game_state == GAME_PLAY) {
            Move move;
            if (dequeue_move(&move)) {
            switch (move) {
                case MOVE_RIGHT: {
                    save_back_board();
                    if (swipe_board_right()) { 
                        game_state = GAME_MOVE_CELLS_RIGHT;
                    }              
                } break;
                case MOVE_DOWN: {
                    save_back_board();
                    if (swipe_board_down()) {
                        game_state = GAME_MOVE_CELLS_DOWN;
                    }            
                } break;
                case MOVE_LEFT: {
                    save_back_board();
                    if (swipe_board_left()) { 
                        game_state = GAME_MOVE_CELLS_LEFT;
                    }           
                } break;
                case MOVE_UP: {
                    save_back_board();
                    if (swipe_board_up()) { 
                        game_state = GAME_MOVE_CELLS_UP;
                    }        
                } break;
                default: break;
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


int main(void) 
{
#ifndef PLATFORM_WEB
    srand(time(0));
    SetTraceLogLevel(LOG_WARNING);
#endif

    InitWindow(FIELD_WIDTH + FIELD_GAP*2, FIELD_HEIGHT + FIELD_GAP*2, "2048");
    SetTargetFPS(60);    
    
    font = LoadFontEx(font_path, CELL_VALUE_DEFAULT_FONT_SIZE, NULL, 0);
    
    add_random_cell();
    save_back_board();

#ifdef PLATFORM_WEB
    raylib_js_set_entry(game_frame);
#else
    while(!WindowShouldClose()) {
        game_frame();
    }
    CloseWindow();
#endif

    return 0;
}
