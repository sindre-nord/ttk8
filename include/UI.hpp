#ifndef UI_HPP
#define UI_HPP
/*
PINS:
OLED Display:
    SDA: 21
    SCL: 22
Joystick:
    X: 34
    Y: 35
*/

#include <Arduino.h>

// Constants for the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Forward declaration of the display object
#include <Adafruit_SSD1306.h>
extern Adafruit_SSD1306 display;

// Menu entry structure
typedef struct menu_entry {
    const char* name;
    void (*action)(void);
} menu_entry_t;

// Menu structure
typedef struct menu {
    uint8_t selected;
    uint8_t size;
    const menu_entry_t* items;
} menu_t;

// Externally accessible menu
extern menu_t main_menu;

// Function prototypes for menu operations
void render_menu(menu_t menu);
void move_menu(menu_t* menu, int8_t direction);
void render_text(const char* text, uint8_t x, uint8_t y);
void render_text_centered(const char* text, uint8_t y);


// Action function prototypes
void calibrate_dummy();
void settings_dummy();
void scan_dummy();
void request_calibration();
void request_UID();
void request_scan();



// Joystick direction enumeration
enum JoystickDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NONE
};

// Function prototypes for joystick handling
JoystickDirection evaluate_joystick(int joystick_adc_value, int axis);
void evaluate_UI();
int init_UI();

#endif // UI_HPP