/* ===== UNO + L298N + 5ch Digital + PID + STOP BOX + LAST-SEEN RECOVERY + ULTRASONIC STOP ===== */

const int ENA = 5, IN1 = 8,  IN2 = 9;
const int ENB = 6, IN3 = 10, IN4 = 11;

// Sensors
const int L2 = 2, L1 = 3, M = 4, R1 = 7, R2 = 12;

// Button
const int BTN = 13;  // button between D13 and GND
bool running = false;

// ===== Ultrasonic (HC-SR04) =====
const int TRIG_PIN = A0;  // ✅ TRIG على A0
const int ECHO_PIN = A1;  // ✅ ECHO على A1

const int OBSTACLE_CM = 15;
const int OBSTACLE_CLEAR_CM = 18;
bool obstacleHold = false;

// Polarity
const bool BLACK_IS_1 = false;   // black=0, white=1

// PID (خففنا شوي)
float Kp = 22.0;
float Kd = 120.0;   // شوي أعلى للاستقرار

// Speeds (أبطأ بكثير)
int baseSpeed = 50;
int maxSpeed  = 70;

// ✅ قصّ للـ PID حتى ما يطير
int PID_CLAMP = 60;   // جرّب 40~70

// Stop Box
const unsigned long STOPBOX_MS = 200;
const unsigned long STOP_GRACE_MS = 50;

// Recovery
const unsigned long LOST_FORWARD_MS = 60;
int recoverSpinFast = 90;   // كان 170 (سريع جداً)
int recoverSpinSlow = 75;   // كان 140

// Internal
float prevError = 0;
int lastSeenDir = 0; // -1 left, +1 right
bool stopboxLatched = false;

unsigned long allBlackSince = 0;
unsigned long lostSince = 0;

// لتخفيف ضغط قراءة الألتراسونك (كل 40ms)
unsigned long usLast = 0;
int usCached = 999;

struct Sense5 {
  int sL2, sL1, sM, sR1, sR2;
  int sum;
  float pos;
};

int readLineBit(int pin) {
  int v = digitalRead(pin);
  return BLACK_IS_1 ? v : !v;
}

bool buttonPressedToggle() {
  static bool lastStable = HIGH;
  static bool lastRead   = HIGH;
  static unsigned long t0 = 0;

  bool r = digitalRead(BTN);
  if (r != lastRead) { lastRead = r; t0 = millis(); }

  if (millis() - t0 > 40) {
    if (lastStable != lastRead) {
      lastStable = lastRead;
      if (lastStable == LOW) return true;
    }
  }
  return false;
}

void setMotorRaw(int enPin, int inA, int inB, int speedSigned) {
  bool forward = (speedSigned >= 0);
  int spd = abs(speedSigned);
  if (spd > maxSpeed) spd = maxSpeed;

  digitalWrite(inA, forward ? HIGH : LOW);
  digitalWrite(inB, forward ? LOW  : HIGH);
  analogWrite(enPin, spd);
}

void motor(int left, int right) {
  // قلب اليمين/اليسار حسب توصيلاتك
  setMotorRaw(ENA, IN1, IN2, right);
  setMotorRaw(ENB, IN3, IN4, left);
}

void stopAll() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

Sense5 readSense() {
  Sense5 s;
  s.sL2 = readLineBit(L2);
  s.sL1 = readLineBit(L1);
  s.sM  = readLineBit(M);
  s.sR1 = readLineBit(R1);
  s.sR2 = readLineBit(R2);

  s.sum = s.sL2 + s.sL1 + s.sM + s.sR1 + s.sR2;

  if (s.sum > 0) {
    int weighted = (-2*s.sL2) + (-1*s.sL1) + (0*s.sM) + (1*s.sR1) + (2*s.sR2);
    s.pos = (float)weighted / (float)s.sum;

    if (s.pos < -0.2) lastSeenDir = -1;
    else if (s.pos > 0.2) lastSeenDir = +1;
  } else {
    s.pos = 0;
  }
  return s;
}

void latchStopBox() {
  stopAll();
  running = false;
  stopboxLatched = true;
}

void recoverLastSeen(unsigned long dtLost) {
  if (dtLost < LOST_FORWARD_MS) {
    motor(baseSpeed, baseSpeed);
    return;
  }

  int dir = (lastSeenDir >= 0) ? +1 : -1;
  int spin = (dtLost < 900) ? recoverSpinFast : recoverSpinSlow;
  motor(-dir * spin, dir * spin);
}

// ===== Ultrasonic =====
int readDistanceCM_raw() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long dur = pulseIn(ECHO_PIN, HIGH, 25000UL);
  if (dur == 0) return 999;
  return (int)(dur / 58UL);
}

int readDistanceCM() {
  // قراءة كل 40ms عشان ما تعطل اللوب
  if (millis() - usLast >= 40) {
    usLast = millis();
    usCached = readDistanceCM_raw();
  }
  return usCached;
}

bool obstacleDetected() {
  int cm = readDistanceCM();

  if (!obstacleHold) {
    if (cm <= OBSTACLE_CM) obstacleHold = true;
  } else {
    if (cm >= OBSTACLE_CLEAR_CM) obstacleHold = false;
  }
  return obstacleHold;
}

void setup() {
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  pinMode(L2, INPUT);
  pinMode(L1, INPUT);
  pinMode(M,  INPUT);
  pinMode(R1, INPUT);
  pinMode(R2, INPUT);

  pinMode(BTN, INPUT_PULLUP);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  Serial.begin(9600);
  stopAll();
}

void loop() {
  if (buttonPressedToggle()) {
    if (stopboxLatched) {
      stopboxLatched = false;
      running = true;
      allBlackSince = 0;
      lostSince = 0;
      prevError = 0;
    } else {
      running = !running;
      if (!running) stopAll();
      allBlackSince = 0;
      lostSince = 0;
      prevError = 0;
    }
  }

  if (stopboxLatched) { stopAll(); return; }
  if (!running) return;

  // أمان: وقف عند وجود عائق
  if (obstacleDetected()) {
    stopAll();
    return;
  }

  Sense5 s = readSense();

  // STOP BOX
  if (s.sum == 5) {
    if (allBlackSince == 0) allBlackSince = millis();
    unsigned long dt = millis() - allBlackSince;

    int slow1 = (int)(baseSpeed * 0.45);
    int slow2 = (int)(baseSpeed * 0.25);

    if (dt < STOP_GRACE_MS) { motor(slow1, slow1); return; }
    if (dt < STOPBOX_MS)    { motor(slow2, slow2); return; }

    latchStopBox();
    return;
  } else {
    allBlackSince = 0;
  }

  // LOST
  if (s.sum == 0) {
    if (lostSince == 0) lostSince = millis();
    recoverLastSeen(millis() - lostSince);
    return;
  } else {
    lostSince = 0;
  }

  // PID follow
  float error = 0.0 - s.pos;
  float d = error - prevError;
  prevError = error;

  float pid = (Kp * error) + (Kd * d);

  // ✅ clamp pid
  if (pid > PID_CLAMP) pid = PID_CLAMP;
  if (pid < -PID_CLAMP) pid = -PID_CLAMP;

  int left  = (int)(baseSpeed + pid);
  int right = (int)(baseSpeed - pid);

  if (left  < 0) left  = 0;
  if (right < 0) right = 0;
  if (left  > maxSpeed) left  = maxSpeed;
  if (right > maxSpeed) right = maxSpeed;

  motor(left, right);
}
