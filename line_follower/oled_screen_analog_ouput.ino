#include <Wire.h>
#include <U8g2lib.h>

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

int sensor[16] = {
  12, 45, 78, 78,
  123, 90,
  111, 67,
  89, 56,
  11, 22, 33, 44, 55, 66
};

const int W = 128;
const int rows = 5;

int rowCount[5] = {2, 3, 3, 4, 4}; 
int rowH = 12;

void drawCentered(int xCenter, int y, const char* txt) {
  int w = u8g2.getStrWidth(txt);
  int x = xCenter - (w / 2);

  if (x < 0) x = 0;
  if (x + w > W) x = W - w;

  u8g2.drawStr(x, y, txt);
}

void setup() {
  u8g2.begin();
}

void loop() {

  u8g2.firstPage();
  do {

    u8g2.setFont(u8g2_font_5x7_tr);

    int index = 0;

    for (int r = 0; r < rows; r++) {

      int items = rowCount[r];
      int y = 10 + r * rowH;

      int center = W / 2;

      int spacing = 18 + r * 5;

      for (int c = 0; c < items; c++) {

        int dataIndex = index % 16;

        int offset = (c - (items - 1) / 2.0) * spacing;

        int x = center + offset;

        char buf[5];
        sprintf(buf, "%03d", sensor[dataIndex]);

        drawCentered(x, y, buf);

        index++;
      }
    }

  } while (u8g2.nextPage());

  delay(100);
}
