#pragma once
#include <stdint.h>

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


int can_set_ship(struct field *f, enum SHIP_TYPE st, enum DIRECTION dir);
enum SHIP_TYPE clear_ship(struct field *f);
int place_ship(struct field *f, enum SHIP_TYPE st, enum DIRECTION dir);
int bite_ship(struct field *f);
