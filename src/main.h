#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include "VL53L0X.h"
#include "iBUS.h"
#include "cppQueue.h"


VL53L0X sensor0;
VL53L0X sensor1;
const int timing_budget = 25000; // micro seconds
const unsigned int sensor_xshut[] = {16, 14};
const unsigned int sensor_int[] = {17, 15};
volatile uint32_t sensor_sample_times[2][2]; // Last two times the sensors were sampled [uS]
void init_sensors();
void handle_sensors();

// Tuning constants
const float Kp = 0.05;
const float Ki = 0;
const float Kd = 0;

// Eeprom
IntervalTimer throttle_sample_ticker;
const int throttle_sample_period = 20000; // 20 ms between samples
const uint8_t eeprom_addr_offset = 10;
Queue	Q(sizeof(uint16_t), 1000, FIFO, true);
void flush_to_eeprom();
void eeprom_print();

// Coms
IntervalTimer ibus_ticker;
const int ibus_ticker_period = 10000; // 10 ms between handling ibus
iBus ibus(Serial2);
void ibus_wrap();
void ibus_ticker_isr();


// Control loop related
int distances[2][2]; // Stores two samples, of two different distances. d[0][0] = sensor 0, last distance, d[0][1] is lastest distance
int distance_set_point_base = 0;
int grav_offset = 0;
int regulator(int sp, int min, int max);
void aux_handle();

uint32_t last_sample = 0;
int max_jitter = 0;
int samples = 0;
int average_sample_rate = 0;


void debug_print(unsigned int send_period = 500);




// ISRs and flags
void throttle_sample();