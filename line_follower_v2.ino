// Sensor Configuration
const int sensorPins[] = {19, 18, 17, 16, 15, 14}; // Left to right
const int numSensors = 6;
int sensorValues[6];
int threshold = 800;
const int irEmitterPin = 13;

// Motor Configuration
#define IN1 7
#define IN2 6
#define IN3 5
#define IN4 4
#define ENA 3
#define ENB 11

// Ultrasonic Sensor
#define trigPin 12
#define echoPin 10

// PID Constants
const float Kp = 0.22;
const float Ki = 0.005;
const float Kd = 0.23;
const int targetPosition = 2500;

// Motor Control
int baseSpeed = 90;
int maxSpeed = 100;

// PID Variables
float error = 0, lastError = 0;
float integral = 0, derivative = 0;
float output = 0;

void setup() {
  Serial.begin(9600);

  // Motor Pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);

  // Ultrasonic Pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // IR Emitter
  pinMode(irEmitterPin, OUTPUT);
  digitalWrite(irEmitterPin, HIGH);

  calibrateSensors();
}

void loop() {
  if (getDistanceCM() < 10) {
    avoidObstacle();
    return;
  }

  int position = getLinePosition();

  if (position == -1) {
    stopMotors();
    delay(100);
    if (!recoverLine()) {
      searchForLine();
    }
    return;
  }

  // PID Control
  error = targetPosition - position;
  integral = constrain(integral + error, -500, 500);
  derivative = error - lastError;
  output = Kp * error + Ki * integral + Kd * derivative;
  lastError = error;

  output = constrain(output, -baseSpeed, baseSpeed);

  int leftSpeed = baseSpeed + output;
  int rightSpeed = baseSpeed - output;

  leftSpeed = constrain(leftSpeed, 0, maxSpeed);
  rightSpeed = constrain(rightSpeed, 0, maxSpeed);

  setMotorSpeeds(leftSpeed, rightSpeed);
  printDebug(position, output, leftSpeed, rightSpeed);

  delay(10);
}

int getLinePosition() {
  int position = 0;
  int activeSensors = 0;

  for (int i = 0; i < numSensors; i++) {
    sensorValues[i] = analogRead(sensorPins[i]);
    if (sensorValues[i] > threshold) {
      position += i * 1000;
      activeSensors++;
    }
  }

  return activeSensors > 0 ? position / activeSensors : -1;
}

void setMotorSpeeds(int left, int right) {
  // Left Motor
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, left);

  // Right Motor
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, right);
}

void calibrateSensors() {
  Serial.println("Calibrating sensors...");
  int minVal = 1024, maxVal = 0;

  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < numSensors; j++) {
      int val = analogRead(sensorPins[j]);
      minVal = min(minVal, val);
      maxVal = max(maxVal, val);
    }
    delay(10);
  }

  threshold = (minVal + maxVal) / 2;
  Serial.print("Calibration complete. Threshold: ");
  Serial.println(threshold);
}

void printDebug(int pos, float pid, int l, int r) {
  Serial.print("Pos:"); Serial.print(pos);
  Serial.print(" PID:"); Serial.print(pid, 1);
  Serial.print(" L:"); Serial.print(l);
  Serial.print(" R:"); Serial.print(r);
  Serial.print(" Sensors:");
  for (int i = 0; i < numSensors; i++) {
    Serial.print(sensorValues[i] > threshold ? "B" : "W");
  }
  Serial.println();
}

long getDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

void avoidObstacle() {
  stopMotors();
  delay(500);

  // Turn right
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, 0);
  delay(300);
  stopMotors();
  delay(300);

  // Move forward
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);
  delay(1000);
  stopMotors();
  delay(300);
  if (getLinePosition() != -1) return;

  // Turn left
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, baseSpeed);
  delay(300);
  stopMotors();
  delay(300);

  // Search for line
  unsigned long startTime = millis();
  while (millis() - startTime < 1500) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    analogWrite(ENA, baseSpeed);
    analogWrite(ENB, baseSpeed);
    if (getLinePosition() != -1) break;
  }
  stopMotors();
}

bool recoverLine() {
  // Move backward briefly
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);
  delay(300);
  stopMotors();
  return (getLinePosition() != -1);
}

void searchForLine() {
  unsigned long startTime = millis();
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);

  while (millis() - startTime < 1200) {
    if (getLinePosition() != -1) return;
  }
  stopMotors();
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
