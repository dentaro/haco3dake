#ifndef EDITOR_H
#define EDITOR_H

#include <FS.h>
#include <regex>
#include <LovyanGFX.h>  // 必要なヘッダを追加
#include "KbdRptParser.h"

#define FORMAT_SPIFFS_IF_FAILED false
#define KILO_VERSION "0.0.1"
#define KILO_TAB_STOP 2
#define ABUF_INIT {NULL, 0}

class Editor {

  private:
  String statusMessage = "";
  // int preCx =4;
  // int preCy =4;
  // int preRx =4;

  int keyMillis = 0;
  int keywaitTime = 200;
  
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
    void initEditor(LovyanGFX& tft, int _xpos, int _ypos);
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

    // void editorProcessKeypress(int c, fs::FS &fs, fs::FS &SD);
    void editorProcessKeypress(int c, fs::FS &fs);
    // void editorProcessKeypress(int c);
    // void editorMoveCursor(int key);
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

    // void editorPageMove(int c, KbdRptParser &Prs);
    void editorPageMove(char c, KbdRptParser &Prs);

    int utf8_char_width(char c);
    // void update(LovyanGFX& tft,fs::FS &fs, fs::FS &SD, KbdRptParser &Prs);
    // void update(LovyanGFX& tft, fs::FS &fs, fs::FS &SD, char _keychar);
    void update(LovyanGFX& tft, fs::FS &fs, char _keychar);
    void writeFile(fs::FS &fs, const char * path, const char * message);
    void appendFile(fs::FS &fs, const char * path, const char * message);

    void getCursorConfig(String _wrfile);
    void setCursorConfig();
    void setCursorConfig(int _cx, int _cy, int _rx);

    void setCursor(int _cx,int _cy,int _rx);
};

#endif
