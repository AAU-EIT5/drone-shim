#include <main.h>

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

  sensor0.setMeasurementTimingBudget(25000);
  sensor1.setMeasurementTimingBudget(25000);


  Serial.println("Starting continous ranging");
  sensor0.startContinuous();
  sensor1.startContinuous();
}

unsigned long last_send = 0;

void loop()
{
  distances[0] = sensor0.readRangeContinuousMillimeters();
  if (sensor0.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  distances[1] = sensor1.readRangeContinuousMillimeters();
  if (sensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

  count++;

  if(millis() - last_send > 1000)
  {
    last_send = millis();
    Serial.print(count); Serial.print(": "); Serial.print(distances[0]); Serial.print(" | "); Serial.println(distances[1]);
  }
}

30: 45 | 45
85: 48 | 46
140: 43 | 49
195: 43 | 48
250: 43 | 46
305: 42 | 45
360: 46 | 44
415: 45 | 51
470: 52 | 53
525: 47 | 57
580: 42 | 51
635: 49 | 52
690: 47 | 53
745: 48 | 50
800: 50 | 55
855: 44 | 56
910: 51 | 54
965: 44 | 50
1020: 49 | 50
1075: 48 | 52
1130: 49 | 49
1185: 48 | 49
1240: 45 | 52
1295: 47 | 52
1350: 47 | 48
1405: 41 | 53
1460: 50 | 48
1515: 47 | 46
1570: 47 | 50
1625: 48 | 49
1680: 44 | 48
1735: 51 | 52
1790: 48 | 54
1845: 48 | 51
1900: 47 | 52
1955: 52 | 48
2010: 47 | 50