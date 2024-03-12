#include "KbdRptParser.h"

bool KbdRptParser::getShiftF(){
  return shiftF;
}

uint8_t KbdRptParser::getKeycode()
{
  
    return gkeycode;
}

uint8_t KbdRptParser::getasciicode()
{
  
    return gasciicode;
}

void KbdRptParser::setKeycode(uint8_t _keycode){
  gkeycode = _keycode;
  
}

uint8_t KbdRptParser::keyremap(uint8_t mod, uint8_t _keycode){
return _keycode;
}

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t _keycode)
{
  keyDownMillis = millis();
  Serial.print("DN ");
  gkeycode = keyremap(mod, _keycode);
  uint8_t asciiedc = keyremap(mod, _keycode);
  if (asciiedc){
    OnKeyPressed(asciiedc);
    }
}

int KbdRptParser::getKeyDownMillis(){
  return keyDownMillis; 
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t _keycode)
{
  Serial.print("UP ");
  gkeycode = _keycode;
  //初期化
  keyDownMillis = -1;
}

void KbdRptParser::OnKeyPressed(uint8_t _remapkeycode)
{
  Serial.print("ASCII: ");
  Serial.print((char)_remapkeycode);
  Serial.print("[");
  Serial.print(_remapkeycode);
  Serial.println("]");
  gkeycode = _remapkeycode;
  gasciicode = _remapkeycode;
}