#include "game_engine/game_field.h"


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

    if ((st = check_ship_bitten(f, x, y)) == SHIP_TYPE_NO_SHIP) return 1;

    if (x != 9 && HAS_SHIP(f, x + 1, y)) {
        for (int i = y - 1; i <= y + 1; ++i) {
            for (int j = x - 1; j <= x + (int)st; ++j) {
                if (0 <= j && j <= 9 && 0 <= i && i <= 9) BITE(f, j, i);
            }
        }
    } else {
        for (int i = x - 1; i <= x + 1; ++i) {
            for (int j = y - 1; j <= y + (int)st; ++j) {
                if (0 <= j && j <= 9 && 0 <= i && i <= 9) BITE(f, i, j);
            }
        }
    }

    return 1;
}
