/*
   تشغيل 6 ريليهات بنظام 24 ساعة + اختبارات كاملة
   أيام العمل: الأحد–الخميس
   ساعات العمل: من 06:00 إلى 11:59
*/

#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

// 6 ريليهات من Pin 2 إلى Pin 7
int relayPins[6] = {2, 3, 4, 5, 6, 7};

const int TEST_DELAY_MS = 2000; // تأخير 2 ثانية بين كل اختبار

void testLogicAndControlRelays(int testDay, int testHour);

String getDayName(int d) {
  String names[] = {"الأحد", "الإثنين", "الثلاثاء", "الأربعاء", "الخميس", "الجمعة", "السبت"};
  return names[d];
}

void force24HourMode() {
  // قراءة سجل الساعة من DS1307
  Wire.beginTransmission(0x68);
  Wire.write(0x02);  // مسجل الساعة
  Wire.endTransmission();

  Wire.requestFrom(0x68, 1);
  byte hourReg = Wire.read();

  // إذا كانت الشريحة على نظام 12 ساعة → نحولها 24 ساعة
  if (hourReg & 0b01000000) {
    hourReg &= 0b10111111; // clear bit 6 → enforce 24h format

    Wire.beginTransmission(0x68);
    Wire.write(0x02);
    Wire.write(hourReg);
    Wire.endTransmission();

    Serial.println("✔ تم تحويل DS1307 إلى نظام 24 ساعة.");
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("=================================================");
  Serial.println("🔧 بدء الإعداد وتشغيل الاختبارات…");

  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("❌ RTC DS1307 غير متصل!");
    while (1);
  }

  // إجبار نظام 24 ساعة
  force24HourMode();

  // تجهيز الريليهات
  for (int i = 0; i < 6; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);
  }

  // ===============================
  // تشغيل جميع الاختبارات
  // ===============================
  Serial.println("\n--- 🧠 بدء الاختبارات (نظام 24 ساعة) ---");

  testLogicAndControlRelays(1, 5);   // الاثنين 05:00 → OFF
  testLogicAndControlRelays(2, 10);  // الثلاثاء 10:00 → ON
  testLogicAndControlRelays(6, 11);  // السبت 11:00 → OFF
  testLogicAndControlRelays(0, 6);   // الأحد 06:00 → ON
  testLogicAndControlRelays(4, 15);  // الخميس 15:00 → OFF
  testLogicAndControlRelays(4, 11);  // الخميس 11:00 → ON
  testLogicAndControlRelays(3, 12);  // الأربعاء 12:00 → OFF
  testLogicAndControlRelays(5, 8);   // الجمعة 08:00 → OFF

  Serial.println("\n🔚 نهاية الاختبارات، إطفاء جميع الريليهات...");
  for (int i = 0; i < 6; i++) digitalWrite(relayPins[i], HIGH);

  Serial.println("=================================================");
  Serial.println("الدخول إلى الوضع التشغيلي الحقيقي…");
}

void loop() {

  // تأكيد دائم أن DS1307 تعمل 24 ساعة
  force24HourMode();

  // قراءة الوقت الحقيقي
  DateTime now = rtc.now();

  int day  = now.dayOfTheWeek(); // 0 = الأحد
  int hour = now.hour();         // نظام 24 ساعة

  bool allowedDays = (day >= 0 && day <= 4);   // الأحد → الخميس
  bool allowedHours = (hour >= 6 && hour < 12);

  if (allowedDays && allowedHours) {
    for (int i = 0; i < 6; i++) digitalWrite(relayPins[i], LOW);
  } else {
    for (int i = 0; i < 6; i++) digitalWrite(relayPins[i], HIGH);
  }

  delay(1000);
}

// ===============================
// دالة الاختبار البصري
// ===============================
void testLogicAndControlRelays(int testDay, int testHour) {

  bool allowedDays = (testDay >= 0 && testDay <= 4);
  bool allowedHours = (testHour >= 6 && testHour < 12);

  Serial.print("📝 اختبار: ");
  Serial.print(getDayName(testDay));
  Serial.print(" - ");
  Serial.print(testHour);
  Serial.print(":00");

  if (allowedDays && allowedHours) {
    for (int i = 0; i < 6; i++) digitalWrite(relayPins[i], LOW);
    Serial.println(" → 🟢 تشغيل (ON)");
  } else {
    for (int i = 0; i < 6; i++) digitalWrite(relayPins[i], HIGH);
    Serial.println(" → 🔴 إيقاف (OFF)");
  }

  delay(TEST_DELAY_MS);
}
