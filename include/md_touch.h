#ifndef _MD_TOUCH_H_
  #define _MD_TOUCH_H_

  #include "config.h"
  #include "md_util.h"

  #ifdef USE_TOUCHSCREEN
    #include "FS.h"
    #include <SPI.h>
    #include <TFT_eSPI.h>      // Hardware-specific library

      // Using two fonts since numbers are nice when bold
    #define NORM_FONT &FreeSansOblique12pt7b // Key label font 1
    #define BOLD_FONT &FreeSansBold12pt7b    // Key label font 2
    class md_touch
    {
      protected:
        msTimer clrT     = msTimer();
        msTimer minT     = msTimer(STAT_TIMEMIN);
        bool    isStatus = false; // status text visible
        char    outTxt[STAT_LINELEN + 1] = "";
        bool    isAct    = false ;

      public:
        bool startTouch();
        bool calTouch();
        void runTouch(char* pStatus);
        bool wrTouch(const char *msg, uint8_t spalte, uint8_t zeile);
        bool wrStatus();
        bool wrStatus(const char* msg);
        bool wrStatus(const char* msg, uint32_t stayTime);

      protected:
        bool _drawScreen();
        bool _drawKeypad();
        bool _drawStatus(char* outStat);
        bool _clearTFT();
        bool _initTFT(const uint8_t csTFT);

    };
  #endif
#endif

// template
#ifdef DUMMY
  // Keypad start position, key sizes and spacing
  #define KEY_X 40 // Centre of key
  #define KEY_Y 96
  #define KEY_W 62 // Width and height
  #define KEY_H 30
  #define KEY_SPACING_X 18 // X and Y gap
  #define KEY_SPACING_Y 20
  #define KEY_TEXTSIZE 1   // Font size multiplier

  // Using two fonts since numbers are nice when bold
  #define NORM_FONT &FreeSansOblique12pt7b // Key label font 1
  #define BOLD_FONT &FreeSansBold12pt7b    // Key label font 2

  // Numeric display box size and location
  #define DISP_X 1
  #define DISP_Y 10
  #define DISP_W 238
  #define DISP_H 50
  #define DISP_TSIZE 3
  #define DISP_TCOLOR 0x07FF // TFT_CYAN

  // We have a status line for messages
  #define STATUS_X 120 // Centred on this
  #define STATUS_Y 65

  // Number length, buffer for storing it and character index
  #define NUM_LEN 12
#endif
