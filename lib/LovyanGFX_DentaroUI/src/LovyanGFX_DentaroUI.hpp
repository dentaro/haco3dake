#pragma once //インクルードガード
#include <SD.h>
#include <SPIFFS.h>
#include "LovyanGFX_DentaroUI.hpp"
#include <Arduino.h>
#include "LGFX_ESP32_custom_haco3dake.hpp"

using namespace std;

#include <deque>

#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>

#include <limits.h>
#include <stddef.h>
#include <vector>

#include "DentaroVector.hpp"
#include "DentaroPhisics.hpp"

// //開発者表記
// #define CHAR_1_BYTE_5 5//=EN 5文字
// #define CHAR_1_BYTE_9 9//=EN 9文字
// #define CHAR_3_BYTE_5 15//=JP 5文字
// #define CHAR_3_BYTE_9 27//=JP 9文字


// //開発者表記
// #define CHAR_3_BYTE 0//=JP
// #define CHAR_1_BYTE 2//=EN
// #define NUMERIC 4
// //ユーザー表記(Arduino側でしか使わない)
// #define JP 0
// #define EN 2



// #define SHOW_NAMED 0
// #define SHOW_ALL 1

// #define TOUCH_MODE 0
// #define PHYSICAL_MODE 1
// #define ANIME_MODE 2

// #define TOGGLE_MODE true

// #define PARENT_LCD 0
// #define PARENT_SPRITE 1

// #define ROW_MODE false
// #define COL_MODE true

// #define SHIFT_NUM 5//シフトパネルの数
// #define HENKAN_NUM 57

// #define VISIBLE true
// #define INVISIBLE false

//--地図用
// #define BUF_PNG_NUM 1
// #define BUF_PNG_NUM 9

class MyTFT_eSPI : public LGFX {
  public:

    void drawObako(uint16_t* data){
    int _width = 128;
    int _height = 128;

    pushImage(0,0,128,128,data);
    }
};

class MyTFT_eSprite : public LGFX_Sprite {
  public:
    MyTFT_eSprite(MyTFT_eSPI* tft): LGFX_Sprite(tft){
      _mytft = tft;
    }

    // MyTFT_eSprite(MyTFT_eSprite* sprite): LGFX_Sprite(sprite){
    //   _mysprite = sprite;
    // }

    void myDrawRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color){
      drawFastHLine(x, y, w, color);
      drawFastHLine(x, y + h - 1, w, color);
      // Avoid drawing corner pixels twice
      drawFastVLine(x, y+1, h-2, color);
      drawFastVLine(x + w - 1, y+1, h-2, color);
    }

    void drawObako(){
      _mytft->drawObako(_img);
    }

    // void drawObakoTft(){
    //   _mytft->drawObako(_img);
    // }
    // void drawObakoSprite(){
    //   _mytft->drawObako(_img);
    // }
  private:
    uint16_t* _img;
    MyTFT_eSPI* _mytft;
    MyTFT_eSprite * _mysprite;
};

struct Vec2{
  double x;
  double y;

  Vec2() = default; // デフォルトコンストラクタ

  Vec2(double _x, double _y) // コンストラクタ
  : x(_x)
  , y(_y) {}
};


class LovyanGFX_DentaroUI {
  
  private:
  
  // uint32_t eventBits = 0b00000000000000000000000000000000;
  // uint8_t tapCount = 0;//タップカウンタ
  // uint8_t lastTapCount = 0;
  // bool jadgeF = false;
  
  //   uint32_t btnState = B00000000;
  //   uint16_t touchState = B00000000;
  //   unsigned long touchStartTime = 0;
  //   unsigned long preTouchStartTime = 0;
  //   unsigned long lastTappedTime = 0;
  //   unsigned long firstTappedTime = 0;

  //   unsigned long sTime = 0;
  //   unsigned long tappedTime = 0;
    
  //   int eventState = -1;
  //   int flickState = -1;
  //   int tap_flick_thre = 82000;//タップとフリックの閾値
  //   int dist_thre = 40;
  //   int uiID = -1;
  //   int gPanelID = 0;
  //   int gBtnID = 0;
  //   uint constantBtnID = 9999;
  //   bool constantGetF = false;
  //   int selectBtnBoxID = -1;
  //   int runEventNo =  -1;
  //   int parentID = 0;//初期値0 =　PARENT_LCD
  //   bool availableF = false;
  //   int uiMode = TOUCH_MODE;
  //   int showFlickPanelNo = 0;

    // int timeCnt = 0;
    // uint16_t clist[5] = {0,0,0,0,0};
    // int preEventState = -1;
    // int AngleCount = 0;
    // int uiBoxes_num = 0;
    // int uiAddBoxes_num = 0;

    // int layoutSprite_w = 0;
    // int layoutSprite_h = 0;
    // bool toggle_mode = false;
    // String m_url = "";

    int sec, psec;
    int fps = 0;
    int frame_count = 0;
    // bool use_flickUiSpriteF = false;
    // int charNo=0;

    // int fpNo = 0;
    // bool selectModeF = false;
    // int curbtnID;//現在の行番号
    // int curKanaRowNo = 0;
    // int curKanaColNo = 0;
    // int addBtnNum = 0;

    // bool allpushF = false;
    // int ecnt = 0;
    
    // String ROI_m_url ="";

    // int phbtnState[4];//物理ボリューム

    //----Map用ここまで

public:
    LovyanGFX_DentaroUI( LGFX* _lcd );
    LGFX* lcd;
    void begin( LGFX& _lcd, int _colBit, int _rotateNo);
    void begin(LGFX& _lcd);
    void showInfo( LovyanGFX& _lgfx, int _infox, int _infoy);
    std::uint32_t getHitValue();
};
