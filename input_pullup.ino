// قم بتوصيل السلك الآخر مع GND وليس أي دبوس آخر
#define INPUT_PIN 8 
#define RELAY_PIN 4

void setup() {
  Serial.begin(9600);

  pinMode(INPUT_PIN, INPUT_PULLUP);  // إدخال مع مقاومة سحب داخلية
  pinMode(RELAY_PIN, OUTPUT);        // مخرج للمرحل

  digitalWrite(RELAY_PIN, LOW);      // افتراضياً: المرحل متوقف
}

void loop() {
  int inputState = digitalRead(INPUT_PIN);

  Serial.println(inputState);  // طباعة حالة الدبوس 8

  if (inputState == LOW) {
    // الدبوس موصول بالأرض → نشغّل المرحل
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    // الدبوس غير موصول → نوقف المرحل
    digitalWrite(RELAY_PIN, LOW);
  }

  delay(200); // تأخير بسيط لتقليل سرعة الطباعة
}
