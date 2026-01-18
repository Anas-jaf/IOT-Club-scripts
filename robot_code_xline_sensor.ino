// --- دبابيس حساس XLINE ---
const int s0 = 18;
const int s1 = 19;
const int s2 = 21;
const int s3 = 22;
const int outDigital = 23; // المخرج الرقمي
const int outAnalog = 5;   // المخرج التماثلي P5

// --- دبابيس درايفر L298 ---
const int IN1 = 26;
const int IN2 = 25;
const int IN3 = 33;
const int IN4 = 32;

void setup() {
  Serial.begin(115200);
  
  // إعداد دبابيس الحساس
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(outDigital, INPUT);
  
  // إعداد دبابيس المحركات
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  Serial.println("System Ready: XLINE + L298");
}

void loop() {
  int sensorValues[16];
  int analogValues[16];

  for (int i = 0; i < 16; i++) {
    // اختيار الحساس
    digitalWrite(s0, (i & 0x01));
    digitalWrite(s1, (i >> 1) & 0x01);
    digitalWrite(s2, (i >> 2) & 0x01);
    digitalWrite(s3, (i >> 3) & 0x01);

    delayMicroseconds(10); 

    // قراءة النوعين
    sensorValues[i] = digitalRead(outDigital);
    analogValues[i] = analogRead(outAnalog); // قراءة دقيقة للشدة
  }

  // مثال لتحريك المحركات للأمام إذا كان الحساس الأوسط (رقم 7 و 8) يرى الخط
  if (sensorValues[7] == 1 || sensorValues[8] == 1) {
    moveForward();
  } else {
    stopMotors();
  }

  // طباعة قيم الحساس التماثلي للمعايرة
  Serial.print("Analog Sensor 7: ");
  Serial.println(analogValues[7]);
  
  delay(50);
}

// دالة التحرك للأمام
void moveForward() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}
