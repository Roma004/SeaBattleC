#include <stdint.h>
#include <stdio.h>
#include "game_engine/game_field.h"
#include "frontend/console.h"

int main() {
    struct field f1 = {};
    struct field f2 = {};
    int quit_flag;

    if (configure_player(&f1) == 1) return 0;
    if (configure_player(&f2) == 1) return 0;

    if ((quit_flag = start_battle(&f1, &f2)) == 1) return 0;

    draw_field_battle_mode(&f1, &f2, quit_flag - 2);
    printf("\n\n");
    if (quit_flag == 2) print_player_1_won();
    if (quit_flag == 3) print_player_2_won();

    return 0;
}
