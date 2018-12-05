#include <main.h>

void setup() 
{
  Serial.begin(115200);
  while(!Serial && millis() < 5000)
  {;}

  init_sensors();

  pinMode(LED_BUILTIN, OUTPUT);

  //attachInterrupt(sensor_int[0], sensor0_data_ready, FALLING);
  //attachInterrupt(sensor_int[1], sensor1_data_ready, FALLING);

}

void loop()
{

  ibus.handle();
  handle_sensors();
  ibus_wrap(); // Little function to do the repeated iBUS things

  distance_set_point = map(ibus.get_channel(5), 1000, 2000, 0, 500);
  
  if(distance_set_point > 200)
  {
    ibus.set_channel(2, 1000 + regulator(0, 350));
  } 
  
  // Print sensor readings
  debug_print(500);
}

int regulator(int min, int max)
{
  int Err = distance_set_point - distances[0][1];
  
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
    ibus.set_channel(i, ibus.get_channel(i));
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
  sensor0.setTimeout(500);
  sensor0.setAddress(0x10);
  sensor0.init();

  Serial.println("Enable sensor 1");
  pinMode(sensor_xshut[1], INPUT); // Enable the second sensor
  delay(150);
  sensor1.setTimeout(500);
  sensor1.setAddress(0x20);
  sensor1.init();

  Serial.print(sensor0.getAddress(), HEX); Serial.print(" : "); Serial.println(sensor1.getAddress(), HEX);

  sensor0.setMeasurementTimingBudget(timing_budget);
  sensor1.setMeasurementTimingBudget(timing_budget);


  Serial.println("Starting continous ranging");
  sensor0.startContinuous();
  delay(5); // Start the second sensor slightly delayed, to limit data_ready interrupt colisions
  sensor1.startContinuous();
}

void handle_sensors()
{
  //if(sensor_data_ready[0])
  //{
    //Serial.println("beep0");
    //sensor_data_ready[0] = false;
    distances[0][0] = distances[0][1];
    distances[0][1] = sensor0.readRangeContinuousMillimeters();
    if (sensor0.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  //}
  
  //if(sensor_data_ready[1])
  //{
    //Serial.println("beep1");
    //sensor_data_ready[1] = false;
    distances[1][0] = distances[1][1];
    distances[1][1] = sensor1.readRangeContinuousMillimeters();
    if (sensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
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

void sensor0_data_ready()
{
  sensor_data_ready[0] = true;
  sensor_sample_times[0][0] = sensor_sample_times[0][1];
  sensor_sample_times[0][1] = micros();
}

void sensor1_data_ready()
{
  sensor_data_ready[1] = true;
  sensor_sample_times[1][0] = sensor_sample_times[1][1];
  sensor_sample_times[1][1] = micros();
}