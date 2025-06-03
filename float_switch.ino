const int floatSwitchPin = 2;     // Pin connected to float switch
const int ledPin = 13;            // Pin connected to LED or relay

void setup() {
  pinMode(floatSwitchPin, INPUT_PULLUP); // Use internal pull-up resistor
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int switchState = digitalRead(floatSwitchPin);

  if (switchState == LOW) {  // Switch closed (floating)
    digitalWrite(ledPin, LOW);   // Turn off LED/pump
    Serial.println("Float switch not activated: LOW LEVEL");
  } else {
    digitalWrite(ledPin, HIGH);  // Turn on LED/pump
    Serial.println("Float switch activated: HIGH LEVEL");
  }

  delay(500); // Delay to prevent flooding the serial monitor
}
