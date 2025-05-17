
void setup() {
  pinMode(0, OUTPUT); // GPIO0
  pinMode(1, OUTPUT); // Tx GPIO1
  pinMode(2, OUTPUT); // GPIO2
  pinMode(3, OUTPUT); // Rx GPIO3
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  
  digitalWrite(0, HIGH);
  delay(1000);
  digitalWrite(0, LOW);
  digitalWrite(1, HIGH);
  delay(1000);
  digitalWrite(1, LOW);  // Tx = GPIO1
  digitalWrite(2, HIGH);
  delay(1000);
  digitalWrite(2, LOW);
  digitalWrite(3, HIGH);
  delay(1000);
  digitalWrite(3, LOW);  // Rx = GPIO3  
}
