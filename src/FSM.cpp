#include "FSM.hpp"
#include "states.hpp"
#include "UI.hpp"
#include "gantry.hpp"

// Define the global FSM object
FSM fsm;

// Define the event buffer and index
Event event_buffer[10];
int events_in_buffer = 0;

// FSM functions
FSM::FSM() {
    state = INIT;
}

State FSM::getState() {
    return state;
}
State FSM::getPreviousState() {
    return previous_state;
}

void FSM::setTargetUID(String UID) {
    this->targetUID = UID;
}
String FSM::getTargetUID() {
    return this->targetUID;
}

// This function could trigger a start and end functionality for the 
// states. An example would be to write busy to the screen when going 
// out of IDLE.
void FSM::setState(State state) {
    this->previous_state = this->state;
    this->state = state;
    if(state == IDLE){
        render_menu(main_menu);
    }
}

Event pop_event_queue() {
    Event event = event_buffer[events_in_buffer-1];
    events_in_buffer--;
    return event;
}

void push_event_to_queue(Event event) {
    event_buffer[events_in_buffer] = event;
    events_in_buffer++;
}

void evaluate_event_queue() {
    // Check if there are any events in the event buffer
    Event event = {NO_EVENT, ""};
    if (events_in_buffer > 0) {
        event = pop_event_queue();
    }
    // Check if the event is changing away from IDLE,
    // if so, write "Busy" to the display
    if (event.event_name != NO_EVENT && fsm.getState() != IDLE) {
        render_text_centered("Busy", 30);
    }
    switch (event.event_name) {
        case SHELF_REQUESTED:
            fsm.setState(GETTING_SHELF);
            if (event.value != "") { // Allows returning to look for same shelf after scanning
                fsm.setTargetUID(event.value);
            }
            break;
        case SHELF_FETCHED:
            fsm.setState(WAITING_FOR_USER_TO_INTERACT_WITH_SHELF);
            break;
        case SHELF_INTERACTED:
            fsm.setState(PUTTING_BACK_SHELF);
            break;
        case SHELF_PUT_BACK:
            fsm.setState(IDLE);
            break;
        case CALIBRATION_REQUESTED:
            fsm.setState(CALIBRATING);
            break;
        case CALIBRATION_COMPLETE:
            fsm.setState(IDLE);
            break;
        case SCANNING_REQUESTED:
            fsm.setState(SCANNING);
            break;
        case SCANNING_COMPLETE:
            fsm.setState(IDLE);
            break;
        case INIT_COMPLETE:
            fsm.setState(IDLE);
            break;
        case REQUEST_UID_READ:
            fsm.setState(SCAN_UID);
            break;
        case ERROR_OCCURED:
            Serial.println("Some error occured... Going back to idle");
            fsm.setState(IDLE);
            break;
        case NO_EVENT:
            break;
        default:
            Serial.println("Unknown event");
            break;
    }
    //render_menu(main_menu);
}

void run_FSM() {
    evaluate_event_queue();
    switch (fsm.getState()) {
        case INIT:
            run_INIT();
            break;
        case IDLE: // This is usually the default state
            run_IDLE();
            break;
        case GETTING_SHELF:
            run_GETTING_SHELF(fsm.getTargetUID());
            break;
        case PUTTING_BACK_SHELF:
            run_PUTTING_BACK_SHELF();
            break;
        case WAITING_FOR_USER_TO_INTERACT_WITH_SHELF:
            run_WAITING_FOR_USER_TO_INTERACT_WITH_SHELF();
            break;
        case CALIBRATING:
            render_text_centered("Calibrating", 30);
            if (fsm.getPreviousState() == GETTING_SHELF) {
                run_CALIBRATING(true);
            } else {
                run_CALIBRATING(false);
            }
            break;
        case SCANNING:
            run_SCANNING();
            break;
        case SCAN_UID:
            run_SCAN_UID();
            break;
        default:
            break;
    }
}