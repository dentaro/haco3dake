#include <Arduino.h>
// #include <PS2Kbd.h>
#include <PS2Keyboard.h>
#include "Wire.h"
#include "SPI.h"

#define KEYBOARD_DATA 32
#define KEYBOARD_CLK  33

// PS2Kbd keyboard(KEYBOARD_DATA, KEYBOARD_CLK);
PS2Keyboard keyboard;

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

class LGFX : public lgfx::LGFX_Device
{
public:

  lgfx::Panel_CVBS _panel_instance;

  LGFX(void)
  {
    { // 表示パネル制御の設定を行います。
      auto cfg = _panel_instance.config();    // 表示パネル設定用の構造体を取得します。

      // 出力解像度を設定;
      cfg.memory_width  = 240;//208;//240; // 出力解像度 幅
      cfg.memory_height = 160;//128;//160; // 出力解像度 高さ

      // 実際に利用する解像度を設定;
      cfg.panel_width  = 208;  // 実際に使用する幅   (memory_width と同値か小さい値を設定する)
      cfg.panel_height = 128;  // 実際に使用する高さ (memory_heightと同値か小さい値を設定する)

      // 表示位置オフセット量を設定;
      cfg.offset_x = 16;       // 表示位置を右にずらす量 (初期値 0)
      cfg.offset_y = 16;       // 表示位置を下にずらす量 (初期値 0)

      _panel_instance.config(cfg);


// 通常は memory_width と panel_width に同じ値を指定し、 offset_x = 0 で使用します。;
// 画面端の表示が画面外に隠れるのを防止したい場合は、 panel_width の値をmemory_widthより小さくし、offset_x で左右の位置調整をします。;
// 例えば memory_width より panel_width を 32 小さい値に設定した場合、offset_x に 16 を設定することで左右位置が中央寄せになります。;
// 上下方向 (memory_height , panel_height , offset_y ) についても同様に、必要に応じて調整してください。;

    }

    {
      auto cfg = _panel_instance.config_detail();

      // 出力信号の種類を設定;
      cfg.signal_type = cfg.signal_type_t::NTSC;
      // cfg.signal_type = cfg.signal_type_t::NTSC_J;
      // cfg.signal_type = cfg.signal_type_t::PAL;
      // cfg.signal_type = cfg.signal_type_t::PAL_M;
      // cfg.signal_type = cfg.signal_type_t::PAL_N;

      // 出力先のGPIO番号を設定;
      cfg.pin_dac = 25;       // DACを使用するため、 25 または 26 のみが選択できます;

      // PSRAMメモリ割当の設定;
      cfg.use_psram = 2;      // 0=PSRAM不使用 / 1=PSRAMとSRAMを半々使用 / 2=全部PSRAM使用;

      // 出力信号の振幅の強さを設定;
      cfg.output_level = 200; // 初期値128
      // ※ GPIOに保護抵抗が付いている等の理由で信号が減衰する場合は数値を上げる。;
      // ※ M5StackCore2 はGPIOに保護抵抗が付いているため 200 を推奨。;

      // 彩度信号の振幅の強さを設定;
      cfg.chroma_level = 128; // 初期値128
      // 数値を下げると彩度が下がり、0で白黒になります。数値を上げると彩度が上がります。;

      // バックグラウンドでPSRAMの読出しを行うタスクの優先度を設定;
      // cfg.task_priority = 25;

      // バックグラウンドでPSRAMの読出しを行うタスクを実行するCPUを選択 (APP_CPU_NUM or PRO_CPU_NUM);
      // cfg.task_pinned_core = PRO_CPU_NUM;

      _panel_instance.config_detail(cfg);
    }

    setPanel(&_panel_instance);
  }
};

LGFX gfx;

void setup(void) {

  Serial.begin(115200);

  // pinMode(KEYBOARD_DATA, INPUT_PULLUP);
  // pinMode(KEYBOARD_CLK,  INPUT_PULLUP);

  // keyboard.begin();
  keyboard.begin(KEYBOARD_DATA, KEYBOARD_CLK);
  
  gfx.init();

  for (int x = 0; x < gfx.width(); ++x) {
    int v = x * 256 / gfx.width();
    gfx.fillRect(x, 0 * gfx.height() >> 3, 7, gfx.height() >> 3, gfx.color888(v, v, v));
    gfx.fillRect(x, 1 * gfx.height() >> 3, 7, gfx.height() >> 3, gfx.color888(v, 0 ,0));
    gfx.fillRect(x, 2 * gfx.height() >> 3, 7, gfx.height() >> 3, gfx.color888(0, v, 0));
    gfx.fillRect(x, 3 * gfx.height() >> 3, 7, gfx.height() >> 3, gfx.color888(0, 0, v));
  }
  delay(1000);
  gfx.drawLine(0,0, gfx.width() - 1, gfx.height() - 1, TFT_GREEN);
  gfx.drawRect(0,0, gfx.width(), gfx.height(), TFT_RED);
  gfx.drawRect(1,1, gfx.width() - 2, gfx.height() - 2, TFT_GREEN);

  gfx.drawRect(2,2, gfx.width() - 4, gfx.height() - 4, TFT_BLUE);
  gfx.fillRect(gfx.width()/2, (gfx.height() >> 3) * 4, gfx.height() / 3, gfx.height() / 3, TFT_PURPLE);


  Serial.println("Keyboard Test:");

}

void loop(void) {
  gfx.setCursor(20, (gfx.height() >> 3) * 4);
  gfx.setTextSize(0);
  gfx.println("hello at " + String(millis()));

   if (keyboard.available()) {

    // read the next key
    char c = keyboard.read();

    // check for some of the special keys
    if (c == PS2_ENTER) {
      Serial.println();
    } else if (c == PS2_TAB) {
      Serial.print("[Tab]");
    } else if (c == PS2_ESC) {
      Serial.print("[ESC]");
    } else if (c == PS2_PAGEDOWN) {
      Serial.print("[PgDn]");
    } else if (c == PS2_PAGEUP) {
      Serial.print("[PgUp]");
    } else if (c == PS2_LEFTARROW) {
      Serial.print("[Left]");
    } else if (c == PS2_RIGHTARROW) {
      Serial.print("[Right]");
    } else if (c == PS2_UPARROW) {
      Serial.print("[Up]");
    } else if (c == PS2_DOWNARROW) {
      Serial.print("[Down]");
    } else if (c == PS2_DELETE) {
      Serial.print("[Del]");
      gfx.clear();
      gfx.setCursor(16, 16);
      gfx.display();
    } else {

      gfx.setCursor(16, 16);

      // otherwise, just print all normal characters
      Serial.print(c);

      gfx.print(c);
      // gfx.display();

    }
  }

  // if (keyboard.available()) {
  //   Serial.write(keyboard.read());
  // }
}
//----------------------------------------------------------------

// LGFX gfx;

// void setup(void)
// {
// // 色数の指定 (省略時は rgb332_1Byte)
// //gfx.setColorDepth( 8);        // RGB332 256色
// //gfx.setColorDepth(16);        // RGB565 65536色
// //gfx.setColorDepth(lgfx::color_depth_t::rgb332_1Byte);   // RGB332 256色
// //gfx.setColorDepth(lgfx::color_depth_t::rgb565_2Byte);   // RGB565 65536色
// //gfx.setColorDepth(lgfx::color_depth_t::grayscale_8bit); // モノクロ 256階調

// //※ 実行中に setColorDepth で色数を変更することも可能ですが、
// //   メモリの再割当を実行するため描画内容は無効になります。

//   gfx.init();

//   for (int x = 0; x < gfx.width(); ++x)
//   {
//     int v = x * 256 / gfx.width();
//     gfx.fillRect(x, 0 * gfx.height() >> 3, 7, gfx.height() >> 3, gfx.color888(v, v, v));
//     gfx.fillRect(x, 1 * gfx.height() >> 3, 7, gfx.height() >> 3, gfx.color888(v, 0 ,0));
//     gfx.fillRect(x, 2 * gfx.height() >> 3, 7, gfx.height() >> 3, gfx.color888(0, v, 0));
//     gfx.fillRect(x, 3 * gfx.height() >> 3, 7, gfx.height() >> 3, gfx.color888(0, 0, v));
//   }
//   delay(1000);
// }

// void loop(void)
// {
//   gfx.fillRect(rand() % gfx.width() - 8, rand() % gfx.height() - 8, 16, 16, rand());
// }

// void setup() {
//   // pinMode(32, ADC_11db);
//   // pinMode(33, ADC_11db);
//     Serial.begin(115200);

//     keyboard.begin();
// }

// void loop() {
//     if (keyboard.available()) {
//         Serial.write(keyboard.read());
//     }
// }

//---------------------------------------------------------------------------


// //-----------------------------------------------------------------------------------------
// // #define SCREEN_WIDTH 128 // OLED display width, in pixels
// // #define SCREEN_HEIGHT 32 // OLED display height, in pixels   
// #include <PS2Keyboard.h>
// // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// // #define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
// // Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// const int DataPin = 32;
// const int IRQpin =  33;

// PS2Keyboard keyboard;

// void setup() {
//   delay(1000);
//   Serial.begin(115200);
// //   // Serial.begin(9600)

//   // M5.begin(true, false, true);
//   // Wire.begin(21, 25);
//   // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
//   // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
//   //   Serial.println(F("SSD1306 allocation failed"));
//   //   for (;;); // Don't proceed, loop forever
//   // }
//   keyboard.begin(DataPin, IRQpin);
//   // display.clearDisplay();
//   // display.setCursor(0, 0);
//   // display.setRotation(2);
//   // display.setTextSize(3);             // Normal 1:1 pixel scale
//   // display.setTextColor(SSD1306_WHITE);        // Draw white text

//   Serial.println("Keyboard Test:");
// }

// void loop() {
//   if (keyboard.available()) {
//     Serial.println("available");

//     // read the next key
//     char c = keyboard.read();

//     // check for some of the special keys
//     if (c == PS2_ENTER) {
//       Serial.println();
//     } else if (c == PS2_TAB) {
//       Serial.print("[Tab]");
//     } else if (c == PS2_ESC) {
//       Serial.print("[ESC]");
//     } else if (c == PS2_PAGEDOWN) {
//       Serial.print("[PgDn]");
//     } else if (c == PS2_PAGEUP) {
//       Serial.print("[PgUp]");
//     } else if (c == PS2_LEFTARROW) {
//       Serial.print("[Left]");
//     } else if (c == PS2_RIGHTARROW) {
//       Serial.print("[Right]");
//     } else if (c == PS2_UPARROW) {
//       Serial.print("[Up]");
//     } else if (c == PS2_DOWNARROW) {
//       Serial.print("[Down]");
//     } else if (c == PS2_DELETE) {
//       Serial.print("[Del]");
//       // display.clearDisplay();
//       // display.setCursor(0, 0);
//       // display.display();
//     } else {

//       // otherwise, just print all normal characters
//       Serial.print(c);

//       // display.print(c);
//       // display.display();

//     }
//   }
// }