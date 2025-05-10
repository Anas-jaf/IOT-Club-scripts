void setup() {
  Serial.begin(9600);

  // Set both pins to INPUT_PULLUP initially
  pinMode(2, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
}

void loop() {
  // Set row (pin 2) to OUTPUT and LOW
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  // Read column (pin 10)
  int val = digitalRead(10);

  if (val == LOW) {
    Serial.println("Switch CLOSED (pressed or magnetic piece present)");
  } else {
    Serial.println("Switch OPEN");
  }

  // Set pin 2 back to INPUT_PULLUP (reset state)
  pinMode(2, INPUT_PULLUP);

  delay(300); // Add delay to avoid flooding Serial
}
