/* ===== UNO + L298N + 5ch Digital + PID + STOP BOX + LAST-SEEN RECOVERY =====
   Your sensor:
   BLACK line  = 0
   WHITE floor = 1
   => We invert so LINE becomes 1 internally.

   When line is lost (ALL WHITE): robot turns ONLY toward last seen direction
   until line is found again.
*/

const int ENA = 5, IN1 = 8,  IN2 = 9;
const int ENB = 6, IN3 = 10, IN4 = 11;

// Sensors
const int L2 = 2, L1 = 3, M = 4, R1 = 7, R2 = 12;

// Button
const int BTN = 13;  // connect button between D13 and GND
bool running = false;

// Polarity
const bool BLACK_IS_1 = false;   // ✅ black=0, white=1

// PID
float Kp = 22.0;
float Kd = 120.0;   // شوي أعلى للاستقرار

// Speeds
int baseSpeed = 150;  // قللتها شوي للاستقرار
int maxSpeed  = 255;

// Stop Box: all BLACK (sum==5) for duration
const unsigned long STOPBOX_MS = 200;
const unsigned long STOP_GRACE_MS = 50;

// Recovery: last-seen direction only
const unsigned long LOST_FORWARD_MS = 60;  // يمشي للأمام لحظة
int recoverSpinFast = 170;                 // قوة اللفة
int recoverSpinSlow = 140;

// Internal
float prevError = 0;
int lastSeenDir = 0; // -1 left, +1 right
bool stopboxLatched = false;

unsigned long allBlackSince = 0;
unsigned long lostSince = 0;

struct Sense5 {
  int sL2, sL1, sM, sR1, sR2;
  int sum;
  float pos;
};

int readLineBit(int pin) {
  int v = digitalRead(pin);
  return BLACK_IS_1 ? v : !v; // invert since black=0
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
  // 🔁 قلب: أعطِ سرعة اليسار للموتور اليمين والعكس
  setMotorRaw(ENA, IN1, IN2, right);  // صار يشغّل الموتور اليمين
  setMotorRaw(ENB, IN3, IN4, left);   // صار يشغّل الموتور اليسار
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

    // update last seen direction only when meaningful
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
  // مرحلة 1: للأمام لحظة قصيرة
  if (dtLost < LOST_FORWARD_MS) {
    motor(baseSpeed, baseSpeed);
    return;
  }

  // مرحلة 2: لف باتجاه آخر خط شوفتو (بدون تبديل)
  int dir = (lastSeenDir >= 0) ? +1 : -1;

  // أول ثانية: لف أسرع، بعدها أبطأ لتجنب overshoot
  int spin = (dtLost < 900) ? recoverSpinFast : recoverSpinSlow;

  motor(-dir * spin, dir * spin);
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

  Serial.begin(9600);
  stopAll();
}

void loop() {
  // Start/Stop/Restart
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

  Sense5 s = readSense();

// STOP BOX: all black (sum==5)  -> slow down then stop
  if (s.sum == 5) {
    if (allBlackSince == 0) allBlackSince = millis();
    unsigned long dt = millis() - allBlackSince;

    // 1) أول لحظة: تبطيء قوي (قرب/دخول الصندوق)
    // بدل ما تمشي baseSpeed
    int slow1 = (int)(baseSpeed * 0.45);   // 45% من السرعة
    int slow2 = (int)(baseSpeed * 0.25);   // 25% من السرعة (أبطأ)

    // مرحلة A: مباشرة تبطيء
    if (dt < STOP_GRACE_MS) {
      motor(slow1, slow1);
      return;
    }

    // مرحلة B: تبطيء أكثر أثناء التأكيد
    if (dt < STOPBOX_MS) {
      motor(slow2, slow2);
      return;
    }

    // مرحلة C: توقف نهائي + Latch
    latchStopBox();
    return;
  } else {
    allBlackSince = 0;
  }

  // LOST: all white (sum==0)
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

  int left  = (int)(baseSpeed + pid);
  int right = (int)(baseSpeed - pid);

  if (left  < 0) left  = 0;
  if (right < 0) right = 0;
  if (left  > maxSpeed) left  = maxSpeed;
  if (right > maxSpeed) right = maxSpeed;

  motor(left, right);
}
