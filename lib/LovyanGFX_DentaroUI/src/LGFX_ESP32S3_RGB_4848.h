#include <iostream>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <driver/i2c.h>

// class LGFX : public lgfx::LGFX_Device
// {
// public:

//   lgfx::Bus_RGB      _bus_instance;
//   lgfx::Panel_ST7701 _panel_instance;
//   lgfx::Touch_GT911  _touch_instance;
//   lgfx::Light_PWM   _light_instance;

//   LGFX(void)
//   {
//     {
//       auto cfg = _panel_instance.config();

//       cfg.memory_width  = 480;
//       cfg.memory_height = 480;
//       cfg.panel_width  = 480;
//       cfg.panel_height = 480;

//       cfg.offset_x = 0;
//       cfg.offset_y = 0;

//       _panel_instance.config(cfg);
//     }

//     {
//       auto cfg = _panel_instance.config_detail();
      
//       cfg.pin_cs = 39;//40;//1;
//       cfg.pin_sclk = 48;//36;//12;
//       cfg.pin_mosi = 47;//35;//11;

//       _panel_instance.config_detail(cfg);
//     }

//     {
//       auto cfg = _bus_instance.config();
//       cfg.panel = &_panel_instance;
//       cfg.pin_d0  = GPIO_NUM_11; // B0//4
//       cfg.pin_d1  = GPIO_NUM_12; // B1//5
//       cfg.pin_d2  = GPIO_NUM_13; // B2//6
//       cfg.pin_d3  = GPIO_NUM_14; // B3//7
//       cfg.pin_d4  = GPIO_NUM_0; // B4//15
//       cfg.pin_d5  = GPIO_NUM_8; // G0//8
//       cfg.pin_d6  = GPIO_NUM_20;  // G1//9
//       cfg.pin_d7  = GPIO_NUM_3;  // G2//3
//       cfg.pin_d8  = GPIO_NUM_46;  // G3//46
//       cfg.pin_d9  = GPIO_NUM_9;  // G4//9
//       cfg.pin_d10 = GPIO_NUM_10;  // G5//10
//       cfg.pin_d11 = GPIO_NUM_4;  // R0//11
//       cfg.pin_d12 = GPIO_NUM_5;  // R1//12
//       cfg.pin_d13 = GPIO_NUM_6;  // R2//13
//       cfg.pin_d14 = GPIO_NUM_7;  // R3//14
//       cfg.pin_d15 = GPIO_NUM_15;  // R4//0

//       cfg.pin_henable = GPIO_NUM_18;//18//DE?
//       cfg.pin_vsync   = GPIO_NUM_17;//17
//       cfg.pin_hsync   = GPIO_NUM_16;//16
//       cfg.pin_pclk    = GPIO_NUM_21;//21
//       cfg.freq_write  = 16000000;

//       cfg.hsync_polarity    = 1;
//       cfg.hsync_front_porch = 10;
//       cfg.hsync_pulse_width = 8;
//       cfg.hsync_back_porch  = 50;
//       cfg.vsync_polarity    = 1;
//       cfg.vsync_front_porch = 10;
//       cfg.vsync_pulse_width = 8;
//       cfg.vsync_back_porch  = 20;
//       cfg.pclk_idle_high    = 0;
//       cfg.de_idle_high      = 1;
//       _bus_instance.config(cfg);
//     }
//     _panel_instance.setBus(&_bus_instance);

//     {
//       auto cfg = _touch_instance.config();
//       cfg.x_min      = 0;
//       cfg.x_max      = 480;
//       cfg.y_min      = 0;
//       cfg.y_max      = 480;
//       cfg.bus_shared = true;
//       cfg.offset_rotation = 0;

//       cfg.i2c_port   = I2C_NUM_1;

//       cfg.pin_int    = GPIO_NUM_NC;
//       cfg.pin_sda    = GPIO_NUM_19;//47
//       cfg.pin_scl    = GPIO_NUM_45;//45
//       cfg.pin_rst    = GPIO_NUM_NC;//

//       cfg.freq       = 400000;
//       _touch_instance.config(cfg);
//       _panel_instance.setTouch(&_touch_instance);
//     }

//     {
//       auto cfg = _light_instance.config();
//       cfg.pin_bl = GPIO_NUM_38;
//       _light_instance.config(cfg);
//     }
//     _panel_instance.light(&_light_instance);

//     setPanel(&_panel_instance);
//   }
// };

// https://github.com/lovyan03/LovyanGFX/blob/master/examples/HowToUse/2_user_setting/2_user_setting.ino
// class LGFX : public lgfx::LGFX_Device {
//   lgfx::Panel_ST7701   _panel_instance;
//   lgfx::Bus_Parallel16 _bus_instance;   //16bit Parallelのインスタンス(ESP32s3)
//   lgfx::Touch_GT911  _touch_instance;

// //   lgfx::Bus_RGB      _bus_instance;
// //   lgfx::Light_PWM   _light_instance;

// public:
//   LGFX(void) {               // バス制御の設定を行います。
//     auto cfg = _bus_instance.config();       // バス設定用の構造体を取得します。
//     // 16ビットパラレルバスの設定
//     cfg.freq_write = 20000000;               // 送信クロック(最大20MHz,80MHzを整数割の値に丸める)
//     cfg.pin_wr  =  8;                        // WR を接続しているピン番号
//     cfg.pin_rd  = 12;                        // RD を接続しているピン番号
//     cfg.pin_rs  = 11;                        // RS(D/C)を接続しているピン番号
//     cfg.pin_d0  = 18;                        // D0 を接続しているピン番号
//     cfg.pin_d1  = 36;                        // D1 を接続しているピン番号
//     cfg.pin_d2  = 17;                        // D2 を接続しているピン番号
//     cfg.pin_d3  = 37;                        // D3 を接続しているピン番号
//     cfg.pin_d4  = 16;                        // D4 を接続しているピン番号
//     cfg.pin_d5  = 38;                        // D5 を接続しているピン番号
//     cfg.pin_d6  = 15;                        // D6 を接続しているピン番号s
//     cfg.pin_d7  = 39;                        // D7 を接続しているピン番号
//     cfg.pin_d8  =  2;                        // D8 を接続しているピン番号
//     cfg.pin_d9  =  4;                        // D9 を接続しているピン番号
//     cfg.pin_d10 = 42;                        // D10を接続しているピン番号
//     cfg.pin_d11 =  5;                        // D11を接続しているピン番号
//     cfg.pin_d12 = 41;                        // D12を接続しているピン番号
//     cfg.pin_d13 =  6;                        // D13を接続しているピン番号
//     cfg.pin_d14 = 40;                        // D14を接続しているピン番号
//     cfg.pin_d15 =  7;                        // D15を接続しているピン番号
//     _bus_instance.config(cfg);               // 設定値をバスに反映します。
//     _panel_instance.setBus(&_bus_instance);  // バスをパネルにセットします。
//     {                                        // 表示パネル制御の設定
//       auto cfg = _panel_instance.config();   // 表示パネル設定用の構造体を取得します。
//       cfg.pin_cs   = 10;                     // CSが接続されているピン番号   (-1 = disable)
//       cfg.pin_rst  =  9;                     // RSTが接続されているピン番号  (-1 = disable)
//       cfg.pin_busy = -1;                     // BUSYが接続されているピン番号 (-1 = disable)
//       cfg.memory_width = 480;                // ドライバICがサポートしている最大の幅
//       cfg.memory_height = 800;               // ドライバICがサポートしている最大の高さ
//       cfg.panel_width = 480;                 // 実際に表示可能な幅
//       cfg.panel_height = 800;                // 実際に表示可能な高さ
//       cfg.offset_x = 0;                      // パネルのX方向オフセット量
//       cfg.offset_y = 0;                      // パネルのY方向オフセット量
//       cfg.offset_rotation = 0;               // 回転方向の値のオフセット 0~7 (4~7は上下反転)
//       cfg.dummy_read_pixel = 8;              // ピクセル読出し前のダミーリードのビット数
//       cfg.dummy_read_bits = 1;               // ピクセル以外のデータ読出し前のダミーリードのビット数
//       cfg.readable = false;                  // データ読出しが可能な場合 trueに設定
//       cfg.invert = false;                    // パネルの明暗が反転してしまう場合 trueに設定
//       cfg.rgb_order = true;                  // パネルの赤と青が入れ替わってしまう場合 trueに設定
//       cfg.dlen_16bit = true;                 // データ長を16bit単位で送信するパネルの場合trueに設定
//       cfg.bus_shared = true;                 // SDカードとバス共有はtrueに設定
//       _panel_instance.config(cfg);
//     }

//     {  // タッチスクリーン制御の設定を行います。（必要なければ削除）
//       auto cfg = _touch_instance.config();
//       cfg.x_min = 300;          // タッチスクリーンから得られる最小のX値(生の値)
//       cfg.x_max = 3800;         // タッチスクリーンから得られる最大のX値(生の値)
//       cfg.y_min = 180;          // タッチスクリーンから得られる最小のY値(生の値)
//       cfg.y_max = 3900;         // タッチスクリーンから得られる最大のY値(生の値)
//       cfg.pin_int = 13;         // INTが接続されているピン番号
//       cfg.bus_shared = false;   // 画面と共通のバスを使用している場合 trueを設定
//       cfg.offset_rotation = 6;  // 表示とタッチの向きのが一致しない場合の調整 0~7の値で設定
//       // SPI接続の場合
//       cfg.spi_host = SPI2_HOST;  // 使用するSPIを選択 (HSPI_HOST or VSPI_HOST)
//       cfg.freq = 1000000;        // SPIクロックを設定, Max 2.5MHz, 8bit(7bit) mode
//       cfg.pin_sclk = 35;         // SCLKが接続されているピン番号, TP CLK
//       cfg.pin_mosi = 21;         // MOSIが接続されているピン番号, TP DIN
//       cfg.pin_miso = 47;         // MISOが接続されているピン番号, TP DOUT
//       cfg.pin_cs   = 48;         // CS  が接続されているピン番号, TP CS
//       _touch_instance.config(cfg);
//       _panel_instance.setTouch(&_touch_instance);  // タッチスクリーンをパネルにセットします。
//     }

//     setPanel(&_panel_instance);  // 使用するパネルをセットします。
//   }
// };

namespace lgfx
{
  struct Panel_SenseCapD1 : public lgfx::Panel_ST7701
  {
    static constexpr int32_t i2c_freq = 400000;
    static constexpr int_fast16_t pca9535_i2c_addr = 0x20;
    static constexpr int_fast16_t i2c_port = I2C_NUM_0;
    static constexpr int_fast16_t i2c_sda = GPIO_NUM_39;
    static constexpr int_fast16_t i2c_scl = GPIO_NUM_40;
    Panel_SenseCapD1(void)
    {
    }
    bool init(bool use_reset) override
    {
      lgfx::i2c::init(i2c_port, i2c_sda, i2c_scl);

      lgfx::i2c::writeRegister8(i2c_port, pca9535_i2c_addr, 0x06, 0, ~(1<<4), i2c_freq);  // IO0_4: output
      lgfx::i2c::writeRegister8(i2c_port, pca9535_i2c_addr, 0x06, 0, ~(1<<5), i2c_freq);  // IO0_5: output
      lgfx::i2c::writeRegister8(i2c_port, pca9535_i2c_addr, 0x07, 0, ~(1<<0), i2c_freq);  // IO1_0: output
      lgfx::i2c::writeRegister8(i2c_port, pca9535_i2c_addr, 0x07, 0, ~(1<<2), i2c_freq);  // IO1_10: output

      if (!Panel_RGB::init(use_reset))
      {
        return false;
      }

      int32_t pin_mosi = _config_detail.pin_mosi;
      int32_t pin_sclk = _config_detail.pin_sclk;
      if (pin_mosi >= 0 && pin_sclk >= 0)
      {
        lgfx::gpio::pin_backup_t backup_pins[] = { (gpio_num_t)pin_mosi, (gpio_num_t)pin_sclk };

        lgfx::gpio_lo(pin_mosi);
        lgfx::pinMode(pin_mosi, pin_mode_t::output);
        lgfx::gpio_lo(pin_sclk);
        lgfx::pinMode(pin_sclk, pin_mode_t::output);


  //      int32_t pin_cs = _config_detail.pin_cs;
  //      lgfx::gpio_lo(pin_cs);
        cs_control(false);

        writeCommand(0xFF, 1);
        writeData(0x77, 1);
        writeData(0x01, 1);
        writeData(0x00, 2);
        writeData(0x10, 1);

        // 0xC0 : LNSET : Display Line Setting
        writeCommand(0xC0, 1);
        uint32_t line1 = (_cfg.panel_height >> 3) + 1;
        uint32_t line2 = (_cfg.panel_height >> 1) & 3;
        writeData(line1 + (line2 ? 0x80 : 0x00), 1);
        writeData(line2, 1);

        // 0xC3 : RGBCTRL
        auto cfg = ((Bus_RGB*)_bus)->config();
        writeCommand(0xC3, 1);
        uint32_t rgbctrl = 0;
        if ( cfg.de_idle_high  ) rgbctrl += 0x01;
        if ( cfg.pclk_idle_high) rgbctrl += 0x02;
        if (!cfg.hsync_polarity) rgbctrl += 0x04;
        if (!cfg.vsync_polarity) rgbctrl += 0x08;
        writeData(rgbctrl, 1);
        writeData(0x10, 1);
        writeData(0x08, 1);

        for (uint8_t i = 0; auto cmds = getInitCommands(i); i++)
        {
          command_list(cmds);
        }

  //      lgfx::gpio_hi(pin_cs);
        cs_control(true);
        for (auto &bup : backup_pins) { bup.restore(); }
      }

      return true;
    }

    void reset(void)
    {
      lgfx::i2c::writeRegister8(i2c_port, pca9535_i2c_addr, 0x02, 0, ~(1<<5), i2c_freq);  // LCD_RST
      lgfx::delay(40);
      lgfx::i2c::writeRegister8(i2c_port, pca9535_i2c_addr, 0x02, (1<<5), ~0, i2c_freq);  // LCD_RST
      lgfx::delay(140);
    }

    void cs_control(bool flg) override
    {
      if (flg)
      {
        lgfx::i2c::writeRegister8(i2c_port, pca9535_i2c_addr, 0x02, (1<<4), ~0, i2c_freq);  // LCD_CS
      }
      else
      {
        lgfx::i2c::writeRegister8(i2c_port, pca9535_i2c_addr, 0x02, 0, ~(1<<4), i2c_freq);  // LCD_CS
      }
    }
  };

  struct Touch_SenseCapD1 : public lgfx::Touch_GT911
  {
    static constexpr int32_t i2c_freq = 400000;
    static constexpr int_fast16_t pca9535_i2c_addr = 0x20;
    static constexpr int_fast16_t i2c_port = I2C_NUM_0;
    Touch_SenseCapD1(void)
    {
    }

    bool init(void) override
    {
      lgfx::i2c::writeRegister8(i2c_port, pca9535_i2c_addr, 0x02, 0, ~(1<<7), i2c_freq);  // TP_RST
      lgfx::delay(10);
      lgfx::i2c::writeRegister8(i2c_port, pca9535_i2c_addr, 0x02, (1<<7), ~0, i2c_freq);  // TP_RST
      lgfx::delay(10);

      return lgfx::Touch_GT911::init();
    }
  };


}


class LGFX : public lgfx::LGFX_Device
{
public:

  lgfx::Bus_RGB          _bus_instance;
  // lgfx::Bus_Parallel16 _bus_instance;   //16bit Parallelのインスタンス(ESP32s3)
  lgfx::Panel_SenseCapD1 _panel_instance;
  lgfx::Touch_SenseCapD1 _touch_instance;
  lgfx::Light_PWM        _light_instance;

  LGFX(void)
  {
    {
      auto cfg = _panel_instance.config();

      cfg.memory_width  = 480;
      cfg.memory_height = 480;
      cfg.panel_width  = 480;
      cfg.panel_height = 480;

      cfg.offset_x = 0;
      cfg.offset_y = 0;
      cfg.offset_rotation  = 2;
      cfg.invert = 1;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _panel_instance.config_detail();

      cfg.use_psram = 1;
      cfg.pin_cs = 39;
      cfg.pin_sclk = 48;
//      cfg.pin_miso = 47;
      cfg.pin_mosi = 47;

      _panel_instance.config_detail(cfg);
    }

    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;

      // cfg.pin_d0  = GPIO_NUM_0; // B0//0
      // cfg.pin_d1  = GPIO_NUM_14; // B1//14
      // cfg.pin_d2  = GPIO_NUM_13; // B2//13
      // cfg.pin_d3  = GPIO_NUM_12; // B3//12
      // cfg.pin_d4  = GPIO_NUM_11; // B4//11
      // cfg.pin_d5  = GPIO_NUM_8; // G0//8
      // cfg.pin_d6  = GPIO_NUM_20;  // G1//9
      // cfg.pin_d7  = GPIO_NUM_3;  // G2//3
      // cfg.pin_d8  = GPIO_NUM_46;  // G3//46
      // cfg.pin_d9  = GPIO_NUM_9;  // G4//9
      // cfg.pin_d10 = GPIO_NUM_10;  // G5//10
      // cfg.pin_d11 = GPIO_NUM_15;  // R0//15
      // cfg.pin_d12 = GPIO_NUM_7;  // R1//7
      // cfg.pin_d13 = GPIO_NUM_6;  // R2//6
      // cfg.pin_d14 = GPIO_NUM_5;  // R3//5
      // cfg.pin_d15 = GPIO_NUM_4;  // R4//4

      cfg.pin_d0  = GPIO_NUM_11; // B0//4
      cfg.pin_d1  = GPIO_NUM_12; // B1//5
      cfg.pin_d2  = GPIO_NUM_13; // B2//6
      cfg.pin_d3  = GPIO_NUM_14; // B3//7
      cfg.pin_d4  = GPIO_NUM_0; // B4//15
      cfg.pin_d5  = GPIO_NUM_8; // G0//8
      cfg.pin_d6  = GPIO_NUM_20;  // G1//9
      cfg.pin_d7  = GPIO_NUM_3;  // G2//3
      cfg.pin_d8  = GPIO_NUM_46;  // G3//46
      cfg.pin_d9  = GPIO_NUM_9;  // G4//9
      cfg.pin_d10 = GPIO_NUM_10;  // G5//10
      cfg.pin_d11 = GPIO_NUM_4;  // R0//11
      cfg.pin_d12 = GPIO_NUM_5;  // R1//12
      cfg.pin_d13 = GPIO_NUM_6;  // R2//13
      cfg.pin_d14 = GPIO_NUM_7;  // R3//14
      cfg.pin_d15 = GPIO_NUM_15;  // R4//0

      cfg.pin_henable = GPIO_NUM_18;//18//DE?
      cfg.pin_vsync   = GPIO_NUM_17;//17
      cfg.pin_hsync   = GPIO_NUM_16;//16
      cfg.pin_pclk    = GPIO_NUM_21;//21
      cfg.freq_write  = 16000000;

      cfg.hsync_polarity    = 1;
      cfg.hsync_front_porch = 10;
      cfg.hsync_pulse_width = 8;
      cfg.hsync_back_porch  = 50;
      cfg.vsync_polarity    = 1;
      cfg.vsync_front_porch = 10;
      cfg.vsync_pulse_width = 8;
      cfg.vsync_back_porch  = 20;
      cfg.pclk_idle_high    = 0;
      cfg.de_idle_high      = 1;
      
      _bus_instance.config(cfg);
    }
    _panel_instance.setBus(&_bus_instance);

    {
      auto cfg = _touch_instance.config();
      cfg.x_min      = 0;
      cfg.x_max      = 480;
      cfg.y_min      = 0;
      cfg.y_max      = 480;
      cfg.bus_shared = true;

      cfg.i2c_port   = I2C_NUM_1;

      cfg.pin_int    = GPIO_NUM_NC;
      cfg.pin_sda    = GPIO_NUM_19;//47
      cfg.pin_scl    = GPIO_NUM_45;//45
      cfg.pin_rst    = GPIO_NUM_NC;//

      cfg.freq       = 400000;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }

    {
      auto cfg = _light_instance.config();
      cfg.pin_bl = GPIO_NUM_38;
      cfg.pwm_channel = 0;
      cfg.invert = true;
      _light_instance.config(cfg);
    }
      _panel_instance.light(&_light_instance);

    setPanel(&_panel_instance);
  }
};