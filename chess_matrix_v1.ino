// تعريف دبابيس الصفوف والأعمدة
const int rows[8] = {2, 3, 4, 5, 6, 7, 8, 9};         // أفقياً (من H إلى A)
const int cols[8] = {10, 11, 12, 13, 14, 15, 16, 17}; // عمودياً (من 1 إلى 8)

// مصفوفة تمثل لوحة الشطرنج
char board[8][8];

void setup() {
  Serial.begin(9600);

  // تهيئة دبابيس الصفوف والأعمدة كـ INPUT_PULLUP
  for (int i = 0; i < 8; i++) {
    pinMode(rows[i], INPUT_PULLUP);
    pinMode(cols[i], INPUT_PULLUP);
  }

  // تهيئة اللوحة كمربعات فارغة
  for (int r = 0; r < 8; r++)
    for (int c = 0; c < 8; c++)
      board[r][c] = '.';
}

void loop() {
  // إعادة تعيين اللوحة
  for (int r = 0; r < 8; r++)
    for (int c = 0; c < 8; c++)
      board[r][c] = '.';

  // المسح
  for (int r = 0; r < 8; r++) {
    pinMode(rows[r], OUTPUT);
    digitalWrite(rows[r], LOW);
    delayMicroseconds(50);

    for (int c = 0; c < 8; c++) {
      if (digitalRead(cols[c]) == LOW) {
        board[r][c] = 'X';
      }
    }

    pinMode(rows[r], INPUT_PULLUP);
  }

  // طباعة اللوحة بشكل عكسي بالأحرف فقط
  Serial.println("Current Board:");
  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 0; file < 8; file++) {
      Serial.print(board[file][rank]);
      Serial.print(" ");
    }
    Serial.print("  "); Serial.println(rank + 1);
  }

  // طباعة أسماء الأعمدة معكوسة
  Serial.println("A B C D E F G H");  // ← الأعمدة كانت H G F ... والآن نعرضها كـ A B C ...
  Serial.println();

  delay(1000);
}
