#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor0;
VL53L0X sensor1;

const unsigned int sensor_xshut[] = {8, 7};

int distances[2];