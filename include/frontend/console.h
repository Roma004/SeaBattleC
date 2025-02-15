#pragma once
#include <game_engine/game_field.h>
#include <stdint.h>

struct editor_context {
    struct field *f;
    enum SHIP_TYPE st;
    enum DIRECTION dir;
    enum GAMEMODE *mode;
    uint8_t *ships_left;
};

struct clear_context {
    struct field *f;
    enum GAMEMODE *mode;
    uint8_t *ships_left;
};

struct battle_context {
    struct field *f1;
    struct field *f2;
    int step;
};

void draw_field_battle_mode(struct field *f1, struct field *f2, int step);

void draw_field_cursor_mode(struct field *f);

void draw_field_editor_mode(
    struct field *f, enum SHIP_TYPE st, enum DIRECTION dir
);

void print_player_2_won();
void print_player_1_won();
void print_help_general();
void print_help_editor();
void print_help_clear(int can_continue);
int handle_clear_mode(struct clear_context *ctx);
int handle_editor_mode(struct editor_context *ctx);
int handle_player_step(struct battle_context *ctx);
int configure_player(struct field *f);
int start_battle(struct field *f1, struct field *f2);
