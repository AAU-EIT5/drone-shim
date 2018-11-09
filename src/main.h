#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor0;
VL53L0X sensor1;

const unsigned int sensor_xshut[] = {7, 8};

uint32_t count=0;

int distances[2];