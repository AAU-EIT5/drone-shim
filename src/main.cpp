#include <main.h>
#include <EEPROM.h>

void setup() 
{
  Serial.begin(115200);
  while(!Serial && millis() < 5000)
  {;}

  int base = (EEPROM.read(0) << 8) | EEPROM.read(1);
  int step = (EEPROM.read(2) << 8) | EEPROM.read(3);
  
  Serial.print("Base: "); Serial.println(base);
  Serial.print("Step: "); Serial.println(step);

  init_sensors();

  pinMode(LED_BUILTIN, OUTPUT);
}

int base = 0, step = 0;

void init_sensors()
{
  Wire.begin();

  // Enable the first sensor
  Serial.println("Enable sensor 0");
  pinMode(sensor_xshut[0], INPUT); 
  delay(150);
  sensor0.setTimeout(500);
  sensor0.setAddress(0xA0);
  sensor0.init();

  sensor0.setMeasurementTimingBudget(25000);

  Serial.println("Starting continous ranging");
  sensor0.startContinuous();
}

void handle_sensors()
{
  distances[0] = sensor0.readRangeContinuousMillimeters();
  if (sensor0.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
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
 
 
  if(ibus.get_channel(5) > 1550)
  {
    base = ibus.get_channel(2);
    step = base + 10;  
    uint32_t start = millis();
    while(ibus.get_channel(5) > 1550 && (millis() - start) < 1000)
    {
      has_run = true;
      ibus.handle();
      // Pass through all channels, because we're stuck in this while
      for(int i=0; i<14; i++)
      {
        ibus.set_channel(i, ibus.get_channel(i));
      }

      // Modify throttle
      ibus.set_channel(2, step);
    }
    EEPROM.write(0, (base >> 8)); EEPROM.write(1, (base & 0x00FF));
    EEPROM.write(2, (step >> 8)); EEPROM.write(3, (step & 0x00FF));
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