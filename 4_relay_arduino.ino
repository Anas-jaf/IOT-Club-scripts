/*
  مشروع تشغيل 4 ريليهات (Relay 4 Module)
  يعمل من الأحد إلى الخميس - من 6 صباحًا إلى 12 ظهرًا
  **التعديل: اختبار شامل وبصري في setup() مع تأخير بين الحالات**
*/

#include <Wire.h>
#include "RTClib.h"

// استخدام شريحة DS1307
RTC_DS1307 rtc;

// تعريف قنوات الريلاي الأربعة
int relayPins[4] = {2, 3, 4, 5};
const int TEST_DELAY_MS = 2000; // تأخير 2 ثانية بين كل اختبار

// تصريح مسبق لدالة الاختبار التي تتحكم بالريليهات
void testLogicAndControlRelays(int testDay, int testHour);

// دالة مساعدة لطباعة اسم اليوم
String getDayName(int dayIndex) {
  String dayNames[] = {"الأحد", "الإثنين", "الثلاثاء", "الأربعاء", "الخميس", "الجمعة", "السبت"};
  if (dayIndex >= 0 && dayIndex <= 6) {
    return dayNames[dayIndex];
  }
  return "غير معروف";
}

void setup() {
  Serial.begin(9600);
  Serial.println("=================================================");
  Serial.println("--- Starting System Setup and Visual Tests ---");

  // إعداد الريليهات
  for (int i = 0; i < 4; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);  // إطفاء الريلاي في البداية
  }

  Wire.begin();

  if (!rtc.begin()) {
    Serial.println("❌ RTC DS1307 غير متصل! يرجى التحقق من التوصيلات.");
    while (1); 
  }
  
  // =========================================================
  // 🔔 مرحلة الاختبار الشامل: تعمل مرة واحدة فقط في setup()
  Serial.println("\n--- 🧠 Running Visual Test Cases ---");
  
// 1. (OFF): الإثنين، قبل وقت العمل (5:00 ص)
  testLogicAndControlRelays(1, 5); 
  
  // 2. (ON): الثلاثاء، وقت العمل (10:00 ص)
  testLogicAndControlRelays(2, 10);
  
  // 3. (OFF): السبت، وقت العمل (11:00 ص) - عطلة نهاية أسبوع
  testLogicAndControlRelays(6, 11); 
  
  // 4. (ON): الأحد، وقت العمل (6:00 ص) - بداية الحد الأدنى للتشغيل
  testLogicAndControlRelays(0, 6);  
  
  // 5. (OFF): الخميس، بعد وقت العمل (3:00 م)
  testLogicAndControlRelays(4, 15);
  
  // 6. (ON): الخميس، وقت العمل (11:00 ص) - الحد الأقصى للتشغيل
  testLogicAndControlRelays(4, 11);
  
  // 7. (OFF): الأربعاء، بعد وقت العمل (12:00 ظهراً) - حد الساعة غير المشمول
  testLogicAndControlRelays(3, 12); 
  
  // 8. (OFF): الجمعة، وقت العمل (8:00 ص) - عطلة نهاية أسبوع
  testLogicAndControlRelays(5, 8);  
  
  Serial.println("\n--- Tests Complete. Turning OFF all relays. ---");
  for (int i = 0; i < 4; i++) {
    digitalWrite(relayPins[i], HIGH);
  }
  delay(1000); // تأخير بسيط قبل الدخول إلى اللوب
  
  Serial.println("=================================================");
  Serial.println("Entering main loop, checking time every second...");
}

void loop() {
  // الكود هنا يستمر في العمل باستخدام القراءة الفعلية لـ RTC
  DateTime now = rtc.now();

  int day = now.dayOfTheWeek();    // 0 = الأحد ، 6 = السبت
  int hour = now.hour();

  // منطق التشغيل الفعلي
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

// 🔔 دالة الاختبار التي تنفذ المنطق وتتحكم بالريليهات
void testLogicAndControlRelays(int testDay, int testHour) {
  // تطبيق نفس منطق الكود الفعلي
  bool allowedDays = (testDay >= 0 && testDay <= 4); 
  bool allowedHours = (testHour >= 6 && testHour < 12); 
  
  Serial.print(" - Test Time: ");
  Serial.print(getDayName(testDay));
  Serial.print(" (");
  Serial.print(testHour);
  Serial.print(":00)");
  
  if (allowedDays && allowedHours) {
    // 💡 تشغيل الريليهات بصرياً
    for (int i = 0; i < 4; i++) {
      digitalWrite(relayPins[i], LOW); // تشغيل
    }
    Serial.println(" -> Result: ✅ ON (Wait 2s)");
  } else {
    // 💡 إطفاء الريليهات بصرياً
    for (int i = 0; i < 4; i++) {
      digitalWrite(relayPins[i], HIGH); // إطفاء
    }
    Serial.println(" -> Result: ❌ OFF (Wait 2s)");
  }
  
  // ⏱️ التأخير المطلوب للمراقبة البصرية
  delay(TEST_DELAY_MS); 
}
