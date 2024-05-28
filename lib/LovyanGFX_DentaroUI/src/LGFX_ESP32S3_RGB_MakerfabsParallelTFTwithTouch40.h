#include <iostream>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <lgfx/v1/platforms/esp32s3/Panel_RGB.hpp>
#include <lgfx/v1/platforms/esp32s3/Bus_RGB.hpp>
#include <driver/i2c.h>

class LGFX : public lgfx::LGFX_Device
{
public:

  lgfx::Bus_RGB      _bus_instance;
  lgfx::Panel_ST7701 _panel_instance;
  lgfx::Touch_GT911  _touch_instance;
  lgfx::Light_PWM   _light_instance;

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

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _panel_instance.config_detail();
      
      cfg.pin_cs = 39;//40;//1;
      cfg.pin_sclk = 48;//36;//12;
      cfg.pin_mosi = 47;//35;//11;

      _panel_instance.config_detail(cfg);
    }

    {
      auto cfg = _bus_instance.config();
      cfg.panel = &_panel_instance;

      cfg.pin_d11  = GPIO_NUM_11; // R0//0//11
      cfg.pin_d12  = GPIO_NUM_12; // R1//14//12
      cfg.pin_d13  = GPIO_NUM_13; // R2//13//13
      cfg.pin_d14  = GPIO_NUM_14; // R3//12//14
      cfg.pin_d15  = GPIO_NUM_0; // R4//11//0

      cfg.pin_d5  = GPIO_NUM_8; // G0//10//8
      cfg.pin_d6  = GPIO_NUM_20;  // G1//9//20
      cfg.pin_d7  = GPIO_NUM_3;  // G2//46//3
      cfg.pin_d8  = GPIO_NUM_46;  // G3//3//46
      cfg.pin_d9  = GPIO_NUM_9;  // G4//20//9
      cfg.pin_d10 = GPIO_NUM_10;  // G5//8//10

      cfg.pin_d4 = GPIO_NUM_15;  // B0//15//4
      cfg.pin_d3 = GPIO_NUM_7;  // B1//7//5
      cfg.pin_d2 = GPIO_NUM_6;  // B2//6//6
      cfg.pin_d1 = GPIO_NUM_5;  // B3//5//7
      cfg.pin_d0 = GPIO_NUM_4;  // B4//4//15


      // cfg.pin_d15  = GPIO_NUM_11; // R0//0//11
      // cfg.pin_d14  = GPIO_NUM_12; // R1//14//12
      // cfg.pin_d13  = GPIO_NUM_13; // R2//13//13
      // cfg.pin_d12  = GPIO_NUM_14; // R3//12//14
      // cfg.pin_d11  = GPIO_NUM_0; // R4//11//0

      // cfg.pin_d10 = GPIO_NUM_8; // G0//10//8
      // cfg.pin_d9  = GPIO_NUM_20;  // G1//9//20
      // cfg.pin_d8  = GPIO_NUM_3;  // G2//46//3
      // cfg.pin_d7  = GPIO_NUM_46;  // G3//3//46
      // cfg.pin_d6  = GPIO_NUM_9;  // G4//20//9
      // cfg.pin_d5  = GPIO_NUM_10;  // G5//8//10

      // cfg.pin_d4 = GPIO_NUM_4;  // B0//15//4
      // cfg.pin_d3 = GPIO_NUM_5;  // B1//7//5
      // cfg.pin_d2 = GPIO_NUM_6;  // B2//6//6
      // cfg.pin_d1 = GPIO_NUM_7;  // B3//5//7
      // cfg.pin_d0 = GPIO_NUM_15;  // B4//4//15


      // cfg.pin_d0  = GPIO_NUM_0; // B0//0
      // cfg.pin_d1  = GPIO_NUM_14; // B1//14
      // cfg.pin_d2  = GPIO_NUM_13; // B2//13
      // cfg.pin_d3  = GPIO_NUM_12; // B3//12
      // cfg.pin_d4  = GPIO_NUM_11; // B4//11

      // cfg.pin_d5  = GPIO_NUM_10; // G0//10
      // cfg.pin_d6  = GPIO_NUM_9;  // G1//9
      // cfg.pin_d7  = GPIO_NUM_46;  // G2//46
      // cfg.pin_d8  = GPIO_NUM_3;  // G3//3
      // cfg.pin_d9  = GPIO_NUM_20;  // G4//20
      // cfg.pin_d10 = GPIO_NUM_8;  // G5//8

      // cfg.pin_d11 = GPIO_NUM_15;  // R0//15
      // cfg.pin_d12 = GPIO_NUM_7;  // R1//7
      // cfg.pin_d13 = GPIO_NUM_6;  // R2//6
      // cfg.pin_d14 = GPIO_NUM_5;  // R3//5
      // cfg.pin_d15 = GPIO_NUM_4;  // R4//4

      // cfg.pin_d11  = GPIO_NUM_11; // B0//4
      // cfg.pin_d12  = GPIO_NUM_12; // B1//5
      // cfg.pin_d13  = GPIO_NUM_13; // B2//6
      // cfg.pin_d14  = GPIO_NUM_14; // B3//7
      // cfg.pin_d15  = GPIO_NUM_0; // B4//15

      // cfg.pin_d5  = GPIO_NUM_8; // G0//8
      // cfg.pin_d6  = GPIO_NUM_20;  // G1//9
      // cfg.pin_d7  = GPIO_NUM_3;  // G2//3
      // cfg.pin_d8  = GPIO_NUM_46;  // G3//46
      // cfg.pin_d9  = GPIO_NUM_9;  // G4//9
      // cfg.pin_d10 = GPIO_NUM_10;  // G5//10

      // cfg.pin_d0 = GPIO_NUM_4;  // R0//11
      // cfg.pin_d1 = GPIO_NUM_5;  // R1//12
      // cfg.pin_d2 = GPIO_NUM_6;  // R2//13
      // cfg.pin_d3 = GPIO_NUM_7;  // R3//14
      // cfg.pin_d4 = GPIO_NUM_15;  // R4//0

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
      cfg.offset_rotation = 0;

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
      _light_instance.config(cfg);
    }
    _panel_instance.light(&_light_instance);

    setPanel(&_panel_instance);
  }
};
