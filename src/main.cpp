#include <main.h>
#include <EEPROM.h>

void setup() 
{
  Serial.begin(115200);
  while(!Serial && millis() < 5000)
  {;}

  Serial.println(EEPROM.read(eeprom_addr_percent));

  init_sensors();

  pinMode(LED_BUILTIN, OUTPUT);
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
  sensor0.setAddress(0xA0);
  sensor0.init();

  Serial.println("Enable sensor 1");
  pinMode(sensor_xshut[1], INPUT); // Enable the second sensor
  delay(150);
  sensor1.setTimeout(500);
  sensor1.setAddress(0xB0);
  sensor1.init();

  sensor0.setMeasurementTimingBudget(25000);
  sensor1.setMeasurementTimingBudget(25000);


  Serial.println("Starting continous ranging");
  sensor0.startContinuous();
  sensor1.startContinuous();
}

void handle_sensors()
{
  distances[0] = sensor0.readRangeContinuousMillimeters();
  if (sensor0.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  distances[1] = sensor1.readRangeContinuousMillimeters();
  if (sensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
}

bool has_run = false;
void aux_handle()
{
  if(ibus.get_channel(5) < 1450)
  {
    has_run = false;
  }
  if(has_run)
  {
    return;
  }
  // 
  // // If SWD is thrown, save the current throttle value
  // if(ibus.get_channel(5) > 1300 && !eeprom_written)
  // {
    // eeprom_written = true;
    // EEPROM.write(eeprom_addr_percent, map(ibus.get_channel(2), 1000, 2000, 0, 100));
  // }

  /* If SWD is thrown:
   * step throttle to 30% for 1 second,
   * then to 50% for 1 second, 
   * then to 30% for 1 second, 
   * then pass through stick value
   * wait for SWD to be released
  */

  const int baseline1 = 10000, step1 = 1000, baseline2 = 10000;

  uint32_t start = millis();
  while(ibus.get_channel(5) > 1550 && (millis() - start) < (baseline1 + baseline2 + step1))
  {
    has_run = true;
    ibus.handle();
    // Pass through all channels, because we're stuck in this while
    for(int i=0; i<14; i++)
    {
      ibus.set_channel(i, ibus.get_channel(i));
    }
  
    // Modify throttle
    if(millis() - start < baseline1)
    {
      ibus.set_channel(2, 1300); // 30%
    }
    else if(millis() - start < (baseline1 + step1))
    {
      ibus.set_channel(2, 1350); // 35%
    } 
    else if(millis() - start < (baseline1 + step1 + baseline2))
    {
      ibus.set_channel(2, 1300); // 30%
    }
    else
    {
      break;
    }
  }
}

unsigned long last_send = 0;
void loop()
{
  ibus.handle();
  handle_sensors();

  // Pass recieved channels transparrently
  for(int i=0; i<14; i++)
  {
    ibus.set_channel(i, ibus.get_channel(i));
  }

  // Do stuff based on aux channel
  aux_handle();

  // If RF is lost, turn on LED
  if(ibus.is_alive())
  {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  
  // Print sensor readings
  #if SERIAL_DBG
  if(millis() - last_send > 1000)
  {
    last_send = millis();
    Serial.print(": "); Serial.print(distances[0]); Serial.print(" | "); Serial.println(distances[1]);
  }
  #endif
}