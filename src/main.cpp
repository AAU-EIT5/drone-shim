#include <main.h>
// Hello world!

void setup() 
{

  Serial.begin(115200);
  Wire.begin();

  for(int i=0; i<sizeof(sensor_xshut)/sizeof(sensor_xshut[0]); i++)
  {
    pinMode(sensor_xshut[i], OUTPUT);
    digitalWrite(sensor_xshut[i], LOW);
  }

  Serial.println("Enabled sensor 0");
  pinMode(sensor_xshut[0], INPUT); // Enable the first sensor
  delay(150);

  Serial.println("Init sensor 0");
  sensor0.setTimeout(500);
  sensor0.setAddress(0xA0);
  sensor0.init();

  Serial.println("Enabled sensor 1");
  pinMode(sensor_xshut[1], INPUT); // Enable the second sensor
  delay(150);

  sensor1.setTimeout(500);
  sensor1.setAddress(0xB0);
  sensor1.init();

  Serial.println("Starting continous ranging");
  sensor0.startContinuous(100);
  sensor1.startContinuous(100);
}

unsigned long last_send = 0;

void loop()
{
  distances[0] = sensor0.readRangeContinuousMillimeters();
  if (sensor0.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  distances[1] = sensor1.readRangeContinuousMillimeters();
  if (sensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  if(millis() - last_send > 1000)
  {
    last_send = millis();
    Serial.print(distances[0]); Serial.print(" | "); Serial.println(distances[1]);
  }
}
