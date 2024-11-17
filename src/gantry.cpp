#include <Arduino.h>
#include "stepper.hpp"
#include "gantry.hpp"
#include "tag_finder.hpp"

/*
*/

shelf_location_t shelves[NUM_ROWS][NUM_COLS];
shelf_location_t* held_shelf = NULL;

/**
 * @brief Set the held shelf object
 * 
 * @param shelfUID 
 */
void set_held_shelf(uint8_t* shelfUID){
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            if (memcmp(shelves[i][j].present_UID, shelfUID, 12) == 0) {
                held_shelf = &shelves[i][j];
                return; // No need to keep seaching once the shelf is found
            }
        }
    }
}
int init_gantry(){
    setupStepper();
    startStepper();
    initialize_shelves();
    calibrate_gantry();
    move_gantry_to_user();
    return 0;
}

void initialize_shelves() {
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            shelf_location* current_shelf = &shelves[i][j];
            current_shelf->x_pos = X_AXIS_SHELVES_START + i * X_AXIS_STEPS_BETWEEN_ROWS;
            current_shelf->y_pos = Y_AXIS_SHELVES_START + j * Y_AXIS_STEPS_BETWEEN_COLS;
            // Initialize the UID to all zeros (no tag assigned yet)
            memset(current_shelf->present_UID, 0, sizeof(current_shelf->present_UID));
            current_shelf->is_occupied = false;
        }
    }
}
void scan_shelves() {
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {

            shelf_location* current_shelf = &shelves[i][j];
            Serial.print("Scanning shelf at: ");
            Serial.print(current_shelf->x_pos);
            Serial.print(", ");
            Serial.println(current_shelf->y_pos);

            moveXandYinParallel(current_shelf->x_pos, current_shelf->y_pos);
            Serial.println("Moved to shelf location");

            // Try to read the UID
            Serial.println("Trying to read UID");
            if (read_shelf_UID_into_shelf(current_shelf)) {
                Serial.println("Read UID successfully.");
                current_shelf->is_occupied = true;
            } else {
                Serial.println("Failed to read UID.");
                current_shelf->is_occupied = false;
            }
        }
    }

    pretty_shelf_table_print();
    // Move the gantry back to the user
    move_gantry_to_user();
}

void pretty_shelf_table_print()
{
    Serial.println("\nShelf Status (UID values):");

    // Print the header row with Y positions
    Serial.print(" X \\ Y ");
    for (int j = NUM_COLS - 1; j >= 0; j--)
    { // Iterate through columns (counting down)
        Serial.print("|   ");
        Serial.print(shelves[0][j].y_pos); // Access y_pos for the columns
        Serial.print("   ");
    }
    Serial.println("|");
    Serial.println("--------+--------+--------+--------");

    // Print the data rows with X positions
    for (int i = NUM_ROWS - 1; i >= 0; i--)
    { // Iterate through rows (counting down)
        Serial.print("   ");
        Serial.print(shelves[i][0].x_pos); // Print x_pos for each row
        Serial.print("   ");
        for (int j = NUM_COLS - 1; j >= 0; j--)
        { // Iterate through columns (counting down)
            Serial.print("|   ");
            if (shelves[i][j].is_occupied)
            {
                printUID(shelves[i][j].present_UID); // Print UID if occupied
            }
            else
            {
                Serial.print("None"); // Print "None" if not occupied
            }
            Serial.print(" ");
        }
        Serial.println("|");
        Serial.println("--------+--------+--------+--------");
    }
}

// Helper function to print the 12-byte UID in hexadecimal format
void printUID(uint8_t* UID) {
    // Print UID as an ASCII string
    // Serial.print("UID (as ASCII): ");
    for (int i = 0; i < 12; i++) {
        Serial.print((char)UID[i]);  // Cast each byte to a char and print it
    }
    // Serial.println();  // Add a newline at the end
}

void move_gantry_to_shelf_location(uint8_t* target_UID) {
    for (int i = 0; i < NUM_ROWS; i++) {
        for (int j = 0; j < NUM_COLS; j++) {
            shelf_location* shelf_to_evaluate = &shelves[i][j];

            // Serial.printf("Checking shelf at: %d, %d\n", shelf_to_evaluate->x_pos, shelf_to_evaluate->y_pos);
            // Serial.print("Current UID: "); printUID(shelf_to_evaluate->present_UID);
            // Serial.print("Target UID: "); printUID(target_UID);
            // Serial.println();

            // Compare the 12-byte present_UID with the target_UID
            if (memcmp(shelf_to_evaluate->present_UID, target_UID, 12) == 0) {
                // If the UID matches, move the gantry to the shelf location
                moveXandYinParallel(shelf_to_evaluate->x_pos, shelf_to_evaluate->y_pos);
                return;  // Exit the function once the shelf is found
            }
        }
    }
    Serial.println("Shelf with specified UID not found.");
}

/**
 * @brief Read the UID of the shelf at the current position
 * 
 * @param shelf Pointer to the shelf_location struct
 * @return true if a UID was found
 * @return false if no UID was found
 */
bool read_shelf_UID_into_shelf(shelf_location_t* shelf){
    // Move the Z axis a little bit forward (to get closer to the shelf)
    
    // Tell the tag reader to start looking by sending
    // the START command.
    // Wait for the ack, confirming that the tag reader
    // has started looking for a tag. If we don't get an
    // ack. If we don't get an ack, we need to return false

    // Set a start time, and then wait for that long check is
    // we have recived the UID. If we have, return true
    // if we time out, return false

    Serial.println("Moving Z axis to shelf...");
    // Move the Z axis forward (pseudocode, replace with real code)
    //move_Z_axis_forward();

    uint8_t UID[12];  // Buffer for the 12-byte UID

    // Read the UID from the NFC reader
    if (!read_shelf_UID(UID)) {
        Serial.println("Failed to read UID");
        return false;
    }

    // Copy the 12-byte UID into the shelf's present_UID field
    memcpy(shelf->present_UID, UID, 12);  // Copy the UID

    // Optionally mark the shelf as occupied
    shelf->is_occupied = true;

    return true;
}

bool read_shelf_UID(uint8_t* UID) {
    Serial.println("Trying to read UID...");

    // Send START command to request UID
    sendCANcommand(START);

    // Wait for the ACK response (ID = 0x103)
    Serial.println("Waiting for ACK...");
    unsigned long start_time = millis();
    const unsigned long ACK_TIMEOUT = 100;

    // Wait for ACK with timeout
    while (!CANMessageReceived) {
        if (millis() - start_time > ACK_TIMEOUT) {
            Serial.println("No ACK received, terminating");
            sendCANcommand(STOP);  // Optionally stop the process
            return false;
        }
    }
    Serial.println("ACK received");
    CANMessageReceived = false;

    // Now wait for the UID packets (ID = 0x104)
    const unsigned long UID_TIMEOUT = 200;
    start_time = millis();

    int byteCount = 0;  // Keep track of how many bytes have been received

    while (byteCount < 12) {  // Expect 12 bytes in total
        // Block and wait for a CAN message to arrive
        while (!CANMessageReceived) {
            if (millis() - start_time > UID_TIMEOUT) {
                Serial.println("No UID received, timing out");
                Serial.print("Bytes received: ");
                Serial.println(byteCount);
                sendCANcommand(STOP);  // Optionally send STOP command if timed out
                return false;
            }
        }
        CANMessageReceived = false;  // Reset the flag

        // Read the CAN packet
        if (CAN.available()) {
            while (CAN.available() && byteCount < 12) {
                UID[byteCount++] = (uint8_t)CAN.read();  // Read byte-by-byte into UID array
            }
        }
    }

    // Convert and print the received UID as ASCII string
    Serial.print("Received UID (as ASCII): ");
    char uid_str[13];  // 12 characters + 1 null terminator for the string
    for (int i = 0; i < 12; i++) {
        uid_str[i] = (char)UID[i];  // Convert each byte to its corresponding ASCII character
    }
    uid_str[12] = '\0';  // Null-terminate the string

    Serial.println(uid_str);  // Print the UID as a string

    return true;
}

void pick_up_shelf(){
    jogAxis(750, Y_AXIS, BACKWARD); 
    jogAxis(13000, Z_AXIS, FORWARD); 
    jogAxis(750, Y_AXIS, FORWARD);
    jogAxis(13000, Z_AXIS, BACKWARD);
    return;
}

/**
 * @brief Moves the gantry to a place where the user can interact with it.
 * 
 */
void move_gantry_to_user(){
    // Move the gantry to the user
    // This is a hardcoded position for now, should ideally be dynamic
    moveXandYinParallel(100, 100);

    //Rotate the self or something maybe?
}

void put_back_shelf(){
    // Move the Z axis down to put back the shelf
    // Serial.printf("Putting back shelf at: %d, %d\n", held_shelf->x_pos, held_shelf->y_pos);
    Serial.printf("Putting shelf back with UID %s\n", held_shelf->present_UID);
    move_gantry_to_shelf_location(held_shelf->present_UID);
    //moveXandYinParallel(held_shelf->x_pos, held_shelf->y_pos);
    jogAxis(50, Y_AXIS, BACKWARD);
    jogAxis(13000, Z_AXIS, FORWARD);
    jogAxis(700, Y_AXIS, BACKWARD);
    jogAxis(13000, Z_AXIS, BACKWARD);

    move_gantry_to_user();
    return;
}

void calibrate_gantry(){
    Serial.println("Calibrating x-axis");
    long xsteps = calibrateAxis(X_AXIS);
    Serial.print("X steps: ");
    Serial.println(xsteps);

    Serial.println("Calibrating y-axis");
    long ysteps = calibrateAxis(Y_AXIS);
    Serial.print("Y steps: ");
    Serial.println(ysteps);

    Serial.println("Calibrating z-axis");
    long zsteps = calibrateAxis(Z_AXIS);
    Serial.print("Z steps: ");
    Serial.println(zsteps);

    return;
}