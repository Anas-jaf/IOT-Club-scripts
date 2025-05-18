const int rows[8] = {2, 3, 4, 5, 6, 7, 8, 9};
const int cols[8] = {10, 11, 12, 13, 14, 15, 16, 17};

char board[8][8];
char prevBoard[8][8];  // مصفوفة لحفظ الحالة السابقة

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 8; i++) {
    pinMode(rows[i], INPUT_PULLUP);
    pinMode(cols[i], INPUT_PULLUP);
  }

  for (int r = 0; r < 8; r++)
    for (int c = 0; c < 8; c++) {
      board[r][c] = '.';
      prevBoard[r][c] = '.'; // تهيئة الحالة السابقة بنفس القيمة
    }
}

void loop() {
  // إعادة تعيين اللوحة
  for (int r = 0; r < 8; r++)
    for (int c = 0; c < 8; c++)
      board[r][c] = '.';

  // مسح الشبكة
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

  // التحقق هل هناك تغيير مقارنة بالحالة السابقة
  bool changed = false;
  for (int r = 0; r < 8 && !changed; r++) {
    for (int c = 0; c < 8; c++) {
      if (board[r][c] != prevBoard[r][c]) {
        changed = true;
        break;
      }
    }
  }

  if (changed) {
    // تحديث الحالة السابقة
    for (int r = 0; r < 8; r++)
      for (int c = 0; c < 8; c++)
        prevBoard[r][c] = board[r][c];

    // طباعة اللوحة بشكل عكسي (حسب كودك)
    Serial.println("Current Board:");
    for (int rank = 7; rank >= 0; rank--) {
      for (int file = 0; file < 8; file++) {
        Serial.print(board[file][rank]);
        Serial.print(" ");
      }
      Serial.print("  "); Serial.println(rank + 1);
    }

    Serial.println("A B C D E F G H");
    Serial.println();
  }

  delay(1000);
}
