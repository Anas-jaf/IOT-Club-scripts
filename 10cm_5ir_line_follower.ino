// ===== Pin Definitions =====
#define S1 3   // Left most IR sensor
#define S2 4
#define S3 6   // Middle sensor
#define S4 7
#define S5 8   // Right most IR sensor

#define LM_F 10   // Left Motor Forward
#define LM_B 11   // Left Motor Backward
#define RM_F 5    // Right Motor Forward
#define RM_B 9    // Right Motor Backward

// Line disconnect / move forward settings
const unsigned long moveDistanceTime = 500; // ~10 cm, adjust as needed
bool movingForward = false;
unsigned long moveStartTime = 0;

// Last known direction for smarter recovery
int lastDirection = 0; // -1 = left, 0 = straight, 1 = right

void setup() {
  pinMode(LM_F, OUTPUT);
  pinMode(LM_B, OUTPUT);
  pinMode(RM_F, OUTPUT);
  pinMode(RM_B, OUTPUT);

  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(S5, INPUT);
}

void loop() {
  int s1 = digitalRead(S1);
  int s2 = digitalRead(S2);
  int s3 = digitalRead(S3);
  int s4 = digitalRead(S4);
  int s5 = digitalRead(S5);

  // ================= Line Lost / Move Forward =================
  if(s1==1 && s2==1 && s3==1 && s4==1 && s5==1) { // no line
    if(!movingForward) {
      // Start moving forward
      analogWrite(RM_F, 200);  // adjust speed if needed
      analogWrite(LM_F, 200);
      digitalWrite(RM_B, LOW);
      digitalWrite(LM_B, LOW);

      moveStartTime = millis();
      movingForward = true;
    } else {
      // Check if forward movement finished (~10 cm)
      if(millis() - moveStartTime >= moveDistanceTime) {
        // Stop and check sensors
        analogWrite(RM_F, 0);
        analogWrite(LM_F, 0);
        digitalWrite(RM_B, LOW);
        digitalWrite(LM_B, LOW);

        movingForward = false;

        // Read sensors again
        s1 = digitalRead(S1);
        s2 = digitalRead(S2);
        s3 = digitalRead(S3);
        s4 = digitalRead(S4);
        s5 = digitalRead(S5);

        if(s1==1 && s2==1 && s3==1 && s4==1 && s5==1) {
          // Still no line: stop or take alternate action
          return; // skip line following
        }
        // else: continue normal line following
      }
    }
    return; // skip normal line following while moving forward
  } else {
    movingForward = false; // line detected
  }

  // ================= Normal Line Following =================
  // Straight
  if((s1 == 1) && (s2 == 1) && (s3 == 0) && (s4 == 1) && (s5 == 1)) {
    analogWrite(RM_F, 255);
    analogWrite(LM_F, 255);
    digitalWrite(RM_B, LOW);
    digitalWrite(LM_B, LOW);
    lastDirection = 0;
  }
  // Slight right
  else if((s1 == 1) && (s2 == 0) && (s3 == 1) && (s4 == 1) && (s5 == 1)) {
    analogWrite(RM_F, 255);
    analogWrite(LM_F, 255);
    digitalWrite(RM_B, LOW);
    digitalWrite(LM_B, LOW);
    digitalWrite(LM_F, LOW);
    lastDirection = -1;
  }
  // Hard right
  else if((s1 == 0) && (s2 == 1) && (s3 == 1) && (s4 == 1) && (s5 == 1)) {
    analogWrite(RM_F, 255);
    analogWrite(LM_F, 255);
    digitalWrite(RM_B, LOW);
    digitalWrite(LM_B, LOW);
    digitalWrite(LM_F, LOW);
    digitalWrite(LM_B, HIGH);
    lastDirection = -1;
  }
  // Slight left
  else if((s1 == 1) && (s2 == 1) && (s3 == 1) && (s4 == 0) && (s5 == 1)) {
    analogWrite(RM_F, 255);
    analogWrite(LM_F, 255);
    digitalWrite(RM_B, LOW);
    digitalWrite(LM_B, LOW);
    digitalWrite(RM_F, LOW);
    lastDirection = 1;
  }
  // Hard left
  else if((s1 == 1) && (s2 == 1) && (s3 == 1) && (s4 == 1) && (s5 == 0)) {
    analogWrite(RM_F, 255);
    analogWrite(LM_F, 255);
    digitalWrite(RM_B, LOW);
    digitalWrite(LM_B, LOW);
    digitalWrite(RM_F, LOW);
    digitalWrite(RM_B, HIGH);
    lastDirection = 1;
  }
  // Middle + right
  else if((s1 == 1) && (s2 == 1) && (s3 == 0) && (s4 == 0) && (s5 == 1)) {
    analogWrite(RM_F, 255);
    analogWrite(LM_F, 255);
    digitalWrite(RM_B, LOW);
    digitalWrite(LM_B, LOW);
    digitalWrite(RM_F, LOW);
    lastDirection = 1;
  }
  // Middle + left
  else if((s1 == 1) && (s2 == 0) && (s3 == 0) && (s4 == 1) && (s5 == 1)) {
    analogWrite(RM_F, 255);
    analogWrite(LM_F, 255);
    digitalWrite(RM_B, LOW);
    digitalWrite(LM_B, LOW);
    digitalWrite(LM_F, LOW);
    lastDirection = -1;
  }
  // Middle + left + leftmost
  else if((s1 == 0) && (s2 == 0) && (s3 == 0) && (s4 == 1) && (s5 == 1)) {
    analogWrite(RM_F, 255);
    analogWrite(LM_F, 255);
    digitalWrite(RM_B, LOW);
    digitalWrite(LM_B, LOW);
    digitalWrite(LM_F, LOW);
    lastDirection = -1;
  }
  // Middle + right + rightmost
  else if((s1 == 1) && (s2 == 1) && (s3 == 0) && (s4 == 0) && (s5 == 0)) {
    analogWrite(RM_F, 255);
    analogWrite(LM_F, 255);
    digitalWrite(RM_B, LOW);
    digitalWrite(LM_B, LOW);
    digitalWrite(RM_F, LOW);
    lastDirection = 1;
  }
  // All on black (stop)
  else if((s1 == 0) && (s2 == 0) && (s3 == 0) && (s4 == 0) && (s5 == 0)) {
    digitalWrite(LM_F, LOW);
    digitalWrite(LM_B, LOW);
    digitalWrite(RM_F, LOW);
    digitalWrite(RM_B, LOW);
  }
}
