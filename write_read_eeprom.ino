#include <EEPROM.h>

// === Function to write a name into EEPROM ===
void writeNameToEEPROM(const char* name) {
  int i = 0;
  while (name[i] != '\0') {
    EEPROM.write(i, name[i]);
    i++;
  }
  EEPROM.write(i, '\0'); // Add string terminator
  Serial.println("Name written to EEPROM successfully!");
}

// === Function to read a name from EEPROM ===
void readNameFromEEPROM() {
  char name[50];  // buffer for name
  int i = 0;
  while (true) {
    char c = EEPROM.read(i);
    name[i] = c;
    if (c == '\0') break;
    i++;
  }
  Serial.print("Stored name: ");
  Serial.println(name);
}

void setup() {
  Serial.begin(9600);
  delay(500);

  // 🟢 Uncomment one of the following lines as needed:
  
  // --- To save the name once ---
  // writeNameToEEPROM("Anas Aljaafreh");
  
  // --- To read the name anytime ---
  readNameFromEEPROM();
}

void loop() {
  // nothing
}
