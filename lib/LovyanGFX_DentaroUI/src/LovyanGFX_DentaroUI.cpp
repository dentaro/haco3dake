#include "LovyanGFX_DentaroUI.hpp"
#include <iostream>
#include <string>
#include <vector>
// #include <esp_heap_caps.h>
#include "FS.h"
#include "SPIFFS.h"
#define FORMAT_SPIFFS_IF_FAILED true

#include <driver/adc.h>//アナログボタンはこのヘッダファイルを忘れないように！！
using namespace std;

#define REPEAT_CAL false

// 閾値のリスト
  float statebtn_vals[6] = {
      1150, 1050, 560, 180, 20, 0
  };

LovyanGFX_DentaroUI::LovyanGFX_DentaroUI(LGFX* _lcd)
{
  lcd = _lcd;
}


void LovyanGFX_DentaroUI::begin( LGFX& _lcd, int _colBit, int _rotateNo)
{

  _lcd.init();
  _lcd.begin();
  // _lcd.startWrite();//CSアサート開始
  // _lcd.setColorDepth( _colBit );
  // touchCalibrationF = _calibF;
  // begin(_lcd);
  // _lcd.setRotation( _rotateNo );
  // showSavedCalData(_lcd);//タッチキャリブレーションの値を表示

}
 
std::vector<std::string> split(std::string str, char del) {
    int first = 0;
    int last = str.find_first_of(del);
 
    std::vector<std::string> result;
 
    while (first < str.size()) {
        std::string subStr(str, first, last - first);
 
        result.push_back(subStr);
 
        first = last + 1;
        last = str.find_first_of(del, first);
 
        if (last == std::string::npos) {
            last = str.size();
        }
    }
 
    return result;
}


void LovyanGFX_DentaroUI::begin( LGFX& _lcd)
{

}

void LovyanGFX_DentaroUI::showInfo(LovyanGFX& _lgfx , int _infox, int _infoy){
  //フレームレート情報などを表示します。
  _lgfx.setTextSize(1);
  _lgfx.setFont(&lgfxJapanGothicP_8);
  // _lgfx.fillRect( _infox, _infoy, 150, 10, TFT_BLACK );
  _lgfx.setTextColor( TFT_WHITE , TFT_BLACK );
  _lgfx.setCursor( _infox,_infoy );
  _lgfx.print( fps );
  _lgfx.print( ":" );
  ++frame_count;sec = millis() / 1000;
  if ( psec != sec ) {
    psec = sec; fps = frame_count;
    frame_count = 0;
    _lgfx.setAddrWindow( 0, 0, _lgfx.width(), _lgfx.height() );
  }

  // _lgfx.setCursor( 0,_infoy-20 );
  //   // ヒープメモリの全体サイズを取得
  // size_t totalHeap = ESP.getHeapSize();
  // // 現在の空きメモリ量を取得
  // size_t freeHeap = ESP.getFreeHeap();
  
  // // ヒープメモリの使用率を計算（0～100の値に変換）
  // uint8_t usagePercentage = (100 * (totalHeap - freeHeap)) / totalHeap;
  // _lgfx.printf("%u%%\n", usagePercentage);
}