#ifndef STATES_HPP
#define STATES_HPP

#include <Arduino.h>

void run_INIT();
void run_IDLE();
void run_GETTING_SHELF(String UID);
void run_PUTTING_BACK_SHELF();
void run_WAITING_FOR_USER_TO_INTERACT_WITH_SHELF();
void run_CALIBRATING(bool fromGettingShelf);
void run_SCANNING();
void run_SCAN_UID();

#endif // STATES_HPP
