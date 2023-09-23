#include <Arduino.h>
#include <FS.h>
#include "SPIFFS.h"
#include "haco8/runHaco8Game.h"
#include "Tunes.h"
#include "KbdRptParser.h"
#include "Editor.h"
#include <PS2Keyboard.h>

#define KEYBOARD_DATA 32
#define KEYBOARD_CLK  33

PS2Keyboard keyboard;

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#define TFT_RUN_MODE false
#define TFT_EDIT_MODE true




uint64_t frame = 0;

// bool isEditMode = true;
bool isEditMode;
bool firstBootF = true;
bool difffileF = false;//前と違うファイルを開こうとしたときに立つフラグ

std::deque<int> buttonState;//ボタンの個数未定
// int buttonState[9];

enum struct FileType {
  LUA,
  JS,
  BMP,
  PNG,
  TXT,
  OTHER
};


//キーボード関連

Editor editor;

// KbdRptParser Prs;

char keychar;//キーボードから毎フレーム入ってくる文字

//esp-idfのライブラリを使う！
//https://qiita.com/norippy_i/items/0ed46e06427a1d574625
#include <driver/adc.h>//アナログボタンはこのヘッダファイルを忘れないように！！


// どこかでMapDictionaryのインスタンスを取得しておく
// MapDictionary& dict = MapDictionary::getInstance();

using namespace std;

#define MAX_CHAR 1
#define FORMAT_SPIFFS_IF_FAILED true

#define BUF_PNG_NUM 0

//outputmode最終描画の仕方
// int outputMode = FAST_MODE;//50FPS程度128*128 速いけど小さい画面　速度が必要なモード
int outputMode = WIDE_MODE;//20FPS程度240*240 遅いけれどタッチしやすい画面　パズルなど

// WifiGame* wifiGame = NULL;

uint8_t xpos, ypos = 0;
uint8_t colValR = 0;
uint8_t colValG = 0;
uint8_t colValB = 0;

uint8_t charSpritex = 0;
uint8_t charSpritey = 0;
int pressedBtnID = -1;//この値をタッチボタン、物理ボタンの両方から操作してbtnStateを間接的に操作している

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


  static constexpr int HACO3_C0    = 0x0000;
  static constexpr int HACO3_C1    = 6474;//27,42,86 
  static constexpr int HACO3_C2    = 35018;
  static constexpr int HACO3_C3    = 1097;
  static constexpr int HACO3_C4    = 45669;
  static constexpr int HACO3_C5    = 25257;
  static constexpr int HACO3_C6    = 50712;
  static constexpr int HACO3_C7    = 65436;
  static constexpr int HACO3_C8    = 63496;//0xF802;
  static constexpr int HACO3_C9    = 64768;
  static constexpr int HACO3_C10   = 65376;
  static constexpr int HACO3_C11   = 1856;
  static constexpr int HACO3_C12   = 1407;
  static constexpr int HACO3_C13   = 33715;
  static constexpr int HACO3_C14   = 64341;
  static constexpr int HACO3_C15   = 65108;


// //2倍拡大表示用のパラメータ
// float matrix_side[6] = {2.0,   // 横2倍
//                      -0.0,  // 横傾き
//                      258.0,   // X座標
//                      0.0,   // 縦傾き
//                      2.0,   // 縦2倍
//                      0.0    // Y座標
//                     };

// //2倍拡大表示用のパラメータ
// float matrix_game[6] = {1.0,   // 横2倍
//                      -0.0,  // 横傾き
//                      0.0,   // X座標
//                      0.0,   // 縦傾き
//                      1.0,   // 縦2倍
//                      0.0    // Y座標
//                     };

LGFX screen;//LGFXを継承

LovyanGFX_DentaroUI ui(&screen);
LGFX_Sprite tft(&screen);
// LGFX_Sprite scaler(&screen);

// #include "MapDictionary.h"
// MapDictionary& dict = MapDictionary::getInstance();

LGFX_Sprite sprite88_roi = LGFX_Sprite(&tft);
LGFX_Sprite sprite11_roi = LGFX_Sprite(&tft);
LGFX_Sprite sprite64 = LGFX_Sprite();

LGFX_Sprite buffSprite = LGFX_Sprite(&tft);

LGFX_Sprite sprite88_0 = LGFX_Sprite(&tft);

// LGFX_Sprite mapTileSprites[1];
// static LGFX_Sprite sliderSprite( &tft );//スライダ用

BaseGame* game;
Tunes tunes;
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

uint8_t mapArray[MAPWH][MAPWH];
bool mapready = false;

int8_t sprbits[128];//fgetでアクセスするスプライト属性を格納するための配列

char buf[MAX_CHAR];
char str[100];//情報表示用
int mode = 0;//記号モード //0はrun 1はexit
int gameState = 0;
String appNameStr = "init";
int soundNo = -1;
float soundSpeed = 1.0;
int musicNo = -1;
bool musicflag = false;
bool sfxflag = false;
bool toneflag = false;
bool firstLoopF = true;

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
// SemaphoreHandle_t xSemaphore = NULL;
// タスクが終了したことを示すセマフォ
// SemaphoreHandle_t taskSemaphore = NULL;

int vol_value; //analog値を代入する変数を定義
int statebtn_value; //analog値を代入する変数を定義
int jsx_value; //analog値を代入する変数を定義
int jsy_value; //analog値を代入する変数を定義

//エディタ、キーイベント用
// enum editorKey {
//   BACKSPACE = 42,
//   ARROW_LEFT = 80,
//   ARROW_RIGHT = 79,
//   ARROW_UP = 82,
//   ARROW_DOWN = 81,
//   DEL_KEY = 76,
//   HOME_KEY = 74,
//   END_KEY = 77,
//   PAGE_UP = 75,
//   PAGE_DOWN = 78,
//   ENTER_KEY = 135,//13,//通常は13
//   ESCAPE_KEY = 41
// };

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

// Vector2<int> getKey2Sign(String _currentKey, String _targetKey) {
#include <LovyanGFX_DentaroUI.hpp>
// #include <SPI.h>
// #include <Wire.h>
#include <map>
// #include <SD.h>
#include <Arduino.h>
#include <FS.h>
#include "SPIFFS.h"
// // #include "runLuaGame.h"
// #include "haco8/runHaco8Game.h"

// #include "Tunes.h"
// #include "runJsGame.h"
// #include "wifiGame.h"
// MapTile構造体の定義
// #define BUF_PNG_NUM 0

// class MapTile {
//   private:
//     LGFX_Sprite* sprptr;
//     Vector2<int> tileNo;
//     // int xtileNo;
//     // int ytileNo;
//     String filePath;
//     String newKey;
//     String targetKey;
//     String oldKey;
//     int dirno;
//     bool writableF;
//     int oldreadNo;
//   public:
//   // 他のメンバ変数やメソッドの定義
//     MapTile()
//     : tileNo(0,0), sprptr(nullptr), dirno(0), filePath(""), newKey(""), oldKey(""), writableF(false), oldreadNo(-1)
//     {
//     }

//     // 初期化メソッドなどを追加する場合もあります
//     void init() {
//         // 初期化処理を記述
//     }
    
//     void drawMap2(LovyanGFX& _buffSprite, int _bufx, int _bufy) {

//        if (sprptr != nullptr) {
//             // 登録されたスプライトを描画
//             sprptr->pushSprite(&_buffSprite, _bufx, _bufy);

//         }
//     }

//     void setOldreadNo(int _oldreadNo){
//       oldreadNo = _oldreadNo;
//     }

//     int getOldreadNo(){
//       return oldreadNo;
//     }

//     void setWritableF2(bool _writableF){
//       writableF = _writableF;
//     }

//     bool getWritableF2(){
//       return writableF;
//     }
    
    
//     void setDirNo2( int _dirno ){
//       dirno = _dirno;
//     }

//     int getDirNo2(){
//       return dirno;
//     }
//     LGFX_Sprite* getSprptr2() {
//         return sprptr;
//     }

//     // メンバ変数をセットするメソッド
//     void setNewKey2(const String& _Key) {
//       // Serial.println(_Key);
//       newKey = _Key;
//     }

//     void setTileNo2(Vector2<int> _tileNo){
//       tileNo = _tileNo;
//     }

//     // メンバ変数をゲットするメソッド
//     const String getNewKey2() const {
//         return newKey;
//     }

//     void setSprptr2(LGFX_Sprite* _sprptr) {
//      sprptr = _sprptr; // メンバ変数にスプライトへのポインタを代入
//     }

// };

// MapDictionaryクラスの定義
//マップの検索を行う
// class MapDictionary {
// private:
//     MapTile maptils[BUF_PNG_NUM];
//     std::map<String, LGFX_Sprite*> key2ptr;

//     String newKeys[BUF_PNG_NUM];
//     String oldKeys[BUF_PNG_NUM];
//     int dirNos[BUF_PNG_NUM];

// public:
//     static MapDictionary& getInstance() {
//       static MapDictionary instance;
//       return instance;
//     }

//     LGFX_Sprite* getSprptr(int no){
//       return maptils[no].getSprptr2();
//     } 

//     void setSprptr(int no, LGFX_Sprite* _sprptr){
//       maptils[no].setSprptr2(_sprptr);
//     }

//     void copy2buff(LovyanGFX& _buffSprite, LGFX_Sprite* _sprptr, int no) {
//         // maptils[no].setSprptr2(_sprptr);
//         // setSprptr(no, _sprptr);
//         // maptils[no].drawMap2(_buffSprite,0,0);

//         // _buffSprite.
//         _buffSprite.fillRect(20*no, 20*no, 20,20, TFT_BLUE);

//         // _sprptr->pushSprite(&_buffSprite, 0,0);
//     }

//     void setWritableF(int no, bool _writableF){
//       maptils[no].setWritableF2(_writableF);
//     }

//     bool getWritableF(int no){
//       return maptils[no].getWritableF2();
//     }


//     void setDirNo( int no, int _dirno){
//       maptils[no].setDirNo2( _dirno );
//     }

//   void setReadNos( int _dirno){
//     if(_dirno == -1){//止まっていたら
//       for(int i=0;i<9;i++){
//         maptils[i].setDirNo2( i - 1 );//ひとまず0方向と同じ読み込み順にする
//       }
//     }else{

//       maptils[0].setDirNo2( -1 );//現在位置
//       // 進行方向を中心にした読み込み順を決める
//       maptils[1].setDirNo2(  _dirno );//進行方向
//       maptils[2].setDirNo2( (_dirno + 8 - 1) % 8);
//       maptils[3].setDirNo2( (_dirno + 8 + 1) % 8);
//       maptils[4].setDirNo2( (_dirno + 8 - 2) % 8);
//       maptils[5].setDirNo2( (_dirno + 8 + 2) % 8);
//       maptils[6].setDirNo2( (_dirno + 8 - 3) % 8);
//       maptils[7].setDirNo2( (_dirno + 8 + 3) % 8);
//       maptils[8].setDirNo2( (_dirno + 8 - 4) % 8);
//     }
//     maptils[0].setDirNo2( -1 );//現在位置
//   }

// int getDirNo(int no) {
//     return maptils[no].getDirNo2(); 
// }

//   Vector2<int> getTilePos(int _dirNo) {
//     Vector2<int> directions[] = {//ベクトル { X, Y, DirNo}
//         { 0,  0},  //-1
//         { 1,  0},  // 0
//         { 1,  1},  // 1
//         { 0,  1},  // 2
//         {-1,  1},  // 3
//         {-1,  0},  // 4
//         {-1, -1},  // 5
//         { 0, -1},  // 6
//         { 1, -1}   // 7
//     };
    
//     if (_dirNo >= -1 && _dirNo <= 7) {
//         return directions[_dirNo + 1]; // 配列のインデックスは-1から始まるため、+1する
//     } else {
//         return {0, 0}; // デフォルト
//     }
//   }

//   int getTargetDirNo(float _x, float _y){
//     float angle = atan2(_y, _x) + 2 * M_PI;
//       return fmod(angle / (M_PI / 4), 8);
//   }

//   int getTargetDirNo(Vector3<float> _pos){
//     float angle = atan2(_pos.getY(), _pos.getX()) + 2 * M_PI;
//       return fmod(angle / (M_PI / 4), 8);
//   }

//   MapTile getMapTile(int no){
//     return maptils[no];//ディクショナリー内から
//   }

//   void setNewKey(int no, String& _Key) {
//     newKeys[no] = _Key;//ディクショナリー内に登録

//     maptils[no].setTileNo2(getKey2tileNo(_Key));

//     if (no >= 0 && no < BUF_PNG_NUM) {
//       maptils[no].setNewKey2(_Key);  // //マップタイル自体に登録する

//       if (key2ptr.count(_Key) == 0) {
//           key2ptr[_Key] = getSprptr(no);  // キーをポインタに紐付ける
//       } else {
//           // 既にキーが存在する場合の処理
//           // 例えば、上書きするかどうかの判断など
//       }
//     }
//   }

//   void showKeyInfo(String& _Key){
//     if(key2ptr.count(_Key)>0){
//       Serial.print(_Key+"の数：");
//       Serial.println(key2ptr.count(_Key));
//     }
//   }


//   LGFX_Sprite* getKey2sprptr(const String& _Key) {

//     if (key2ptr.count(_Key) > 0) {
      
//         auto it = key2ptr.find(_Key);
//         return it->second;
//     } else {
//         // キーが見つからない場合はnullptrを返す
//         return nullptr;
//     }
//   }

//   Vector2<int> getKey2tileNo(String& _Key) {
//     if (key2ptr.count(_Key) > 0) {
//         int slashPos = _Key.indexOf('/'); // '/'の位置を取得
//         if (slashPos != -1) { // '/'が見つかった場合
//             String numA_str = _Key.substring(0, slashPos); // '/'より前の部分を取得
//             String numB_str = _Key.substring(slashPos + 1); // '/'より後の部分を取得
//             int numA = numA_str.toInt(); // 数字に変換
//             int numB = numB_str.toInt(); // 数字に変換
//             return {numA, numB}; // Vector2<int>を返す
//         }
//     }
//     // キーが見つからない場合や'/'が見つからない場合はデフォルトのVector2<int>を返す
//     return {-1, -1};
// }


//   String getNewKey(int no) {
//       if (no >= 0 && no < BUF_PNG_NUM) {
//           return maptils[no].getNewKey2();
//       }
//       return ""; // エラー時は空文字列を返す
//   }

//   bool getAlreadyDownlordedF(String& _Key){
//     if (key2ptr.count(_Key) > 0) return true;
//     else return false;

//   }

// };

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
  Serial.println("Download Keys:");
  for (const auto& key : downloadKeys) {
      Serial.print(key);
  }
  Serial.println("");
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
      game = new RunHaco8Game(_gameState, _mn);
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
  
  ui.setConstantGetF(false);//初期化処理 タッチポイントの常時取得を切る
  
  appfileName = s;
  mode = 1;//exit to run

}

// タイマー
hw_timer_t * timer = NULL;

// 画面描画タスクハンドル
// TaskHandle_t taskHandle;

// タイマー割り込み
// void IRAM_ATTR onTimer() {
//   xTaskNotifyFromISR(taskHandle, 0, eIncrement, NULL);
// }

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
   Serial.print("Deleting file: ");
   Serial.println(path);
   if(fs.remove(path)) Serial.print("− file deleted\n\r");
   else { Serial.print("− delete failed\n\r"); }
}

void listDir(fs::FS &fs){
   File root = fs.open("/");
   File file = root.openNextFile();
   while(file){
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.print(file.size());
      file = root.openNextFile();
   }
}

String rFirstAppName(String _wrfile){
  File fr = SPIFFS.open(_wrfile.c_str(), "r");// ⑩ファイルを読み込みモードで開く
  String _readStr = fr.readStringUntil('\n');// ⑪改行まで１行読み出し
  fr.close();	// ⑫	ファイルを閉じる
  return _readStr;
}

void getOpenConfig(String _wrfile) {
  File fr = SPIFFS.open(_wrfile, "r");
  String line;
  while (fr.available()) {
    line = fr.readStringUntil('\n');
    if (!line.isEmpty()) {
      int commaIndex = line.indexOf(',');
        String val = line.substring(0, commaIndex);
        if(val != NULL)appfileName =  val;
        else appfileName = "/init/main.lua";//configファイルが壊れていても強制的に値を入れて立ち上げる
          int nextCommaIndex = line.indexOf(',', commaIndex + 1);//一つ先のカンマ区切りの値に進める
          if (nextCommaIndex != -1) {
            val = line.substring(commaIndex + 1, nextCommaIndex);
            if(val.toInt() != NULL){
              isEditMode = val.toInt();
              Serial.print("editmode[");Serial.print(isEditMode);Serial.println("]");
            }else{
              isEditMode = 0;//configファイルが壊れていても強制的に値を入れて立ち上げる
            }
          }
    }
  }
  fr.close();
}

void setOpenConfig(String fileName, int _isEditMode) {
  char numStr[64];//64文字まで
  sprintf(numStr, "%s,%d,", 
    fileName.c_str(), _isEditMode
  );

  Serial.println(fileName.c_str());
  Serial.println(_isEditMode);

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

using namespace std;

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

void setTFTedit(bool _iseditmode){
  if(_iseditmode == true){
    tft.setPsram( false );//DMA利用のためPSRAMは切る
    tft.createSprite( TFT_WIDTH, TFT_HEIGHT );//PSRAMを使わないギリギリ
    tft.startWrite();//CSアサート開始
  }else if(_iseditmode == false){
    tft.setPsram( false );//DMA利用のためPSRAMは切る
    tft.createSprite( TFT_WIDTH, TFT_HEIGHT );
    tft.startWrite();//CSアサート開始
  }
}


void setup()
{
  Serial.begin(115200);
  keyboard.begin(KEYBOARD_DATA, KEYBOARD_CLK);

  // editor.setCursorConfig(0,0,0);//カーソルの位置を強制リセット保存
  // delay(50);

  editor.getCursorConfig("/init/param/editor.txt");//カーソルの位置をよみこむ
  delay(50);
  
  // screen.init();
  
  if(firstBootF == true){
    difffileF = false;

    #if !defined(__MIPSEL__)
      while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
      #endif
      Serial.println("Keyboard Start");

    if (!SPIFFS.begin(true))
    {
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  }

  getOpenConfig("/init/param/openconfig.txt");//最初に立ち上げるゲームのパスとモードをSPIFFSのファイルopenconfig.txtから読み込む
 

  if(isEditMode == TFT_RUN_MODE){

    if(firstBootF == false){
      tft.deleteSprite();
      delay(100);
    }
    setTFTedit(TFT_RUN_MODE);

  //   //外部物理ボタンの設定
  // adc1_config_width(ADC_WIDTH_BIT_12);
  // //何ビットのADCを使うか設定する。今回は12bitにします。
  // //adc1の場合はこのように使うチャンネル全体の設定をするコマンドが用意されている。
  // adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);//39pin　4つのボタン
  // adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);//33pin　ボリューム
  // adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);//34pin　ジョイスティックX
  // adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);//35pin　ジョイスティックY

  //sin 0~90度をメモリに保持する

  ui.begin( screen, 16, 1);

    // for (int i = 0; i < 90; ++i) {
    //   float radians = i * M_PI / 180.0;
    //   sinValues[i] = sin(radians);
    // }
    // sinValues[90] = 1.0;
    // sinValues[270] = -1.0;

// if(firstBootF == true)
// {
  sprite88_0.setPsram(false );
  sprite88_0.setColorDepth(16);//子スプライトの色深度
  sprite88_0.createSprite(8, 8);//ゲーム画面用スプライトメモリ確保

  //sprite88_0.drawPngFile(SPIFFS, "/init/sprite.png", -8*1, -8*0);

  sprite64.setPsram(false );
  sprite64.setColorDepth(16);//子スプライトの色深度
  sprite64.createSprite(64, 64);//ゲーム画面用スプライトメモリ確保//wroomだと64*128だとメモリオーバーしちゃう

  sprite64.drawPngFile(SPIFFS, "/init/initspr.png", 0, 0);
  
  //psram使えない-------------------------------------------
  // buffSprite.setPsram( true );
  // buffSprite.setColorDepth(16);//子スプライトの色深度
  // buffSprite.createSprite(256, 256);//ゲーム画面用スプライトメモリ確保

  // for( int i = 0; i < BUF_PNG_NUM; i++ ){
  //   mapTileSprites[i].setPsram(true);
  //   mapTileSprites[i].setColorDepth(16);
  //   mapTileSprites[i].createSprite(256,256);
  //   // MapTile クラスをインスタンス化し、スプライトに描画して返す
  //   dict.copy2buff(buffSprite, &mapTileSprites[i], i);
  // }

  // //キーと紐づけ、初期設定のキー0~9と値のペアを適当に登録しておく
  // for(int j = 0; j<3; j++){
  //   for(int i = 0; i<3; i++){
  //     dict.setSprptr(i*3+j, &mapTileSprites[i]);
  //     dict.setNewKey(i*3+j, String(xtileNo+i) + "/" + String(ytileNo+j));
  //     dict.showKeyInfo(String(xtileNo+i) + "/" + String(ytileNo+j));
  //   }
  // }
  //psram使えない-------------------------------------------

  sprite88_roi.setPsram(false );
  sprite88_roi.setColorDepth(16);//子スプライトの色深度
  sprite88_roi.createSprite(8, 8);//ゲーム画面用スプライトメモリ確保

  sprite11_roi.setPsram(false );
  sprite11_roi.setColorDepth(16);//子スプライトの色深度
  sprite11_roi.createSprite(1, 1);//ゲーム画面用スプライトメモリ確保

  spriteMap.setPsram(false );
  spriteMap.setColorDepth(16);//子スプライトの色深度
  spriteMap.createSprite(MAPWH, MAPWH/divnum);//マップ展開用スプライトメモリ確保

  if(firstBootF == true)
{

  //タッチボタンを生成
  File fr = SPIFFS.open("/init/param/uiinfo.txt", "r");
  String line;

  while (fr.available()) {
    line = fr.readStringUntil('\n');
    if (!line.isEmpty()) {
      int commaIndex = line.indexOf(',');
      if (commaIndex != -1) {
        String val = line.substring(0, commaIndex);
        addUiNum[0] = val.toInt();

        if(addUiNum[0]!=-1){//-1の時は生成しない

          for (int i = 1; i < 6; i++) {
            int nextCommaIndex = line.indexOf(',', commaIndex + 1);
            if (nextCommaIndex != -1) {
              val = line.substring(commaIndex + 1, nextCommaIndex);
              addUiNum[i] = val.toInt();
              commaIndex = nextCommaIndex;
            }
          }
          ui.createPanel( addUiNum[0], addUiNum[1], addUiNum[2], addUiNum[3], addUiNum[4], addUiNum[5], TOUCH, ui.getTouchZoom());//ホームボタン
          allAddUiNum++;
        }
      }
    }
  }
  fr.close();

  // delay(100);

  // mapready = false;
  // while(mapready == false){
  //   readMap(1);
  // };//マップ番号を指定し、読み込み終わるまで待機

  mapFileName = "/init/map/0.png";
  readMap();
  delay(100);

  game = nextGameObject(&appfileName, gameState, mapFileName);//ホームゲームを立ち上げる（オブジェクト生成している）
  game->init();//（オブジェクト生成している）
  tunes.init();//（オブジェクト生成している）
  }

  frame=0;
  }else if(isEditMode == TFT_EDIT_MODE){//エディットモードの時
    if(firstBootF == false){
      tft.deleteSprite();
      delay(10);
    }
    setTFTedit(TFT_EDIT_MODE);

    ui.begin( screen, 16, 1);
 
    if(firstBootF == true)
    {

      if (SPIFFS.exists(appfileName)) {
        File file = SPIFFS.open(appfileName, FILE_READ);
        if (!file) {
          Serial.println("ファイルを開けませんでした");
          return;
        }
        // ファイルからデータを読み込み、シリアルモニターに出力
        while (file.available()) {
          Serial.write(file.read());
        }
        // ファイルを閉じる
        file.close();
      }

      //タッチボタンを生成
      File fr = SPIFFS.open("/init/param/uiinfo.txt", "r");
      String line;

      while (fr.available()) {
        line = fr.readStringUntil('\n');
        if (!line.isEmpty()) {
          int commaIndex = line.indexOf(',');
          if (commaIndex != -1) {
            String val = line.substring(0, commaIndex);
            addUiNum[0] = val.toInt();

            if(addUiNum[0]!=-1){//-1の時は生成しない

              for (int i = 1; i < 6; i++) {
                int nextCommaIndex = line.indexOf(',', commaIndex + 1);
                if (nextCommaIndex != -1) {
                  val = line.substring(commaIndex + 1, nextCommaIndex);
                  addUiNum[i] = val.toInt();
                  commaIndex = nextCommaIndex;
                }
              }
              ui.createPanel( addUiNum[0], addUiNum[1], addUiNum[2], addUiNum[3], addUiNum[4], addUiNum[5], TOUCH, ui.getTouchZoom());//ホームボタン
              allAddUiNum++;
            }
          }
        }
      }
      fr.close();
      game = nextGameObject(&appfileName, gameState, mapFileName);//ホームゲームを立ち上げる（オブジェクト生成している）
      game->init();//（オブジェクト生成している）
      tunes.init();//（オブジェクト生成している）

      frame=0;

      editor.initEditor(tft);
      editor.readFile(SPIFFS, appfileName.c_str());
      editor.editorOpen(SPIFFS, appfileName.c_str());
      editor.editorSetStatusMessage("Press ESCAPE to save file");

    }

  }
  savedAppfileName = appfileName;//起動したゲームのパスを取得しておく
  firstBootF = false;
}

int btn(int btnno){
  return buttonState[btnno];//ボタンの個数未定
}

void restart(String _fileName, int _isEditMode)
{
  setOpenConfig(_fileName, _isEditMode);
  
  editor.setCursorConfig();//カーソルの位置を保存
  delay(100);

  firstBootF = false;
  setup();

  tunes.pause();
  game->pause();
  free(game);
  firstLoopF = true;
  toneflag = false;
  sfxflag = false;
  musicflag = false;
  // txtName = _fileName;
  game = nextGameObject(&_fileName, gameState, mapFileName);//ファイルの種類を判別して適したゲームオブジェクトを生成
  game->init();//resume()（再開処理）を呼び出し、ゲームで利用する関数などを準備
  tunes.resume();
}

void loop()
{
  //キー取得
  pressedBtnID = -1;//リセット
  if (keyboard.available()) {
    keychar = keyboard.read();
    // check for some of the special keys
    // if(keychar != NULL){
           if (keychar == PS2_ENTER) {
      Serial.println();
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
      Serial.print("[Del]");
      // tft.clear();
      // tft.setCursor(16, 16);
      // tft.display();
    } else {
      //通常の文字
      // Serial.print(keychar);
      // Serial.println(keychar);
    }

    // editor.update(tft, SPIFFS, SD, keychar);
    // editor.update(tft, SPIFFS, keychar);
    editor.editorProcessKeypress(keychar, SPIFFS);
    buttonState[pressedBtnID]++;
    // }
  
  }else{//キーアップの時など押されていないとき
  if(pressedBtnID!=-1)buttonState[pressedBtnID] = -1;
    pressedBtnID = -1;//リセット    
  }

  editor.editorRefreshScreen(tft);

  // screen.setCursor(20, (screen.height() >> 3) * 4);
  // screen.setTextSize(0);
  // screen.println("hello at " + String(millis()));

  // ui.setConstantGetF(true);//trueだとタッチポイントのボタンIDを連続取得するモード
  // ui.update(screen);//タッチイベントを取るので、LGFXが基底クラスでないといけない

    // if(ui.getTouchBtnID() == RELEASE){//リリースされたら
    //   pressedBtnID = -1;
    // }
  
  uint32_t now = millis();
  uint32_t remainTime= (now - preTime);
  preTime = now;

  if( isEditMode == 0 ){
    //ゲーム内のprint時の文字設定をしておく
    tft.setTextSize(1);//サイズ
    tft.setFont(&lgfxJapanGothicP_8);//日本語可
    tft.setCursor(0, 0);//位置
    tft.setTextWrap(true);

    // == tune task ==
    tunes.run();

    // == game task ==
    mode = game->run(remainTime);//exitは1が返ってくる　mode=１ 次のゲームを起動

    //0ボタンで強制終了
    if (pressedBtnID == 0)
    { // reload

      editor.setCursorConfig(0,0,0);//カーソルの位置を強制リセット保存
      delay(50);

      ui.setConstantGetF(false);//初期化処理 タッチポイントの常時取得を切る
      appfileName = "/init/main.lua";
      
      firstLoopF = true;
      toneflag = false;
      sfxflag = false;
      musicflag = false;

      // getOpenConfig("/init/param/openconfig.txt");
      
      // game->setWifiDebugRequest(false);//外部ファイルから書き換えてWifiモードにできる
      // game->setWifiDebugSelf(false);
      mode = 1;//exit
    }

    if (pressedBtnID == 9999)
    { // reload
      ui.setConstantGetF(false);//初期化処理 タッチポイントの常時取得を切る
      mode = 1;//exit
      pressedBtnID = -1;
    }

    if(mode != 0){ // exit request//次のゲームを立ち上げるフラグ値、「modeが１＝次のゲームを起動」であれば
      tunes.pause();
      game->pause();
      // ui.clearAddBtns();//個別のゲーム内で追加したタッチボタンを消去する
      free(game);
      firstLoopF = true;
      toneflag = false;
      sfxflag = false;
      musicflag = false;
      // txtName = appfileName;
      game = nextGameObject(&appfileName, gameState, mapFileName);//ファイルの種類を判別して適したゲームオブジェクトを生成
      game->init();//resume()（再開処理）を呼び出し、ゲームで利用する関数などを準備
      tunes.resume();
      
    }

    // ui.showTouchEventInfo( tft, 0, 100 );//タッチイベントを視覚化する
    // ui.showInfo( tft, 0, 100+8 );//ボタン情報、フレームレート情報などを表示します。

    
    if(outputMode == WIDE_MODE){
      // tft.pushAffine(matrix_game);//ゲーム画面を最終描画する

      //Affineを使わない書き方
      tft.setPivot(0, 0);
      tft.pushRotateZoom(&screen, 0, 0, 0, 1, 1);
    }
    else if(outputMode == FAST_MODE){
      tft.pushSprite(&screen,TFT_OFFSET_X,TFT_OFFSET_Y);//ゲーム画面を小さく高速描画する
    }

    if(pressedBtnID == 5){//PAGEUP//キーボードからエディタ再起動

      // tft.deleteSprite();
      // setTFTedit(true);//エディットモードにセット

      
      // if(appfileName != savedAppfileName){//違うゲームファイルを開こうとしていたら
        // editor.setCursor(0,0,0);//カーソルの座標をリセット
        // editor.setCursorConfig();//

        // editor.getCursorConfig("/init/param/editor.txt");//外部ファイルに保存
      // }
      // setOpenConfig(appfileName, 1);//エディットモードで再起動
      // setup();
      restart(appfileName, 1);
      
      // reboot();
    }

    }else if(isEditMode == 1){//エディットモードの時

      float codeunit = 128.0/float(editor.getNumRows());
      float codelen = codeunit*10;
      
      float curpos = codeunit*editor.getCy();
      float codepos = codeunit*(editor.getCy() - editor.getScreenRow());
      
      tft.fillRect(156,0, 4,128, HACO3_C5);//コードの全体の長さを表示
      tft.fillRect(156,int(codepos), 4,codelen, HACO3_C6);//コードの位置と範囲を表示
      if(codeunit>=1){tft.fillRect(155, int(curpos), 4, codeunit, HACO3_C8);}//コードの位置と範囲を表示
      else{tft.fillRect(155, int(curpos), 4, 1, HACO3_C8);}//１ピクセル未満の時は見えなくなるので１に
      
      tft.pushSprite(&screen,0,0);
      
      if(pressedBtnID == 0)//ESC
      {

        editor.setCursorConfig(0,0,0);//カーソルの位置を保存
        delay(100);

        restart("/init/main.lua", 0);

        // setOpenConfig("/init/main.lua", 0);
        // // setup();
        // reboot();
      }

      if(pressedBtnID == 6){//PAGEDOWN

      // savedAppfileName = appfileName;



        editor.editorSave(SPIFFS);//SPIFFSに保存
        delay(100);//ちょっと待つ
        // editor.editorSaveSD(SD);//SDに同時にバックアップ保存する
        restart(appfileName, 0);
      }
    }

    frame++;
    if(frame > 18446744073709551615)frame = 0;

    // if (!keyboard.available()){
    //   tft.print("キーボードが使えません。");
    // }
  // delay(1);
  
}
