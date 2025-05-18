// Pins
#define LS 10      // left sensor
#define RS 9       // right sensor

#define trigPin 12
#define echoPin 8

#define IN1 7      // Left Motor IN1
#define IN2 6      // Left Motor IN2
#define IN3 5      // Right Motor IN3
#define IN4 4      // Right Motor IN4

#define ENA 3     // Speed control for left motor
#define ENB 11    // Speed control for right motor

int speedLeft = 90;   // Speed: 0 - 255
int speedRight = 90;

void setup() {
  pinMode(LS, INPUT);
  pinMode(RS, INPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(trigPin , OUTPUT);
  pinMode(echoPin , INPUT);
  
}

void loop() {
  long distance = getDistanceCM();

  if (distance < 10) { // جسم قريب
    avoidObstacle();
    return;  // لا تُكمل الحركة
  }
  
  bool left = digitalRead(LS);
  bool right = digitalRead(RS);


  if (!left && !right) {
    moveForward();
  } else if (left && !right) {
    turnLeft();
  } else if (!left && right) {
    turnRight();
  } else {
    // الخط مفقود؟ أولاً حاول الاسترداد
    if (!recoverLine()) {
      // إذا فشلت المحاولة، ابحث للأمام
      searchForLine();
    }
  }
}

void avoidObstacle() {
  int degree = 300 ; 
  stopMotors();
  delay(500);

  // 1. انعطاف 90° لليمين (محرك يسار فقط يعمل للأمام)
  digitalWrite(IN1, HIGH);  // يسار أمام
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);   // يمين متوقف
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speedLeft);
  analogWrite(ENB, 0);
  delay(degree); // المدة تقريبية حسب نوع الروبوت
  stopMotors();
  delay(300);

  // 2. تقدم للأمام لتجاوز العائق
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speedLeft);
  analogWrite(ENB, speedRight);
  delay(1000); // تقدم تقريبي ~20 سم
  stopMotors();
  delay(300);
  if (!digitalRead(LS) || !digitalRead(RS)) return;

  // 3. انعطاف 90° لليسار (محرك يمين فقط يعمل للأمام)
  digitalWrite(IN1, LOW);   // يسار متوقف
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);  // يمين أمام
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, speedRight);
  delay(degree); // نفس مدة الانعطاف الأول
  stopMotors();
  delay(300);
  

  // 4. تقدم للأمام للعودة إلى الخط
  unsigned long startTime = millis();
  unsigned long maxDuration = 1500;  // أقصى مدة تقدم للأمام (مثلاً 1.5 ثانية)

  while (millis() - startTime < maxDuration) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, speedLeft);
    analogWrite(ENB, speedRight);

    if (!digitalRead(LS) || !digitalRead(RS)) {
      break;  // تم العثور على الخط
    }
  }

  stopMotors();
}



long getDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  long distanceCM = duration * 0.034 / 2;

  return distanceCM;
}


void searchForLine() {
  unsigned long startTime = millis();
  unsigned long moveDuration = 1200; // تقريبًا 10 سم حسب السرعة

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speedLeft);
  analogWrite(ENB, speedRight);

  while (millis() - startTime < moveDuration) {
    if (!digitalRead(LS) || !digitalRead(RS)) {
      return; // تم العثور على الخط
    }
  }

  stopMotors();  // لم يُعثر على الخط بعد المسافة المحددة
}

bool recoverLine() {
  // التراجع للخلف فقط
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speedLeft);
  analogWrite(ENB, speedRight);
  
  delay(300); // تراجع لمدة 300 مللي ثانية

  stopMotors();

  // بعد التراجع، تحقق إذا تم العثور على الخط
  if (!digitalRead(LS) || !digitalRead(RS)) {
    return true;  // وجد الخط
  }

  return false;  // لم يجد الخط
}

void moveForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, speedLeft);
  analogWrite(ENB, speedRight);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);  // Left motor off

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);         // No speed left
  analogWrite(ENB, speedRight);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);  // Right motor off

  analogWrite(ENA, speedLeft);
  analogWrite(ENB, 0);         // No speed right
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}


