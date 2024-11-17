#include "UI.hpp"
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "tag_finder.hpp"
#include "FSM.hpp"

extern volatile bool newCommand;
extern volatile String command;

// #######################################
// #####         OLED Menu            #####
// #######################################

// Initialize the display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Menu items
// const char* menu_strings[] = {"Calibrate", "Settings", "Scan"};
// #define MENU_SIZE (sizeof(menu_strings) / sizeof(menu_strings[0]))

const menu_entry_t menu_entries[] = {
    // {"Calibrate", calibrate_dummy},
    {"Calibrate", request_calibration},
    {"Settings",  settings_dummy},
    {"Scan",      request_scan},
    {"Request UID", request_UID}
};

#define MENU_SIZE (sizeof(menu_entries) / sizeof(menu_entries[0]))

menu_t main_menu = {0, MENU_SIZE, menu_entries};

// Render the menu on the OLED display
void render_menu(menu_t menu) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    for (uint8_t i = 0; i < MENU_SIZE; i++) {
        display.setCursor(0, 10 + i * 10);
        if (i == menu.selected) {
            display.print(">");
        } else {
            display.print(" ");
        }
        display.print(menu.items[i].name);
    }
    display.display();
}

// Move the menu selection up or down
void move_menu(menu_t* menu, int8_t direction) {
    if (direction > 0) {
        menu->selected = (menu->selected + 1) % menu->size;
    } else {
        menu->selected = (menu->selected - 1 + menu->size) % menu->size;
    }
}

void render_text(const char* text, uint8_t x, uint8_t y) {
    display.clearDisplay();
    display.setCursor(x, y);
    display.print(text);
    display.display();
}
/**
 * @brief Renders text centered on the OLED display
 * 
 * @param text The text to render
 * @param y Vertical position of the text
 */
void render_text_centered(const char* text, uint8_t y) {
    display.clearDisplay();
    display.setCursor((SCREEN_WIDTH - strlen(text) * 6) / 2, y);
    display.print(text);
    display.display();
}

// Init function 


// Dummy action functions
void calibrate_dummy() {
    display.clearDisplay();
    display.setCursor(20, 30);
    display.print("Calibrating");
    display.display();
    delay(400);
    display.print(".");
    display.display();
    delay(400);
    display.print(".");
    display.display();
    delay(400);
    display.print(".");
    display.display();
    delay(400);
}

void settings_dummy() {
    display.clearDisplay();
    display.setCursor(20, 30);
    display.print("Settings");
    display.display();
    delay(1000);
}

void scan_dummy() {
    display.clearDisplay();
    display.setCursor(20, 30);
    display.print("Scanning");
    display.display();
    delay(1000);
}

void request_calibration(){
    display.clearDisplay();
    display.setCursor(20, 30);
    display.print("Calibrating");
    display.display();
    // Call the function to calibrate the gantry
    Serial.println("Trying to push event to queue");
    push_event_to_queue({CALIBRATION_REQUESTED, "0"});
}

void request_UID(){
    render_text_centered("Requesting UID", 30);
    // Call the function to find the tag
    Serial.println("Trying to push event to queue");
    push_event_to_queue({REQUEST_UID_READ, "0"});
    // delay(1000);
}

void request_scan(){
    render_text_centered("Scanning", 30);
    push_event_to_queue({SCANNING_REQUESTED, "0"});
}

// #######################################
// #####        Joystick Input        #####
// #######################################

// Debounce variables
static unsigned long last_y_millis = 0;
static unsigned long last_x_millis = 0;
const int DEBOUNCE_DELAY = 500;

// Joystick pins
const int JOYSTICK_Y = 34;
const int JOYSTICK_X = 35;

// Joystick thresholds
#define JOYSTICK_REST_VALUE 205
#define JOYSTICK_DEADZONE 50
#define JOYSTICK_LOW_TRIGGER_THRESHOLD (JOYSTICK_REST_VALUE - JOYSTICK_DEADZONE)
#define JOYSTICK_HIGH_TRIGGER_THRESHOLD (JOYSTICK_REST_VALUE + JOYSTICK_DEADZONE)

// Evaluate the joystick direction based on ADC values
JoystickDirection evaluate_joystick(int joystick_adc_value, int axis) {
    if (axis == JOYSTICK_Y) {
        if (joystick_adc_value < JOYSTICK_LOW_TRIGGER_THRESHOLD) {
            return UP;
        } else if (joystick_adc_value > JOYSTICK_HIGH_TRIGGER_THRESHOLD) {
            return DOWN;
        }
    } else if (axis == JOYSTICK_X) {
        if (joystick_adc_value < JOYSTICK_LOW_TRIGGER_THRESHOLD) {
            return LEFT;
        } else if (joystick_adc_value > JOYSTICK_HIGH_TRIGGER_THRESHOLD) {
            return RIGHT;
        }
    }
    return NONE;
}

// Main function to handle UI interaction
void evaluate_UI() {
    unsigned long currentMillis = millis();

    // Handle vertical movement (Y-axis)
    if (currentMillis - last_y_millis >= DEBOUNCE_DELAY) {
        int joystick_y = analogRead(JOYSTICK_Y);
        JoystickDirection y_direction = evaluate_joystick(joystick_y, JOYSTICK_Y);
        switch (y_direction) {
            case UP:
                move_menu(&main_menu, -1);
                render_menu(main_menu);
                last_y_millis = currentMillis;
                break;
            case DOWN:
                move_menu(&main_menu, 1);
                render_menu(main_menu);
                last_y_millis = currentMillis;
                break;
            default:
                break;
        }
    }

    // Handle horizontal movement (X-axis)
    if (currentMillis - last_x_millis >= DEBOUNCE_DELAY) {
        int joystick_x = analogRead(JOYSTICK_X);
        JoystickDirection x_direction = evaluate_joystick(joystick_x, JOYSTICK_X);
        switch (x_direction) {
            case LEFT:
                main_menu.items[main_menu.selected].action();
                last_x_millis = currentMillis;
                render_menu(main_menu);
                break;
            default:
                break;
        }
    }
}

int init_UI(){
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
        // Serial.println(F("SSD1306 allocation failed"));
        return -1;
    }
    delay(2000);
    // Clear and init the display
    display.clearDisplay();
    display.display();
    display.setTextSize(2);
    // Setup adc for joystick
    analogReadResolution(9);

    render_menu(main_menu);
    return 0;
}

// Example code used to test and try out the menu system
// void setup() {
//   Serial.begin(115200);

//   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
//     Serial.println(F("SSD1306 allocation failed"));
//     for(;;); // Stall program.
//   }
//   delay(2000);
//   display.clearDisplay();
//   display.display();
//   display.setTextSize(2);
//   // Setup adc
//   analogReadResolution(9);
//   render_menu(main_menu);
// }

// void loop() {
//   // Check adc value on pin 34
//   int joystick_y = analogRead(JOYSTICK_Y);
//   int joystick_x = analogRead(JOYSTICK_X);
//   if (joystick_y < JOYSTICK_LOW_TRIGGER_THEESHOLD && millis() - last_y_millis >= DEBOUNCE_DELAY){
//     move_menu(&main_menu, 1);
//     render_menu(main_menu);
//     last_y_millis = millis();
//   } else if (joystick_y > JOYSTICK_HIGH_TRIGGER_THEESHOLD && millis() - last_y_millis >= DEBOUNCE_DELAY){
//     move_menu(&main_menu, -1);
//     render_menu(main_menu);
//     last_y_millis = millis();
//   }
//   if (joystick_x < JOYSTICK_LOW_TRIGGER_THEESHOLD && millis() - last_x_millis >= DEBOUNCE_DELAY){
//     main_menu.items[main_menu.selected].action();
//     last_x_millis = millis();
//     render_menu(main_menu);
//   }
//   delay(200);
// }