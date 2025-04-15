int in1 = 7; // محرك 1 - اتجاه 1
int in2 = 6; // محرك 1 - اتجاه 2
int in3 = 5; // محرك 2 - اتجاه 1
int in4 = 4; // محرك 2 - اتجاه 2

void setup() {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
}

void loop() {
  // المحركين للأمام
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  delay(2000);

  // توقف
  stopMotors();
  delay(1000);

  // المحركين للخلف
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  delay(2000);

  // توقف
  stopMotors();
  delay(1000);
}

void stopMotors() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}
