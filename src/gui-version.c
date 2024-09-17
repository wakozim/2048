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

#define CELL_SIZE 100
#define CELL_GAP 10
#define FIELD_MARGIN 20
#define FIELD_WIDTH ((COLUMNS * CELL_SIZE) + ((COLUMNS - 1) * CELL_GAP) + FIELD_MARGIN*2)
#define FIELD_HEIGHT ((ROWS * CELL_SIZE) + ((ROWS - 1) * CELL_GAP) + FIELD_MARGIN*2)
#define FIELD_GAP 25
#define SCORE_HEIGHT 70
#define SCORE_PAD 50
#define GAME_HEIGHT (SCORE_HEIGHT + FIELD_GAP + FIELD_HEIGHT)
#define BUTTON_SIZE SCORE_HEIGHT
#define ARROW_THICK 10
#define ARROW_PAD 10


#ifdef PLATFORM_WEB
    #define ROUNDNESS 0.05f
    #define CELL_VALUE_DEFAULT_FONT_SIZE 40
    #define SCORE_LABEL_TEXT_SIZE 17
    #define SCORE_VALUE_TEXT_SIZE 45
#else
    #define ROUNDNESS 0.15f
    #define CELL_VALUE_DEFAULT_FONT_SIZE 50
    #define SCORE_LABEL_TEXT_SIZE 20
    #define SCORE_VALUE_TEXT_SIZE 50
#endif

#define EMPTY_CELL_COLOR ColorFromHSV(0, 0.00f, 0.45f) 
#define BACKGROUND_COLOR ColorFromHSV(0, 0.00f, 0.20f) 
#define BOARD_COLOR      ColorFromHSV(0, 0.00f, 0.30f) 
#define TEXT_COLOR       ColorFromHSV(0, 0.00f, 0.90f)
#define ARROW_COLOR      ColorFromHSV(0, 0.55f, 0.85f)

void raylib_js_set_entry(void (*entry)(void));


const char *font_path = "./assets/fonts/Roboto-Bold.ttf";
Font default_font;
Font score_label_font;


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


void _draw_arrow(int x, int y, float angle, float angle2, Color arrow_color) 
{
    int arrow_radius = (BUTTON_SIZE - ARROW_PAD*2) / 2;
    int xx = x + cosf(angle) * (arrow_radius + 5);
    int yy = y + sinf(angle) * (arrow_radius + 5);

    int xx1 = x + cosf(angle) * (arrow_radius - ARROW_THICK - 5);
    int yy1 = y + sinf(angle) * (arrow_radius - ARROW_THICK - 5);

    int xx2 = x + cosf(angle2) * (arrow_radius - ARROW_THICK/2);
    int yy2 = y + sinf(angle2) * (arrow_radius - ARROW_THICK/2);

    Vector2 v1 = {xx, yy};
    Vector2 v2 = {xx1, yy1};
    Vector2 v3 = {xx2, yy2};
    DrawTriangle(v1, v3, v2, arrow_color);
}


void draw_cell_value(int cell_value, int x, int y)
{
    static char text_buffer[4096] = {0};
    stbsp_snprintf(text_buffer, sizeof(text_buffer), "%d", cell_value);
    Vector2 text_size = MeasureTextEx(default_font, text_buffer, CELL_VALUE_DEFAULT_FONT_SIZE, 1);
    Vector2 pos = {
        .x = x + CELL_SIZE/2 - text_size.x/2,
        .y = y + CELL_SIZE/2 - text_size.y/2,
    };
    DrawTextEx(default_font, text_buffer, pos, CELL_VALUE_DEFAULT_FONT_SIZE, 1, TEXT_COLOR);
}


Color get_cell_color(int cell_value)
{
    int hue = (int)(0 + log2(cell_value) * 10) % 360; 
    Color color = ColorFromHSV(hue, 0.45f, 0.85f);
    return color;
}


void draw_move_cells(void)
{
    int sx = GetScreenWidth()/2 - FIELD_WIDTH/2 + FIELD_MARGIN;
    int sy = GetScreenHeight()/2 - GAME_HEIGHT/2 + SCORE_HEIGHT + FIELD_GAP + FIELD_MARGIN;   

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
    int sx = GetScreenWidth()/2 - FIELD_WIDTH/2 + FIELD_MARGIN;
    int sy = GetScreenHeight()/2 - GAME_HEIGHT/2 + SCORE_HEIGHT + FIELD_GAP + FIELD_MARGIN;   
    
    Rectangle board_rec = {sx - FIELD_MARGIN, sy - FIELD_MARGIN,  FIELD_WIDTH, FIELD_HEIGHT};
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


void draw_score(void)
{
    static char text_buffer[4096] = {0};
    int sx = GetScreenWidth()/2 - FIELD_WIDTH/2;
    int sy = GetScreenHeight()/2 - GAME_HEIGHT/2;   


    int score = get_score();
    stbsp_snprintf(text_buffer, sizeof(text_buffer), "%d", score);
    
    Vector2 text_size = MeasureTextEx(score_label_font, "Score", SCORE_LABEL_TEXT_SIZE, 1);
    Vector2 value_text_size = MeasureTextEx(default_font, text_buffer, SCORE_VALUE_TEXT_SIZE, 1);
    int score_width = value_text_size.x + SCORE_PAD*2;

    Vector2 value_pos = {sx + score_width/2 - value_text_size.x/2, sy + 20};
    Vector2 pos = {.x = sx + score_width/2 - text_size.x/2, .y = sy};


    DrawRectangle(sx, sy, score_width, SCORE_HEIGHT, BOARD_COLOR);
    DrawTextEx(score_label_font, "Score", pos, SCORE_LABEL_TEXT_SIZE, 1, TEXT_COLOR);
    DrawTextEx(default_font, text_buffer, value_pos, SCORE_VALUE_TEXT_SIZE, 1, TEXT_COLOR);
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

void restart_game(void)
{
    reset_score();
    clear_board();
    add_random_cell();
    save_back_board();
    game_state = GAME_PLAY;
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
        restart_game();
    } 
}

void draw_restart_button(void)
{
    int x = GetScreenWidth()/2 + FIELD_WIDTH/2 - BUTTON_SIZE;
    int y = GetScreenHeight()/2 - GAME_HEIGHT/2;   

    int arrow_radius = (BUTTON_SIZE - ARROW_PAD*2) / 2;
    Vector2 button_center = {.x = x + BUTTON_SIZE/2, .y = y + BUTTON_SIZE/2};
    Rectangle button_rec = {x, y, BUTTON_SIZE, BUTTON_SIZE};
    bool hoverover = CheckCollisionPointRec(GetMousePosition(), button_rec); 
    Color button_color = hoverover ? ARROW_COLOR : BOARD_COLOR;
    DrawRectangleRec(button_rec, button_color);

    Color arrow_color = hoverover ? BOARD_COLOR : ARROW_COLOR;
    Vector2 center = {.x = x + BUTTON_SIZE/2, 
                      .y = y + BUTTON_SIZE/2 - 1};
    DrawRing(center, arrow_radius-ARROW_THICK, arrow_radius, -160, -10, 20, arrow_color);
    DrawRing(center, arrow_radius-ARROW_THICK, arrow_radius, 170, 20, 20, arrow_color);
      
    _draw_arrow(button_center.x, button_center.y, -150*PI/180, -175*PI/180, arrow_color);
    _draw_arrow(button_center.x, button_center.y, 30*PI/180, 0*PI/180, arrow_color);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hoverover) {
        restart_game(); 
    }
}


void game_frame(void)
{
    BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        
        draw_board();           
        draw_score(); 
        draw_restart_button();
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

    InitWindow(FIELD_WIDTH + FIELD_GAP*2, FIELD_HEIGHT + FIELD_GAP*2 + SCORE_HEIGHT, "2048");
    SetTargetFPS(60);    
    
    default_font = LoadFontEx(font_path, CELL_VALUE_DEFAULT_FONT_SIZE, NULL, 0);
    score_label_font = LoadFontEx(font_path, SCORE_LABEL_TEXT_SIZE, NULL, 0);
    
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
