#ifndef EDITOR_H
#define EDITOR_H

#include <Arduino.h>
#include "SPIFFS.h"

#include <FS.h>
#include <regex>
#include <LovyanGFX.h>  // 必要なヘッダを追加
// #include "KbdRptParser.h"
#include <regex>
#include <PS2Keyboard.h>

#define FORMAT_SPIFFS_IF_FAILED false
#define KILO_VERSION "0.0.1"
#define KILO_TAB_STOP 2
#define ABUF_INIT {NULL, 0}

//メモリが足りない場合に、エディタの表示文字列を少なくする
// #define EDITOR_ROWS 11
// #define EDITOR_COLS 20
#define EDITOR_ROWS 16
#define EDITOR_COLS 26

extern int HACO3_C0;
extern int HACO3_C1;
extern int HACO3_C2;
extern int HACO3_C3;
extern int HACO3_C4;
extern int HACO3_C5;
extern int HACO3_C6;
extern int HACO3_C7;
extern int HACO3_C8;
extern int HACO3_C9;
extern int HACO3_C10;
extern int HACO3_C11;
extern int HACO3_C12;
extern int HACO3_C13;
extern int HACO3_C14;
extern int HACO3_C15;

class Editor {

  private:
  String statusMessage = "";

  bool shiftF = false;

  int keyMillis = 0;
  int keywaitTime = 200;

  struct abuf {
    char *b;
    int len;
  };

  typedef struct erow {
    int size;
    int rsize;
    char *chars;
    char *render;
  } erow;

  struct editorConfig {
    int cx, cy;
    int preCx, preCy;
    int rx;
    int preRx;
    int rowoff;
    int coloff;
    int screenrows;
    int screencols;
    int screenrow;
    int screencol;
    int dirty;
    int numrows;
    erow *row;
    char *filename;
    char statusmsg[80];
    time_t statusmsg_time;
  }; struct editorConfig E;


  public:
    Editor();
    void editorDrawMessageBar(LovyanGFX& tft, struct abuf *ab);
    void editorDrawStatusBar(LovyanGFX& tft, struct abuf *ab);
    void initEditor(LovyanGFX& tft);
    void readFile(fs::FS &fs, const char * path);
    void editorOpen(fs::FS &fs, const char *filename);
    void editorUpdateRow(erow *row);
    void editorInsertRow(int at, const char *s, size_t len);
    void editorInsertRow2(int at, const char *s, size_t len);//改行付き
    void editorRefreshScreen(LovyanGFX& tft);
    void editorScroll();
    int editorRowCxToRx(erow *row, int cx);
    void editorDrawRows(struct abuf *ab);
    void abAppend(struct abuf *ab, const char *s, int len);
    void abFree(struct abuf *ab);

    void editorProcessKeypress(int c, fs::FS &fs);
    void editorInsertChar(int c);
    void editorMoveCursor(int c);
    void editorRowInsertChar(erow *row, int at, int c);
    void editorSetStatusMessage(const char *fmt, ...);

    void editorInsertNewline();
    void editorDelChar();
    void editorDelChar3() ;
    void editorRowDelChar(erow *row, int at);
    void editorRowAppendString(erow *row, char *s, size_t len);
    void editorDelRow(int at);
    void editorFreeRow(erow *row);
    void editorSave(fs::FS &fs);
    // void editorSaveSD(fs::FS &fs);

    char *editorRowsToString(int *buflen);
    int getTargetTime();
    void setTargetTime(int _targetTime);

    int getNumRows();
    int getCy();
    int getCx();
    int getRx();
    int getPreCy();
    int getPreCx();
    int getPreRx();

    int getScreenCol();
    int getScreenRow();

    bool getShiftF();
    // void editorPageMove(int c, KbdRptParser &Prs);
    void editorPageMove(char c);

    int utf8_char_width(char c);
    // void update(LovyanGFX& tft,fs::FS &fs, fs::FS &SD, KbdRptParser &Prs);
    // void update(LovyanGFX& tft, fs::FS &fs, fs::FS &SD, char _keychar);
    void update(LovyanGFX& tft, fs::FS &fs, int _keychar);
    void writeFile(fs::FS &fs, const char * path, const char * message);
    void appendFile(fs::FS &fs, const char * path, const char * message);

    void getCursorConfig(String _wrfile);
    void setCursorConfig();
    void setCursorConfig(int _cx, int _cy, int _rx);

    void setCursor(int _cx,int _cy,int _rx);
};

#endif
