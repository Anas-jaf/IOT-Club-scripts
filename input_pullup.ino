#define INPUT_PIN 8 

void setup() {
  Serial.begin(9600);
  pinMode(INPUT_PIN, INPUT_PULLUP);
}

void loop() {
  Serial.println(digitalRead(INPUT_PIN));
}
