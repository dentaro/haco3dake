#pragma once //インクルードガード

#define SDCARD_SS_PIN 5 //M5toughのSD_CSは4
#define SDCARD_SPI SPI

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "Arduino.h"

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

//M5stackなどの製品で使用するときはAUTODETECTを使って下さい。
//#include <LGFX_AUTODETECT.hpp>

//自作基板などで、AUTODETECTの代わりにカスタム設定を使いたいときはこちらを編集して下さい。
// #include "LGFX_ESP32S3_RGB_MakerfabsParallelTFTwithTouch40.h"
#include "LGFX_ESP32_custom_haco3dake.hpp"
// #include "LGFX_ESP32S3_RGB_4848.h"

using namespace std;

#include <list>


#define USE_PSRAM true

#define COL_DEPTH 16
#define TILE_COL_DEPTH 16
// #define TILE_CHILD_COL_DEPTH 8

#define TOUCH_NONE_MODE -1
#define TOUCH_BTN_MODE 0
#define TOUCH_TILE_MODE 1
#define TOUCH_SLIDER_MODE 2
#define TOUCH_FLICK_MODE 3
#define TOUCH_TOGGLE_MODE 4
#define TOUCH_OBTN_MODE 5

#define TFT4_BLACK 0
#define TFT4_BLUE 1
#define TFT4_GREEN 2
#define TFT4_RED 3
#define TFT4_CYAN 4
#define TFT4_MAGENTA 5
#define TFT4_YELLOW 6
#define TFT4_ORANGE 7
#define TFT4_WHITE 15

#define USE_PSRAM true

#define B_UNTOUCH B00000000
#define B_TOUCH B00000001
#define B_MOVE B00000011
#define B_RELEASE B00000010

//eventState
#define NO_EVENT -1
#define UNTOUCH 0
#define DRAG 2
#define TAPPED 3
#define WTAPPED 4
#define FLICK 5 //フリックの中心、始まりのイベント
#define FLICKED 6 //フリック終わりのイベント

#define TOUCH 7
#define WAIT 8
#define MOVE 9
#define RELEASE 10

//flickState
#define RIGHT_FLICK 11
#define U_R_FLICK 12
#define UP_FLICK 13
#define L_U_FLICK 14
#define LEFT_FLICK 15
#define D_L_FLICK 16
#define DOWN_FLICK 17
#define R_D_FLICK 18

#define MULTI_EVENT 31

#define X_VAL 0
#define Y_VAL 1
#define XY_VAL 2

// #define BTN_MODE_NONE -1
// #define BTN_MODE_NORMAL 0
// #define BTN_MODE_TOGGLE 1
// #define BTN_MODE_FLICK 2

class DelegateBase2 {
public:
  DelegateBase2(){};
  virtual ~DelegateBase2(){};
  virtual int operator()( int value0 ) = 0;  // 関数プロトタイプ「int xxx(int);」
};

template < class T >
class Delegate2 : public DelegateBase2 {
  public:
    Delegate2(){};
    virtual ~Delegate2(){};

    // オペレータ実装
    virtual int operator()( int value0 ) {   // 関数プロトタイプ「int xxx(int);」
      (m_obj->*m_func)(value0);   // ハンドラ実行：関数プロトタイプ「int xxx(int);」
      return ( value0 );
    }
    typedef int (T::*EventFunc)(int);    // 関数プロトタイプ「int xxx(int);」


    // オブジェクトとメソッドを登録
    void set( T* obj, EventFunc func ) {
      m_obj = obj;
      m_func = func;
    }

    // デリゲータ生成関数
    static DelegateBase2* createDelegator2( T* obj, EventFunc func ) {
      Delegate2* dg = new Delegate2;
      dg->set( obj, func );
      return dg;
    }

  protected:
    T* m_obj;               // オブジェクト
    EventFunc m_func;       // 関数ポインタ
};

class TouchBtn {
  private:
    std::list<DelegateBase2*> lim2;
    int b_x = 0;
    int b_y = 0;

    int b_w = 0;
    int b_h = 0;
    int b_hw = 0;
    int b_hh = 0;
    int b_qw = 0;

    int b_a = 0;
    int b_a0 = 360;
    int b_a1 = 0;
    int b_r0 = 50;
    int b_r1 = 0;
    int b_n = 1;
    int btns_starAngle = 0;

    int s_x = 0;
    int s_y = 0;
    int s_w = 40;
    int s_h = 80;
    int s_hw = 0;
    int s_hh = 0;
    int s_qw = 0;

    float b_str_hw = 4;//文字列の半分の長さを計算

    int sliderPosx = 0;
    int sliderPosy = 0;
    int xy_mode = XY_VAL;

    uint8_t bgColorIndex;
    LGFX_Sprite divSprite0;
    uint16_t color = TFT_WHITE;
    int btnID = 0;//すべてのボタンの固有のID
    int btnNo = 0;//UI内での順番
    String btnIDlabel = "";//ボタン番号
    bool selectBtnF = false;
    String btn_name = "";//ボタンの名前
    String btn_nameFalse;
    int eventState = -1;
    int runEventNo = -1;
    int AngleCount = 0;
    uint8_t bgColIndex = 0;
    bool availableF = false;
    bool visibleF = false;

    int btn_mode = TOUCH_NONE_MODE;
    bool toggleVal = false;

    lgfx::v1::touch_point_t sp;
    lgfx::v1::touch_point_t tp;
    lgfx::v1::touch_point_t uiSpritePos;//ボタン用の位置
    lgfx::v1::touch_point_t layoutSpritePos;
    lgfx::v1::touch_point_t tilePos;//basic_spriteの位置
    float sliderValx = 0.5;
    float sliderValy = 0.5;

    bool drawFinishF = false;

public:
    // TouchBtn(LGFX* _lcd): lcd(_lcd) {}
    // LGFX* lcd;
    TouchBtn();

    void initBtn(int _btnID, String _btnIDlabel, int _b_x, int _b_y, int _b_w, int _b_h, String _btn_name,
    lgfx::v1::touch_point_t _lcdPos,
    int _btn_mode);

    void setOBtnPos( int _b_x, int _b_y);
    void initOBtn(int _btnID, String _btnIDlabel, int _btnNo, int _btns_starAngle, int _b_x, int _b_y, int _b_r0, int _b_r1, int _b_n, String _btn_name,
    lgfx::v1::touch_point_t _layoutSpritePos,
    lgfx::v1::touch_point_t _uiSpritePos,
    int _btn_mode);

    void setlayoutSpritePos(lgfx::v1::touch_point_t _layoutSpritePos);

    void initSlider(int _btnID, int _s_x, int _s_y, int _s_w, int _s_h, String _btn_name,
    lgfx::v1::touch_point_t _layoutSpritePos,
    lgfx::v1::touch_point_t _uiSpritePos,
    int _visible_mode );

    // void initTile(int _btnID,String _btn_name, lgfx::v1::touch_point_t _layoutSpritePos, int layoutSprite_w, int layoutSprite_h, LGFX_Sprite& _layoutSprite, LGFX_Sprite& _g_basic_sprite);
    // void setBtnName(String _btnName);
    void setBtnNameFalse(String _btnNameFalse);
    // void btnDraw(LGFX_Sprite& _lcd);
    void btnDraw(LovyanGFX& _lcd);
    void btnDraw(LovyanGFX&  _lcd, int _x, int _y );
    void flickDraw(LovyanGFX& _lcd, int _x, int _y );
    void sliderDraw(LovyanGFX& _lcd, lgfx::v1::touch_point_t _tp, int _x, int _y);
    void tileDraw(LovyanGFX& _lgfx, LGFX_Sprite& _layoutSprite, lgfx::v1::touch_point_t _layoutSpritePos, lgfx::v1::touch_point_t _sp, uint8_t _bgColIndex, LGFX_Sprite& _g_basic_sprite);
    void setAvailableF(bool _availableF);
    bool getAvailableF();
    void setVisibleF(bool _visibleF);
    bool getVisibleF();
    void addHandler(DelegateBase2* _func);
    void delHandlers2();
    void run2( int _btnID, int _btnNo, lgfx::v1::touch_point_t _sp, lgfx::v1::touch_point_t _tp, int _eventState, int _runEventNo);
    void run2( int _btnID, int _btnNo, int _sx, int _sy, int _tx, int _ty, int _eventState, int _runEventNo);

    // void run2( int _btnID, lgfx::v1::touch_point_t _sp, lgfx::v1::touch_point_t _tp, int _eventState, int _runEventNo);
    // void run2(int _btnID, int _btnNo, int _sx, int _sy, int _tx, int _ty, int _eventState, int _runEventNo);
    // void run2(int _btnID, int _btnNo, lgfx::v1::touch_point_t _sp, lgfx::v1::touch_point_t _tp, int _eventState, int _runEventNo);

    void setColor(uint16_t _color);
    void setBgColorIndex(uint16_t _bgColorIndex);
    int getBtnID();
    int getBtnMode();

    void setBtnName(String _btn_name);
    String getBtnName();

    // void setPngTile(fs::FS &fs, String _m_url, LGFX_Sprite& _g_basic_sprite);
    lgfx::v1::touch_point_t getTouchPoint(int _x, int _y);
    lgfx::v1::touch_point_t getBtnPos();


    void setSelectBtnF(bool _selectBtnF);
    void switchToggleVal();
    //private対策
    float getSliderValx();
    float getSliderValy();
    void setSliderVal(float _x, float _y);
    bool getToggleVal();
    // lgfx::v1::touch_point_t  getTilePos();
    void setTilePos(lgfx::v1::touch_point_t _pos);
    int get_xy_mode();

    void setDrawFinishF(bool _drawFinishF);
    bool getDrawFinishF();

};
