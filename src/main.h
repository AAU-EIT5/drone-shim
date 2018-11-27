#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "VL53L0X.h"
#include "iBUS.h"

VL53L0X sensor0;
VL53L0X sensor1;

const uint8_t eeprom_addr_percent = 10;
bool eeprom_written = false;

const unsigned int sensor_xshut[] = {7, 8};
iBus ibus(Serial2);

int distances[2];

void init_sensors();