 // if you are using arduino uno you have to connect  module scl , sal with arduino a4 and a5 not scl , sal ports 
#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Always set the time for demonstration (you can add checks)
  Serial.println("Setting RTC time to compile time + 10 seconds...");

  // Get compile time
  DateTime compileTime(__DATE__, __TIME__);

  // Add 10 seconds manually for accommodate for the time of uploading to arduino 
  uint32_t adjustedUnix = compileTime.unixtime() + 8;
  DateTime adjustedTime(adjustedUnix);  // Create new DateTime with adjusted time

  // Apply the adjusted time
  rtc.adjust(adjustedTime);

  Serial.print("Adjusted time set: ");
  Serial.print(adjustedTime.timestamp());  // ISO 8601 format
  Serial.println();
}

void loop() {
  DateTime now = rtc.now();

  Serial.print("Current RTC time: ");
  Serial.println(now.timestamp());

  delay(1000);
}
