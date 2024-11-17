#ifndef GANTRY_HPP
#define GANTRY_HPP

#include <Arduino.h>
#include "stepper.hpp"

/*
This file provide conveinient functions to control the gantry.
It holds information about the shelf system and at what XY positions the shelfs reside at.
This is hardcoded for now, but it would be cool to have this be dynamic, however the stickers with
the NFC in them dont really provide the best resolution.
*/

#define X_AXIS_TOTAL_STEPS 3000
#define Y_AXIS_TOTAL_STEPS 3000
#define Z_AXIS_TOTAL_STEPS 10000


// Strictly speaking this is not correct, but it will suffice for testing.
#define NUM_ROWS 2
#define NUM_COLS 2

#define X_AXIS_STEPS_BETWEEN_ROWS 800
#define Y_AXIS_STEPS_BETWEEN_COLS 800

#define X_AXIS_SHELVES_START 580
#define Y_AXIS_SHELVES_START 850

typedef struct shelf_location {
    uint16_t x_pos;             // X coordinate of the shelf
    uint16_t y_pos;             // Y coordinate of the shelf
    uint8_t present_UID[12];    // 12-byte UID of the shelf
    bool is_occupied;           // Whether the shelf is occupied
} shelf_location_t;

void set_held_shelf(uint8_t* shelfUID);

int init_gantry();
void initialize_shelves(); // Inits the values in the shelves array

void scan_shelves(); // Iterates over all positions and scans the NFC tags
void pretty_shelf_table_print();
void move_gantry_to_shelf_location(uint8_t* target_UID);
bool read_shelf_UID_into_shelf(shelf_location_t* shelf);
bool read_shelf_UID(uint8_t* UID);
void printUID(uint8_t* UID);

void pick_up_shelf();
void move_gantry_to_user();
void put_back_shelf();

void calibrate_gantry();


#endif // GANTRY_HPP