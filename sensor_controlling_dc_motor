#define LS 10      // left sensor
#define RS 9       // right sensor
#define LM1 5      //motor Driver in1
#define LM2 4     //motor Driver in2
#define RM1 7      //motor Driver in3
#define RM2 6      //motor Driver in4

void setup()
{
  pinMode(LS, INPUT);
  pinMode(RS, INPUT);
  pinMode(LM1, OUTPUT);
  pinMode(LM2, OUTPUT);
  pinMode(RM1, OUTPUT);
  pinMode(RM2, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  bool left = digitalRead(LS);
  bool right = digitalRead(RS);

  // Serial.print("LS: ");
  // Serial.print(left);
  // Serial.print(" | RS: ");
  // Serial.println(right);
  // delay(100);

  if (!left && !right) {
    // Forward
    moveForward();
  }
  else if (left && !right) {
    // Turn left
    turnLeft();
    // turnRight();
  }
  else if (!left && right) {
    // Turn right
    turnRight();
    // turnLeft();
  }
  else {
    // Stop
    stopMotors();
  }
}

void moveForward() {
  digitalWrite(LM1, HIGH);
  digitalWrite(LM2, LOW);
  digitalWrite(RM1, HIGH);
  digitalWrite(RM2, LOW);
}

void turnLeft() {
  digitalWrite(LM1, LOW);
  digitalWrite(LM2, LOW);
  digitalWrite(RM1, HIGH);
  digitalWrite(RM2, LOW);
}

void turnRight() {
  digitalWrite(LM1, HIGH);
  digitalWrite(LM2, LOW);
  digitalWrite(RM1, LOW);
  digitalWrite(RM2, LOW);
}

void stopMotors() {
  digitalWrite(LM1, LOW);
  digitalWrite(LM2, LOW);
  digitalWrite(RM1, LOW);
  digitalWrite(RM2, LOW);
}
