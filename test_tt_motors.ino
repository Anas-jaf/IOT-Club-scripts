// ===== ESP32 + L298N: MODES + SPEED INSIDE F/B/L/R COMMANDS =====
//
// Timed commands (with optional speed override):
//   F500            -> forward 500ms using current speeds
//   F500@150,150    -> forward 500ms with L=150 R=150 (only for this run)
//   B500@120,140    -> backward 500ms with L=120 R=140
//   L500@140,140    -> left pivot 500ms with L=140 R=140
//   R500@140,140    -> right pivot 500ms with L=140 R=140
//
// Modes:
//   CF              -> continuous forward (uses current speeds)
//   P               -> pulse mode (forward pulseMs then backward pulseMs repeat)
//   T500            -> set pulseMs
//
// Speed set (global):
//   V150,150        -> set current speeds (L,R)
//   L160            -> set left speed
//   R140            -> set right speed
//
// Other:
//   S               -> stop
//   ?               -> status

// LEFT MOTOR
#define ENA 25
#define IN1 26
#define IN2 27

// RIGHT MOTOR
#define ENB 33
#define IN3 14
#define IN4 13

const int PWM_FREQ = 1000;
const int PWM_RES  = 8; // 0..255

int leftSpeed  = 150;
int rightSpeed = 150;
unsigned long pulseMs = 500;

enum Mode { MODE_STOP, MODE_TIMED, MODE_CONT_FWD, MODE_PULSE };
Mode mode = MODE_STOP;

unsigned long endTime = 0;       // for timed action
unsigned long phaseStart = 0;    // for pulse
bool pulseForward = true;

// ---------- Motor helpers ----------
void setPWM(int l, int r) {
  ledcWrite(ENA, constrain(l, 0, 255));
  ledcWrite(ENB, constrain(r, 0, 255));
}
void setPWMCurrent() { setPWM(leftSpeed, rightSpeed); }

void stopMotors() {
  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  mode = MODE_STOP;
}

void forwardDir() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}
void backwardDir() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}
void turnLeftDir() {   // pivot left
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}
void turnRightDir() {  // pivot right
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

// ---------- Parsing helpers ----------
int parseNumber(const String& s) {
  String num = "";
  for (int i = 0; i < (int)s.length(); i++) {
    if (isDigit(s[i])) num += s[i];
    else if (num.length()) break;
  }
  return num.length() ? num.toInt() : -1;
}

bool parseCSV(const String& s, int &a, int &b) {
  int comma = s.indexOf(',');
  if (comma < 0) return false;
  a = s.substring(0, comma).toInt();
  b = s.substring(comma + 1).toInt();
  return true;
}

// Parse optional "@L,R" at end of command.
// Example: "F500@150,140" -> returns true and sets outL/outR
bool parseSpeedOverride(const String& cmd, int &outL, int &outR) {
  int atPos = cmd.indexOf('@');
  if (atPos < 0) return false;

  String tail = cmd.substring(atPos + 1);
  tail.trim();
  int a, b;
  if (!parseCSV(tail, a, b)) return false;

  outL = constrain(a, 0, 255);
  outR = constrain(b, 0, 255);
  return true;
}

String stripAfterAt(const String& cmd) {
  int atPos = cmd.indexOf('@');
  if (atPos < 0) return cmd;
  return cmd.substring(0, atPos);
}

// ---------- Status ----------
void printStatus() {
  Serial.print("Mode=");
  switch (mode) {
    case MODE_STOP:     Serial.print("STOP"); break;
    case MODE_TIMED:    Serial.print("TIMED"); break;
    case MODE_CONT_FWD: Serial.print("CONT_FWD"); break;
    case MODE_PULSE:    Serial.print("PULSE"); break;
  }
  Serial.print(" | L="); Serial.print(leftSpeed);
  Serial.print(" R="); Serial.print(rightSpeed);
  Serial.print(" | pulse="); Serial.print(pulseMs); Serial.println("ms");
}

// ---------- Mode starters ----------
void startTimed(char type, unsigned long ms, int runL, int runR) {
  mode = MODE_TIMED;

  if (type == 'F') forwardDir();
  else if (type == 'B') backwardDir();
  else if (type == 'L') turnLeftDir();
  else if (type == 'R') turnRightDir();
  else { stopMotors(); return; }

  setPWM(runL, runR);
  endTime = millis() + ms;
}

void startContForward() {
  mode = MODE_CONT_FWD;
  forwardDir();
  setPWMCurrent();
}

void startPulse() {
  mode = MODE_PULSE;
  pulseForward = true;
  phaseStart = millis();
  forwardDir();
  setPWMCurrent();
}

// ---------- setup ----------
void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  // ESP32 Arduino Core v3.x PWM API
  ledcAttach(ENA, PWM_FREQ, PWM_RES);
  ledcAttach(ENB, PWM_FREQ, PWM_RES);

  stopMotors();

  Serial.println("\n=== MOTOR TEST (speed inside F/B/L/R) ===");
  Serial.println("Timed: F500@150,150  B500@120,140  L500@140,140  R500@140,140");
  Serial.println("Modes: CF (cont forward) | P (pulse) | T500 (pulse time)");
  Serial.println("Speed: V150,150  L160  R140");
  Serial.println("S stop | ? status");
  printStatus();
}

// ---------- loop ----------
void loop() {
  unsigned long now = millis();

  // Timed stop
  if (mode == MODE_TIMED && now >= endTime) {
    stopMotors();
    Serial.println("DONE -> STOP");
  }

  // Pulse mode timing
  if (mode == MODE_PULSE) {
    if (now - phaseStart >= pulseMs) {
      phaseStart = now;
      pulseForward = !pulseForward;
      if (pulseForward) forwardDir();
      else              backwardDir();
      setPWMCurrent();
    }
  }

  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (!cmd.length()) return;

  // STOP
  if (cmd.equalsIgnoreCase("S")) {
    stopMotors();
    Serial.println("STOP");
    printStatus();
    return;
  }

  // STATUS
  if (cmd == "?") {
    printStatus();
    return;
  }

  // Continuous forward
  if (cmd.equalsIgnoreCase("CF")) {
    startContForward();
    Serial.println("Mode: CONTINUOUS FORWARD");
    printStatus();
    return;
  }

  // Pulse mode
  if (cmd.equalsIgnoreCase("P")) {
    startPulse();
    Serial.println("Mode: PULSE");
    printStatus();
    return;
  }

  // Pulse time
  if (cmd.startsWith("T") || cmd.startsWith("t")) {
    int v = parseNumber(cmd.substring(1));
    if (v > 0) {
      pulseMs = (unsigned long)v;
      Serial.println("Pulse time updated.");
      printStatus();
    } else {
      Serial.println("Bad T command. Example: T500");
    }
    return;
  }

  // Global speed set: V150,150
  if (cmd.startsWith("V") || cmd.startsWith("v")) {
    String tail = cmd.substring(1);
    int a, b;
    if (parseCSV(tail, a, b)) {
      leftSpeed  = constrain(a, 0, 255);
      rightSpeed = constrain(b, 0, 255);
      Serial.println("Global speeds updated.");
      if (mode != MODE_STOP) setPWMCurrent();
      printStatus();
    } else {
      Serial.println("Bad V command. Example: V150,150");
    }
    return;
  }

  // Global left speed: L160 (NOTE: conflicts with timed L500..., so require "LS160" for left speed)
  // To avoid conflict with L500 (left turn), we use:
  //   LS160 -> set left speed
  //   RS140 -> set right speed
  if (cmd.startsWith("LS") || cmd.startsWith("ls")) {
    int v = parseNumber(cmd.substring(2));
    if (v >= 0) {
      leftSpeed = constrain(v, 0, 255);
      Serial.println("Left speed updated.");
      if (mode != MODE_STOP) setPWMCurrent();
      printStatus();
    }
    return;
  }
  if (cmd.startsWith("RS") || cmd.startsWith("rs")) {
    int v = parseNumber(cmd.substring(2));
    if (v >= 0) {
      rightSpeed = constrain(v, 0, 255);
      Serial.println("Right speed updated.");
      if (mode != MODE_STOP) setPWMCurrent();
      printStatus();
    }
    return;
  }

  // Timed moves with optional override
  // Examples: F500, F500@150,150
  char first = toupper(cmd[0]);
  if (first == 'F' || first == 'B' || first == 'L' || first == 'R') {
    int runL = leftSpeed;
    int runR = rightSpeed;

    int oL, oR;
    if (parseSpeedOverride(cmd, oL, oR)) {
      runL = oL; runR = oR;
    }

    String base = stripAfterAt(cmd);
    int t = parseNumber(base.substring(1));
    if (t > 0) {
      startTimed(first, (unsigned long)t, runL, runR);
      Serial.print("RUN "); Serial.print(first);
      Serial.print(" for "); Serial.print(t);
      Serial.print(" ms @ L="); Serial.print(runL);
      Serial.print(" R="); Serial.println(runR);
      return;
    } else {
      Serial.println("Bad timed command. Example: F500@150,150");
      return;
    }
  }

  Serial.println("Unknown. Use F500@150,150  B500@...  L500@...  R500@...  CF  P  V150,150  LS160  RS140  T500  S  ?");
}
