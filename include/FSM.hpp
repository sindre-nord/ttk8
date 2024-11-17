#ifndef FSM_HPP
#define FSM_HPP

#include <Arduino.h>
/*
State explenations:
IDLE: The system is waiting for an event to occur. Either the system gets an event from the 
        UI or the server.
GETTING_SHELF: The system is controlling the gantry to fetch a shelf.
PUTTING_BACK_SHELF: The system is controlling the gantry to put back a shelf.
WAITING_FOR_USER_TO_INTERACT_WITH_SHELF: The system is waiting for the user to interact with the shelf
        
CALIBRATING: The system is calibrating the gantry. This means returning the number of steps it takes
        to traverse the whole axis.
SCANNING: The system is mapping the shelves in the warehouse. This is done by going over every shelf
        position and scanning the tag. The system will update the server with the new info.
*/

enum State {
    INIT,
    IDLE,
    GETTING_SHELF, //Very illegal to touch system after this state goes to WAITING_FOR_USER_TO_INTERACT_WITH_SHELF
    PUTTING_BACK_SHELF,
    WAITING_FOR_USER_TO_INTERACT_WITH_SHELF, // NO TOUCHY SYSTEM
    CALIBRATING,
    SCANNING,
    SCAN_UID
};

class FSM {
    public:
        FSM();
        State getState();
        State getPreviousState();
        void evaluate_event_queue();
        void run_FSM();
        void setState(State state);
        void setTargetUID(String UID);
        String getTargetUID();
    private:
        State state;
        State previous_state;
        String targetUID;
};

extern FSM fsm;

enum EventName{
    NO_EVENT,
    INIT_COMPLETE,
    SHELF_REQUESTED,
    SHELF_FETCHED,
    SHELF_PUT_BACK,
    SHELF_INTERACTED,
    CALIBRATION_REQUESTED,
    CALIBRATION_COMPLETE,
    SCANNING_REQUESTED,
    SCANNING_COMPLETE,
    REQUEST_UID_READ,
    ERROR_OCCURED
};

typedef struct Event{
    EventName event_name;
    String value; // Only used for UID at the moment
} Event_t;

// Event buffer 
// Event event_buffer[10];
// int event_buffer_index;

void push_event_to_queue(Event event);
Event pop_event_queue();


void evaluate_event_queue(Event event);
void run_FSM();

#endif // FSM_HPP