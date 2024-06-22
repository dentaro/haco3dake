#include <Arduino.h>
#include "baseGame.h"

#include <string> // 必要に応じて追加
#include <chrono>

#include <time.h>
#include <fstream>
#include <iostream>
#include <string>
#include "runLuaGame.h"
#include <sstream>

#include <FS.h>
#include "SPIFFS.h"
// #include "Tunes.h"
#include "Editor.h"
// #include <PS2Keyboard.h>
#include <KbdRptParser.h>

// #include "Speaker_Class.hpp"
// #include "Channel.hpp"
// #include <esp_now.h>
// #include <WiFi.h>
#include <LovyanGFX_DentaroUI.hpp>
#include <map>
#include "Channel.hpp"

#include <hidboot.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#define KEYBOARD_DATA 22//32
#define KEYBOARD_CLK  19//33

// PS2Keyboard keyboard;
KbdRptParser keyboard;

uint8_t charSpritex = 0;
uint8_t charSpritey = 0;
int pressedBtnID = -1;//この値をタッチボタン、物理ボタンの両方から操作してbtnStateを間接的に操作している
// TaskHandle_t taskHandle[2];
//キーボード関連
Editor editor;

char keychar;//キーボードから毎フレーム入ってくる文字

bool isSoftLED[LED_NUM];

int gameState = 0;

Speaker_Class* speaker;
uint64_t frame = 0;

int isEditMode;
bool firstBootF = true;
bool difffileF = false;//前と違うファイルを開こうとしたときに立つフラグ

std::deque<int> buttonState;//ボタンの個数未定
// int buttonState[9];
uint8_t sprno;
uint8_t repeatnum;
// std::vector<std::vector<uint8_t>> rowData(16);

bool useMouseF = false;

int mp[3] = {0,0,0};

class MouseRptParser : public MouseReportParser
{
protected:
	void OnMouseMove	(MOUSEINFO *mi);
	void OnLeftButtonUp	(MOUSEINFO *mi);
	void OnLeftButtonDown	(MOUSEINFO *mi);
	void OnRightButtonUp	(MOUSEINFO *mi);
	void OnRightButtonDown	(MOUSEINFO *mi);
	void OnMiddleButtonUp	(MOUSEINFO *mi);
	void OnMiddleButtonDown	(MOUSEINFO *mi);
};

void MouseRptParser::OnMouseMove(MOUSEINFO *mi)
{
    // Serial.print("dx=");
    // Serial.print(mi->dX, DEC);
    // Serial.print(" dy=");
    // Serial.println(mi->dY, DEC);

    // mp[0] = ((mp[0] + mi->dX)+160)%160;
    // mp[1] = ((mp[1] + mi->dY)+128)%128;
    
    mp[0] = mp[0] + mi->dX;
    mp[1] = mp[1] + mi->dY;
    if(mp[0]>160)mp[0]=160;
    if(mp[0]<0)  mp[0]=0;
    if(mp[1]>128)mp[1]=128;
    if(mp[1]<0)  mp[1]=0;

    // mp[0] = (mp[0]+160)%160;
    // mp[1] = (mp[1]+128)%128;

    // Serial.print("mx=");
    // Serial.print(mp[0]);
    // Serial.print(" my=");
    // Serial.println(mp[1]);

    // setMousePos(mi->dX,mi->dY);
};
void MouseRptParser::OnLeftButtonUp	(MOUSEINFO *mi)
{
    //Serial.println("L Butt Up");
    mp[2] = 0;
};
void MouseRptParser::OnLeftButtonDown	(MOUSEINFO *mi)
{
    //Serial.println("L Butt Dn");
    mp[2] = 1;
};
void MouseRptParser::OnRightButtonUp	(MOUSEINFO *mi)
{
    //Serial.println("R Butt Up");
    mp[2] = 0;
};
void MouseRptParser::OnRightButtonDown	(MOUSEINFO *mi)
{
    //Serial.println("R Butt Dn");
    mp[2] = 2;
};
void MouseRptParser::OnMiddleButtonUp	(MOUSEINFO *mi)
{
    //Serial.println("M Butt Up");
    mp[2] = 0;
};
void MouseRptParser::OnMiddleButtonDown	(MOUSEINFO *mi)
{
    //Serial.println("M Butt Dn");
    mp[2] = 3;
};

USB     Usb;
USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_MOUSE>    HidMouse(&Usb);

MouseRptParser                               Prs;

void mouseSetup()
{

  keyboard.begin(KEYBOARD_DATA, KEYBOARD_CLK);

    Serial.begin( 115200 );
#if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
    //Serial.println("Start");

    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");

    delay( 200 );

    HidMouse.SetReportParser(0, &Prs);
}

// void mouseloop()
// {
//   Usb.Task();
// }

uint32_t cnt = ~0;
int fps=60;//デフォルト
bool btnpF = false;
bool prebtnpF = false;
int btnptick = 0;
int prebtnptick = 0;
int btnpms = 0;
bool textMoveF = false;

unsigned long startTime = millis();

uint32_t remainTime;
uint32_t currentTime;
uint32_t elapsedTime;
uint32_t currentTime2;
uint32_t targettime2;

size_t patternID = 0;
Channel* channels = new Channel();

int soundNo = -1;
float soundSpeed = 1.0;
int musicNo = -1;
bool musicflag = false;
bool sfxflag = false;
bool toneflag = false;
bool firstLoopF = true;


uint8_t sfxlistNo = 0;
uint8_t sfxnos[8] ={0,1,2,3,4,5,6,7};

uint8_t loopStart = 0;
uint8_t loopEnd = 63;
uint8_t looplen = (loopEnd - loopStart)+1;
float bpm = 120;

// // //音関連
uint8_t buffAreaNo = 0;
uint8_t gEfectNo = 0;
uint8_t effectVal = 0.0f;
uint8_t toneTickNo = 0;
uint8_t sfxTickNo = 0;
uint8_t instrument = 0;
uint8_t targetChannelNo = 0;//描画編集する効果音番号を設定（sfx(n)のnで効果音番号を指定することで作った効果音がなる）
uint8_t tickTime = 125;//125ms*8chはbpm60
uint8_t tickSpeed = 5;//連動してない

uint8_t sfxNo;
uint8_t wavNo;
uint8_t sfxChNo;
uint8_t sfxVol;
float sfxspeed;
uint8_t sfxmusicNo;
uint8_t masterVol;
uint8_t toolNo;

int waittime = 0;

uint8_t bgmodeNo = 0;
uint8_t pngimgW = 0;
uint8_t pngimgH = 0;
uint8_t pngimgX = 0;
uint8_t pngimgY = 0;
uint8_t pngimgTransCn = 0;
String pngimgPath = "";

static int menu_x = 2;
static int menu_y = 20;
static int menu_w = 120;
static int menu_h = 30;
static int menu_padding = 36;


// #define SPEAKER_PIN 25

#define MAPWH 16//マップのpixelサイズ
#define BUF_PNG_NUM 0
// #define TFT_WIDTH 160
// #define TFT_HEIGHT 128
#define TFT_WIDTH_HALF 80
#define TFT_HEIGHT_HALF 64

#define VEC_FRAME_COUNT 10

#define TFT_RUN_MODE 0
#define TFT_EDIT_MODE 1
// #define TFT_WIFI_MODE 2


int gWx;
int gWy;
int gSpr8numX = 8;
int gSpr8numY = 8;
int gSprw = 8;
int gSprh = 8;

int HACO3_C0    = 0x0000;
int HACO3_C1    = 6474;//27,42,86 
int HACO3_C2    = 35018;
int HACO3_C3    = 1097;
int HACO3_C4    = 45669;
int HACO3_C5    = 25257;
int HACO3_C6    = 50712;
int HACO3_C7    = 65436;
int HACO3_C8    = 63496;//0xF802;
int HACO3_C9    = 64768;
int HACO3_C10   = 65376;
int HACO3_C11   = 1856;
int HACO3_C12   = 1407;
int HACO3_C13   = 33715;
int HACO3_C14   = 64341;
int HACO3_C15   = 65108;

uint8_t clist2[16][3] =
  {
  { 0,0,0},//0: 黒色
  { 27,42,86 },//1: 暗い青色
  { 137,24,84 },//2: 暗い紫色
  { 0,139,75 },//3: 暗い緑色
  { 183,76,45 },//4: 茶色
  { 97,87,78 },//5: 暗い灰色
  { 194,195,199 },//6: 明るい灰色
  { 255,241,231 },//7: 白色
  { 255,0,70 },//8: 赤色
  { 255,160,0 },//9: オレンジ
  { 255,238,0 },//10: 黄色
  { 0,234,0 },//11: 緑色
  { 0,173,255 },//12: 水色
  { 134,116,159 },//13: 藍色
  { 255,107,169 },//14: ピンク
  { 255,202,165}//15: 桃色
  };

enum struct FileType {
  LUA,
  JS,
  BMP,
  PNG,
  TXT,
  OTHER
};



//esp-idfのライブラリを使う！
//https://qiita.com/norippy_i/items/0ed46e06427a1d574625
// #include <driver/adc.h>//アナログボタンはこのヘッダファイルを忘れないように！！

using namespace std;

#define MAX_CHAR 512
#define FORMAT_SPIFFS_IF_FAILED true

#define BUF_PNG_NUM 0

uint8_t mainVol = 180;

//outputmode最終描画の仕方
// int outputMode = FAST_MODE;//50FPS程度128*128 速いけど小さい画面　速度が必要なモード
int outputMode = WIDE_MODE;//20FPS程度240*240 遅いけれどタッチしやすい画面　パズルなど

uint8_t xpos, ypos = 0;
uint8_t colValR = 0;
uint8_t colValG = 0;
uint8_t colValB = 0;


// esp_now_peer_info_t slave;

int mapsprnos[16];
// int mapsprnos[16] = { 20, 11, 32, 44, 53, 49, 54, 32, 52, 41, 46, 42, 45, 50, 43, 38 };

const uint8_t RGBValues[][3] PROGMEM = {//16bit用
  {0, 0, 0},     // 0: 黒色=なし
  {24, 40, 82},  // 1: 暗い青色
  {140, 24, 82}, // 2: 暗い紫色
  {0, 138, 74},  // 3: 暗い緑色
  {181, 77, 41}, // 4: 茶色 
  {99, 85, 74},  // 5: 暗い灰色
  {198, 195, 198}, // 6: 明るい灰色
  {255, 243, 231}, // 7: 白色
  {255, 0, 66},  // 8: 赤色
  {255, 162, 0}, // 9: オレンジ
  {255, 239, 0}, // 10: 黄色
  {0, 235, 0},   // 11: 緑色
  {0, 174, 255}, // 12: 水色
  {132, 117, 156}, // 13: 藍色
  {255, 105, 173}, // 14: ピンク
  {255, 203, 165}  // 15: 桃色
};

LGFX screen;//LGFXを継承

LovyanGFX_DentaroUI ui(&screen);
LGFX_Sprite tft(&screen);
// LGFX_Sprite scaler(&screen);

// #include "MapDictionary.h"
// MapDictionary& dict = MapDictionary::getInstance();

LGFX_Sprite sprite88_roi = LGFX_Sprite(&tft);
LGFX_Sprite sprite11_roi = LGFX_Sprite(&tft);
LGFX_Sprite sprite64 = LGFX_Sprite();
// uint8_t sprite64cnos[4096];//64*64

// uint8_t sprite64cnos[PNG_SPRITE_HEIGHT * PNG_SPRITE_WIDTH];//64*128
std::vector<uint8_t> sprite64cnos_vector;

LGFX_Sprite buffSprite = LGFX_Sprite(&tft);
LGFX_Sprite sprite88_0 = LGFX_Sprite(&tft);

// LGFX_Sprite mapTileSprites[1];
// static LGFX_Sprite sliderSprite( &tft );//スライダ用

BaseGame* game;
// Tunes tunes;
String appfileName = "";//最初に実行されるアプリ名
String savedAppfileName = "";
// String txtName = "/init/txt/sample.txt";//実行されるファイル名

uint8_t mapsx = 0;
uint8_t mapsy = 0;
String mapFileName = "/init/map/0.png";
int readmapno = 0;
int divnum = 1;
bool readMapF = false;
//divnumが大きいほど少ない領域で展開できる(2の乗数)
// LGFX_Sprite spritebg[16];//16種類のスプライトを背景で使えるようにする
LGFX_Sprite spriteMap;//地図用スプライト

// uint8_t mapArray[MAPWH][MAPWH];
// uint8_t mapArray[MAPH][MAPW];
uint8_t mapArray[16][20];
bool mapready = false;

int8_t sprbits[128];//fgetでアクセスするスプライト属性を格納するための配列

char buf[MAX_CHAR];
// char str[100];//情報表示用
int mode = 0;//記号モード //0はrun 1はexit
// int gameState = 0;
String appNameStr = "init";

float sliderval[2] = {0,0};
bool optionuiflag = false;
// uint64_t frame = 0;
// float radone = PI/180;

// float sinValues[90];// 0から89度までの91個の要素

int addUiNum[4];
int allAddUiNum = 0;

// bool downloadF = true;
// bool isCardMounted = false; // SDカードがマウントされているかのフラグ

int xtile = 0;
int ytile = 0;
float ztile = 0.0;

int xtileNo = 29100;
int ytileNo = 12909;

LGFX_Sprite sprref;
String oldKeys[BUF_PNG_NUM];

int vol_value; //analog値を代入する変数を定義
int statebtn_value; //analog値を代入する変数を定義
int jsx_value; //analog値を代入する変数を定義
int jsy_value; //analog値を代入する変数を定義
// getSign関数をMapDictionaryクラス外に移動
Vector2<int> getSign(int dirno) {
    if (dirno == -1) {
        // 方向を持たない場合、(0.0, 0.0, 0.0)を返す
        return {0, 0};
    } else {
        float dx = (dirno == 0 || dirno == 1 || dirno == 7) ? 1.0 : ((dirno == 3 || dirno == 4 || dirno == 5) ? -1.0 : 0.0);
        float dy = (dirno == 1 || dirno == 2 || dirno == 3) ? 1.0 : ((dirno == 5 || dirno == 6 || dirno == 7) ? -1.0 : 0.0);
        return {int(dx), int(dy)};
    }
}

uint16_t gethaco3Col(uint8_t haco3ColNo) {
    uint16_t result = ((static_cast<uint16_t>(clist2[haco3ColNo][0]) >> 3) << 11) |
                      ((static_cast<uint16_t>(clist2[haco3ColNo][1]) >> 2) << 5) |
                       (static_cast<uint16_t>(clist2[haco3ColNo][2]) >> 3);
    return result;
}

vector<string> split(string& input, char delimiter)
{
    istringstream stream(input);
    string field;
    vector<string> result;
    while (getline(stream, field, delimiter)) {
        result.push_back(field);
    }
    return result;
}

// 送信コールバック
// void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // char macStr[18];
  // snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
  //          mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // tft.print("Last Packet Sent to: ");
  // tft.println(macStr);
  // tft.print("Last Packet Send Status: ");
  // tft.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
// }

// 受信コールバック
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  char msg[1];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  // tft.printf("Last Packet Recv from: %s\n", macStr);//MACアドレスを表示させる
  tft.printf("Last Packet Recv Data(%d): ", data_len);
  for ( int i = 0 ; i < data_len ; i++ ) {
    msg[1] = data[i];
    tft.print(msg[1]);
  }
  tft.println("");
}

Vector2<int> getKey2Sign(String _currentKey, String _targetKey) {
    int slashPos = _currentKey.indexOf('/'); // '/'の位置を取得
    if (slashPos != -1) { // '/'が見つかった場合
        String numA_str = _currentKey.substring(0, slashPos); // '/'より前の部分を取得
        String numB_str = _currentKey.substring(slashPos + 1); // '/'より後の部分を取得
        int numA_current = numA_str.toInt(); // 数字に変換
        int numB_current = numB_str.toInt(); // 数字に変換
        
        slashPos = _targetKey.indexOf('/'); // '/'の位置を取得
        if (slashPos != -1) { // '/'が見つかった場合
            numA_str = _targetKey.substring(0, slashPos); // '/'より前の部分を取得
            numB_str = _targetKey.substring(slashPos + 1); // '/'より後の部分を取得
            int numA_target = numA_str.toInt(); // 数字に変換
            int numB_target = numB_str.toInt(); // 数字に変換
            
            int dx = numA_target - numA_current;
            int dy = numB_target - numB_current;
            
            return {dx, dy};
        }
    }
    
    return {0, 0}; // デフォルトの値
}

Vector3<float> currentgpos = {0,0,0};;
Vector3<float> prePos= {0.0, 0.0, 0.0};
Vector3<float> currentPos = {0,0,0};
Vector3<float> diffPos = {0.0,0.0,0.0};

int dirNos[9];
int shouldNo = 0;
int downloadLimitNum = 0;
String targetKey = "";
float tileZoom = 15.0;
float bairitu = 1.0;

std::vector<String> temporaryKeys;
std::vector<String> previousKeys;
std::vector<String> writableKeys;
std::vector<String> downloadKeys;
std::vector<String> predownloadKeys;
std::vector<String> allKeys;
std::vector<String> preallKeys;

void printDownloadKeys() {
  //Serial.println("Download Keys:");
  for (const auto& key : downloadKeys) {
      //Serial.print(key);
  }
  // Serial.println("");
}

void reboot()
{
  ESP.restart();
}

FileType detectFileType(String *appfileName)
{
  if(appfileName->endsWith(".js")){
    return FileType::JS;
  }else if(appfileName->endsWith(".lua")){
    return FileType::LUA;
  }else if(appfileName->endsWith(".bmp")){
    return FileType::BMP;
  }else if(appfileName->endsWith(".png")){
    return FileType::PNG;
  }else if(appfileName->endsWith(".txt")){
    return FileType::TXT;
  }
  return FileType::OTHER;
}

String *targetfileName;
BaseGame* nextGameObject(String* _appfileName, int _gameState, String _mn)
{

  switch(detectFileType(_appfileName)){
    case FileType::JS:  
      // game = new RunJsGame(); 
      break;
    case FileType::LUA: 
      game = new runLuaGame(_gameState, _mn);
      break;
    case FileType::TXT: 
      // game = new RunJsGame(); 
      // //ファイル名がもし/init/param/caldata.txtなら
      // if(*_appfileName == CALIBRATION_FILE)
      // {
      //   ui.calibrationRun(screen);//キャリブレーション実行してcaldata.txtファイルを更新して
      //   drawLogo();//サイドボタンを書き直して
      // }
      // appfileName = "/init/txt/main.js";//txtエディタで開く
      break; //txteditorを立ち上げてtxtを開く
    case FileType::BMP: // todo: error
      game = NULL;
      break;
    case FileType::PNG: // todo: error
      // game = new RunJsGame(); 
      // appfileName = "/init/png/main.js";//pngエディタで開く
      break;
    case FileType::OTHER: // todo: error
      game = NULL;
      break;
  }

  return game;

}

// char *A;

uint32_t preTime;
// void setFileName(String s){
//   appfileName = s;
// }

void runFileName(String s){
  
  //ui.setConstantGetF(false);//初期化処理 タッチポイントの常時取得を切る
  
  appfileName = s;
  mode = 1;//exit to run

}


int getcno2tftc(uint8_t _cno){
  switch (_cno)
  {
  case 0:return HACO3_C0;break;
  case 1:return HACO3_C1;break;
  case 2:return HACO3_C2;break;
  case 3:return HACO3_C3;break;
  case 4:return HACO3_C4;break;
  case 5:return HACO3_C5;break;
  case 6:return HACO3_C6;break;
  case 7:return HACO3_C7;break;
  case 8:return HACO3_C8;break;
  case 9:return HACO3_C9;break;
  case 10:return HACO3_C10;break;
  case 11:return HACO3_C11;break;
  case 12:return HACO3_C12;break;
  case 13:return HACO3_C13;break;
  case 14:return HACO3_C14;break;
  case 15:return HACO3_C15;break;

  default:
  return HACO3_C0;
    break;
  }
}


// タイマー
hw_timer_t * timer = NULL;

void readFile(fs::FS &fs, const char * path) {
   File file = fs.open(path);
   while(file.available()) file.read();
  //  while(file.available()) Serial.print(file.read());
}

//ファイル書き込み
void writeFile(fs::FS &fs, const char * path, const char * message){
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        return;
    }
    file.print(message);
}

void deleteFile(fs::FS &fs, const char * path){
   //Serial.print("Deleting file: ");
   //Serial.println(path);
  //  if(fs.remove(path)) {Serial.print("− file deleted\n\r");}
  //  else {Serial.print("− delete failed\n\r"); }
}

void listDir(fs::FS &fs){
   File root = fs.open("/");
   File file = root.openNextFile();
   while(file){
      //Serial.print("  FILE: ");
      //Serial.print(file.name());
      //Serial.print("\tSIZE: ");
      //Serial.print(file.size());
      file = root.openNextFile();
   }
}

String rFirstAppName(String _wrfile){
  File fr = SPIFFS.open(_wrfile.c_str(), "r");// ⑩ファイルを読み込みモードで開く
  String _readStr = fr.readStringUntil('\n');// ⑪改行まで１行読み出し
  fr.close();	// ⑫	ファイルを閉じる
  return _readStr;
}



void getOpenConfig()
{
  File fr;

  fr = SPIFFS.open(SPRBITS_FILE, "r");
  for (int i = 0; i < 128; i++) {
    String _readStr = fr.readStringUntil(','); // ,まで１つ読み出し
    std::string _readstr = _readStr.c_str();

    // 改行を取り除く処理
    const char CR = '\r';
    const char LF = '\n';
    std::string destStr;
    for (std::string::const_iterator it = _readstr.begin(); it != _readstr.end(); ++it) {
      if (*it != CR && *it != LF && *it != '\0') {
        destStr += *it;
      }
    }

    _readstr = destStr;

    uint8_t bdata = 0b00000000;
    uint8_t bitfilter = 0b10000000; // 書き換え対象ビット指定用

    for (int j = 0; j < _readstr.length(); ++j) {
        char ch = _readstr[j];
        // Serial.print(ch);
        if (ch == '1') {
            bdata |= bitfilter; // 状態を重ね合わせて合成
        }
        bitfilter = bitfilter >> 1; // 書き換え対象ビットを一つずらす
    }

    sprbits[i] = bdata;

  }
  fr.close();

  fr = SPIFFS.open("/init/param/openconfig.txt", "r");
  String line;
  while (fr.available()) {
    line = fr.readStringUntil('\n');
    if (!line.isEmpty()) {
      int commaIndex = line.indexOf(',');
        String val = line.substring(0, commaIndex);
        if(val != NULL){
          appfileName =  val;

          // Serial.print(appfileName.c_str());
          // Serial.println("<-");

        }else {
          appfileName = "/init/main.lua";//configファイルが壊れていても強制的に値を入れて立ち上げる
        }
          int nextCommaIndex = line.indexOf(',', commaIndex + 1);//一つ先のカンマ区切りの値に進める
          if (nextCommaIndex != -1) {
            val = line.substring(commaIndex + 1, nextCommaIndex);
            if(val.toInt() != NULL){
              isEditMode = val.toInt();
              //Serial.print("editmode[");Serial.print(isEditMode);Serial.println("]");
            }else{
              isEditMode = 0;//configファイルが壊れていても強制的に値を入れて立ち上げる
            }
          }
    }
  }
  fr.close();

  //Serial.print(appfileName.c_str());
  //Serial.println("<---");

  string str1 = appfileName.c_str();
  int i=0;
  char delimiter = '/';

  std::vector<std::string> result = split(str1, delimiter);

    // 分割結果の表示
    for (const std::string& s : result) {
        if(i==1){
        appNameStr = s.c_str();

        // Serial.print("/" + appNameStr + "/mapinfo.txt");
        // Serial.println("<-------");

        fr = SPIFFS.open("/" + appNameStr + "/mapinfo.txt", "r");// ⑩ファイルを読み込みモードで開く
      }
      i++;
    }
      
  // アプリで使うマップ名を取得する
  String _readStr;
  while (fr.available()) {
      String line = fr.readStringUntil('\n');
      int j = 0; // 列のインデックス
      int startIndex = 0;

      if (!line.isEmpty()) {
          // Serial.print(line);
          // Serial.println("<--------");

          while (j < 16) {
              int commaIndex = line.indexOf(',', startIndex);

              // if (j < 16) { // 0から15番目まで
                  if (commaIndex != -1) {
                      String columnValue = line.substring(startIndex, commaIndex);
                      mapsprnos[j] = atoi(columnValue.c_str());
                  } else {
                      // 行の末尾まで達した場合
                      mapsprnos[j] = atoi(line.substring(startIndex).c_str());
                  }
              if (commaIndex == -1) {
                  // 行の末尾まで達した場合
                  break;
              }

              startIndex = commaIndex + 1;
              j++;
          }

          i++;
      }
  }
  fr.close();
  mapFileName = "/init/param/map/"+_readStr;
}

void setOpenConfig(String fileName, uint8_t _isEditMode) {
  char numStr[64];//64文字まで
  sprintf(numStr, "%s,%d,", 
    fileName.c_str(), _isEditMode
  );

  String writeStr = numStr;  // 書き込み文字列を設定
  File fw = SPIFFS.open("/init/param/openconfig.txt", "w"); // ファイルを書き込みモードで開く
  fw.println(writeStr);  // ファイルに書き込み
  // savedAppfileName = fileName;
  delay(50);
  fw.close(); // ファイルを閉じる
}

int readMap()
{
  mapready = false;

  for(int n = 0; n<divnum; n++)
  {
    spriteMap.drawPngFile( SPIFFS, mapFileName, 0, (int32_t)(-MAPWH*n/divnum) );
    for(int j = 0; j<MAPWH/divnum; j++){
      for(int i = 0; i<MAPWH; i++){

        int k = j+(MAPWH/divnum)*(n);//マップ下部
        colValR = spriteMap.readPixelRGB(i,j).R8();
        colValG = spriteMap.readPixelRGB(i,j).G8();
        colValB = spriteMap.readPixelRGB(i,j).B8();

  //16ビットRGB（24ビットRGB）
        if(colValR==0&&colValG==0&&colValB==0){//0: 黒色=なし
          mapArray[i][k] = mapsprnos[0];//20;
        }else if(colValR==24&&colValG==40&&colValB==82){//{ 27,42,86 },//1: 暗い青色
          mapArray[i][k] = mapsprnos[1];//11;//5*8+5;
        }else if(colValR==140&&colValG==24&&colValB==82){//{ 137,24,84 },//2: 暗い紫色
          mapArray[i][k] = mapsprnos[2];//32;//5*8+5;
        }else if(colValR==0&&colValG==138&&colValB==74){//{ 0,139,75 },//3: 暗い緑色
          mapArray[i][k] = mapsprnos[3];//44;//5*8+5;
        }else if(colValR==181&&colValG==77&&colValB==41){//{ 183,76,45 },//4: 茶色 
          mapArray[i][k] = mapsprnos[4];//53;//5*8+5;
        }else if(colValR==99&&colValG==85&&colValB==74){//{ 97,87,78 },//5: 暗い灰色
          mapArray[i][k] = mapsprnos[5];//49;
        }else if(colValR==198&&colValG==195&&colValB==198){//{ 194,195,199 },//6: 明るい灰色
          mapArray[i][k] = mapsprnos[6];//54;//5*8+5;
        }else if(colValR==255&&colValG==243&&colValB==231){//{ 255,241,231 },//7: 白色
          mapArray[i][k] = mapsprnos[7];//32;
        }else if(colValR==255&&colValG==0&&colValB==66){//{ 255,0,70 },//8: 赤色
          mapArray[i][k] = mapsprnos[8];//52;
        }else if(colValR==255&&colValG==162&&colValB==0){//{ 255,160,0 },//9: オレンジ
          mapArray[i][k] = mapsprnos[9];//41;//5*8+5;
        }else if(colValR==255&&colValG==239&&colValB==0){//{ 255,238,0 },//10: 黄色
          mapArray[i][k] = mapsprnos[10];//46;
        }else if(colValR==0&&colValG==235&&colValB==0){//{ 0,234,0 },//11: 緑色
          mapArray[i][k] = mapsprnos[11];//42;
        }else if(colValR==0&&colValG==174&&colValB==255){//{ 0,173,255 },//12: 水色
          mapArray[i][k] = mapsprnos[12];//45;//5*8+5;
        }else if(colValR==132&&colValG==117&&colValB==156){//{ 134,116,159 },//13: 藍色
          mapArray[i][k] = mapsprnos[13];//50;
        }else if(colValR==255&&colValG==105&&colValB==173){//{ 255,107,169 },//14: ピンク
          mapArray[i][k] = mapsprnos[14];//43;//5*8+5;
        }else if(colValR==255&&colValG==203&&colValB==165){//{ 255,202,165}//15: 桃色
          mapArray[i][k] = mapsprnos[15];//38;//5*8+5;
        }
        if(i==MAPWH-1 && k==MAPWH-1){mapready = true;return 1;}//読み込み終わったら1をリターン
      }
    }
  }
  // spriteMap.deleteSprite();//メモリに格納したら解放する
  return 1;
}

using namespace std;
#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <sstream>
#include <cmath>

void reboot(String _fileName, uint8_t _isEditMode)
{
  setOpenConfig(_fileName, _isEditMode);
  editor.setCursorConfig();//カーソルの位置を保存
  delay(100);
  ESP.restart();
}

void restart(String _fileName, int _isEditMode)
{
  setOpenConfig(_fileName, _isEditMode);
  
  editor.setCursorConfig();//カーソルの位置を保存
  delay(100);

  firstBootF = false;
  setup();

  // tunes.pause();
  game->pause();
  free(game);
  firstLoopF = true;
  toneflag = false;
  sfxflag = false;
  musicflag = false;
  // txtName = _fileName;
  game = nextGameObject(&_fileName, gameState, mapFileName);//ファイルの種類を判別して適したゲームオブジェクトを生成
  game->init();//resume()（再開処理）を呼び出し、ゲームで利用する関数などを準備
  // tunes.resume();
}


// void broadchat() {
//   if ("/init/chat/m.txt" == NULL) return;
//   File fp = SPIFFS.open("/init/chat/m.txt", FILE_READ); // SPIFFSからファイルを読み込み

//   if (!fp) {
//     editor.editorSetStatusMessage("Failed to open file");
//     return;
//   }

//   std::vector<uint8_t> data;
//   while (fp.available()) {
//     char c = fp.read();
//     data.push_back(c);
//     if (data.size() >= 150) {
//       esp_err_t result = esp_now_send(slave.peer_addr, data.data(), data.size());
//       tft.println(result);
//       data.clear(); // データを送信したらクリア
//       if (result != ESP_OK) {
//         editor.editorSetStatusMessage("Failed to send message");
//         fp.close();
//         return;
//       }
//     }
//   }

//   // ファイルの残りのデータを送信
//   if (data.size() > 0) {
//     esp_err_t result = esp_now_send(slave.peer_addr, data.data(), data.size());
//     if (result != ESP_OK) {
//       editor.editorSetStatusMessage("Failed to send message");
//       fp.close();
//       return;
//     }
//   }

//   fp.close();
//   editor.editorSetStatusMessage("Message sent");
// }


uint8_t readpixel(int i, int j)
{
        // int k = j+(MAPWH/divnum)*(n);//マップ下部
        colValR = sprite64.readPixelRGB(i,j).R8();
        colValG = sprite64.readPixelRGB(i,j).G8();
        colValB = sprite64.readPixelRGB(i,j).B8();

  //16ビットRGB（24ビットRGB）
        if(colValR==0&&colValG==0&&colValB==0){//0: 黒色=なし
          return 0;//20;
        }else if(colValR==24&&colValG==40&&colValB==82){//{ 27,42,86 },//1: 暗い青色
          return 1;//11;//5*8+5;
        }else if(colValR==140&&colValG==24&&colValB==82){//{ 137,24,84 },//2: 暗い紫色
          return 2;//32;//5*8+5;
        }else if(colValR==0&&colValG==138&&colValB==74){//{ 0,139,75 },//3: 暗い緑色
          return 3;//44;//5*8+5;
        }else if(colValR==181&&colValG==77&&colValB==41){//{ 183,76,45 },//4: 茶色 
          return 4;//53;//5*8+5;
        }else if(colValR==99&&colValG==85&&colValB==74){//{ 97,87,78 },//5: 暗い灰色
          return 5;//49;
        }else if(colValR==198&&colValG==195&&colValB==198){//{ 194,195,199 },//6: 明るい灰色
          return 6;//54;//5*8+5;
        }else if(colValR==255&&colValG==243&&colValB==231){//{ 255,241,231 },//7: 白色
          return 7;//32;
        }else if(colValR==255&&colValG==0&&colValB==66){//{ 255,0,70 },//8: 赤色
          return 8;//52;
        }else if(colValR==255&&colValG==162&&colValB==0){//{ 255,160,0 },//9: オレンジ
          return 9;//41;//5*8+5;
        }else if(colValR==255&&colValG==239&&colValB==0){//{ 255,238,0 },//10: 黄色
          return 10;//46;
        }else if(colValR==0&&colValG==235&&colValB==0){//{ 0,234,0 },//11: 緑色
          return 11;//42;
        }else if(colValR==0&&colValG==174&&colValB==255){//{ 0,173,255 },//12: 水色
          return 12;//45;//5*8+5;
        }else if(colValR==132&&colValG==117&&colValB==156){//{ 134,116,159 },//13: 藍色
          return 13;//50;
        }else if(colValR==255&&colValG==105&&colValB==173){//{ 255,107,169 },//14: ピンク
          return 14;//43;//5*8+5;
        }else if(colValR==255&&colValG==203&&colValB==165){//{ 255,202,165}//15: 桃色
          return 15;//38;//5*8+5;
        }
}


static void tone_up(bool holding)
{
  // static int tone_hz;
  // if (!holding) { tone_hz = 100; }
  // speaker->tone(++tone_hz, 1000, 1);
}

static void bgm_play_stop(bool holding = false)
{
  // if (holding) { return; }
  // if (speaker->isPlaying(0))
  // {
  //   speaker->stop(0);
  // }
  // else
  // {
  //   speaker->playWav(wav_with_header, sizeof(wav_with_header), ~0u, 0, true);
  // }
}

static void m_volume_up(bool)
{
  // int v = speaker->getVolume() + 1;
  // if (v < 256) { speaker->setVolume(v); }
}

static void m_volume_down(bool)
{
  // int v = speaker->getVolume() - 1;
  // if (v >= 0) { speaker->setVolume(v); }
}

static void c_volume_up(bool)
{
  // int v = speaker->getChannelVolume(0) + 1;
  // if (v < 256) { speaker->setChannelVolume(0, v); }
}

static void c_volume_down(bool)
{
  // int v = speaker->getChannelVolume(0) - 1;
  // if (v >= 0) { speaker->setChannelVolume(0, v); }
}

struct menu_item_t
{
  const char* title;
  void (*func)(bool);
};

// static const menu_item_t menus[] =
// {
//   { "tone"      , tone_up       },
//   { "play/stop" , bgm_play_stop },
//   { "ms vol u"  , m_volume_up   },
//   { "ms vol d"  , m_volume_down },
//   { "ch vol u"  , c_volume_up   },
//   { "ch vol d"  , c_volume_down },
// };
// const uint8_t menu_count = sizeof(menus) / sizeof(menus[0]);

void speakersetup(){

  
  // setToneChannel(0);
  // tone(SPEAKER_PIN,2000,100);
  // noTone(SPEAKER_PIN);
  
  // tone(SPEAKER_PIN,1000,100);
  // noTone(SPEAKER_PIN);

  // begin(cfg);
  // { /// I2S Custom configurations are available if you desire.
  //   auto spk_cfg = speaker->config();

  //   if (spk_cfg.use_dac || spk_cfg.buzzer)
  //   {
  //   /// Increasing the sample_rate will improve the sound quality instead of increasing the CPU load.
  //     spk_cfg.sample_rate = 64000; // default:64000 (64kHz)  e.g. 48000 , 50000 , 80000 , 96000 , 100000 , 128000 , 144000 , 192000 , 200000
  //   }

  //   speaker->config(spk_cfg);
  // }
  

  // speaker->begin();

  // //  The setVolume function can be set the master volume in the range of 0-255. (default : 64)
  // speaker->setVolume(255);

  // /// The setAllChannelVolume function can be set the all virtual channel volume in the range of 0-255. (default : 255)
  // speaker->setAllChannelVolume(255);

  // /// The setChannelVolume function can be set the specified virtual channel volume in the range of 0-255. (default : 255)
  // speaker->setChannelVolume(0, 255);

  // /// play do Hz tone sound, 100 msec. 
  // speaker->tone(2000, 100,1);

  // delay(100);

  // /// play mi Hz tone sound, 100 msec. 
  // speaker->tone(1000, 100,2);

  // delay(100);

  
  // // /// stop output sound.
  // speaker->stop();

  // delay(500);

  // // speaker->playRaw( wav_unsigned_8bit_click, sizeof(wav_unsigned_8bit_click) / sizeof(wav_unsigned_8bit_click[0]), 44100, false);

  // while (speaker->isPlaying()) { delay(1); } // Wait for the output to finish.

  // delay(500);

  // The 2nd argument of the tone function can be used to specify the output time (milliseconds).
  // speaker->tone(440, 1000);  // 440Hz sound  output for 1 seconds.

  // while (speaker->isPlaying()) { delay(1); } // Wait for the output to finish.

  // delay(500);

  // speaker->setVolume(0);
  // speaker->tone(880);  // tone 880Hz sound output. (Keeps output until it stops.)
  // for (int i = 0; i <= 151; i++)
  // {
  //   speaker->setVolume(i); // Volume can be changed during sound output.
  //   delay(25);
  // }
  // speaker->stop();  // stop sound output.

  // delay(500);

  // //---------------------------------------------
  

  // The tone function can specify a virtual channel number as its 3rd argument.
  // If the tone function is used on the same channel number, the previous tone will be stopped and a new tone will be played.
  // speaker->tone(261.626, 1000, 1);  // tone 261.626Hz  output for 1 seconds, use channel 1
  // delay(200);
  // speaker->tone(329.628, 1000, 1);  // tone 329.628Hz  output for 1 seconds, use channel 1
  // delay(200);
  // speaker->tone(391.995, 1000, 1);  // tone 391.995Hz  output for 1 seconds, use channel 1

  // while (speaker->isPlaying()) { delay(1); } // Wait for the output to finish.

  // delay(500);

  // // By specifying different channels, multiple sounds can be output simultaneously.
  // speaker->tone(261.626, 1000, 1);  // tone 261.626Hz  output for 1 seconds, use channel 1
  // delay(200);
  // speaker->tone(329.628, 1000, 2);  // tone 329.628Hz  output for 1 seconds, use channel 2
  // delay(200);
  // speaker->tone(391.995, 1000, 3);  // tone 391.995Hz  output for 1 seconds, use channel 3

  // while (speaker->isPlaying()) { delay(1); } // Wait for the output to finish.

  // delay(500);
}
uint8_t cursor_index = 0;


void safeReboot(){
  editor.setCursorConfig(0,0,0);//カーソルの位置を強制リセット保存
      delay(50);

      //ui.setConstantGetF(false);//初期化処理 タッチポイントの常時取得を切る
      appfileName = "/init/main.lua";
      
      firstLoopF = true;
      toneflag = false;
      sfxflag = false;
      musicflag = false;

      editor.editorSave(SPIFFS);//SPIFFSに保存
      delay(100);//ちょっと待つ
      reboot(appfileName, TFT_RUN_MODE);//現状rebootしないと初期化が完全にできない
}




void ledSetup()
{
for(int n=0; n<LED_NUM; n++)
  {
    isSoftLED[n] = false;
  }
}

void ledUpdate(){
  for(int n=0; n<LED_NUM; n++)
  {
    if(isSoftLED[n]==true){
      screen.fillCircle(8, 8+11*n,4,TFT_RED);
      screen.fillCircle(6, 4+11*n,1,TFT_WHITE);
      // digitalWrite(OUTPIN_0, HIGH);
    }else{
      screen.fillCircle(8,8+11*n,4,TFT_DARKGRAY);
      // digitalWrite(OUTPIN_0, LOW);
    }
    screen.drawCircle(8,8+11*n,4,TFT_LIGHTGRAY);
  }

  digitalWrite(OUTPIN_0, isSoftLED[0]);//物理LED
}

TaskHandle_t taskHandle[3];
// SemaphoreHandle_t syncSemaphore;
QueueHandle_t keyQueue;
SemaphoreHandle_t nextFrameSemaphore;
size_t tick = 0;

size_t patternNo = 0;//0~63
size_t _octave = 4;// (124 - ui.getPos().y)>>2 / 12 + 4;
size_t _pitch = 0;
size_t _volume = 0;
size_t addTones[8];

bool createChannels()
{

String line;
int j = 0;

// パターンファイルを読み込む
File fr = SPIFFS.open("/init/sound/patterns.csv", "r");
if (!fr)
{
  //Serial.println("Failed to open patterns.csv");
  return true; // とりあえず進む
}

j = 0;
while (fr.available()) // 64行だけ読み込む
{
  line = fr.readStringUntil('\n'); // 64行文のパターン（小節）があります
  line.trim();                      // 空白を削除

  if (!line.isEmpty())
  {
    int commaIndex = line.indexOf(',');
    if (commaIndex != -1)
    {
      String val = line.substring(0, commaIndex);
      addTones[0] = val.toInt();

      for (int i = 1; i < 8; i++)
      {
        int nextCommaIndex = line.indexOf(',', commaIndex + 1);
        if (nextCommaIndex != -1)
        {
          val = line.substring(commaIndex + 1, nextCommaIndex);
          addTones[i] = val.toInt();
          commaIndex = nextCommaIndex;
        }
        else
        {
          // Handle the case where there is no trailing comma
          val = line.substring(commaIndex + 1);
          addTones[i] = val.toInt();
          break; // Exit the loop since we reached the end of the line
        }
      }

      for(size_t n=0; n<CHANNEL_NUM; n++){
        channels->setPatterns(j, n, addTones[n]);
      }
      
      j++;
    }
  }
}
fr.close();
  //すべてが終わったらtrueを返す
  return true;
}

bool readTones(size_t _patternNo, size_t buffAreaNo)
{
  String line;
  int j = 0;

  // トーンファイルを読み込む
  for (int chno = 0; chno < CHANNEL_NUM; chno++)
    {
    j = 0;
    patternID = channels->getPatternID( _patternNo, chno);
    File fr = SPIFFS.open("/init/sound/pattern/"+String(patternID)+".csv", "r");
    if (!fr)
    {
      //Serial.println("Failed to open tones.csv");
      return true;//とりあえず進む
    }
    while (fr.available())
    {
      line = fr.readStringUntil('\n');
      // line.trim(); // 空白を削除
      if (!line.isEmpty())
      {
        int commaIndex = line.indexOf(',');
        if (commaIndex != -1)
        {
          String val = line.substring(0, commaIndex);

          for (int i = 0; i < 8; i++)
          {
            int nextCommaIndex = line.indexOf(',', commaIndex + 1);
            if (nextCommaIndex != -1)
            {
              val = line.substring(commaIndex + 1, nextCommaIndex);
              addTones[i] = val.toInt();
              commaIndex = nextCommaIndex;
            }
          }

          channels->setTones(
              1,
              addTones[0], addTones[1],
              addTones[2], addTones[3],
              addTones[4], addTones[5],
              addTones[6], addTones[7], j, chno, buffAreaNo);
          j++;
        }
      }
    }
    fr.close();
  }

  //すべてが終わったらtrueを返す
  return true;
}

// uint8_t sfxlistNo = 0;
// uint8_t sfxnos[8] ={0,1,2,3,4,5,6,7};
void readsfxlist() {
  // 読み込む効果音を外部ファイルを使い指定する
  File fr = SPIFFS.open("/init/sound/sfxes/sfxlist"+String(sfxlistNo)+".txt", "r");
  String line;
  int index = 0; // インデックスを追加
  while (fr.available()) {
    line = fr.readStringUntil('\n');
    if (!line.isEmpty()) {
      int commaIndex = line.indexOf(',');
      String val = line.substring(0, commaIndex);
      if (val.toInt() != 0) { // 0 でないことを確認
        sfxnos[index] = val.toInt(); // インデックスを使用して sfxnos 配列に値を代入
        index++; // インデックスをインクリメント
        if (index >= 8) break; // sfxnos 配列がオーバーフローしないようにする
      }
    }
  }
  fr.close();
}

bool readsfx()//行32列9のデータ
{
  //読み込む効果音を外部ファイルを使い指定する
  readsfxlist();
  // トーンファイルを読み込む
  File fr;
  for (int sfxno = 0; sfxno < SFX_NUM; sfxno++)
    {
    uint8_t addTones[32];
    String line;
    int j = 0;

    fr = SPIFFS.open("/init/sound/sfxes/"+String(sfxnos[sfxno])+".csv", "r");
    if (!fr)
    {
      //Serial.println("Failed to open tones.csv");
      return true;//とりあえず進む
    }
    while (fr.available())
    {
      line = fr.readStringUntil('\n');
      // line.trim(); // 空白を削除
      if (!line.isEmpty())
      {
        int commaIndex = line.indexOf(',');
        if (commaIndex != -1)
        {
          String val = line.substring(0, commaIndex);//一個目はここで読み込む

          addTones[0] = val.toInt();
          if(j==0)channels->sfxdata[sfxno][0].onoffF = addTones[0];
          else if(j==1)channels->sfxdata[sfxno][0].instrument = addTones[0];//pitch
          else if(j==2)channels->sfxdata[sfxno][0].pitch = addTones[0];
          else if(j==3){
            channels->sfxdata[sfxno][0].octave = addTones[0];
            channels->sfxdata[sfxno][0].hz = channels->calculateFrequency(
              channels->sfxdata[sfxno][0].pitch, 
              channels->sfxdata[sfxno][0].octave);
          }
          else if(j==4)channels->sfxdata[sfxno][0].sfxno = addTones[0];
          else if(j==5)channels->sfxdata[sfxno][0].volume = addTones[0];

          for (int i = 1; i < 32; i++)
          {

            int nextCommaIndex = line.indexOf(',', commaIndex + 1);
            if (nextCommaIndex != -1)
            {
              val = line.substring(commaIndex + 1, nextCommaIndex);
              addTones[i] = val.toInt();
              commaIndex = nextCommaIndex;

              if(j==0)channels->sfxdata[sfxno][i].onoffF = addTones[i];
              else if(j==1)channels->sfxdata[sfxno][i].instrument = addTones[i];//pitch
              else if(j==2)channels->sfxdata[sfxno][i].pitch = addTones[i];
              else if(j==3){
                channels->sfxdata[sfxno][i].octave = addTones[i];
                channels->sfxdata[sfxno][i].hz = channels->calculateFrequency(
                  channels->sfxdata[sfxno][i].pitch, 
                  channels->sfxdata[sfxno][i].octave);
              }
              else if(j==4)channels->sfxdata[sfxno][i].sfxno = addTones[i];
              else if(j==5)channels->sfxdata[sfxno][i].volume = addTones[i];

            }
            
          }
          j++;
        }
      }
    }
  }
  fr.close();

  //すべてが終わったらtrueを返す
  return true;
}

// void createChannelsTask(void *pvParameters) {
//     while (true) {
//             while (!createChannels()) {
//                 delay(10);
//             }
//             readTones(patternNo, 0);
//             readTones(patternNo + 1, 1);
//             xSemaphoreGive(syncSemaphore);
//             delay(10);
//     }
// }


// void sfxTask(void *pvParameters) {
//   while (true) {
// if(sfxflag==true){
//       channels->begin();
//       channels->setVolume(masterVol); // 0-255

//       for(int n=0;n < 32;n++){
//         sfxTickNo = n;
//         channels->sfx(sfxChNo, sfxNo, wavNo, sfxVol, sfxspeed);
//       }
//       channels->stop();

//       if(sfxTickNo == 31){sfxflag=false;sfxTickNo=0;}
//     }
//         delay(1);
//   }
// }

// void musicTask(void *pvParameters) {
//     while (true) {
//           // 何らかの条件が満たされるまで待機
//         if (xSemaphoreTake(syncSemaphore, portMAX_DELAY)) {
//             // 同期が取れたらここに入る
//             channels->begin();
//             // channels->setVolume(200); // 0-255
//             // channels->setAllChannelVolume(127);
//             // channels->note(0, tick, patternNo);
//             // channels->note(1, tick, patternNo);
//             // channels->note(2, tick, patternNo);
//             // channels->note(3, tick, patternNo);

//             // channels->note(4, tick, patternNo);
//             // channels->note(5, tick, patternNo);
//             // channels->note(6, tick, patternNo);
//             // channels->note(7, tick, patternNo);
//             // channels->stop();
//             xSemaphoreGive(syncSemaphore);
//         }
            
//             tick++;
//             tick %= TONE_NUM;

//             if (tick == 0) {
//                 patternNo++;

//                 if (patternNo >= PATTERN_NUM) {
//                     patternNo = 0;
//                 }
//             }

//             // delay(1);
//         }

//         // 他の処理や適切な待機時間をここに追加
//         // delay(10);
// }
bool usbFlag = true;
char prekeychar;

void keyloop() {
    // char keychar;
    
    
    while (xQueueReceive(keyQueue, &keychar, 0) == pdTRUE) {

      if(prekeychar == keychar){
        keychar = NULL;
      }
      
        if (keychar == NULL) {
            pressedBtnID = -1;
        } else if (keychar == PS2_ENTER) {
            // Serial.println();
        } else if (keychar == PS2_TAB) {
            // pressedBtnID = 7;
        } else if (keychar == PS2_ESC) {
            pressedBtnID = 0;
        } else if (keychar == PS2_PAGEDOWN) {
            pressedBtnID = 6;
        } else if (keychar == PS2_PAGEUP) {
            pressedBtnID = 5;
        } else if (keychar == PS2_LEFTARROW) {
            pressedBtnID = 1;
        } else if (keychar == PS2_RIGHTARROW) {
            pressedBtnID = 2;
        } else if (keychar == PS2_UPARROW) {
            pressedBtnID = 3;
        } else if (keychar == PS2_DOWNARROW) {
            pressedBtnID = 4;
        } else if (keychar == PS2_DELETE) {
            editor.editorProcessKeypress(keychar, SPIFFS);
            Serial.println("[Del]");
        } else {
            // 通常の文字
            editor.editorProcessKeypress(keychar, SPIFFS);
        }
        // Serial.println(keychar);
    }

    
    
    prekeychar = keychar;

    
}

void keyMonitorTask(void *pvParameters) {
  // char keychar;
  while (true) 
  {
    keychar = keyboard.read(); // 直接キーキャラクタを読み取る
    if (keychar != -1) {
      
        xQueueSend(keyQueue, &keychar, portMAX_DELAY); // キー入力をキューに送信
      
    }

    // keyboard.OnKeyUp(0,53);

    // if (keyboard.released()) {
    //   Serial.println("keyreleased");
    // }

    
    delay(1); // 他のタスクの実行を許可
  }
}


void mouseMonitorTask(void *pvParameters) {

  // USB Host Shield 2.0の初期化
  // USBHost usb;
  
  Usb.Init();

  while (1) {
    // USBホストの処理
    // Usb.Task();
    
    // 適宜、他の処理を追加
    delay(1);  // 適切なディレイを追加
  }


  // while (true) 
  // {
  //   // Usb.Task();

  //   delay(1); // 他のタスクの実行を許可
  // }
}

// void keySetup(){

//   syncSemaphore = xSemaphoreCreateBinary();//セマフォを準備

//       xTaskCreatePinnedToCore(
//         keyTask,
//         "keyTask",
//         2048,////1024だと動く//1500だと非力だけど動く//2048だと動く
//         NULL,
//         1,
//         &taskHandle[0],//NULL,// タスクハンドルを取得
//         0 // タスクを実行するコア（0または1）
//       );
// }

void setup()
{
  Serial.begin(115200);

  keyQueue = xQueueCreate(10, sizeof(char)); // キューを作成
  // xTaskCreate(keyMonitorTask, "Key Monitor Task", 2048, NULL, 1, NULL); // キーボード監視タスクを作成
      xTaskCreatePinnedToCore(
        keyMonitorTask,
        "keyMonitorTask",
        2048,//2048だと動く
        NULL,
        1,
        &taskHandle[0],//NULL,// タスクハンドルを取得
        0 // タスクを実行するコア（0または1）
      );


  // xTaskCreate(keyMonitorTask, "Key Monitor Task", 2048, NULL, 1, NULL); // キーボード監視タスクを作成
      xTaskCreatePinnedToCore(
        mouseMonitorTask,
        "mouseMonitorTask",
        2048,//だと動く
        NULL,
        2,
        &taskHandle[1],//NULL,// タスクハンドルを取得
        0 // タスクを実行するコア（0または1）
      );

  pinMode(OUTPIN_0, OUTPUT);
  pinMode(INPIN_0, INPUT);
  ledSetup();

  ui.begin( screen, 16, 1);
  // Serial.begin(115200);
  mouseSetup();
  // keySetup();

  // keyboard.begin(KEYBOARD_DATA, KEYBOARD_CLK);

  editor.getCursorConfig("/init/param/editor.txt");//エディタカーソルの位置をよみこむ

  delay(50);

  if(firstBootF == true){
    difffileF = false;

    #if !defined(__MIPSEL__)
      while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
      #endif
      // Serial.println("Keyboard Start");

    if (!SPIFFS.begin(true))
    {
      // Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  }

  // usbSetup();
  
  // soundSetup();
  // speakersetup();

  getOpenConfig();//最初に立ち上げるゲームのパスとモードをSPIFFSのファイルopenconfig.txtから読み込む

  // if(firstBootF == false){
  //   tft.deleteSprite();
  //   delay(100);
  // }

  // setTFTedit(TFT_RUN_MODE);
  tft.setPsram( false );//DMA利用のためPSRAMは切る
  // tft.setColorDepth(16);//子スプライトの色深度
  tft.createSprite( TFT_WIDTH, TFT_HEIGHT );//PSRAMを使わないギリギリ
  tft.startWrite();//CSアサート開始

  sprite88_0.setPsram(false );
  sprite88_0.setColorDepth(16);//子スプライトの色深度
  sprite88_0.createSprite(8, 8);//ゲーム画面用スプライトメモリ確保

  //sprite88_0.drawPngFile(SPIFFS, "/init/sprite.png", -8*1, -8*0);

  sprite64.setPsram(false );
  sprite64.setColorDepth(16);//子スプライトの色深度
  sprite64.createSprite(PNG_SPRITE_WIDTH, PNG_SPRITE_HEIGHT);//ゲーム画面用スプライトメモリ確保//wroomだと64*128だとメモリオーバーしちゃう問題を色番号配列にして回避した

  sprite64.drawPngFile(SPIFFS, "/init/initspr.png", 0, 0);//一時展開する

  sprite64cnos_vector.clear();//初期化処理

  for(int y = 0; y < PNG_SPRITE_HEIGHT; y++) {
      for(int x = 0; x < PNG_SPRITE_WIDTH; x++) {
        if(x%2 == 0){
          uint8_t pixel_data = (readpixel(x, y) << 4) | (readpixel(x + 1, y) & 0b00001111);
          sprite64cnos_vector.push_back(pixel_data);
        }
      }
  }

  //破棄
  sprite64.deleteSprite();

  //psram使えない-------------------------------------------

  sprite88_roi.setPsram(false );
  sprite88_roi.setColorDepth(16);//子スプライトの色深度
  sprite88_roi.createSprite(8, 8);//ゲーム画面用スプライトメモリ確保

  sprite11_roi.setPsram(false );
  sprite11_roi.setColorDepth(16);//子スプライトの色深度
  sprite11_roi.createSprite(1, 1);//ゲーム画面用スプライトメモリ確保

  spriteMap.setPsram(false );
  spriteMap.setColorDepth(16);//子スプライトの色深度
  spriteMap.createSprite(MAPW, MAPH/divnum);//マップ展開用スプライトメモリ確保

  // if(firstBootF == true)
  // {
  // createAbsUI();
  // appfileName = "/min/main.lua";
  // isEditMode = 0;
  // mapFileName = "/init/map/0.png";
  // readMap();
  // delay(50);
    
  game = nextGameObject(&appfileName, gameState, mapFileName);//ホームゲームを立ち上げる（オブジェクト生成している）
  game->init();//（オブジェクト生成している）
  // tunes.init();//（オブジェクト生成している）
  // }

  frame=0;

  // editor.initEditor(tft, EDITOR_ROWS, EDITOR_COLS);
  editor.initEditor(tft);
  editor.readFile(SPIFFS, appfileName.c_str());
  editor.editorOpen(SPIFFS, appfileName.c_str());
  editor.editorSetStatusMessage("Press ESCAPE to save file");

  savedAppfileName = appfileName;//起動したゲームのパスを取得しておく
  firstBootF = false;


  //キー取得
  //luaプログラムがバグで起動不能になった場合、ESCを押しながらリセットをかけると、メニューに戻れるようにする
  //ESCボタンで強制終了

  if (keyboard.available()) {
    keychar = keyboard.read();
    if (keychar == PS2_ESC) {
      safeReboot();
    }
  }

  // Usb.Task();

  // ウォッチドッグ停止
  disableCore0WDT();
  disableCore1WDT();  // 起動直後は有効化されていないのでエラーがでる

  // ウォッチドッグ起動
  enableCore0WDT();
  enableCore1WDT();
  
}

const long usbTaskInterval = 10; // 10ms interval for Usb.Task
unsigned long taskElapsedTime;
unsigned long taskRemainTime;
unsigned long taskPreviousTime = 0;
bool flipF = true;

void loop()
{

  // if (keyboard.released()) {
    // Serial.println(keyboard.released());
  // }
  
  // 現在の時間を取得する
  
  currentTime = millis();

  // 前フレーム処理後からの経過時間を計算する
  elapsedTime = currentTime - startTime;
  // 前フレームからの経過時間を計算する
  remainTime = (currentTime - preTime);
  preTime = currentTime;
//----------------------------
  //ボタンが押されているときだけtickがカウントされる
  int firstwaitms = 1000;
    
    // if(btnpms == 0){
    //   btnpF = true; 
    //   // btnptick++;
    //   // textMoveF=true;
    //   // if(btnpms==0){btnpF = true; btnptick++;}//最初の0だけtrue
    //   // else{btnpF = false;pressedBtnID=-1;}

    // }else{

      if(btnpms <= 150)
      {
        btnpF = true;
        textMoveF=true;
      }else{

        if(btnpms%300 >= 150)
        {
          // Serial.println("定期的にtrue");
          btnpF = false;
          
        }else{
          btnpF = true;
        }

        if(btnpF!= prebtnpF)btnptick++;

        if (btnptick!=prebtnptick) {
          
          if(btnptick<=1||btnptick>=5)
          textMoveF=true;
          else
          textMoveF=false;

        }else{
          textMoveF=false;
        }
      }

    btnpms += elapsedTime;
    prebtnpF = btnpF;
    prebtnptick = btnptick;

  //ボタンが押されているときだけtickがカウントされる
  //btnpms//    0123456...
  //btnpF//     |||||___|||||____|||||____|||||____ //一定時間ずつフラグを立てる
  //textMoveF// |____________|___|____|___|____|    //差があった時にtrueになる最初firstwaitms分はフラグたてない

  if(textMoveF)//どのモードでもbtnpに反応する
  {
    ledUpdate();
  }

  if(pressedBtnID != -1){
    editor.update(tft, SPIFFS, keychar);
  }
  // if(pressedBtnID == -1){
  //   buttonState[pressedBtnID] = -1;
  // }else if(pressedBtnID>=0&&pressedBtnID<=6){//押されたものだけの値をあげる
  //   buttonState[pressedBtnID]++;
  // }
  
  // buttonState[pressedBtnID]++;

  // // // for(int i = 0; i < buttonState.size(); i ++){
  for(int i = 0; i < 7; i ++){
    if(pressedBtnID == -1){
      buttonState[i] = 0;
      
    }else if(pressedBtnID == i){//押されたものだけの値をあげる
      buttonState[i] ++;
    }
  }

      // 経過時間が1/30秒以上経過した場合
if (elapsedTime >= 1000/fps||fps==-1) {

  // if(pressedBtnID != -1){
    keyloop();
  // }

  if( isEditMode == TFT_RUN_MODE ){
    //ゲーム内のprint時の文字設定をしておく
    tft.setTextSize(1);//サイズ
    tft.setFont(&lgfxJapanGothicP_8);//日本語可
    tft.setCursor(0, 0);//位置
    tft.setTextWrap(true);

    // == tune task ==
    // tunes.run();

    // == game task ==

    mode = game->run(remainTime);//exitは1が返ってくる　mode=１ 次のゲームを起動

// Serial.println(pressedBtnID);

    // if(useMouseF == true){//キーイベントがない時だけtrue
        Usb.Task();
    // }

    //ESCボタンで強制終了
    if (pressedBtnID == 0)
    { // reload

      // editor.setCursorConfig(0,0,0);//カーソルの位置を強制リセット保存
      // delay(50);

      appfileName = "/init/main.lua";
      
      firstLoopF = true;
      toneflag = false;
      sfxflag = false;
      musicflag = false;
      // waittime = 0;

      mode = 1;//exit
    }

    if (pressedBtnID == 9999)
    { // reload
      mode = 1;//exit
      pressedBtnID = -1;
    }

    if(mode != 0){ // exit request//次のゲームを立ち上げるフラグ値、「modeが１＝次のゲームを起動」であれば
      // tunes.pause();
      game->pause();
      // ui.clearAddBtns();//個別のゲーム内で追加したタッチボタンを消去する
      free(game);
      firstLoopF = true;
      toneflag = false;
      sfxflag = false;
      musicflag = false;
      fps = 60;
      bgmodeNo = 0;//bgなしモードにリセット
      ledSetup();//ledリセット
      // txtName = appfileName;
      game = nextGameObject(&appfileName, gameState, mapFileName);//ファイルの種類を判別して適したゲームオブジェクトを生成
      game->init();//resume()（再開処理）を呼び出し、ゲームで利用する関数などを準備
      // tunes.resume();
    }

    if(bgmodeNo==2)
    {
      //  tft.drawPngFile(SPIFFS, pngimgPath, pngimgX, pngimgY, 160,128,0,0, 160/pngimgW, 128/pngimgH);
      tft.drawPngFile(SPIFFS, pngimgPath, pngimgX, pngimgY,0,0, 0,0);//tftに直接展開する
      // tft.drawPngFile(SPIFFS, pngimgPath, pngimgX, pngimgY, 0,0,0,0, 160/float(pngimgW), 128/float(pngimgH));//tftに直接展開する
    }

    // ui.showTouchEventInfo( tft, 0, 100 );//タッチイベントを視覚化する
    ui.showInfo( tft, 0, 100+8 );//ボタン情報、フレームレート情報などを表示します。

    // if(enemyF){

    //   sprite64.setPsram(false);
    //   sprite64.setColorDepth(16);    // 子スプライトの色深度
    //   sprite64.createSprite(48, 48); // ゲーム画面用スプライトメモリ確保

    //   sprite64.drawPngFile(SPIFFS, enemyPath, enemyX, enemyY);//sprite64に展開する
    //   sprite64.pushRotateZoom(&tft, enemyX, enemyY, 0, 1, 1, gethaco3Col(enemyTransCn));

    //   sprite64.deleteSprite();//消す

    //   tft.drawPngFile(SPIFFS, enemyPath, enemyX, enemyY);//直接展開する
    // }

     //最終出力
    tft.setPivot(0, 0);
    tft.pushRotateZoom(&screen, 30, 0, 0, 1, 1);

    if(pressedBtnID == 5){//PAGEUP//キーボードからエディタ再起動
      restart(appfileName, 1);//appmodeでリスタートかけるので、いらないかも
    }
  }
  else if(isEditMode == TFT_EDIT_MODE)
  {
    editor.editorRefreshScreen(tft);

    float codeunit = 128.0/float(editor.getNumRows());
    float codelen = codeunit * 14;//14は表示行数
    // int codelen = int(codelen_f + 0.5); // 四捨五入して整数に変換する
    
    float curpos = codeunit*editor.getCy();
    float codepos = codeunit * (editor.getCy() - editor.getScreenRow());
    // int codepos = int(codepos_f + 0.5); // 四捨五入して整数に変換する
    
    tft.fillRect(156,0, 4,128, HACO3_C5);//コードの全体の長さを表示
    tft.fillRect(156,int(codepos), 4,codelen, HACO3_C6);//コードの位置と範囲を表示

    if(curpos>=int(codepos)+codelen)//すこしはみ出たら表示コード内に入れる
    {
      if(codeunit>=1)curpos = int(codepos)+codelen - codeunit;
      else curpos = int(codepos)+codelen - 1;
    }

    if(codeunit>=1){tft.fillRect(155, int(curpos), 4, codeunit, HACO3_C8);}//コードの位置と範囲を表示
    else{tft.fillRect(155, int(curpos), 4, 1, HACO3_C8);}//１ピクセル未満の時は見えなくなるので１に
    
     //最終出力
    tft.setPivot(0, 0);
    tft.pushRotateZoom(&screen, 30, 0, 0, 1, 1);
    
    if(pressedBtnID == 0)//ESC
    {
      editor.setCursorConfig(0,0,0);//カーソルの位置を保存
      delay(50);
      restart("/init/main.lua", 0);
    }

    if(pressedBtnID == 6){//PAGEDOWN
      editor.editorSave(SPIFFS);//SPIFFSに保存
      delay(100);//ちょっと待つ
      reboot(appfileName, TFT_RUN_MODE);//現状rebootしないと初期化が完全にできない
      // restart(appfileName, 0);//初期化がうまくできない（スプライトなど）
      // broadchat();//ファイルの中身をブロードキャスト送信する（ファイルは消えない）
    }

  }

  // xSemaphoreGive(syncSemaphore);
}
// Usb.Task();

  frame++;
  if(frame > 18446744073709551615)frame = 0;
  delay(1);
  
  
}
