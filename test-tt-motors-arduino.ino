// ===== Arduino UNO + L298N MOTOR TEST with BLINK INDEX =====

// LEFT MOTOR
const int ENA = 5;   // PWM
const int IN1 = 8;
const int IN2 = 9;

// RIGHT MOTOR
const int ENB = 6;   // PWM
const int IN3 = 11;
const int IN4 = 10;

// Built-in LED
const int LED_PIN = LED_BUILTIN;

// Speed 0-255
int SPEED = 160;

// Flip direction if needed
bool INV_LEFT  = false;
bool INV_RIGHT = false;

// ---------- Motor helpers ----------
void setLeft(int pwm, bool forward) {
  bool dir = INV_LEFT ? !forward : forward;
  digitalWrite(IN1, dir ? HIGH : LOW);
  digitalWrite(IN2, dir ? LOW  : HIGH);
  analogWrite(ENA, constrain(pwm, 0, 255));
}

void setRight(int pwm, bool forward) {
  bool dir = INV_RIGHT ? !forward : forward;
  digitalWrite(IN3, dir ? HIGH : LOW);
  digitalWrite(IN4, dir ? LOW  : HIGH);
  analogWrite(ENB, constrain(pwm, 0, 255));
}

void stopAll() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

// ---------- Blink helper ----------
void blinkTestNumber(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  delay(600); // pause before the test starts
}

void setup() {
  Serial.begin(9600);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(LED_PIN, OUTPUT);

  stopAll();
  Serial.println("L298N Motor Test with Blink Index Started");
}

void loop() {
  // ===== Test 1: Forward =====
  blinkTestNumber(1);
  Serial.println("TEST 1: FORWARD");
  setLeft(SPEED, true);
  setRight(SPEED, true);
  delay(2000);
  stopAll();
  delay(1000);

  // ===== Test 2: Backward =====
  blinkTestNumber(2);
  Serial.println("TEST 2: BACKWARD");
  setLeft(SPEED, false);
  setRight(SPEED, false);
  delay(2000);
  stopAll();
  delay(1000);

  // ===== Test 3: Turn Left =====
  blinkTestNumber(3);
  Serial.println("TEST 3: TURN LEFT");
  setLeft(SPEED, false);  // left backward
  setRight(SPEED, true);  // right forward
  delay(1500);
  stopAll();
  delay(1000);

  // ===== Test 4: Turn Right =====
  blinkTestNumber(4);
  Serial.println("TEST 4: TURN RIGHT");
  setLeft(SPEED, true);   // left forward
  setRight(SPEED, false); // right backward
  delay(1500);
  stopAll();
  delay(2000); // pause before repeating
}
