/* 
  مشروع تشغيل 4 ريليهات (Relay 4 Module)
  يعمل من الأحد إلى الخميس - من 6 صباحًا إلى 12 ظهرًا
  ملاحظة: الكود من ChatGPT
*/

#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

// تعريف قنوات الريلاي الأربعة
int relayPins[4] = {7, 8, 9, 10};

void setup() {
  // إعداد كل ريلاي كمخرج
  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);  // إطفاء الريلاي (Active LOW)
  }

  Wire.begin();
  rtc.begin();

  // استخدم هذا مرة واحدة فقط لضبط الساعة:
  // rtc.adjust(DateTime(F(_DATE), F(TIME_)));
}

void loop() {
  DateTime now = rtc.now();

  int day = now.dayOfTheWeek();   // 0 = الأحد ، 6 = السبت
  int hour = now.hour();

  bool allowedDays = (day >= 0 && day <= 4);  // الأحد إلى الخميس
  bool allowedHours = (hour >= 6 && hour < 12); // من 6 إلى 12 ظهرًا

  if (allowedDays && allowedHours) {
    // تشغيل الريليهات كلها
    for (int i = 0; i < 4; i++) {
      digitalWrite(relayPins[i], LOW);  // تشغيل (Active LOW)
    }
  } else {
    // إطفاء الريليهات كلها
    for (int i = 0; i < 4; i++) {
      digitalWrite(relayPins[i], HIGH);
    }
  }

  delay(1000); // تحديث كل ثانية
}
