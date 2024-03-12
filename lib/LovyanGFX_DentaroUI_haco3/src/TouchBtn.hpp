#pragma once //インクルードガード

#define SDCARD_SS_PIN 4 //M5toughのSD_CSは4
#define SDCARD_SPI SPI

// #include <Arduino.h>
// #include "LGFX_ESP32_custom_haco3dake.hpp"
using namespace std;
#include <list>
#define USE_PSRAM true
class TouchBtn {
  private:
public:
    TouchBtn();
   ~TouchBtn();
};
