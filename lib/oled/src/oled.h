

#pragma once

#include <stdint.h>

class OLEDClass {
 public:
  OLEDClass() {};

  uint8_t begin();

  uint8_t clear();

  uint8_t drawScreen(uint8_t data[128 * 8]);

  uint8_t drawChar(uint8_t row, uint8_t col, char c);

  uint8_t drawString(uint8_t row, uint8_t col, const char* str);

  uint8_t drawBin(uint8_t row, uint8_t col, uint8_t* data, uint8_t size);

 private:
  uint8_t setCursor(uint8_t x, uint8_t y);
};

extern OLEDClass OLED;