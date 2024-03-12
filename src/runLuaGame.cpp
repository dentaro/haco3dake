#include "runLuaGame.h"

#include <chrono> // std::chrono
#include <thread> // std::this_thread

extern uint8_t mapArray[16][20];
extern int gSpr8numX;
extern int gSpr8numY;
extern int gWx;
extern int gWy;
extern  int gSprw;
extern  int gSprh;
extern bool drawMap();
extern bool readRowFromBinary2(const char *filename);

extern MyTFT_eSprite tft;
// extern LGFX_Sprite sprite64;
// extern LGFX_Sprite sprite256[2][2];

extern uint8_t sprno;
extern uint8_t repeatnum;

// extern std::vector<uint8_t> rowData;
std::vector<std::vector<uint8_t>> rowData(16);

extern LGFX_Sprite sprite88_roi;
extern String appfileName;
extern void startWifiDebug(bool isSelf);
extern void setFileName(String s);
extern bool isWifiDebug();
extern void readMap();
extern void reboot();
extern void reboot(String _fileName, uint8_t _isEditMode);
// extern Tunes tunes;
extern int pressedBtnID;
extern LovyanGFX_DentaroUI ui;
extern int outputMode;
extern int mapsprnos[16];
extern int8_t sprbits[128];//8*16
extern vector<string> split(string& input, char delimiter);
extern String appNameStr;
extern String mapFileName;
extern float sliderval[2];
extern bool optionuiflag;
extern int frame;
extern int boxzerox;
extern int boxzeroy;
extern int allAddUiNum;
extern bool toneflag;
extern bool firstLoopF;
extern float radone;
extern bool downloadF;
extern void setOpenConfig(String fileName, uint8_t _isEditMode);
extern void getOpenConfig(String _wrfile);
extern std::deque<int> buttonState;//ボタンの個数未定

extern void restart(String _fileName, int _isEditMode);
extern int getcno2tftc(uint8_t _cno);

extern char keychar;
extern Editor editor;

extern uint8_t mainVol;

extern String savedAppfileName;
extern bool difffileF;//前と違うファイルを開こうとしたときに立つフラグ

// extern Speaker_Class speaker;

extern std::vector<uint8_t> sprite64cnos_vector;
extern int gameState;
// extern void getOpenConfig(String _wrfile);
extern uint16_t gethaco3Col(uint8_t haco3ColNo);

int cursor = 0;

uint16_t hacol0;
uint16_t hacol1;
uint16_t hacol2;
uint16_t hacol3;

std::vector<uint8_t> buffer;
int bytesRead;


// runLuaGame.cpp ファイル内に以下のように RGBValues 配列を宣言してください。
// const uint8_t RGBValues[][3] PROGMEM = {//24bit

//   { 0,0,0},//0: 黒色
//   { 27,42,86 },//1: 暗い青色
//   { 137,24,84 },//2: 暗い紫色
//   { 0,139,75 },//3: 暗い緑色
//   { 183,76,45 },//4: 茶色
//   { 97,87,78 },//5: 暗い灰色
//   { 194,195,199 },//6: 明るい灰色
//   { 255,241,231 },//7: 白色
//   { 255,0,70 },//8: 赤色
//   { 255,160,0 },//9: オレンジ
//   { 255,238,0 },//10: 黄色
//   { 0,234,0 },//11: 緑色
//   { 0,173,255 },//12: 水色
//   { 134,116,159 },//13: 藍色
//   { 255,107,169 },//14: ピンク
//   { 255,202,165}//15: 桃色

// };

// const uint8_t RGBValues[][3] PROGMEM = {//16bit用
//   {0, 0, 0},     // 0: 黒色=なし
//   {24, 40, 82},  // 1: 暗い青色
//   {140, 24, 82}, // 2: 暗い紫色
//   {0, 138, 74},  // 3: 暗い緑色
//   {181, 77, 41}, // 4: 茶色 
//   {99, 85, 74},  // 5: 暗い灰色
//   {198, 195, 198}, // 6: 明るい灰色
//   {255, 243, 231}, // 7: 白色
//   {255, 0, 66},  // 8: 赤色
//   {255, 162, 0}, // 9: オレンジ
//   {255, 239, 0}, // 10: 黄色
//   {0, 235, 0},   // 11: 緑色
//   {0, 174, 255}, // 12: 水色
//   {132, 117, 156}, // 13: 藍色
//   {255, 105, 173}, // 14: ピンク
//   {255, 203, 165}  // 15: 桃色
// };


extern "C" {
  void gprint(char* s){
    tft.setCursor(3, cursor);
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.print(s);
    cursor += 4;
    if(cursor > 128){
      cursor = 0;
    }
  }

  const char *getF(lua_State *L, void *ud, size_t *size){
    struct LoadF *lf = (struct LoadF *)ud;
    (void)L; /* not used */
    char* ret = NULL;

    if(!lf->f.available()){
      *size = 0;
      return NULL;
    }

    lf->f.readStringUntil('\n').toCharArray(lf->buf, MAX_CHAR);
    ret = lf->buf;
    int len = strnlen(ret, MAX_CHAR);
    ret[len] = '\n'; // todo n, n+1 > MAX_CHAR ?
    ret[len + 1] = 0;

    *size = len + 1;
    // Serial.print("");
    // Serial.print(ret);
    // Serial.println(*size);
    return ret;
  }
}

int runLuaGame::loadSurface(File *fp, uint8_t* buf){
  uint8_t c;
  unsigned long offset;
  unsigned long width, height;
  unsigned long biSize;
  uint16_t bitCount;

  Serial.println("pre read");
  fp->read(&c, 1);
  Serial.println("read1");
  if(c != 'B'){
    printf("header error 0");
    // Serial.print(c);
    Serial.println("unknown header");
    return -1;
  }
  fp->read(&c, 1);
  Serial.println("read2");
  if(c != 'M'){
    printf("header error 1");
    return -1;
  }
  Serial.println("pre seek");
  fp->seek(4 + 2 + 2, SeekCur); // size, resv1, resv2
  fp->read((uint8_t*)&offset, 4);

  fp->read((uint8_t*)&biSize, 4);
  fp->read((uint8_t*)&width, 4);
  fp->read((uint8_t*)&height, 4);
  fp->seek(2, SeekCur); // skip biPlanes
  fp->read((uint8_t*)&bitCount, 2);

  Serial.println("pre check");
  if(width != 128){
    printf("invalid width:%d\n", width);
    return -1;
  }
  if(height != 128){
    printf("invalid height:%d\n", height);
    return -1;
  }
  if(bitCount != 8){
    printf("invalid bitCount:%x\n", bitCount);
    return -1;
  }

  fp->seek(biSize - (4 + 4 + 4 + 2 + 2), SeekCur);
  uint8_t r, g, b;
  for(unsigned int i = 0; i < 256; i ++){
    fp->read(&b, 1);
    fp->read(&g, 1);
    fp->read(&r, 1);
    fp->seek(1, SeekCur);
    // palette[i] = lua_rgb24to16(r, g, b);
    // Serial.print("palette");
    // Serial.println(i);
    // Serial.print(r);
    // Serial.print(g);
    // Serial.print(b);
  }

  Serial.println("pre seek");
  fp->seek(offset, SeekSet); // go to bmp data section

  for(unsigned int i = 0; i < width * height; i ++){
    uint8_t d;
    fp->read(&d, 1);
    *buf = d;
    buf ++;
  }
  return 0;
}

runLuaGame::runLuaGame(int _gameState, String _mn){
  gameState = _gameState;
  mapFileName = _mn;
}

int runLuaGame::l_vol(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int v = lua_tointeger(L, 1);//チャンネル?
  mainVol = v;
  return 0;
}


int runLuaGame::l_tone(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int n = lua_tointeger(L, 1);//チャンネル?
  int f = lua_tointeger(L, 2);//周波数
  int sl = lua_tointeger(L, 3);//音の長さ

  // if(sl!=0){
    // speaker.begin();
    // speaker.setVolume(255);
    // speaker.setChannelVolume(0, 255);
    // speaker.tone(f,sl);
    // delay(sl);
    // speaker.stop();
  // }
  // ledcWriteTone(n, 0);    // 消音

  /// tone data (8bit unsigned wav)
  //ノイズ音

  // const uint8_t wavdata[64] PROGMEM = { 132,138,143,154,151,139,138,140,144,147,147,147,151,159,184,194,203,222,228,227,210,202,197,181,172,169,177,178,172,151,141,131,107,96,87,77,73,66,42,28,17,10,15,25,55,68,76,82,80,74,61,66,79,107,109,103,81,73,86,94,99,112,121,129 };
  //サイン波
//   const uint8_t wavdata[64] PROGMEM = {
//   128, 141, 153, 164, 175, 185, 194, 202, 209, 215, 220, 224, 227, 229, 230, 229, 
//   228, 225, 222, 217, 212, 206, 199, 192, 184, 176, 167, 157, 147, 137, 126, 115, 
//   104,  93,  82,  71,  60,  50,  40,  31,  23,  16,  10,   5,   2,   0,   0,   2,  
//     5,  10,  16,  23,  31,  40,  50,  60,  71,  82,  93, 104, 115, 126, 137, 147
// };


//三角波
//   const uint8_t wavdata[64] PROGMEM = {
//   128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 
//   255, 247, 239, 231, 223, 215, 207, 199, 191, 183, 175, 167, 159, 151, 143, 135, 
//   128, 120, 112, 104,  96,  88,  80,  72,  64,  56,  48,  40,  32,  24,  16,   8,  
//     0,    8,  16,  24,  32,  40,  48,  56,  64,  72,  80,  88,  96, 104, 112, 120
// };

//矩形波
//   const uint8_t wavdata[64] PROGMEM = {
//     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
//     255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 
//     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
//     0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
//   };

// speaker.setVolume(mainVol);//151が出力限界か？

// if(sl!=0){
//   if(sl!=0){
//   // speaker.tone(f, sl, 0, true, wavdata, sizeof(wavdata));
//   // speaker.tone(f, sl, 0);//チャンネル０
//   // speaker.tone(f/2, sl, 1);//チャンネル1
//   // speaker.tone(f*2, sl, 2);//チャンネル2
//   speaker.tone(f, sl, 0, true, wavdata, sizeof(wavdata));//チャンネル0
//   // speaker.tone(f*2, sl, 1, true, wavdata, sizeof(wavdata));//チャンネル1

//   while (speaker.isPlaying()) { delay(1); } // Wait for the output to finish.
//   speaker.stop();

//   }
// }


  // toneflag = true;

  // portENTER_CRITICAL(&Tunes::timerMux);
  // Tunes::d[n] = (uint16_t)(3.2768*f);
  // portEXIT_CRITICAL(&Tunes::timerMux);

  // portENTER_CRITICAL_ISR(&Tunes::timerMux);
  //   if(soundNo > -1)
  //   {
  //     // digitalWrite()//外部給電
  //     // ledcWrite()//パルス幅変調
  //     dacWrite(SPEAKER_PIN, (Wx[soundNo][floor(wcnt)]));//255が最高値　スピーカー無音が128で、上下に波形が出る、ブザーは0~255
  //     wcnt += soundSpeed;
  //     if(wcnt>256)wcnt=0;
  //   }
  // portEXIT_CRITICAL_ISR(&Tunes::timerMux);

  return 0;
}

int runLuaGame::l_pinw(lua_State* L){  
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int no = lua_tointeger(L, 1);
  if(no == 0)digitalWrite(OUTPIN_0, LOW);
  if(no == 1)digitalWrite(OUTPIN_0, HIGH);
  return 0;
}

int runLuaGame::l_pinr(lua_State* L){  
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int no = lua_tointeger(L, 1);
  if(no == 0)digitalWrite(INPIN_0, LOW);
  if(no == 1)digitalWrite(INPIN_0, HIGH);
  return 0;
}


int runLuaGame::l_spr8(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));

  int n = lua_tointeger(L, 1);
  int x = lua_tointeger(L, 2);
  int y = lua_tointeger(L, 3);
  int scalex = lua_tointeger(L, 4);
  int scaley = lua_tointeger(L, 5);
  int angle = lua_tointeger(L, 6);
  int tc = 0;//デフォルトの透明色は黒
  tc = lua_tointeger(L, 7);//透明色

  if( x>=0 && y>=0 && x<DISP_SPR_W_NUM*8 && y<DISP_SPR_H_NUM*8 ){//ディスプレイに入っているときのみ描画する

  uint8_t edgeno = lua_tointeger(L, 8);//縁取りの色

  x >> 3;  // xを8で割る（右シフト演算子を使用）
  y >> 3;  // yを8で割る

  if(y<DISP_SPR_H_NUM && x<DISP_SPR_W_NUM){
    mapArray[y][x] = uint8_t(n);//スプライト位置のマップ情報を書き換える
  }

  int w = 8;
  int h = 8;

  if(edgeno!=NULL){//エッジあり
     w = 10;
     h = 10;
  }

  sprite88_roi.clear();//指定の大きさにスプライトを作り直す
  sprite88_roi.createSprite(w,h);

  int spr8numX = 8;//スプライトシートに並ぶｘ、ｙの個数
  int spr8numY = 8;

  int sx = ((n-1)%spr8numX); //0~7
  int sy = ((n-1)/spr8numY); //整数の割り算はintにいれると切り捨てされる

  // for(int y=0;y<8;y++){
  //   for(int x=0;x<8;x++){
  //     uint8_t bit4;
  //     int sprpos;
  //     sprpos = (sy*8*PNG_SPRITE_WIDTH+sx*8 + y*PNG_SPRITE_WIDTH + x)/2;//４ビット二つで８ビットに入れてるので1/2に
  //     bit4 = sprite64cnos_vector[sprpos];
  //     if(x%2 == 1)bit4 = (bit4 & 0b00001111);
  //     if(x%2 == 0)bit4 = (bit4 >> 4);
  //     sprite88_roi.drawPixel(x,y, gethaco3Col(bit4));
  //   }
  // }

//エッジあり
  if(edgeno!=NULL){
//キャラスプライト
  for(int y=0;y<8;y++){
      for(int x=0;x<8;x++){
        uint8_t bit4;
        int sprpos;

        sprpos = (sy*8*PNG_SPRITE_WIDTH+sx*8 + y*PNG_SPRITE_WIDTH + x)/2;//４ビット二つで８ビットに入れてるので1/2に
        bit4 = sprite64cnos_vector[sprpos];
        if(x%2 == 1)bit4 = (bit4 & 0b00001111);
        if(x%2 == 0)bit4 = (bit4 >> 4);
        sprite88_roi.drawPixel(x+1,y+1, gethaco3Col(bit4));

      }
  }
  sprite88_roi.drawRect(0,0,10,10,gethaco3Col(0));

  bool upCol[10];
  bool preupCol[10];
  bool preCol = 1;
  bool nowCol = 1;
  bool nutta = false;

  uint8_t colValR;
  uint8_t colValG;
  uint8_t colValB;

  for(int y=0;y<10;y++){
    for(int x=0;x<10;x++){

      colValR = sprite88_roi.readPixelRGB(x,y).R8();
      colValG = sprite88_roi.readPixelRGB(x,y).G8();
      colValB = sprite88_roi.readPixelRGB(x,y).B8();

      if(colValR==0&&colValG==0&&colValB==0){//0: 黒色=なし
        upCol[x] = 0;
        nowCol = 0;
      }else{
        upCol[x] = 1;
        nowCol = 1;
        nutta=false;
      }

      if(preupCol[x] == 0&&nowCol == 1){//上が黒だったら
        sprite88_roi.drawPixel(x,y-1, gethaco3Col(edgeno));
      }

      if(preupCol[x] == 1&&nowCol == 0&&y!=0){//上が黒以外だったら
        sprite88_roi.drawPixel(x,y, gethaco3Col(edgeno));
        // nutta = true;
      }

      if(preCol==0&&nowCol == 1){//黒から黒以外になったら
        sprite88_roi.drawPixel(x-1,y, gethaco3Col(edgeno));
      }

      if(preCol==1&&nowCol == 0&& nutta == false){//黒以外から黒になったら
        sprite88_roi.drawPixel(x,y, gethaco3Col(edgeno));
        nutta = true;
      }
      preCol = nowCol;
    }
    memcpy(preupCol, upCol, sizeof(upCol));
  }
  }else{
    for(int y=0;y<8;y++){
      for(int x=0;x<8;x++){
        uint8_t bit4;
        int sprpos;
        //sprite64cnos[(sy*8) * PNG_SPRITE_WIDTH + (sx*8) 取得スタート位置
        //y*PNG_SPRITE_WIDTH + xスプライトのピクセル取得
        sprpos = (sy*8*PNG_SPRITE_WIDTH+sx*8 + y*PNG_SPRITE_WIDTH + x)/2;//４ビット二つで８ビットに入れてるので1/2に
        bit4 = sprite64cnos_vector[sprpos];
        if(x%2 == 1)bit4 = (bit4 & 0b00001111);
        if(x%2 == 0)bit4 = (bit4 >> 4);
        sprite88_roi.drawPixel(x,y, gethaco3Col(bit4));
      }
    }
  }

  // sprite88_roi.setPivot(w/2.0, h/2.0);
  sprite88_roi.setPivot(0, 0);
  if(scalex == NULL && scaley==NULL && angle == NULL){
    sprite88_roi.pushRotateZoom(&tft, x, y, 0, 1, 1, gethaco3Col(tc));
  }else  if(scalex != NULL && scaley!=NULL && angle == NULL){
    sprite88_roi.pushRotateZoom(&tft, x, y, 0, scalex, scaley, gethaco3Col(tc));
  }else  if(scalex != NULL && scaley!=NULL && angle != NULL){
    sprite88_roi.pushRotateZoom(&tft, x, y, angle, scalex, scaley, gethaco3Col(tc));
  }

  }
  return 0;
}


int runLuaGame::l_spr(lua_State* L){  
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));

  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  int w = lua_tointeger(L, 3);
  int h = lua_tointeger(L, 4);
  int sx = lua_tointeger(L, 5);
  int sy = lua_tointeger(L, 6);

  sx  /= 8;
  sy  /= 8;
  // int n = lua_tointeger(L, 1);
  // int x = lua_tointeger(L, 2);
  // int y = lua_tointeger(L, 3);
  // int gw = lua_tointeger(L, 4);
  // int gh = lua_tointeger(L, 5);
  // int scalex = lua_tointeger(L, 6);
  // int scaley = lua_tointeger(L, 7);
  // int angle = lua_tointeger(L, 8);

  // sprite64.pushRotateZoom(&tft, x, y, 0, 1, 1, TFT_BLACK);
  // sprite64.pushSprite(&tft, x, y);
  
  sprite88_roi.clear();//指定の大きさにスプライトを作り直す
  sprite88_roi.createSprite(w,h);

  int spr8numX = 8;//スプライトシートに並ぶｘ、ｙの個数
  int spr8numY = 8;

//キャラスプライト
  for(int y=0;y<8;y++){
      for(int x=0;x<8;x++){
        uint8_t bit4;
        int sprpos;
        //sprite64cnos[(sy*8) * PNG_SPRITE_WIDTH + (sx*8) 取得スタート位置
        //y*PNG_SPRITE_WIDTH + xスプライトのピクセル取得
        sprpos = (sy*8*PNG_SPRITE_WIDTH+sx*8 + y*PNG_SPRITE_WIDTH + x)/2;//４ビット二つで８ビットに入れてるので1/2に
        bit4 = sprite64cnos_vector[sprpos];
        if(x%2 == 1)bit4 = (bit4 & 0b00001111);
        if(x%2 == 0)bit4 = (bit4 >> 4);
        sprite88_roi.drawPixel(x,y, getcno2tftc(bit4));
      }
  }

  // sprite88_roi.setPivot(w/2.0, h/2.0);
  // sprite88_roi.pushRotateZoom(&tft, x, y, 0, 1, 1, TFT_BLACK);
  sprite88_roi.pushSprite(&tft, x, y);//4ずれない
  // sprite88_roi.pushRotateZoom(&tft, roix+n*8+4, roiy+m*8+4, 0, 1, 1, TFT_BLACK);//なぜか４を足さないとずれる(setPivot?)


  // if(scalex == NULL && scaley==NULL && angle == NULL){
  //   sprite88_roi.pushRotateZoom(&tft, x, y, 0, 1, 1, TFT_BLACK);
  // }else  if(scalex != NULL && scaley!=NULL && angle == NULL){
  //   sprite88_roi.pushRotateZoom(&tft, x, y, 0, scalex, scaley, TFT_BLACK);
  // }else  if(scalex != NULL && scaley!=NULL && angle != NULL){
  //   sprite88_roi.pushRotateZoom(&tft, x, y, angle, scalex, scaley, TFT_BLACK);
  // }

  return 0;
}



int runLuaGame::l_scroll(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);

  tft.scroll(x, y);
  return 0;
}


int runLuaGame::l_win(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int state = lua_tointeger(L, 1);
  int x = lua_tointeger(L, 2);
  int y = lua_tointeger(L, 3);
  int row = lua_tointeger(L, 4);
  int col = lua_tointeger(L, 5);
  int charmax = lua_tointeger(L, 6);
  int winID = lua_tointeger(L, 7);
  const char* title = lua_tostring(L, 8);
  
  if(state == 1){//可視化状態
    tft.setFont(&lgfxJapanGothicP_8);
    tft.fillRoundRect(x,y,(charmax*row+row+1)*8,(col+2)*8,4,gethaco3Col(1));
    
    for(int j = 0; j<col; j++){
      for(int i = 0; i<row; i++){
        tft.setTextColor(gethaco3Col(7));

        if(winID==0){
          tft.setCursor(x+8+(i*(charmax+1)*8), y+8+(j*8));
          tft.print("");
          if(strcmp(title, "どうする？") == 0){
            tft.print("ステテコ");
            }
          else if(strcmp(title, "どうぐ") == 0){
            tft.print("みかわしのふく");
            }
          else if(strcmp(title, "むらびと") == 0){
            // tft.setFont(&lgfxJapanGothicP_16);
            // tft.print("曖昧模糊な薔薇の轟");
            // j++;//2倍角なのでjに1足して繰り返し回数を半分にする
            tft.setFont(&lgfxJapanGothicP_8);
            tft.print("画数が少ない文字であれば、8pxでも良");
            // tft.print("曖昧模糊な薔薇の轟曖昧模糊な薔薇の轟");
          }
        }else if(winID==HPMP){
          tft.setFont(&fonts::Font0);
          tft.setCursor(x+4+(i*(charmax+1)*8), y+12+(j*8));
          if(j==0){tft.print("HP");}
          else if(j==1){tft.print("MP");}
          tft.print("999");
        }

        // for(int n = 0; n<charmax; n++){
        //   // tft.drawRect(x+8+(n*8)+(i*(charmax+1)*8), y+8+(j*8), 8,8, gethaco3Col(7));
          // tft.setTextColor(gethaco3Col(7));
          // tft.setCursor(x+8+(n*8)+(i*(charmax+1)*8), y+8+(j*8));
          // tft.print("あ");
        // }
      }
    }
    //ウインドウタイトルを描画
    tft.drawRoundRect(x+1,y+1,(charmax*row+row+1)*8-2,(col+2)*8-2, 4,gethaco3Col(7));//白い枠線
    tft.setTextColor(gethaco3Col(7),gethaco3Col(1));//文字背景あり
    tft.setTextSize(1);
    tft.setFont(&lgfxJapanGothicP_8);
    tft.setCursor(x+4, y);
    tft.println(title);

    // if(strcmp(title, "むらびと") == 0){
    //   tft.setCursor(x+8, y+8);
    //   tft.setClipRect(x+8, y+8, 144, 40);//文字以外のすべての描画にかかわるので最後に指定
    //   tft.setFont(&lgfxJapanGothicP_16);
    //   tft.print("曖昧模糊な薔薇の轟19文字も書ける");
    // }
  }

  // gethaco3Col(1);//紺
  // gethaco3Col(7);//白

  // if(tsize == NULL){
  //   tft.setTextSize(1);
  //   tft.setFont(&fonts::Font0);
  // }else if(tsize==8){
  //   tft.setTextSize(1);
  //   tft.setCursor(x,y);
  //   // tft.setTextColor(lua_rgb24to16(self->col[0], self->col[1], self->col[2]));//文字背景を消せる
  //   tft.setTextColor(lua_rgb24to16(self->col[0], self->col[1], self->col[2]),lua_rgb24to16(self->col2[0], self->col2[1], self->col2[2]));//文字背景を消せる
  //   tft.setFont(&lgfxJapanGothicP_8);
  // }else if(tsize==16){
  //   tft.setTextSize(1);
  //   tft.setCursor(x,y);
  //   // tft.setTextColor(lua_rgb24to16(self->col[0], self->col[1], self->col[2]));//文字背景を消せる
  //   tft.setTextColor(lua_rgb24to16(self->col[0], self->col[1], self->col[2]),lua_rgb24to16(self->col2[0], self->col2[1], self->col2[2]));//文字背景を消せる
  //   tft.setFont(&lgfxJapanGothicP_16);
  // }

  // if(x == NULL && y == NULL && tsize == NULL && cn == NULL){//位置指定なしの場合
  //   tft.setTextColor(TFT_WHITE,TFT_BLACK);//文字背景あり
  //   tft.setCursor(0,0);
  //   tft.setClipRect(0, 0, TFT_WIDTH, TFT_HEIGHT);
  // }else if(x != NULL && y != NULL && tsize == NULL && cn == NULL){//位置指定ありの場合
  //   tft.setTextColor(lua_rgb24to16(self->col[0], self->col[1], self->col[2]));//文字背景を消せる
  //   tft.setTextColor(TFT_WHITE,TFT_BLACK);//文字背景あり
  //   tft.setCursor(x,y);
  //   tft.setClipRect(x, y, TFT_WIDTH-x, TFT_HEIGHT-y);
  // }else if(x != NULL && y != NULL && tsize != NULL && cn == NULL){//位置指定、、色指定ありの場合
  //   tft.setTextColor(TFT_WHITE,TFT_BLACK);//文字背景あり
  //   tft.setCursor(x,y);
  //   tft.setClipRect(x, y, TFT_WIDTH-x, TFT_HEIGHT-y);
  // }else if(x != NULL && y != NULL && tsize != NULL && cn != NULL){//位置指定、、色指定ありの場合
  //   tft.setTextColor(lua_rgb24to16(self->col[0], self->col[1], self->col[2]));//文字背景を消せる
  //   tft.setCursor(x,y);
  //   tft.setClipRect(x, y, TFT_WIDTH-x, TFT_HEIGHT-y);
  // }else if(x != NULL && y != NULL && tsize != NULL && cn != NULL && cn2 != NULL){//位置指定、、色指定,背景色指定ありの場合
  //   tft.setTextColor(lua_rgb24to16(self->col[0], self->col[1], self->col[2]),lua_rgb24to16(self->col2[0], self->col2[1], self->col2[2]));//文字背景を消せる
  //   tft.setCursor(x,y);
  //   tft.setClipRect(x, y, TFT_WIDTH-x, TFT_HEIGHT-y);
  // }
  // tft.println(text);
  // tft.clearClipRect();
  // //描画後は設定をデフォルトに戻しておく
  // tft.setTextColor(TFT_WHITE,TFT_BLACK);//文字背景あり
  return 0;
}


bool drawMap() {
  int ColCursor = 0;
  uint8_t sprno;
  uint8_t presprno = 0; // 初期値を設定
  uint8_t repeatnum;

  for (uint8_t k = 0; k < 16; k++) {
    int j = k;
    // int j = (k+16+targetDispRow)%16;
    for (uint8_t i = 0; i < rowData[k].size(); i += 2) {
      sprno = rowData[k][i];
      repeatnum = rowData[k][i + 1];

      if (sprno == 3 && repeatnum == 31) {
        repeatnum = 0;
        ColCursor = 0;
      } else {
        if (sprno == 7) {
          if (gWy >= 220 || (gWx >= 49 && gWy <= 97)) {
            sprno = 15;
          }
        }

        int sx = sprno % gSpr8numX;
        int sy = sprno / gSpr8numY;

        for (int y = 0; y < 8; y++) {
          for (int x = 0; x < 8; x++) {
            uint8_t bit4;
            int sprpos;
            sprpos = (sy * 8 * PNG_SPRITE_WIDTH + sx * 8 + y * PNG_SPRITE_WIDTH + x) / 2;
            bit4 = sprite64cnos_vector[sprpos];
            if (x % 2 == 1) bit4 = (bit4 & 0b00001111);
            if (x % 2 == 0) bit4 = (bit4 >> 4);
            sprite88_roi.drawPixel(x, y, gethaco3Col(bit4));
          }
        }

        for (int n = 0; n < repeatnum; n++) {
          int sprx = ColCursor + n;
          int displaysprx = sprx - gWx;

          if (sprx >= gWx && sprx < gWx + DISP_SPR_W_NUM) {
            mapArray[j][displaysprx] = sprno;

            // 例: 描画対象が同じ場合は描画をスキップ
            // if (presprno != sprno) {
            //   sprite88_roi.setPivot(4.0, 4.0);
            //   sprite88_roi.pushSprite(&tft, (ColCursor + n - gWx) * 8, j * 8);
            // }

            // sprite88_roi.setPivot(gSprw / 2.0, gSprh / 2.0);
            // sprite88_roi.pushSprite(&tft, (ColCursor + n - gWx) * 8, j * 8);

            sprite88_roi.setPivot(4.0, 4.0);
            sprite88_roi.pushSprite(&tft, (ColCursor + n - gWx) * 8, j * 8);
            // sprite88_roi.pushSprite(&tft, (ColCursor + n - gWx) << 3, j << 3);
            
            // 海岸線の場合は描写を足す
            if (displaysprx != 0) {
              if (presprno == 0 && sprno != 0) { // 海から陸ならば
                tft.fillRect((ColCursor + n - gWx - 1) * 8 + 7, j * 8, 1, 8, gethaco3Col(7));
              }
              if (presprno != 0 && sprno == 0) { // 陸から海ならば
                tft.fillRect((ColCursor + n - gWx) * 8, j * 8, 1, 8, gethaco3Col(7));
              }
            }

            if (j != 0) {
              if (sprno != 0 && mapArray[j - 1][displaysprx] == 0) { // 陸の時上が海か
                tft.fillRect((ColCursor + n - gWx) * 8, (j - 1) * 8 + 7, 8, 1, gethaco3Col(7));
              }

              if (sprno == 0 && mapArray[j - 1][displaysprx] != 0) { // 海の時上が陸か
                tft.fillRect((ColCursor + n - gWx) * 8, j * 8, 8, 1, gethaco3Col(7));
              }
            }

            presprno = sprno;
          }
        }

        ColCursor += repeatnum;
      }
    }
  }

  return true;
}

bool readRowFromBinary2(const char *filename) {
  int _startRow = gWy;
  int nnum = 0;

  File mfile = SPIFFS.open(filename, "rb");

  if (!mfile) {
    // Serial.println("Error opening file");
    return false;
  }

  std::vector<std::vector<uint8_t>> new_rowData(16, std::vector<uint8_t>());

  // buffer のクリアとリサイズ
  buffer.clear();
  buffer.resize(8);

  while (nnum < _startRow + DISP_SPR_H_NUM) {
    bytesRead = mfile.read(buffer.data(), buffer.size());

    if (bytesRead == 0) {
      break;
    }

    for (int i = 0; i < bytesRead; i++) {
      sprno = buffer[i] >> 5;
      repeatnum = buffer[i] & 0b00011111;

      if (nnum >= _startRow && nnum < _startRow + DISP_SPR_H_NUM) {
        if (sprno == 0 && repeatnum == 0) {
          repeatnum = 255;
        }

        new_rowData[nnum - _startRow].push_back(sprno);
        new_rowData[nnum - _startRow].push_back(repeatnum);
      }

      if (sprno == 3 && repeatnum == 31) {
        ++nnum;
      }
    }
  }

  mfile.close();

  // rowData を new_rowData に入れ替える
  rowData.swap(new_rowData);

  mfile = File();  // 不要な行の削除

  // drawWaitF = false;
  return true;
}

int runLuaGame::l_rmap(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  String fn = lua_tostring(L, 1);
  gWx = lua_tointeger(L, 2);
  gWy = lua_tointeger(L, 3);
  mapFileName = fn;
  sprite88_roi.clear(); // 指定の大きさにスプライトを作り直す
  sprite88_roi.createSprite(gSprw, gSprh);

  while(!readRowFromBinary2(fn.c_str())){
    // delay(1);//1/50秒表示
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 20ミリ秒スリープ
  }
  
  while(!drawMap()){
    // delay(1);//1/50秒表示
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 20ミリ秒スリープ
  }

  return 0;
}

int runLuaGame::l_pset(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  float x = lua_tonumber(L, 1);
  float y = lua_tonumber(L, 2);
  int cn = lua_tointeger(L, 3);
  int cn2 = lua_tointeger(L, 4);
  int cn3 = lua_tointeger(L, 5);
  
  if(cn2 == NULL){
    tft.writePixel(x, y, cn);
  }

  if(cn3 != NULL)
  {
    self->col[0] = cn; // 5bit
    self->col[1] = cn2; // 6bit
    self->col[2] = cn3; // 5bit
    tft.drawPixel(x, y, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  }
  return 0;
}

int runLuaGame::l_pget(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);

  uint16_t c = tft.readPixel(x, y);

  uint8_t index = 0;
  // for(unsigned int pi = 0; pi < 256; pi ++){
  //   if(self->palette[pi] == c){
  //     index = pi;
  //     break;
  //   }
  // }
  uint8_t r = ((c >> 11) << 3); // 5bit
  uint8_t g = (((c >> 5) & 0b111111) << 2); // 6bit
  uint8_t b = ((c & 0b11111) << 3);       // 5bit

  lua_pushinteger(L, (lua_Integer)r);
  lua_pushinteger(L, (lua_Integer)g);
  lua_pushinteger(L, (lua_Integer)b);
  lua_pushinteger(L, (lua_Integer)index);
  return 4;
}

extern uint8_t clist2[16][3];
// extern uint8_t clist_p0[16][3];
// extern uint8_t clist_p1[16][3];

int runLuaGame::l_color(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int r,g,b;
  r = lua_tointeger(L, 1);
  g = lua_tointeger(L, 2);
  b = lua_tointeger(L, 3);
    //とにかく一回格納する
  self->col[0] = r;
  self->col[1] = g;
  self->col[2] = b;
  //色番号だったら上書き
  if(g == NULL && b == NULL){
    int cn = lua_tointeger(L, 1);
    self->col[0] = clist2[cn][0]; // 5bit
    self->col[1] = clist2[cn][1]; // 6bit
    self->col[2] = clist2[cn][2]; // 5bit
  }
  return 0;
}


// int runLuaGame::l_color(lua_State* L){
//   runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
//   int r,g,b;
//   r = lua_tointeger(L, 1);
//   g = lua_tointeger(L, 2);
//   b = lua_tointeger(L, 3);
//     //とにかく一回格納する
//   self->col[0] = r;
//   self->col[1] = g;
//   self->col[2] = b;
//   //色番号だったら上書き
//   if(g == NULL && b == NULL){
//     int cn = lua_tointeger(L, 1);
//     self->col[0] = clist2[cn][0]; // 5bit
//     self->col[1] = clist2[cn][1]; // 6bit
//     self->col[2] = clist2[cn][2]; // 5bit
//   }
//   return 0;
// }

int runLuaGame::l_text(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  const char* text = lua_tostring(L, 1);
  int x = lua_tointeger(L, 2);
  int y = lua_tointeger(L, 3);
  tft.setCursor(x,y);
  tft.setTextColor(lua_rgb24to16(self->col[0], self->col[1], self->col[2]));

  tft.setTextSize(1);//サイズ
  tft.setFont(&lgfxJapanGothic_12);//日本語可,等幅

  tft.print(text);
  return 0;
}




int runLuaGame::l_opmode(lua_State* L){//FAST,WIDE
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int _n = lua_tointeger(L, 1);
  outputMode = _n;
  return 0;
}

int runLuaGame::l_drawrect(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  float x = lua_tonumber(L, 1);
  float y = lua_tonumber(L, 2);
  float w = lua_tonumber(L, 3);
  float h = lua_tonumber(L, 4);
  int cn = lua_tointeger(L, 5);
  
  if(cn != NULL)
  {
    self->col[0] = clist2[cn][0]; // 5bit
    self->col[1] = clist2[cn][1]; // 6bit
    self->col[2] = clist2[cn][2]; // 5bit
  }

  tft.drawRect(x, y, w, h, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  return 0;
}

int runLuaGame::l_drawbox(lua_State* L) {
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  
  float px = lua_tonumber(L, 1);
  float py = lua_tonumber(L, 2);
  float x = lua_tonumber(L, 3);
  float y = lua_tonumber(L, 4);
  float z = lua_tonumber(L, 5);
  int cn = lua_tointeger(L, 6);
  
  if (cn != 0) { // NULL ではなく 0 と比較することを推奨します
    self->col[0] = clist2[cn][0]; // 5bit
    self->col[1] = clist2[cn][1]; // 6bit
    self->col[2] = clist2[cn][2]; // 5bit
  }

  self->boxzerox = px;
  self->boxzeroy = py;

  // float unit = 10;
  float rad = atan(0.5);
  const Vector3<float> a(9, 4.5, z);
  const Vector3<float> b(-9, 4.5, z);
  const Vector3<float> c(a.x + b.x, a.y + b.y, a.z + b.z); // ベクトル合成
  const Vector3<float> o(x * cos(rad) - y * cos(rad) + self->boxzerox, x * sin(rad) + y * sin(rad) + self->boxzeroy, z); // クオータービュー（x座標・y座標反転）

  // self->fillFastTriangle(o.x, o.y, o.x - a.x, o.y + a.y, o.x - c.x, o.y + c.y, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  // self->fillFastTriangle(o.x, o.y, o.x - b.x, o.y + b.y, o.x - c.x, o.y + c.y, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));

  self->fillFastTriangle(o.x, o.y-z, o.x - a.x, o.y + a.y-z, o.x - c.x, o.y + c.y-z, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  self->fillFastTriangle(o.x, o.y-z, o.x - b.x, o.y + b.y-z, o.x - c.x, o.y + c.y-z, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));

  self->fillFastTriangle(o.x - c.x, o.y + c.y-z,  o.x - a.x, o.y + a.y-z, o.x - c.x, o.y + c.y, lua_rgb24to16(50, 50, 100));
  self->fillFastTriangle(o.x - a.x, o.y + a.y-z, o.x - a.x, o.y + a.y   , o.x - c.x, o.y + c.y, lua_rgb24to16(50, 50, 100));

  self->fillFastTriangle(o.x - c.x, o.y + c.y-z, o.x - b.x, o.y + b.y-z, o.x - c.x, o.y + c.y,    lua_rgb24to16(100, 100, 100));
  self->fillFastTriangle(o.x - b.x, o.y + b.y,    o.x - c.x, o.y + c.y,    o.x - b.x, o.y + b.y-z, lua_rgb24to16(100, 100, 100));

  // tft.drawRect(x+50, y+50, w, h, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));

  return 0;
}

int runLuaGame::l_drawboxp(lua_State* L) {
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  
  float x = lua_tonumber(L, 1);
  float y = lua_tonumber(L, 2);
  float z = lua_tonumber(L, 3);

  float rad = atan(0.5);
  const Vector3<float> a(9, 4.5, z);
  const Vector3<float> b(-9, 4.5, z);
  const Vector3<float> c(a.x + b.x, a.y + b.y, a.z + b.z); // ベクトル合成
  const Vector3<float> o(x * cos(rad) - y * cos(rad) + self->boxzerox, x * sin(rad) + y * sin(rad) + self->boxzeroy, z); // クオータービュー（x座標・y座標反転）

  // self->fillFastTriangle(o.x, o.y-z, o.x - a.x, o.y + a.y-z, o.x - c.x, o.y + c.y-z, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  // self->fillFastTriangle(o.x, o.y-z, o.x - b.x, o.y + b.y-z, o.x - c.x, o.y + c.y-z, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  self->fillFastTriangle(o.x, o.y-z, o.x - a.x, o.y + a.y-z, o.x - c.x, o.y + c.y-z, lua_rgb24to16(0, 0, 50));
  self->fillFastTriangle(o.x, o.y-z, o.x - b.x, o.y + b.y-z, o.x - c.x, o.y + c.y-z, lua_rgb24to16(0, 0, 50));

  // Create a Lua table
lua_newtable(L);

// Set the values of the table
lua_pushnumber(L, o.x);
lua_setfield(L, -2, "x");

lua_pushnumber(L, o.y-z);
lua_setfield(L, -2, "y");

lua_pushnumber(L, o.z);
lua_setfield(L, -2, "z");

// テーブルのサイズをスタックに積む
lua_pushinteger(L, 3);
return 2;
}


int runLuaGame::l_oval(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  int rx = lua_tointeger(L, 3);
  int ry = lua_tointeger(L, 4);
  int cn = lua_tointeger(L, 5);
  if(cn != NULL)
  {
    hacol0 =  gethaco3Col(cn);
  }
  tft.drawEllipse(x, y, rx, ry, hacol0);
  return 0;
}

// int runLuaGame::l_fps(lua_State* L){
//   runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
//   fps = lua_tointeger(L, 1);
// }

int runLuaGame::l_fillp(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int cn0 = lua_tointeger(L, 1);
  int cn1 = lua_tointeger(L, 2);
  int cn2 = lua_tointeger(L, 3);
  int cn3 = lua_tointeger(L, 4);

  hacol0 =  gethaco3Col(cn0);
  hacol1 =  gethaco3Col(cn1);

  if(cn2==NULL){
    hacol2 =  gethaco3Col(cn0);
    hacol3 =  gethaco3Col(cn1);
  }
  
  if(cn2!=NULL){
    hacol2 =  gethaco3Col(cn2);
  }
  if(cn3!=NULL){
    hacol3 =  gethaco3Col(cn3);
  }
  return 0;
}

int runLuaGame::l_ovalfill(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  int rx = lua_tointeger(L, 3);
  int ry = lua_tointeger(L, 4);
  int cn0 = lua_tointeger(L, 5);
  int cn1 = lua_tointeger(L, 6);

  if(cn0 != NULL)
    {
      hacol0 = gethaco3Col(cn0);
      hacol1 = gethaco3Col(cn0);//仮に入れておく
      hacol2 = gethaco3Col(cn0);//仮に入れておく
      hacol3 = gethaco3Col(cn0);//仮に入れておく
    }
    
    if(cn1 != NULL)
    {
      hacol0 = gethaco3Col(cn0);
      hacol1 = gethaco3Col(cn1);
      hacol2 = gethaco3Col(cn0);
      hacol3 = gethaco3Col(cn1);
    }

    int32_t xt, yt, i;
    int32_t rx2 = rx * rx;
    int32_t ry2 = ry * ry;
    int32_t s;

    tft.startWrite();

    if(y%4==0)tft.setColor(hacol0);
    else if(y%4==1)tft.setColor(hacol1);
    else if(y%4==2)tft.setColor(hacol2);
    else if(y%4==3)tft.setColor(hacol3);
    tft.writeFastHLine(x - rx, y, (rx << 1) + 1);//まんなかの１本
    
    i = 0;
    yt = 0;
    xt = rx;
    s = (rx2 << 1) + ry2 * (1 - (rx << 1));

    do {
      while (s < 0) s += rx2 * ((++yt << 2) + 2);

      for (int32_t k = 0; k < (yt - i); k++) {

        if(((y - yt)+k)%4==0)tft.setColor(hacol0);
        else if(((y - yt)+k)%4==1)tft.setColor(hacol1);
        else if(((y - yt)+k)%4==2)tft.setColor(hacol2);
        else if(((y - yt)+k)%4==3)tft.setColor(hacol3);
        tft.writeFastHLine(x - xt, (y - yt)+k, (xt << 1) + 1);

        if(((y + i + 1)+k)%4==0)tft.setColor(hacol0);
        else if(((y + i + 1)+k)%4==1)tft.setColor(hacol1);
        else if(((y + i + 1)+k)%4==2)tft.setColor(hacol2);
        else if(((y + i + 1)+k)%4==3)tft.setColor(hacol3);
        tft.writeFastHLine(x - xt, (y + i + 1)+k, (xt << 1) + 1);
      }

      i = yt;
      s -= (--xt) * ry2 << 2;
    } while (rx2 * yt <= ry2 * xt);

    xt = 0;
    yt = ry;
    s = (ry2 << 1) + rx2 * (1 - (ry << 1));
    do {
      while (s < 0) s += ry2 * ((++xt << 2) + 2);
        if((y - yt)%4==0)tft.setColor(hacol0);
        else if((y - yt)%4==1)tft.setColor(hacol1);
        else if((y - yt)%4==2)tft.setColor(hacol2);
        else if((y - yt)%4==3)tft.setColor(hacol3);
        tft.writeFastHLine(x - xt, y - yt, (xt << 1) + 1);

        if((y + yt)%4==0)tft.setColor(hacol0);
        else if((y + yt)%4==1)tft.setColor(hacol1);
        else if((y + yt)%4==2)tft.setColor(hacol2);
        else if((y + yt)%4==3)tft.setColor(hacol3);
        tft.writeFastHLine(x - xt, y + yt, (xt << 1) + 1);
        s -= (--yt) * rx2 << 2;
    } while(ry2 * xt <= rx2 * yt);

    tft.endWrite();
  return 0;
}


int runLuaGame::l_fillpoly(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  float x0 = lua_tonumber(L, 1);
  float y0 = lua_tonumber(L, 2);
  float x1 = lua_tonumber(L, 3);
  float y1 = lua_tonumber(L, 4);
  float x2 = lua_tonumber(L, 5);
  float y2 = lua_tonumber(L, 6);
  float x3 = lua_tonumber(L, 7);
  float y3 = lua_tonumber(L, 8);
  int cn = lua_tointeger(L, 9);

  self->col[0] = clist2[cn][0]; // 5bit
  self->col[1] = clist2[cn][1]; // 6bit
  self->col[2] = clist2[cn][2]; // 5bit

  self->fillFastTriangle(x0,y0,x1,y1,x2,y2,lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  self->fillFastTriangle(x2,y2,x3,y3,x0,y0,lua_rgb24to16(self->col[0], self->col[1], self->col[2]));

  // tft.fillTriangle( x0,y0,x1,y1,x2,y2);
  // tft.fillTriangle( x2,y2,x3,y3,x0,y0);
  return 0;

}

int runLuaGame::l_cls(lua_State* L) {
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int n = lua_tointeger(L, 1);

  uint8_t r, g, b;
  if (n >= 0 && n < sizeof(clist2) / sizeof(clist2[0])) {
    r = clist2[n][0];
    g = clist2[n][1];
    b = clist2[n][2];
  } else {
    r = g = b = 0; // Default to black color if n is out of range
  }

  self->col[0] = r; // 5bit
  self->col[1] = g; // 6bit
  self->col[2] = b; // 5bit

  tft.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  // tft.fillScreen(lua_rgb24to16(self->col[0], self->col[1], self->col[2]));

  return 0;
}

int runLuaGame::l_fillrect(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int x = lua_tointeger(L, 1);
  int y = lua_tointeger(L, 2);
  int w = lua_tointeger(L, 3);
  int h = lua_tointeger(L, 4);
  int cn = lua_tointeger(L, 5);


  
  if(cn != NULL)
  {
    // self->col[0] = clist2[cn][0]; // 5bit
    // self->col[1] = clist2[cn][1]; // 6bit
    // self->col[2] = clist2[cn][2]; // 5bit

    tft.fillRect(x, y, w, h, gethaco3Col(cn));
  }else{
    tft.fillRect(x, y, w, h, gethaco3Col(8));
    // tft.fillRect(x, y, w, h, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  }

  return 0;
}


extern bool enemyF;
extern uint8_t enemyX;
extern uint8_t enemyY;
extern uint8_t enemyTransCn;
extern String enemyPath;

int runLuaGame::l_rectfill(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int xa = lua_tointeger(L, 1);
  int ya = lua_tointeger(L, 2);
  int xb = lua_tointeger(L, 3);
  int yb = lua_tointeger(L, 4);
  int cn = lua_tointeger(L, 5);
  String fn = lua_tostring(L, 6);
  if(fn == NULL){
    if(cn != NULL)
    {
    self->col[0] = clist2[cn][0]; // 5bit
    self->col[1] = clist2[cn][1]; // 6bit
    self->col[2] = clist2[cn][2]; // 5bit
    }
    tft.fillRect(xa, ya, xb, yb, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  }else if(fn != NULL){//cnを透明対応色にしたい、、、
    enemyF = true;
    enemyPath = fn;
    enemyX = 0;
    enemyY = 32;
    enemyX = xa;
    enemyY = ya;
    enemyTransCn = cn;
  }
  return 0;
}

// int runLuaGame::l_fillrect(lua_State* L){
//   runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
//   float x = lua_tonumber(L, 1);
//   float y = lua_tonumber(L, 2);
//   float w = lua_tonumber(L, 3);
//   float h = lua_tonumber(L, 4);
//   int cn = lua_tointeger(L, 5);
//   int cn2 = lua_tointeger(L, 6);
//   int cn3 = lua_tointeger(L, 7);
//   int cmode = lua_tointeger(L, 8);

//   if(cn != NULL)
//   {
//     self->col[0] = clist2[cn][0]; // 5bit
//     self->col[1] = clist2[cn][1]; // 6bit
//     self->col[2] = clist2[cn][2]; // 5bit
//   }

//   if(cn3 != NULL)
//   {
//     self->col[0] = cn; // 5bit
//     self->col[1] = cn2; // 6bit
//     self->col[2] = cn3; // 5bit
//   }

//   if(cmode!=NULL||cmode==1){//cmode1のときはｈｓｂ
//     // HSBからRGBに変換
//     int r, g, b;
//     self->hsbToRgb(cn, cn2, cn3, r, g, b);

//     // RGB値を設定
//     self->col[0] = r; // Red
//     self->col[1] = g; // Green
//     self->col[2] = b; // Blue

//     tft.fillRect(x, y, w, h, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
//   }
//   else
//   {
//     tft.fillRect(x, y, w, h, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
//   }

//   return 0;
// }

int runLuaGame::l_drawtri(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int x0 = lua_tointeger(L, 1);
  int y0 = lua_tointeger(L, 2);
  int x1 = lua_tointeger(L, 3);
  int y1 = lua_tointeger(L, 4);
  int x2 = lua_tointeger(L, 5);
  int y2 = lua_tointeger(L, 6);
  int cn = lua_tointeger(L, 7);
  int cn2 = lua_tointeger(L, 8);
  int cn3 = lua_tointeger(L, 9);
  if(cn != NULL)
  {
    self->col[0] = clist2[cn][0]; // 5bit
    self->col[1] = clist2[cn][1]; // 6bit
    self->col[2] = clist2[cn][2]; // 5bit
  }
  if(cn3 != NULL)
  {
    self->col[0] = cn; // 5bit
    self->col[1] = cn2; // 6bit
    self->col[2] = cn3; // 5bit
  }
  tft.drawTriangle(x0, y0, x1, y1, x2, y2, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  return 0;
}

void runLuaGame::hsbToRgb2(float angle, float br, int& r, int& g, int& b) {
  if (angle != -1) {
    float hue = angle / 360.0f;  // 角度を0から1の範囲に正規化

    // 色相に応じて彩度を変化させる
    float si = 0.5f + 0.5f * cos(hue * 2 * M_PI);

    float hueNormalized = hue * 6.0f;
    int i = static_cast<int>(hueNormalized);
    float f = hueNormalized - i;

    float p = br * (1.0f - si);
    float q = br * (1.0f - si * f);
    float t = br * (1.0f - si * (1.0f - f));

    int br255 = static_cast<int>(br);
    int p255 = static_cast<int>(p);
    int q255 = static_cast<int>(q);
    int t255 = static_cast<int>(t);

    switch (i) {
      // ... 同じ
    }
  } else { // 無彩色指定の場合
      r = br;
      g = br;
      b = br;
  }
}

void runLuaGame::hsbToRgb(float angle, float si, float br, int& r, int& g, int& b) {

  if(angle!=-1){
  float hue = angle / 360.0f;  // 角度を0から1の範囲に正規化

  si = 1.0f;  // 彩度を常に最大値として扱う

  if (si == 0) {
    // S彩度が0の場合、色相に関係なく明度がそのままRGB値となる
    r = g = b = static_cast<int>(br);
    return;
  }

  float hueNormalized = hue * 6.0f;
  int i = static_cast<int>(hueNormalized);
  float f = hueNormalized - i;
  float p = br * (1.0f - si);
  float q = br * (1.0f - si * f);
  float t = br * (1.0f - si * (1.0f - f));

  int br255 = static_cast<int>(br);
  int p255 = static_cast<int>(p);
  int q255 = static_cast<int>(q);
  int t255 = static_cast<int>(t);

  switch (i) {
    case 0:
      r = br255;
      g = t255;
      b = p255;
      break;
    case 1:
      r = q255;
      g = br255;
      b = p255;
      break;
    case 2:
      r = p255;
      g = br255;
      b = t255;
      break;
    case 3:
      r = p255;
      g = q255;
      b = br255;
      break;
    case 4:
      r = t255;
      g = p255;
      b = br255;
      break;
    case 5:
      r = br255;
      g = p255;
      b = q255;
      break;
  }
  }else{//無彩色指定の場合
      r = br;
      g = br;
      b = br;
  }
}

void runLuaGame::fillFastTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t c1){
// void runLuaGame::fillFastTriangle(float x0, float y0, float x1, float y1, float x2, float y2, uint16_t c1){

  tft.setColor(c1);
  int16_t a, b;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }
    if (y1 > y2) { std::swap(y2, y1); std::swap(x2, x1); }
    if (y0 > y1) { std::swap(y0, y1); std::swap(x0, x1); }

    if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
      a = b = x0;
      if (x1 < a)      a = x1;
      else if (x1 > b) b = x1;
      if (x2 < a)      a = x2;
      else if (x2 > b) b = x2;
      tft.drawFastHLine(a, y0, b - a + 1);
      return;
    }
    if ((x1-x0) * (y2-y0) == (x2-x0) * (y1-y0)) {
      tft.drawLine(x0,y0,x2,y2);
      return;
    }

    int16_t dy1 = y1 - y0;
    int16_t dy2 = y2 - y0;
    bool change = ((x1 - x0) * dy2 > (x2 - x0) * dy1);
    int16_t dx1 = abs(x1 - x0);
    int16_t dx2 = abs(x2 - x0);
    int16_t xstep1 = x1 < x0 ? -1 : 1;
    int16_t xstep2 = x2 < x0 ? -1 : 1;
    a = b = x0;
    if (change) {
      std::swap(dx1, dx2);
      std::swap(dy1, dy2);
      std::swap(xstep1, xstep2);
    }
    int16_t err1 = (std::max(dx1, dy1) >> 1)
                 + (xstep1 < 0
                   ? std::min(dx1, dy1)
                   : dx1);
    int16_t err2 = (std::max(dx2, dy2) >> 1)
                 + (xstep2 > 0
                   ? std::min(dx2, dy2)
                   : dx2);
    tft.startWrite();
    if (y0 != y1) {
      do {
        err1 -= dx1;
        while (err1 < 0) { err1 += dy1; a += xstep1; }
        err2 -= dx2;
        while (err2 < 0) { err2 += dy2; b += xstep2; }
        // if (y0 % 2 != 0) { // 奇数の行のみを処理
          tft.writeFastHLine(a, y0, b - a + 1);
        // }
      } while (++y0 < y1);
    }

    if (change) {
      b = x1;
      xstep2 = x2 < x1 ? -1 : 1;
      dx2 = abs(x2 - x1);
      dy2 = y2 - y1;
      err2 = (std::max(dx2, dy2) >> 1)
           + (xstep2 > 0
             ? std::min(dx2, dy2)
             : dx2);
    } else {
      a = x1;
      dx1 = abs(x2 - x1);
      dy1 = y2 - y1;
      xstep1 = x2 < x1 ? -1 : 1;
      err1 = (std::max(dx1, dy1) >> 1)
           + (xstep1 < 0
             ? std::min(dx1, dy1)
             : dx1);
    }
    do {
      err1 -= dx1;
      while (err1 < 0) { err1 += dy1; if ((a += xstep1) == x2) break; }
      err2 -= dx2;
      while (err2 < 0) { err2 += dy2; if ((b += xstep2) == x2) break; }
      // if (y0 % 2 != 0) { // 奇数の行のみを処理
        tft.writeFastHLine(a, y0, b - a + 1);
      // }
    } while (++y0 <= y2);
    tft.endWrite();
}


int runLuaGame::l_filltri(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int x0 = lua_tointeger(L, 1);
  int y0 = lua_tointeger(L, 2);
  int x1 = lua_tointeger(L, 3);
  int y1 = lua_tointeger(L, 4);
  int x2 = lua_tointeger(L, 5);
  int y2 = lua_tointeger(L, 6);
  int cn = lua_tointeger(L, 7);
  int cn2 = lua_tointeger(L, 8);
  int cn3 = lua_tointeger(L, 9);
  int cmode = lua_tointeger(L, 10);
  if(cn != NULL)
  {
    self->col[0] = clist2[cn][0]; // 5bit
    self->col[1] = clist2[cn][1]; // 6bit
    self->col[2] = clist2[cn][2]; // 5bit
  }

  if(cn3 != NULL)
  {
    self->col[0] = cn; // 5bit
    self->col[1] = cn2; // 6bit
    self->col[2] = cn3; // 5bit
  }

  
  if(cmode!=NULL||cmode==1){//cmode1のときはｈｓｂ
    // HSBからRGBに変換
    int r, g, b;
    self->hsbToRgb(cn, cn2, cn3, r, g, b);

    // RGB値を設定
    self->col[0] = r; // Red
    self->col[1] = g; // Green
    self->col[2] = b; // Blue

    self->fillFastTriangle(x0, y0, x1, y1, x2, y2, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  }
  else
  {
    self->fillFastTriangle(x0, y0, x1, y1, x2, y2, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  }
  
  return 0;
}

int runLuaGame::l_fillcircle(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));

  float x = lua_tonumber(L, 1);
  float y = lua_tonumber(L, 2);
  float r = lua_tonumber(L, 3);

  tft.fillCircle(x, y, r, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  return 0;
}

int runLuaGame::l_drawcircle(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));

  float x = lua_tonumber(L, 1);
  float y = lua_tonumber(L, 2);
  float r = lua_tonumber(L, 3);

  tft.drawCircle(x, y, r, lua_rgb24to16(self->col[0], self->col[1], self->col[2]));
  return 0;
}


int runLuaGame::l_key(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));

  lua_pushstring(L, &keychar);
  return 1;
}

int runLuaGame::l_btn(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int n = lua_tointeger(L, 1);
  int n2 = lua_tointeger(L, 2);
  float addval = lua_tonumber(L, 3);
  float val = lua_tonumber(L, 4);

  if(n2 == NULL){
    lua_pushinteger(L, (lua_Integer)buttonState[n]);
  }else{
    if (buttonState[n] >= 2) {
      lua_pushnumber(L, val + addval);
    } 
    else if (buttonState[n2] >= 2) {
      lua_pushnumber(L, val - addval);
    }
  }
  return 1;
}

int runLuaGame::l_touch(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  // int n = lua_tointeger(L, 1);
  // lua_pushinteger(L, (lua_Integer)self->touchState);
  return 1;
}

void getPalValue(uint8_t palNo){
  //外部カラーパレットを読み込む
  File fr = SPIFFS.open("/init/param/pal/" + String(palNo) + ".txt", "r");
  String line;
  uint8_t j = 0;
  while (fr.available()) {
    line = fr.readStringUntil('\n');
    
    if (!line.isEmpty()) {
      int commaIndex = line.indexOf(',');
      if (commaIndex != -1) {
        String val = line.substring(0, commaIndex);
        clist2[j][0] = val.toInt();
        // Serial.print(val);
        // Serial.print(",");
          for (int i = 1; i < 3; i++) {
            int nextCommaIndex = line.indexOf(',', commaIndex + 1);
            if (nextCommaIndex != -1) {
              val = line.substring(commaIndex + 1, nextCommaIndex);
              // Serial.print(val);
              // Serial.print(",");
              clist2[j][i] = val.toInt(); // uint8_tに直接キャストする必要はありません
              commaIndex = nextCommaIndex;
            }
          }
      }
      j++;
      // Serial.println("");
    }
  }
  fr.close();
}

int runLuaGame::l_pal(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int palno = lua_tointeger(L, 1);
  getPalValue(palno);
  return 0;
}


int runLuaGame::l_btnp(lua_State* L)
{
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int n = lua_tointeger(L, 1);

  if(buttonState[n] <= 15){
    if(buttonState[n]==1){lua_pushboolean(L, true);}//最初の１だけtrue
    else{lua_pushboolean(L, false);}
  }else{
    if(buttonState[n]%4 == 0){
      // Serial.println("定期的にtrue");
      lua_pushboolean(L, true);
    }else{
      lua_pushboolean(L, false);
    }
  }
  return 1;
}

int runLuaGame::l_sldr(lua_State* L)
{
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int xy = lua_tointeger(L, 1);
  
  // if(xy==0||xy==1){
    optionuiflag = true;
    lua_pushnumber(L, sliderval[xy]);
  // }
  return 1;
}

// int runLuaGame::l_iswifidebug(lua_State* L){
//   runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));

//   lua_pushboolean(L, isWifiDebug()?1:0);
//   return 1;
// }

// int runLuaGame::l_getip(lua_State* L){
//   runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
//   char buf[16];
//   IPAddress ip = WiFi.localIP();
//   if(self->wifiDebugSelf){
//     ip = WiFi.softAPIP();
//   }
//   sprintf(buf, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3] );

//   lua_pushstring(L, buf);
//   return 1;
// }


// int runLuaGame::l_wifiserve(lua_State* L){
//   runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
//   self->wifiDebugSelf = false;
//   if(lua_gettop(L) == 1){
//     const char* text = lua_tostring(L, 1);
//     if(strncmp(text, "ap", 3) == 0){
//       self->wifiDebugSelf = true;
//     }
//   }
//   self->wifiDebugRequest = true;
//   return 0;
// }

String nowappfileName = "";
int runLuaGame::l_run(lua_State* L){//ファイル名を取得して、そのファイルを実行runする

  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  // const char* file = lua_tostring(L, 1);
  appfileName = lua_tostring(L, 1);
  Serial.println(appfileName);

  // setFileName(file);//次のゲームのパスをセット
  self->exitRequest = true;//次のゲームを立ち上げるフラグを立てる
  nowappfileName = appfileName;
  setOpenConfig(appfileName,0);//configに書き込んでおく
  return 0;
}

int runLuaGame::l_appmode(lua_State* L){//ファイル名を取得して、そのファイルを実行runする
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  const char* file = lua_tostring(L, 1);
  const int modeno = lua_tointeger(L, 2);

  Serial.print(file);
  Serial.print("][");
  Serial.println(savedAppfileName);

  if(savedAppfileName != file){//違うゲームファイルを開こうとしていたら
    // editor.setCursor(0,0,0);//カーソルの座標をリセット
    // editor.setEditorConfig();
    difffileF = true;
  }

  // appfileName = file;//開くファイルを更新しておく

  if(modeno==2){setOpenConfig(file,2);delay(100); reboot(file, modeno);}
  else {restart(file, modeno);}
  
  return 0;
}

int runLuaGame::l_appinfo(lua_State* L){//ファイル名を取得して、そのファイルを実行runする
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int infono = lua_tointeger(L, 1);
  if(infono == 0){
    appfileName = nowappfileName;
    // setOpenConfig(appfileName,1);//configに書き込んでおく
    const char *lineChar = appfileName.c_str();
    lua_pushstring(L, lineChar);//引数０の時は名前
  }
  return 1;
}


int runLuaGame::l_editor(lua_State* L){//ファイル名を取得して、そのファイルを実行runする
  return 0;
}

int runLuaGame::l_list(lua_State* L) {
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  int modeno = lua_tointeger(L, 1);

  if(modeno == -1){
  File f;
  // lua_newtable(L);
  // ファイル数をあらかじめカウント
  File root = SPIFFS.open("/");
  f = root.openNextFile();
  int firstCountNo = 0;
  int fileCount = firstCountNo; // ファイル数をカウントするための変数を初期化

  while (f) {
    String filePath = f.path(); // ファイルパスを取得
    
    // フォルダ名を取得
    String folderName = filePath.substring(0, filePath.lastIndexOf('/'));
    // フォルダ名が "/init/" で始まる場合はスキップする
    if (folderName.startsWith("/init/")) {
      f = root.openNextFile();
      continue;
    }

    if (filePath.endsWith(".lua") || filePath.endsWith(".js") || filePath.endsWith("caldata.txt")) { // 拡張子が ".lua"" または ".js" または "caldata.txt" の場合のみ処理
      self->fileNamelist.push_back(filePath);
      // lua_pushstring(L, filePath.c_str()); // パスを文字列にしてリターン
      // lua_rawseti(L, -2, fileCount);
      fileCount++; // ファイル数をインクリメント

    }
    f = root.openNextFile();
  }
  f.close();
  root.close();

  lua_pushinteger(L, fileCount);//ファイル数を返す

  }else if(modeno >= 0){//モードが0以上の時
    // Serial.println(modeno);
    appfileName = self->fileNamelist[modeno];
    lua_pushstring(L, appfileName.c_str());
    
  }

  // テーブルはスタック上に残すため、ここでは解放しない
  return 1; // 1を返してテーブルの数を指定
}


// int runLuaGame::l_require(lua_State* L){
//   bool loadError = false;
//   runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
//   const char* fname = lua_tostring(L, 1);
//   File fp = SPIFFS.open(fname, FILE_READ);

//   struct LoadF lf;
//   lf.f = fp;
//   char cFileName[32];
//   appfileName.toCharArray(cFileName, 32);
//   if(lua_load(L, getF, &lf, cFileName, NULL)){
//     printf("error? %s\n", lua_tostring(L, -1));
//     Serial.printf("error? %s\n", lua_tostring(L, -1));
//     loadError = true;
//   }
//   fp.close();

//   if(loadError == false){
//     if(lua_pcall(L, 0, 1, 0)){
//       Serial.printf("init error? %s\n", lua_tostring(L, -1));

//     }
//   }

//   Serial.println("finish require");


//   return 1;
// }

// int runLuaGame::l_httpsget(lua_State* L){
//   runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
//   const char* host = lua_tostring(L, 1);
//   const char* path = lua_tostring(L, 2);
//   WiFiClientSecure client;
//   const int httpsPort = 443;
//   // Serial.println(host);
//   // Serial.println(path);
//   if(!client.connect(host, httpsPort)){
//     // connection failed
//     Serial.println("connect failed");
//   }
//   client.print(String("GET ") + path + " HTTP/1.1\r\n" +
//     "Host: " + host + "\r\n" +
//     "User-Agent: o-bako\r\n" +
//     "Connection: close\r\n\r\n"
//   );
//   String line;
//   while(client.connected()){
//     line = client.readStringUntil('\n');
//     if(line == "\r"){
//       // headers recieved
//       Serial.println("headers recieved");
//       break;
//     }
//   }
//   line = client.readString();
//   int lineLength = line.length();
//   const char *lineChar = line.c_str();

//   lua_pushstring(L, lineChar);
//   Serial.println("done");
//   return 1;
// }

// int runLuaGame::l_httpsgetfile(lua_State* L){
//   runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
//   const char* host = lua_tostring(L, 1);
//   const char* path = lua_tostring(L, 2);
//   const char* filePath = lua_tostring(L, 3);
//   WiFiClientSecure client;
//   const int httpsPort = 443;
//   if(!client.connect(host, httpsPort)){
//     // connection failed
//     Serial.println("connect failed");
//   }
//   client.print(String("GET ") + path + " HTTP/1.1\r\n" +
//     "Host: " + host + "\r\n" +
//     "User-Agent: o-bako\r\n" +
//     "Connection: close\r\n\r\n"
//   );
//   String line;
//   int len = 0;
//   while(client.connected()){
//     line = client.readStringUntil('\n');
//     if(line == "\r"){
//       // headers recieved
//       Serial.println("headers recieved");
//       break;
//     }
//     if(line.startsWith("Content-Length: ")){
//       len = line.substring(16).toInt();
//     }

//   }

//   tunes.pause();
//   File f = SPIFFS.open(filePath, FILE_WRITE);
//   while(client.available() && len > 0){
//     char c = client.read();
//     f.write(c);
//     len --;
//   }
//   f.close();
//   tunes.resume();
//   return 0;
// }

// int runLuaGame::l_savebmp(lua_State* L){
//   runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
//   const char* path = lua_tostring(L, 1);
//   uint8_t buf[4];
//   long* ltmp = (long*) buf;
//   uint16_t* tmp2 = (uint16_t*)buf;
//   uint8_t tmp1;

//   tunes.pause();
//   File f = SPIFFS.open(path, FILE_WRITE);
//   f.write('B');
//   f.write('M');
//   *ltmp = 14 + 40 + 4 * 256;
//   f.write(buf, 4); // file size
//   *tmp2 = 0;
//   f.write(buf, 2); // reserved1
//   f.write(buf, 2); // reserved2
//   *ltmp = 14 + 40 + 4 * 256;
//   f.write(buf, 4); // header size

//   // BITMAPCOREHEADER
//   *ltmp = 40;
//   f.write(buf, 4); // bc size
//   *ltmp = 128;
//   f.write(buf, 4); // width
//   f.write(buf, 4); // height
//   *tmp2 = 1;
//   f.write(buf, 2); // planes
//   *tmp2 = 8;
//   f.write(buf, 2); // bitcount
//   *ltmp = 0;
//   f.write(buf,4); // compression
//   *ltmp = 0;
//   f.write(buf,4); // size image
//   *ltmp = 0;
//   f.write(buf,4); // horizon dot/m
//   *ltmp = 0;
//   f.write(buf,4); // vertical dot/m
//   *ltmp = 0;
//   f.write(buf,4); // cir used
//   *ltmp = 0;
//   f.write(buf,4); // cir important

//   uint8_t r,g,b;
//   for(unsigned int i = 0; i < 256; i ++){
//     r = ((self->palette[i] >> 11) << 3);
//     g = (((self->palette[i] >> 5) & 0b111111) << 2);
//     b = ((self->palette[i] & 0b11111) << 3);
//     f.write(b);
//     f.write(g);
//     f.write(r);
//     f.write(0); // reserved
//   }
//   int x = 0,y = 127;
//   for(unsigned int i = 0; i < 128*128; i ++){
//     uint16_t d = tft.readPixel(x, y);
//     uint8_t index = 0;
//     for(unsigned int pi = 0; pi < 256; pi ++){
//       if(self->palette[pi] == d){
//         index = pi;
//         break;
//       }
//     }
//     f.write(index);
//     x ++;
//     if(x == 128){
//       x = 0;
//       y --;
//     }
//   }
//   f.close();
//   tunes.resume();
//   return 0;
// }

int runLuaGame::l_reboot(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));

  reboot();
  return 0;
}

int runLuaGame::l_debug(lua_State* L){
  runLuaGame* self = (runLuaGame*)lua_touserdata(L, lua_upvalueindex(1));
  const char* text = lua_tostring(L, 1);

  // Serial.println(text);
  return 0;
}

String runLuaGame::getBitmapName(String s){
  int p = s.lastIndexOf("/");
  if(p == -1){
    p = 0;
  }
  return s.substring(0, p) + "/sprite.bmp";
}

String runLuaGame::getPngName(String s){
  int p = s.lastIndexOf("/");
  if(p == -1){
    p = 0;
  }
  return s.substring(0, p) + "/initspr.png";
}

void runLuaGame::init(){
  this->resume();
}

void runLuaGame::pause(){
  lua_close(L);
}

extern bool firstBootF;

void runLuaGame::resume(){//ゲーム起動時のみ一回だけ走る処理（setupのようなもの)

// if(firstBootF){

L = luaL_newstate();
lua_setglobal(L, "PSRAM");

luaL_Buffer buff;
luaL_buffinit(L, &buff);

// ここでluaBufferに適切なデータを読み込む（例：ファイルからデータを読み込む）
char *luaBuffer = (char *)malloc(LUA_BUFSIZE); // メモリ確保を行う
// ... データをluaBufferに読み込む処理 ...

luaL_addlstring(&buff, luaBuffer, LUA_BUFSIZE);

free(luaBuffer); // メモリ解放は必要ならここで行う

luaL_openlibs(L);

// L = luaL_newstate();
// lua_setglobal(L, "PSRAM");

// char *luaBuffer = (char *)malloc(MAX_CHAR); // メモリ割り当てを行う
// luaL_Buffer buff;
// luaL_buffinit(L, &buff);
// luaL_buffinitsize(L, &buff, MAX_CHAR);
// luaL_addlstring(&buff, luaBuffer, MAX_CHAR);

// // free(luaBuffer); // メモリ解放を行う

//   luaL_openlibs(L);

  // lua_pushlightuserdata(L, this);
  // lua_pushcclosure(L, l_tp, 1);
  // lua_setglobal(L, "tp");

  // lua_pushlightuserdata(L, this);
  // lua_pushcclosure(L, l_tstat, 1);
  // lua_setglobal(L, "tstat");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_vol, 1);
  lua_setglobal(L, "vol");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_pinw, 1);
  lua_setglobal(L, "pinw");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_pinr, 1);
  lua_setglobal(L, "pinr");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_tone, 1);
  lua_setglobal(L, "tone");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_spr8, 1);
  lua_setglobal(L, "spr8");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_spr, 1);
  lua_setglobal(L, "spr");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_scroll, 1);
  lua_setglobal(L, "scroll");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_rmap, 1);
  lua_setglobal(L, "rmap");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_pset, 1);
  lua_setglobal(L, "pset");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_pget, 1);
  lua_setglobal(L, "pget");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_cls, 1);
  lua_setglobal(L, "cls");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_color, 1);
  lua_setglobal(L, "color");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_text, 1);
  lua_setglobal(L, "text");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_opmode, 1);
  lua_setglobal(L, "opmode");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_drawrect, 1);
  lua_setglobal(L, "drawrect");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_fillrect, 1);
  lua_setglobal(L, "fillrect");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_rectfill, 1);
  lua_setglobal(L, "rectfill");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_fillp, 1);
  lua_setglobal(L, "fillp");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_oval, 1);
  lua_setglobal(L, "oval");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_ovalfill, 1);
  lua_setglobal(L, "ovalfill");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_fillpoly, 1);
  lua_setglobal(L, "fillpoly");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_drawbox, 1);
  lua_setglobal(L, "drawbox");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_drawboxp, 1);
  lua_setglobal(L, "drawboxp");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_drawtri, 1);
  lua_setglobal(L, "drawtri");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_filltri, 1);
  lua_setglobal(L, "filltri");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_fillcircle, 1);
  lua_setglobal(L, "fillcircle");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_drawcircle, 1);
  lua_setglobal(L, "drawcircle");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_pal, 1);
  lua_setglobal(L, "pal");
  
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_key, 1);
  lua_setglobal(L, "key");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_btn, 1);
  lua_setglobal(L, "btn");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_touch, 1);
  lua_setglobal(L, "touch");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_btnp, 1);
  lua_setglobal(L, "btnp");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_sldr, 1);
  lua_setglobal(L, "sldr");

  // lua_pushlightuserdata(L, this);
  // lua_pushcclosure(L, l_getip, 1);
  // lua_setglobal(L, "getip");

  // lua_pushlightuserdata(L, this);
  // lua_pushcclosure(L, l_iswifidebug, 1);
  // lua_setglobal(L, "iswifidebug");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_win, 1);
  lua_setglobal(L, "win");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_run, 1);
  lua_setglobal(L, "run");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_appmode, 1);
  lua_setglobal(L, "appmode");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_appinfo, 1);
  lua_setglobal(L, "appinfo");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_editor, 1);
  lua_setglobal(L, "editor");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_list, 1);
  lua_setglobal(L, "list");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_reboot, 1);
  lua_setglobal(L, "reboot");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, l_debug, 1);
  lua_setglobal(L, "debug");

  // lua_pushlightuserdata(L, this);
  // lua_pushcclosure(L, l_require, 1);
  // lua_setglobal(L, "require");

  // lua_pushlightuserdata(L, this);
  // lua_pushcclosure(L, l_httpsget, 1);
  // lua_setglobal(L, "httpsget");

  // lua_pushlightuserdata(L, this);
  // lua_pushcclosure(L, l_httpsgetfile, 1);
  // lua_setglobal(L, "httpsgetfile");

  // lua_pushlightuserdata(L, this);
  // lua_pushcclosure(L, l_savebmp, 1);
  // lua_setglobal(L, "savebmp");


  // haco8resume();//派生クラスでのみこの位置で実行されるダミー関数

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

    // sprbits[i] = bdata;
    
    // Serial.print(":");
    // Serial.print(bdata); // 0～255
    // Serial.print(":");
    // Serial.println("end");
  }
  fr.close();
  //アプリのパスからアプリ名を取得
  string str1 = appfileName.c_str();
  int i=0;

  for (string s : split(str1,'/')) {
    if(i==1){
      appNameStr = s.c_str();
      fr = SPIFFS.open("/" + appNameStr + "/mapinfo.txt", "r");// ⑩ファイルを読み込みモードで開く
    }
     i++;
  }

  for(int i= 0;i<16;i++){//マップを描くときに使うスプライト番号リストを読み込む
    String _readStr = fr.readStringUntil(',');// ⑪,まで１つ読み出し
    mapsprnos[i] = atoi(_readStr.c_str());
  }

  String _readStr = fr.readStringUntil(',');// 最後はマップのパス
  mapFileName = "/init/map/"+_readStr;
  fr.close();	// ⑫	ファイルを閉じる
  readMap();

//-------------------------------------------

LoadF lf(MAX_CHAR); // 2048 バイトのメモリを確保する例
lf.f = SPIFFS.open(appfileName, FILE_READ);

char cFileName[32];
appfileName.toCharArray(cFileName, 32);//char変換

if (lua_load(L, getF, &lf, cFileName, NULL)){//Luaに渡してファイル読み込みに成功したかチェック（成功すると0）
    printf("error? %s\n", lua_tostring(L, -1));
    Serial.printf("error? %s\n", lua_tostring(L, -1));
    runError = true;//エラーが発生
    errorString = lua_tostring(L, -1);
}

if (!runError) {
    if (lua_pcall(L, 0, 0, 0)) {
        Serial.printf("init error? %s\n", lua_tostring(L, -1));
        runError = true;
        errorString = lua_tostring(L, -1);
    }
}

}

int runLuaGame::run(int _remainTime){

  if(exitRequest){//次のゲームを起動するフラグがたったら
    exitRequest = false;//フラグをリセットして、
    return 1; // exit(1をリターンすることで、main.cppの変数modeを１にする)
  }

  // // for(int i = 0; i < buttonState.size(); i ++){
  //   for(int i = 0; i < 7; i ++){
  //   if(pressedBtnID == -1){
  //     buttonState[i] = 0;
  //   }else if(pressedBtnID == i){//押されたものだけの値をあげる
  //     buttonState[i] ++;
  //   }
  // }
  
  // // for(int i = 0; i < buttonState.size(); i ++){
  for(int i = 0; i < 7; i ++){
    if(pressedBtnID == -1){
      buttonState[i] = 0;
    }else if(pressedBtnID == i){//押されたものだけの値をあげる
      buttonState[i] ++;
    }
  }

  if(runError){
      tft.setTextSize(1);
      tft.setTextColor(TFT_WHITE, TFT_BLUE);
      tft.setCursor(0, 0);
      tft.setTextWrap(true);
      tft.print(errorString);
      tft.setTextWrap(false);
    }else{
        if (firstLoopF) {
          luaL_dostring(L, "_init()");
          firstLoopF = false;
        }
        luaL_dostring(L, "_update()");

        if(luaL_dostring(L, "_draw()")){
          lua_pop(L, 1);//最後に入れないとダメ
        }
    }

  //show FPS:ms
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.setCursor(90, 127 - 16);
  tft.print(String(1000/_remainTime) + "FPS");
  tft.setCursor(90, 127 - 8);
  tft.print(String(_remainTime) + "ms");

  return 0;
}



