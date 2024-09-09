#include <stdlib.h>
#include "raylib.h"
#include <math.h>
#include "2048.h"
#include "raymath.h"

#ifndef PLATFORM_WEB
    #include <stdio.h>
    #include <time.h>
    #include <string.h>
#endif

#define CELL_SIZE 100
#define CELL_GAP 10
#define FIELD_WIDTH ((COLUMNS * CELL_SIZE) + ((COLUMNS - 1) * CELL_GAP))
#define FIELD_HEIGHT ((ROWS * CELL_SIZE) + ((ROWS - 1) * CELL_GAP))
#define FIELD_GAP 50


int count_number_digits(long int num)
{
    int count = 0;
    for (; num > 0; num /= 10)
        ++count;
    return count;
}


void draw_number(int number, int x, int y, int font_size, Color color)
{
    int spacing = font_size/10;
    int digits = count_number_digits(number);
    int temp = pow(10, digits - 1);
    for (; number > 0; number %= temp, temp /= 10) {
        //printf("%d - %d\n", number, temp);
        switch (number / temp) {
            case 0: {
                DrawText("0", x, y, font_size, color);
                x += MeasureText("0", font_size) + spacing;
            } break;
            case 1: {
                DrawText("1", x, y, font_size, color);
                x += MeasureText("1", font_size) + spacing;
            } break;
            case 2: {
                DrawText("2", x, y, font_size, color);
                x += MeasureText("2", font_size) + spacing;
            } break;
            case 3: {
                DrawText("3", x, y, font_size, color);
                x += MeasureText("3", font_size) + spacing;
            } break;
            case 4: {
                DrawText("4", x, y, font_size, color);
                x += MeasureText("4", font_size) + spacing;
            } break;
            case 5: {
                DrawText("5", x, y, font_size, color);
                x += MeasureText("5", font_size) + spacing;
            } break;
            case 6: {
                DrawText("6", x, y, font_size, color);
                x += MeasureText("6", font_size) + spacing;
            } break;
            case 7: {
                DrawText("7", x, y, font_size, color);
                x += MeasureText("7", font_size) + spacing;
            } break;
            case 8: {
                DrawText("8", x, y, font_size, color);
                x += MeasureText("8", font_size) + spacing;
            } break;
            case 9: {
                DrawText("9", x, y, font_size, color);
                x += MeasureText("9", font_size) + spacing;
            } break;
                    
        }
    }
    //abort();
}


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


void draw_move_cells(void)
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
                //DrawText(text, tx, ty, font_size, BLACK);
                draw_number(cell_value, tx, ty, font_size, BLACK);
            }
        }
    }
}

void draw_board(void)
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
                //DrawText(text, tx, ty, font_size, BLACK);
                draw_number(cell_value, tx, ty, font_size, BLACK);

            } else {
                DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, (cx + cy) % 2 ? GRAY : GRAY );
            }
        }
    }
    if (game_state != GAME_PLAY) draw_move_cells();
}


void raylib_js_set_entry(void (*entry)(void));

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
        ClearBackground(RAYWHITE);
    
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
#if 0
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
            
            if (IsKeyPressed(KEY_R)) {
                clear_board();
                add_random_cell();
                save_back_board();
            }
#endif
        } else {
            game_time += GetFrameTime();
            if (game_time > max_time) {
                add_random_cell();
                save_back_board();
                game_state = GAME_PLAY;
                game_time = 0.0f;
            }
        }
#define TEXT_SIZE 25
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
