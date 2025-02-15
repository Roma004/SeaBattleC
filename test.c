#include "input.h"
#include <stdint.h>
#include <stdio.h>

// TODO реализовать режим боя

#define HAS_SHIP(f, x, y)   (f->ships_layer[x] & (1 << (y)))
#define SET_SHIP(f, x, y)   (f->ships_layer[x] |= (1 << (y)))
#define UNSET_SHIP(f, x, y) (f->ships_layer[x] &= ~(1 << (y)))
#define BITE(f, x, y)       (f->bitten_layer[x] |= (1 << (y)))
#define BITTEN(f, x, y)     (f->bitten_layer[x] & (1 << (y)))

struct field_position {
    uint8_t x: 4;
    uint8_t y: 4;
};

enum SHIP_TYPE {
    SHIP_TYPE_NO_SHIP = 0,
    SHIP_TYPE_1 = 1,
    SHIP_TYPE_2 = 2,
    SHIP_TYPE_3 = 3,
    SHIP_TYPE_4 = 4,
};

enum DIRECTION {
    DIRECTION_HORIZONTAL = 0,
    DIRECTION_VERTICAL = 1,
};

enum GAMEMODE {
    GAMEMODE_EDITOR,
    GAMEMODE_CLEAR,
};

struct field {
    uint16_t ships_layer[10];
    uint16_t bitten_layer[10];
    struct field_position cursor;
    uint8_t bitten;
};

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

int can_set_ship_horizontal(struct field *f, enum SHIP_TYPE st) {
    int x = f->cursor.x, y = f->cursor.y;
    if (x + st > 10) return 0;
    for (int i = -1; i <= (int)st; ++i) {
        if (x + i < 0 || x + i > 9) continue;
        if (y != 0 && (f->ships_layer[x + i] & (1 << (y - 1)))) return 0;
        if (f->ships_layer[x + i] & (1 << y)) return 0;
        if (y != 9 && (f->ships_layer[x + i] & (1 << (y + 1)))) return 0;
    }
    return 1;
}

int can_set_ship_vertical(struct field *f, enum SHIP_TYPE st) {
    int x = f->cursor.x, y = f->cursor.y;
    if (y + st > 10) return 0;
    for (int i = -1; i <= (int)st; ++i) {
        if (y + i < 0 && y + i > 9) continue;
        if (x != 0 && f->ships_layer[x - 1] & (1 << (y + i))) return 0;
        if (f->ships_layer[x] & (1 << (y + i))) return 0;
        if (x != 9 && f->ships_layer[x + 1] & (1 << (y + i))) return 0;
    }
    return 1;
}

int can_set_ship(struct field *f, enum SHIP_TYPE st, enum DIRECTION dir) {
    if (dir == DIRECTION_HORIZONTAL) return can_set_ship_horizontal(f, st);
    else return can_set_ship_vertical(f, st);
}

struct field_position find_ship_start(struct field *f) {
    int x = f->cursor.x, y = f->cursor.y;

    while (1) {
        if (x != 0 && HAS_SHIP(f, x - 1, y)) x -= 1;
        else if (y != 0 && HAS_SHIP(f, x, y - 1)) y -= 1;
        else break;
    }

    return (struct field_position){x, y};
}

enum SHIP_TYPE clear_ship(struct field *f) {
    int x = f->cursor.x, y = f->cursor.y;
    int res = 0;

    if (!HAS_SHIP(f, x, y)) return 0;

    struct field_position ship_start = find_ship_start(f);
    x = ship_start.x;
    y = ship_start.y;

    while (1) {
        UNSET_SHIP(f, x, y);
        ++res;
        if (x != 9 && HAS_SHIP(f, x + 1, y)) x += 1;
        else if (y != 9 && HAS_SHIP(f, x, y + 1)) y += 1;
        else break;
    }
    return res;
}

enum SHIP_TYPE check_ship_bitten(struct field *f, int x, int y) {
    int res = 0;
    while (1) {
        ++res;
        if (!BITTEN(f, x, y)) return SHIP_TYPE_NO_SHIP;
        if (x != 9 && HAS_SHIP(f, x + 1, y)) x += 1;
        else if (y != 9 && HAS_SHIP(f, x, y + 1)) y += 1;
        else break;
    }
    return res;
}

int place_ship(struct field *f, enum SHIP_TYPE st, enum DIRECTION dir) {
    if (!can_set_ship(f, st, dir)) return 0;

    int x = f->cursor.x, y = f->cursor.y;
    for (int i = 0; i < st; ++i) {
        if (dir == DIRECTION_VERTICAL) {
            f->ships_layer[x] |= 1 << (y + i);
        } else {
            f->ships_layer[x + i] |= 1 << y;
        }
    }
    return 1;
}

int bite_ship(struct field *f) {
    int x = f->cursor.x, y = f->cursor.y;
    if (BITTEN(f, x, y)) return 1;

    BITE(f, x, y);
    if (!HAS_SHIP(f, x, y)) return 0;

    f->bitten += 1;

    enum SHIP_TYPE st;
    struct field_position ship_start = find_ship_start(f);

    x = ship_start.x;
    y = ship_start.y;
    fprintf(stderr, "start: %d %d\n", x, y);

    if ((st = check_ship_bitten(f, x, y)) == SHIP_TYPE_NO_SHIP) return 1;

    fprintf(stderr, "type: %d\n", st);

    if (x != 9 && HAS_SHIP(f, x + 1, y)) {
        fprintf(stderr, "horizontal\n");
        for (int i = y - 1; i <= y + 1; ++i) {
            for (int j = x - 1; j <= x + (int)st; ++j) {
                if (0 <= j && j <= 9 && 0 <= i && i <= 9) BITE(f, j, i);
            }
        }
    } else {
        fprintf(stderr, "vertical\n");
        for (int i = x - 1; i <= x + 1; ++i) {
            for (int j = y - 1; j <= y + (int)st; ++j) {
                if (0 <= j && j <= 9 && 0 <= i && i <= 9) BITE(f, i, j);
            }
        }
    }

    return 1;
}

void draw_field_battle_mode(struct field *f1, struct field *f2, int step) {
    char cur;

    printf("   | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10        "
           "   | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10\n");
    for (int i = 0; i < 10; ++i) {
        printf("---+---+---+---+---+---+---+---+---+---+---        "
               "---+---+---+---+---+---+---+---+---+---+---\n");
        printf(" %c ", "abcdefghijk"[i]);
        for (int j = 0; j < 10; ++j) {
            if (HAS_SHIP(f1, j, i) && BITTEN(f1, j, i)) cur = '#';
            else if (BITTEN(f1, j, i)) cur = '*';
            else cur = ' ';
            int x = f1->cursor.x, y = f1->cursor.y;
            if (step == 0 && j == x && i == y)
                printf("|\033[30;45m %c \033[0m", cur);
            else if (cur == '#') printf("|\033[30;41m %c \033[0m", cur);
            else if (cur == '*') printf("|\033[30;46m %c \033[0m", cur);
            else printf("| %c ", cur);
        }
        printf("        ");
        printf(" %c ", "abcdefghijk"[i]);
        for (int j = 0; j < 10; ++j) {
            if (HAS_SHIP(f2, j, i) && BITTEN(f2, j, i)) cur = '#';
            else if (BITTEN(f2, j, i)) cur = '*';
            else cur = ' ';
            int x = f2->cursor.x, y = f2->cursor.y;
            if (step == 1 && j == x && i == y)
                printf("|\033[30;45m %c \033[0m", cur);
            else if (cur == '#') printf("|\033[30;41m %c \033[0m", cur);
            else if (cur == '*') printf("|\033[30;46m %c \033[0m", cur);
            else printf("| %c ", cur);
        }
        printf("\n");
    }
}

void draw_field_cursor_mode(struct field *f) {
    char cur;

    printf("   | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10\n");
    for (int i = 0; i < 10; ++i) {
        printf("---+---+---+---+---+---+---+---+---+---+---\n");
        printf(" %c ", "abcdefghijk"[i]);
        for (int j = 0; j < 10; ++j) {
            if (f->ships_layer[j] & (1 << i)) cur = '#';
            else cur = ' ';
            int x = f->cursor.x, y = f->cursor.y;
            if (j == x && i == y) printf("|\033[30;45m %c \033[0m", cur);
            else if (cur == '#') printf("|\033[30;43m %c \033[0m", cur);
            else printf("| %c ", cur);
        }
        printf("\n");
    }
}

void draw_field_editor_mode(
    struct field *f, enum SHIP_TYPE st, enum DIRECTION dir
) {
    char cur;

    int is_ok = can_set_ship(f, st, dir);

    printf("   | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10\n");
    for (int i = 0; i < 10; ++i) {
        printf("---+---+---+---+---+---+---+---+---+---+---\n");
        printf(" %c ", "abcdefghijk"[i]);
        for (int j = 0; j < 10; ++j) {
            if (f->ships_layer[j] & (1 << i)) cur = '#';
            else cur = ' ';
            int x = f->cursor.x, y = f->cursor.y;
            if (dir == DIRECTION_VERTICAL) {
                if (((j == x - 1 || j == x + 1) && (i >= y - 1 && i <= y + st))
                    || (j == x && (i == y - 1 || i == y + st)))
                    printf("|\033[30;45m %c \033[0m", cur);
                else if (j == x && (i >= y && i < y + st)) {
                    if (is_ok) printf("|\033[30;42m ");
                    else printf("|\033[30;41m ");
                    printf("%c \033[0m", cur);
                } else {
                    if (cur == '#') printf("|\033[30;43m %c \033[0m", cur);
                    else printf("| %c ", cur);
                }
            }
            if (dir == DIRECTION_HORIZONTAL) {
                if (((i == y - 1 || i == y + 1) && (j >= x - 1 && j <= x + st))
                    || (i == y && (j == x - 1 || j == x + st)))
                    printf("|\033[30;45m %c \033[0m", cur);
                else if (i == y && (j >= x && j < x + st)) {
                    if (is_ok) printf("|\033[30;42m ");
                    else printf("|\033[30;41m ");
                    printf("%c \033[0m", cur);
                } else {
                    if (cur == '#') printf("|\033[30;43m %c \033[0m", cur);
                    else printf("| %c ", cur);
                }
            }
        }
        printf("\n");
    }
}

void clear_screen() { printf("\033c"); }

void print_help_general() {
    printf("movement:\n"
           "  h -- cursor left\n"
           "  j -- cursor down\n"
           "  k -- cursor up\n"
           "  l -- cursor right\n"
           "  q -- quit game\n"
           "\n");
}

void print_help_editor() {
    printf("editor keys:\n"
           "  p -- place ship\n"
           "  r -- rotate ship\n"
           "  c -- enter clear mode\n"
           "\n");
}

void print_help_clear(int can_continue) {
    printf("editor keys:\n"
           "  c -- clear ship at cursor position\n"
           "  e -- enter editor mode\n");
    if (can_continue) printf("  S -- Continue |_____---->\n");
    else printf("\n");
    printf("\n");
}

// clang-format off
void print_player_1_won() {
    printf(
"   _______  _        _______           _______  _______          __                     _______  _       \n"
"  (  ____ )( \\      (  ___  )|\\     /|(  ____ \\(  ____ )        /  \\          |\\     /|(  ___  )( (    /|\n"
"  | (    )|| (      | (   ) |( \\   / )| (    \\/| (    )|        \\/) )         | )   ( || (   ) ||  \\  ( |\n"
"  | (____)|| |      | (___) | \\ (_) / | (__    | (____)|          | |         | | _ | || |   | ||   \\ | |\n"
"  |  _____)| |      |  ___  |  \\   /  |  __)   |     __)          | |         | |( )| || |   | || (\\ \\) |\n"
"  | (      | |      | (   ) |   ) (   | (      | (\\ (             | |         | || || || |   | || | \\   |\n"
"  | )      | (____/\\| )   ( |   | |   | (____/\\| ) \\ \\__        __) (_        | () () || (___) || )  \\  |\n"
"  |/       (_______/|/     \\|   \\_/   (_______/|/   \\__/   _____\\____/_____   (_______)(_______)|/    )_)\n"
"                                                          (_____)    (_____)                             \n"
    );
}


void print_player_2_won() {
    printf(
"   _______  _        _______           _______  _______          _______                   _______  _       \n"
"  (  ____ )( \\      (  ___  )|\\     /|(  ____ \\(  ____ )        / ___   )        |\\     /|(  ___  )( (    /|\n"
"  | (    )|| (      | (   ) |( \\   / )| (    \\/| (    )|        \\/   )  |        | )   ( || (   ) ||  \\  ( |\n"
"  | (____)|| |      | (___) | \\ (_) / | (__    | (____)|            /   )        | | _ | || |   | ||   \\ | |\n"
"  |  _____)| |      |  ___  |  \\   /  |  __)   |     __)          _/   /         | |( )| || |   | || (\\ \\) |\n"
"  | (      | |      | (   ) |   ) (   | (      | (\\ (            /   _/          | || || || |   | || | \\   |\n"
"  | )      | (____/\\| )   ( |   | |   | (____/\\| ) \\ \\__        (   (__/\\        | () () || (___) || )  \\  |\n"
"  |/       (_______/|/     \\|   \\_/   (_______/|/   \\__/   _____\\_______/_____   (_______)(_______)|/    )_)\n"
"                                                          (_____)       (_____)                             \n"
    );
}
// clang-format on

int handle_clear_mode(struct clear_context *ctx) {
    struct field *f = ctx->f;

    enum SHIP_TYPE st;
    int can_continue = 1;
    for (int i = 1; i < 4; ++i)
        if (ctx->ships_left[i] != 0) {
            can_continue = 0;
            break;
        }

    print_help_clear(can_continue);
    draw_field_cursor_mode(f);
    while (!kbhit());
    switch (getchar()) {
    case 'q': return 1;
    case 'k':
        if (f->cursor.y != 0) f->cursor.y -= 1;
        break;
    case 'j':
        if (f->cursor.y != 9) f->cursor.y += 1;
        break;
    case 'h':
        if (f->cursor.x != 0) f->cursor.x -= 1;
        break;
    case 'l':
        if (f->cursor.x != 9) f->cursor.x += 1;
        break;
    case 'c':
        st = clear_ship(f);
        if (st == SHIP_TYPE_NO_SHIP) break;
        ctx->ships_left[st] += 1;
        break;
    case 'e': *ctx->mode = GAMEMODE_EDITOR; break;
    case 'S':
        if (can_continue) return 2;
    }
    return 0;
}

int handle_editor_mode(struct editor_context *ctx) {
    struct field *f = ctx->f;
    enum DIRECTION dir = ctx->dir;
    enum SHIP_TYPE st = ctx->st;

    if (ctx->ships_left[st] == 0) {
        for (int i = 1; i <= 4; ++i)
            if (ctx->ships_left[i] != 0) {
                ctx->st = i;
                st = i;
            }
        if (ctx->ships_left[st] == 0) {
            *ctx->mode = GAMEMODE_CLEAR;
            return 0;
        }
    }

    print_help_editor();
    draw_field_editor_mode(f, st, dir);
    while (!kbhit());
    switch (getchar()) {
    case 'q': return 1;
    case 'k':
        if (f->cursor.y != 0) f->cursor.y -= 1;
        break;
    case 'j':
        if ((dir == DIRECTION_VERTICAL && f->cursor.y != 9 - st + 1)
            || (dir == DIRECTION_HORIZONTAL && f->cursor.y != 9))
            f->cursor.y += 1;
        break;
    case 'h':
        if (f->cursor.x != 0) f->cursor.x -= 1;
        break;
    case 'l':
        if ((dir == DIRECTION_VERTICAL && f->cursor.x != 9)
            || (dir == DIRECTION_HORIZONTAL && f->cursor.x != 9 - st + 1))
            f->cursor.x += 1;
        break;
    case 'r':
        ctx->dir = dir == DIRECTION_HORIZONTAL ? DIRECTION_VERTICAL
                                               : DIRECTION_HORIZONTAL;
        *(uint8_t *)&ctx->f->cursor = 0;
        break;
    case 'p':
        if (place_ship(f, st, dir)) {
            *(uint8_t *)&f->cursor = 0;
            ctx->ships_left[st] -= 1;
        }
        break;
    case 'c': *ctx->mode = GAMEMODE_CLEAR; break;
    case '1':
        if (ctx->ships_left[SHIP_TYPE_1] != 0) {
            ctx->st = SHIP_TYPE_1;
            *(uint8_t *)&f->cursor = 0;
        }
        break;
    case '2':
        if (ctx->ships_left[SHIP_TYPE_2] != 0) {
            ctx->st = SHIP_TYPE_2;
            *(uint8_t *)&f->cursor = 0;
        }
        break;
    case '3':
        if (ctx->ships_left[SHIP_TYPE_3] != 0) {
            ctx->st = SHIP_TYPE_3;
            *(uint8_t *)&f->cursor = 0;
        }
        break;
    case '4':
        if (ctx->ships_left[SHIP_TYPE_4] != 0) {
            ctx->st = SHIP_TYPE_4;
            *(uint8_t *)&f->cursor = 0;
        }
        break;
    }
    return 0;
}

int handle_player_step(struct battle_context *ctx) {
    struct field *f;

    if (ctx->f1->bitten == 1 * 4 + 2 * 3 + 3 * 2 + 4) return 3;
    if (ctx->f2->bitten == 1 * 4 + 2 * 3 + 3 * 2 + 4) return 2;

    if (ctx->step == 0) f = ctx->f1;
    else f = ctx->f2;

    draw_field_battle_mode(ctx->f1, ctx->f2, ctx->step);
    while (!kbhit());
    switch (getchar()) {
    case 'q': return 1;
    case 'k':
        if (f->cursor.y != 0) f->cursor.y -= 1;
        break;
    case 'j':
        if (f->cursor.y != 9) f->cursor.y += 1;
        break;
    case 'h':
        if (f->cursor.x != 0) f->cursor.x -= 1;
        break;
    case 'l':
        if (f->cursor.x != 9) f->cursor.x += 1;
        break;
    case 'b':
        if (!bite_ship(f)) ctx->step = !ctx->step;
        break;
    }
    return 0;
}

int configure_player(struct field *f) {
    int quit_flag = 0;
    enum GAMEMODE mode = GAMEMODE_EDITOR;
    uint8_t ships_left[] = {0, 4, 3, 2, 1};

    struct editor_context e_ctx = {
        .f = f,
        .dir = DIRECTION_HORIZONTAL,
        .st = SHIP_TYPE_4,
        .mode = &mode,
        .ships_left = ships_left,
    };

    struct clear_context c_ctx = {
        .f = f,
        .mode = &mode,
        .ships_left = ships_left,
    };

    while (!quit_flag) {
        clear_screen();
        print_help_general();
        switch (mode) {
        case GAMEMODE_EDITOR: quit_flag = handle_editor_mode(&e_ctx); break;
        case GAMEMODE_CLEAR: quit_flag = handle_clear_mode(&c_ctx); break;
        }
    }

    return quit_flag;
}

int start_battle(struct field *f1, struct field *f2) {
    int quit_flag = 0;
    int step = 0;

    struct battle_context battle_ctx = {
        .f1 = f1,
        .f2 = f2,
        .step = 0,
    };

    while (!quit_flag) {
        clear_screen();
        print_help_general();
        quit_flag = handle_player_step(&battle_ctx);
    }

    return quit_flag;
}

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
