const int rows[8] = {2, 3, 4, 5, 6, 7, 8, 9};         // Horizontal
const int cols[8] = {10, 11, 12, 13, 14, 15, 16, 17}; // Vertical

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 8; i++) {
    pinMode(rows[i], INPUT_PULLUP);
    pinMode(cols[i], INPUT_PULLUP);
  }
}

void loop() {
  for (int r = 0; r < 8; r++) {
    pinMode(rows[r], OUTPUT);
    digitalWrite(rows[r], LOW);
    delayMicroseconds(50); // Stabilize read

    for (int c = 0; c < 8; c++) {
      if (digitalRead(cols[c]) == LOW) {
        // Remap: H8 -> A1, H7 -> B1, ..., A1 -> H8
        char file = 'H' - r;
        int rank = 1 + c;

        Serial.print("Switch closed at: ");
        Serial.print(file);
        Serial.println(rank);
      }
    }

    pinMode(rows[r], INPUT_PULLUP);
  }

  delay(300);
}
