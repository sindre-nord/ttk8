#include <Arduino.h>
#include "states.hpp"
#include "UI.hpp"
#include "FSM.hpp"
#include "tag_finder.hpp"
#include "gantry.hpp"
#include "server_client.hpp"
#include "MQTT.hpp"


/*
This file contains all states and define their behavior.

*/

void run_INIT(){
    // The system is initializing. This means that the system is setting up all the hardware and 
    // software components. The system will be in this state until all components are initialized.

    Serial.println("Initializing subsystems...");
    // Initialize the UI
    Serial.println("Initializing UI...");
    if(init_UI() == -1){
        Serial.println("Failed to initialize UI");
        while(1);
    }  
    render_text_centered("Initializing", 30);

    // Initialize the CAN bus
    Serial.println("Initializing CAN...");
    if(setupCAN() == -1){
        Serial.println("Failed to initialize CAN");
        while(1);
    }

    // Initialize the gantry
    Serial.println("Initializing gantry...");
    if(init_gantry() == -1){
        Serial.println("Failed to initialize gantry");
        while(1);
    }

    render_text_centered("Connecting to wifi...", 30);
    Serial.println("Connecting to wifi...");
    delay(500); // For some reason, modem can be unstable after flashing for a tiny duration
    init_server_client();

    //scan_shelves();

    // update_state(IDLE);
    push_event_to_queue({INIT_COMPLETE, ""});
    Serial.println("System initialized.");
}

void run_IDLE(){
    // The system is waiting for an event to occur. Either the system gets an event from the 
    // UI or the server. The system will be in this state until an event occurs.

    evaluate_UI(); // Check user input
    MQTTclient.loop();
    reconnect();
}

// Helper function
bool convertStringToUID(String uidString, uint8_t* uidArray) {
    if (uidString.length() != 12) {
        Serial.println("Invalid UID string length. Expected 12 characters.");
        return false;
    }

    // Directly copy each ASCII character into the uidArray
    for (int i = 0; i < 12; i++) {
        uidArray[i] = (uint8_t) uidString[i];
    }

    return true;
}

void run_GETTING_SHELF(String UID){
    // Go to shelf location where we expect to find UID
    // Check the UID
    //      FAIL: Go to home, REQUEST_SCAN
    // Pick up shelf
    // Go to user
    //      COMPLETE: event SHELF_FETCHED

    uint8_t target_UID[12];

    // Convert the String UID to a 12-byte array
    if (!convertStringToUID(UID, target_UID)) {
        Serial.println("Failed to convert String UID to byte array.");
        return;
    }
    Serial.print("Target UID: ");
    for (int i = 0; i < 12; i++) {
        Serial.print(target_UID[i], HEX);
    }
    // Move the gantry to the shelf location where we expect to find the target UID
    move_gantry_to_shelf_location(target_UID);

    // Verify the UID by reading it from the shelf
    uint8_t read_UID[12];
    if (!read_shelf_UID(read_UID) || memcmp(read_UID, target_UID, 12) != 0) {
        // If the UID doesn't match, go to the user and request a rescan
        move_gantry_to_user();
        push_event_to_queue({ SCANNING_REQUESTED, "" });
        return;
    }
    // Set the held shelf to the shelf with the target UID
    set_held_shelf(target_UID);
    // If the UID matches, proceed to pick up the shelf and move it to the user
    pick_up_shelf();
    // Set current shelf to the held shelf
    move_gantry_to_user();

    // Signal that the shelf has been fetched
    push_event_to_queue({ SHELF_FETCHED, "" });
}


void run_WAITING_FOR_USER_TO_INTERACT_WITH_SHELF(){
    // Wait for user to interact with shelf
    //      COMPLETE: event SHELF_INTERACTED

    //Dummy
    delay(1000);
    Serial.println("Waiting for user to interact with shelf");
    // Write to the UI that the user should interact with the shelf
    render_text_centered("Please interact with shelf", 30);
    delay(1000);
    push_event_to_queue({SHELF_INTERACTED, ""}); // This should not be called from here
    render_menu(main_menu);
}

void run_PUTTING_BACK_SHELF(){
    // Go to user
    // Put back shelf
    // Go to home
    //      COMPLETE: event SHELF_PUT_BACK
    delay(2000);
    Serial.println("Putting back shelf");
    
    put_back_shelf();
    push_event_to_queue({SHELF_PUT_BACK, ""});
}

void run_CALIBRATING(bool fromGettingShelf){
    // Calibrate the gantry
    //      COMPLETE: event CALIBRATION_COMPLETE

    Serial.print("Calibrating    ");
    calibrate_gantry();
    Serial.println("Calibration complete");
    if(fromGettingShelf){
        Serial.println("Returning to getting shelf");
        push_event_to_queue({SHELF_REQUESTED, ""});
    } else {
        push_event_to_queue({CALIBRATION_COMPLETE, ""});
    }
}

void run_SCANNING(){
    // Scan all shelves
    //      COMPLETE: event SCANNING_COMPLETE

    Serial.print("Scanning shelves   ");
    scan_shelves();
    Serial.println("scanning complete!");

    push_event_to_queue({SCANNING_COMPLETE, ""});
}

void run_SCAN_UID(){
    Serial.println("Scanning UID");
    uint8_t UID[12];
    read_shelf_UID(UID);
    push_event_to_queue({SCANNING_COMPLETE, ""});
}