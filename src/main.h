#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "VL53L0X.h"
#include "iBUS.h"

VL53L0X sensor0;
VL53L0X sensor1;

const int timing_budget = 25000 // micro seconds

const int Kp = 0.1;

const uint8_t eeprom_addr_percent = 10;
bool eeprom_written = false;

const unsigned int sensor_xshut[] = {16, 14};
const unsigned int sensor_int[] = {17, 15};
iBus ibus(Serial2);

int distances[2];
int distance_set_point = 0;

void init_sensors();
void handle_sensors();
void debug_print(int send_period = 500);
int regulator();