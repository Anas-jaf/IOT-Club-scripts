/* * كود تشغيل حساس XLINE مع ESP32
 * هذا الكود يقرأ 16 قناة من خلال Multiplexer
 */

const int s0 = 18;
const int s1 = 19;
const int s2 = 21;
const int s3 = 22;
const int outPin = 23;

void setup() {
  Serial.begin(115200);
  
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(outPin, INPUT);
  
  Serial.println("XLINE Sensor Board Initialized...");
}

void loop() {
  String sensorData = "";

  for (int i = 0; i < 16; i++) {
    // اختيار الحساس المطلوب باستخدام النظام الثنائي (Binary Selection)
    digitalWrite(s0, (i & 0x01));
    digitalWrite(s1, (i >> 1) & 0x01);
    digitalWrite(s2, (i >> 2) & 0x01);
    digitalWrite(s3, (i >> 3) & 0x01);

    // تأخير بسيط جداً لاستقرار الإشارة (مهم جداً مع سرعة ESP32)
    delayMicroseconds(5); 

    // قراءة حالة الحساس (0 للأسود، 1 للأبيض أو العكس حسب النوع)
    int val = digitalRead(outPin);
    
    sensorData += String(val) + " ";
  }

  // طباعة النتائج
  Serial.println(sensorData);
  
  delay(50); // سرعة التحديث
}
