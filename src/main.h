#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "VL53L0X.h"
#include "iBUS.h"

VL53L0X sensor0;
VL53L0X sensor1;

const int timing_budget = 25000; // micro seconds

const float Kp = 0.012;
const float Ki = 0;
const float Kd = 0;

const uint8_t eeprom_addr_percent = 10;
bool eeprom_written = false;

const unsigned int sensor_xshut[] = {16, 14};
const unsigned int sensor_int[] = {17, 15};
iBus ibus(Serial2);

volatile uint32_t sensor_sample_times[2][2]; // Last two times the sensors were sampled [uS]

int distances[2][2]; // Stores two samples, of two different distances. d[0][0] = sensor 0, last distance, d[0][1] is lastest distance
int distance_set_point = 0;

void init_sensors();
void handle_sensors();
void debug_print(unsigned int send_period = 500);
int regulator(int min, int max);
void ibus_wrap();

// ISRs and flags
volatile bool sensor_data_ready[] = {0,0};
void sensor0_data_ready();
void sensor1_data_ready();