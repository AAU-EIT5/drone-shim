#include <main.h>

#define DEBUG_PIN 12

void setup() 
{
  ibus.set_alive_timeout(30);

  Serial.begin(115200);
  while(!Serial && millis() < 5000)
  {;}
  delay(5);

  while(!Q.isFull())
  {
    uint16_t x = 0;
    Q.push(&x);
  }

  Serial.println("Dumping EEPROM");
  eeprom_print();

  ibus_ticker.begin(ibus_ticker_isr, ibus_ticker_period);
  ibus_ticker.priority(100);

  throttle_sample_ticker.begin(throttle_sample, throttle_sample_period);
  throttle_sample_ticker.priority(200);

  init_sensors();

  pinMode(LED_BUILTIN, OUTPUT);

  #if DEBUG_PIN
    pinMode(DEBUG_PIN, OUTPUT);
  #endif
  

  //attachInterrupt(sensor_int[0], sensor0_data_ready, FALLING);
  //attachInterrupt(sensor_int[1], sensor1_data_ready, FALLING);

}

void loop()
{
  handle_sensors();
  ibus_wrap(); // Little function to do the repeated iBUS things
  
  aux_handle();
  
  // Print sensor readings
  debug_print(500);
}

bool eeprom_unflushed = false;
void aux_handle()
{
  int aux = ibus.get_channel(5);
  int throttle = 0;
  int step = 100; // Step in mm

  const int reg_min = -50, reg_max = 50;

  if(aux >= 1400 && aux < 1700) // If switch is in the middle pos
  {
    eeprom_unflushed = true;
    if(grav_offset == 0) 
    {
      grav_offset = ibus.get_tx_channel(2); // Set the gravity offset
      distance_set_point_base = distances[0][1]; // Set the base setpoint
      Serial.print("G: "); Serial.println(grav_offset);
    }

    throttle = regulator(distance_set_point_base, reg_min, reg_max) + grav_offset;

  }
  else if(aux >= 1700)
  {
    eeprom_unflushed = true;
    throttle = regulator(distance_set_point_base + step, reg_min, reg_max) + grav_offset;
  }
  else
  {
    throttle = ibus.get_channel(2);
    grav_offset = 0;

    if(eeprom_unflushed)
    {
      eeprom_unflushed = false;
      flush_to_eeprom();
    }

  }

  ibus.set_channel(2, throttle);
}

void flush_to_eeprom()
{
  noInterrupts();
  for(int i=0; i<1000; i++)
  {
    if(!(i%10))
    {
      ibus_ticker_isr();
    }
    uint16_t x;
    Q.pop(&x);
    Serial.print(i); Serial.print(": "); Serial.println(x);
    EEPROM.write(eeprom_addr_offset + i*2, (x>>8));
    EEPROM.write(eeprom_addr_offset + i*2 + 1, (x & 0x00FF));
  }

  //uint8_t jitter[2], sample_time[2];
  
  //jitter[0] = max_jitter >> 8; jitter[1] = max_jitter & 0x00FF;
  //sample_time[0] = average_sample_rate >> 8; sample_time[1] = average_sample_rate & 0x00FF;

  //EEPROM.write(6, jitter[0]);
  //EEPROM.write(7, jitter[1]);
  //EEPROM.write(8, sample_time[0]);
  //EEPROM.write(9, sample_time[1]);

  interrupts();
}

void eeprom_print()
{
  for(int i=0; i<1000; i++)
  {
    uint16_t x = (EEPROM.read(eeprom_addr_offset + i*2) << 8) | EEPROM.read(eeprom_addr_offset + i*2 +1);
    Serial.print(x);Serial.print(',');
  }
  Serial.println('\n');
  //int jitter = (EEPROM.read(6) << 8) | EEPROM.read(7);
  //int sample_time = (EEPROM.read(8) << 8) | EEPROM.read(9);
  //Serial.print("Jitter: "); Serial.println(jitter);
  //Serial.print("Sample: "); Serial.println(sample_time);
}

int regulator(int sp, int min, int max)
{
  int Err = sp - distances[0][1];
  
  int out = Kp * Err;

  if(out > max)
  {
    out = max;
  }
  if(out < min)
  {
    out = min;
  }

  return out;
}

void ibus_wrap()
{
  // Pass recieved channels transparrently
  for(int i=0; i<14; i++)
  {
    if(i == 2) // Skip throttle
    {
      if(ibus.get_channel(5) <= 1400)
      {
        // Scale throttle to 50% max
        ibus.set_channel(i, map(ibus.get_channel(i), 1000, 2000, 1000, 1500));
      }
    }
    else
    {
      ibus.set_channel(i, ibus.get_channel(i));
    }
  }

  // If RF is lost, turn on LED
  if(ibus.is_alive())
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void init_sensors()
{
  Wire.begin();

  // Turn off all sensors
  for(int i=0; i<sizeof(sensor_xshut)/sizeof(sensor_xshut[0]); i++)
  {
    pinMode(sensor_xshut[i], OUTPUT);
    digitalWrite(sensor_xshut[i], LOW);
  }

  // Enable the first sensor
  Serial.println("Enable sensor 0");
  pinMode(sensor_xshut[0], INPUT); 
  delay(150);
  sensor0.setTimeout(100);
  sensor0.setAddress(0x40);
  sensor0.init();

  //Serial.println("Enable sensor 1");
  //pinMode(sensor_xshut[1], INPUT); // Enable the second sensor
  //delay(150);
  //sensor1.setTimeout(100);
  //sensor1.setAddress(0x50);
  //sensor1.init();

  sensor0.setMeasurementTimingBudget(timing_budget);
  ////sensor1.setMeasurementTimingBudget(timing_budget);


  Serial.println("Starting continous ranging");
  sensor0.startContinuous();
  delay(5); // Start the second sensor slightly delayed, to limit data_ready interrupt colisions
  //sensor1.startContinuous();
}

void pulse()
{
  #if DEBUG_PIN
    digitalWrite(DEBUG_PIN, !digitalRead(DEBUG_PIN));

  #endif
}

void handle_sensors()
{
  //if(sensor_data_ready[0])
  //{
    //Serial.println("beep0");
    //sensor_data_ready[0] = false;
    distances[0][0] = distances[0][1];
    distances[0][1] = sensor0.readRangeContinuousMillimeters();
    if (sensor0.timeoutOccurred()) { Serial.print(" TIMEOUT"); } else {pulse();}

    
    //samples++;
    //int x = millis() - last_sample;
    //last_sample = millis();
//
    //average_sample_rate = average_sample_rate * (samples-1)/samples  +  x/samples;
    //
    //if(x > max_jitter)
    //{
    //  max_jitter = x;
    //}
  
  //}
  
  //if(sensor_data_ready[1])
  //{
    //Serial.println("beep1");
    //sensor_data_ready[1] = false;
    //distances[1][0] = distances[1][1];
    //distances[1][1] = sensor1.readRangeContinuousMillimeters();
    //if (sensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  //}
}

unsigned long last_send = 0;
void debug_print(unsigned int send_period)
{
  #if SERIAL_DBG > 0
  if(millis() - last_send > send_period)
  {
    last_send = millis();
    Serial.print(distances[0][1]); Serial.print(" | "); Serial.println(distances[1][1]);

    #if SERIAL_DBG > 1
    //code
    #endif
  }
  #endif
}

void throttle_sample()
{
  uint16_t throttle = ibus.get_tx_channel(2);
  Q.push(&throttle);
  //Serial.println(throttle);
}

void ibus_ticker_isr()
{
  ibus.handle(2);
}