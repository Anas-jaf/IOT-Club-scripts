int SensorLeftPin = 10;  // OUT pin of left sensor
int SensorRightPin = 9;  // OUT pin of right sensor

void setup() {
  pinMode(SensorRightPin, INPUT);
  pinMode(SensorLeftPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  int SensorLeftPinValue = digitalRead(SensorLeftPin);
  int SensorRightPinValue = digitalRead(SensorRightPin);

  Serial.print("Left: ");
  Serial.print(SensorLeftPinValue);
  Serial.print(" | Right: ");
  Serial.println(SensorRightPinValue);

  delay(100);
}
