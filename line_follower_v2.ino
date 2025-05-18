// Analog Line Sensor Pins
const int sensorPins[] = {14 ,15, 16 , 17 ,18 ,19}; // 6 analog sensors
const int numSensors = 6;
const int threshold = 600; // Adjust based on your surface

// Motor Control Pins
#define IN1 7      // Left Motor IN1
#define IN2 6      // Left Motor IN2
#define IN3 5      // Right Motor IN3
#define IN4 4      // Right Motor IN4
#define ENA 3      // Left Motor PWM
#define ENB 11     // Right Motor PWM

// Ultrasonic Sensor Pins
#define trigPin 12
#define echoPin 10

// PID Constants (tune these)
const float Kp = 0.8;
const float Ki = 0.01;
const float Kd = 0.3;
const int targetPosition = 2500; // Center position (0-5000 scale)

// Motor Speeds
int baseSpeed = 90;  // Base speed (0-255)
int maxSpeed = 180;  // Maximum speed

// PID Variables
float error = 0, lastError = 0;
float integral = 0, derivative = 0;
float output = 0;

void setup() {
  Serial.begin(9600);
  
  // Initialize motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  // Initialize ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  long distance = getDistanceCM();

  if (distance < 10) { // Obstacle detected
    avoidObstacle();
    return;
  }
  
  int linePosition = getLinePosition();
  
  if (linePosition == -1) {
    // No line detected - implement recovery
    if (!recoverLine()) {
      searchForLine();
    }
  } else {
    // PID Line Following
    error = targetPosition - linePosition;
    integral += error;
    derivative = error - lastError;
    output = (Kp * error) + (Ki * integral) + (Kd * derivative);
    lastError = error;
    
    // Calculate motor speeds
    int leftSpeed = baseSpeed + output;
    int rightSpeed = baseSpeed - output;
    
    // Constrain speeds
    leftSpeed = constrain(leftSpeed, 0, maxSpeed);
    rightSpeed = constrain(rightSpeed, 0, maxSpeed);
    
    // Set motor directions (always forward for PID)
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    
    // Apply motor speeds
    analogWrite(ENA, leftSpeed);
    analogWrite(ENB, rightSpeed);
    
    // Debug output
    Serial.print("Pos: ");
    Serial.print(linePosition);
    Serial.print(" | PID: ");
    Serial.print(output);
    Serial.print(" | Speeds: L=");
    Serial.print(leftSpeed);
    Serial.print(" R=");
    Serial.println(rightSpeed);
  }
  delay(10);
}

int getLinePosition() {
  int position = 0;
  int activeSensors = 0;
  
  for (int i = 0; i < numSensors; i++) {
    int value = analogRead(sensorPins[i]);
    if (value > threshold) {
      position += (i * 1000); // Weighted position
      activeSensors++;
    }
  }
  
  return activeSensors > 0 ? position / activeSensors : -1;
}

// Your existing functions with minor adjustments
void avoidObstacle() {
  int degree = 300; 
  stopMotors();
  delay(500);

  // 1. Turn right 90° (left motor forward only)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, 0);
  delay(degree);
  stopMotors();
  delay(300);

  // 2. Move forward to pass obstacle
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);
  delay(1000);
  stopMotors();
  delay(300);
  if (getLinePosition() != -1) return;

  // 3. Turn left 90° (right motor forward only)
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, baseSpeed);
  delay(degree);
  stopMotors();
  delay(300);
  
  // 4. Move forward to find line
  unsigned long startTime = millis();
  unsigned long maxDuration = 1500;

  while (millis() - startTime < maxDuration) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENA, baseSpeed);
    analogWrite(ENB, baseSpeed);

    if (getLinePosition() != -1) {
      break;
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
  return duration * 0.034 / 2;
}

bool recoverLine() {
  // Move backward
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);
  delay(300);
  stopMotors();

  // Check if line found
  return (getLinePosition() != -1);
}

void searchForLine() {
  unsigned long startTime = millis();
  unsigned long moveDuration = 1200;

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, baseSpeed);
  analogWrite(ENB, baseSpeed);

  while (millis() - startTime < moveDuration) {
    if (getLinePosition() != -1) {
      return;
    }
  }
  stopMotors();
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
