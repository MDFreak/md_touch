//#ifdef _MD_TOUCH_H_
#ifndef _MD_TOUCH_H_
#define _MD_TOUCH_H_

#ifndef DEBUG_MODE
#define DEBUG_MODE CFG_DEBUG_STARTUP
#endif
//#include "FS.h"
#include <SPI.h>
#include <md_defines.h>
#include <md_util.h>
#include <md_spiffs.h>
#include <XPT2046_Touchscreen.h>
#include <Adafruit_GFX.h>     //Grafik Bibliothek
#include <Adafruit_ILI9341.h> // Display Treiber
#include <md_TouchEvent.h>    //Auswertung von Touchscreen Ereignissen
#include <linked_list.hpp>
#include <esp_task_wdt.h>

// --- system
#define WDT_TTIMEOUT 3

// --- color definitions
#define MD_BLACK 0x0000       /*   0,   0,   0 */
#define MD_NAVY 0x000F        /*   0,   0, 128 */
#define MD_DARKGREEN 0x03E0   /*   0, 128,   0 */
#define MD_DARKCYAN 0x03EF    /*   0, 128, 128 */
#define MD_MAROON 0x7800      /* 128,   0,   0 */
#define MD_PURPLE 0x780F      /* 128,   0, 128 */
#define MD_OLIVE 0x7BE0       /* 128, 128,   0 */
#define MD_LIGHTGREY 0xC618   /* 192, 192, 192 */
#define MD_DARKGREY 0x7BEF    /* 128, 128, 128 */
#define MD_BLUE 0x001F        /*   0,   0, 255 */
#define MD_GREEN 0x07E0       /*   0, 255,   0 */
#define MD_CYAN 0x07FF        /*   0, 255, 255 */
#define MD_RED 0xF800         /* 255,   0,   0 */
#define MD_MAGENTA 0xF81F     /* 255,   0, 255 */
#define MD_YELLOW 0xFFE0      /* 255, 255,   0 */
#define MD_WHITE 0xFFFF       /* 255, 255, 255 */
#define MD_ORANGE 0xFD20      /* 255, 165,   0 */
#define MD_GREENYELLOW 0xAFE5 /* 173, 255,  47 */
#define MD_PINK 0xF81F

// --- parameters for touchscreen
#define MINPRESSURE 10 // pressure to detect touch
// touchscreen

#ifndef TS_MINX
#define TS_MINX 230  // minimal x return value
#define TS_MINY 350  // minimal y return value
#define TS_MAXX 3700 // maximal x return value
#define TS_MAXY 3900 // maximal y return value
#endif
// 496 //262 //3997 //3925

void actLev(uint8_t level);
uint8_t actLev(void);
void oldLev(uint8_t level);
uint8_t oldLev(void);
void actLev(uint8_t level);
uint8_t actLev(void);
void oldLev(uint8_t level);
uint8_t oldLev(void);

/*
  #ifndef TS_MINX
      #define TS_MINX 325
      #define TS_MINY 200
      #define TS_MAXX 3850
      #define TS_MAXY 3700
    #endif
*/

// text defines
#define TXT_LEFT 0
#define TXT_CENTER 1
#define TXT_RIGHT 2
#define TXT_CHAR1_H 8
#define TXT_CHAR1_W 6
// colors
#define COL_BUT_DEF MD_BLACK
#define COL_BUT_SEL MD_PURPLE
#define COL_BUT_RDY MD_DARKGREEN
#define COL_BUT_EME MD_RED
#define COL_TXT_DEF MD_ORANGE
#define COL_TXT_SEL MD_GREENYELLOW
#define COL_TXT_RDY MD_WHITE
#define COL_TXT_EME MD_GREENYELLOW // MD_PURPLE
#define COL_BACK MD_BLACK

// status window
#define STAT_X 4
#define STAT_Y 229
#define STAT_W 314
#define STAT_H 12
#define STAT_SIZE 1
#define STAT_ORIENT TXT_CENTER
// calibration
#define CAL_FCONF "/conf.dat"
#define CAL_FCALIB "/tcalib.dat"

// --- parameters for menu
// colunm position
#define MEN_BUT_SIZE 24
#define MEN_COL1_X 5
#define MEN_COL_W 31
#define MEN_TITLE_X MENU_COL_W * 2 + MENU_COL1_X
#define MEN_TITLE_W MENU_COL_W * 5 + MENU_BUT_SIZE + MENU_TITLE_X
#define MEN_MENU_X MENU_COL_W * 5 + MENU_COL1_X
// row positions
#define MEN_BUT1_Y 10
#define MEN_BUTROW_H 30
#define MEN_TITLE_Y 2
#define MEN_TITLE_H MENU_BUTROW_H + MENU_BUT1_Y - MEN_TITLE_Y
#define MEN_MENU1_Y MENU_BUTROW_H + MENU_BUT1_Y
#define MEN_OUT1_Y MENU_BUTROW_H + MENU_BUT1_Y
#define MEN_OUT_H MEN_BUT_SIZE

// --- tasks
// task control
#define TTASK_ON_RUN 0
#define TTASK_HSHAKE 1
#define TTASK_ON_HOLD 2
#define TTASK_NEWVAL TTASK_HSHAKE + TTASK_ON_HOLD

void handleTouch(void *pvParameters);
// void handleMenu(void * pvParameters);

// --- class tColors
  class tColors
    {
      private:
        uint16_t _cols[4];

      public:
        tColors(void) {}
        tColors(uint16_t colDef, uint16_t colSel, uint16_t colRdy, uint16_t colEme)
          {
            _cols[0] = colDef;
            _cols[1] = colSel;
            _cols[2] = colRdy;
            _cols[3] = colEme;
          }

        void operator=(tColors p)
          {
            _cols[0] = p._cols[0];
            _cols[1] = p._cols[1];
            _cols[2] = p._cols[2];
            _cols[3] = p._cols[3];
          };

        uint16_t cols(uint8_t mode = MD_DEF)
          {
            return _cols[mode];
          }
        void col(uint16_t color, uint8_t mode = MD_DEF)
          {
            _cols[mode] = color;
          }
    };

// --- class tBoxColors
  class tBoxColors
    {
      private:
        tColors _colbut = tColors(COL_BUT_DEF, COL_BUT_SEL, COL_BUT_RDY, COL_BUT_EME);
        tColors _coltxt = tColors(COL_TXT_DEF, COL_TXT_SEL, COL_TXT_RDY, COL_TXT_EME);
        uint8_t _colhide = COL_BACK;

      public:
        tBoxColors(void) {}

        void operator=(tBoxColors p)
          {
            _colbut = p._colbut;
            _coltxt = p._coltxt;
            _colhide = p._colhide;
          };

        uint16_t colBut(uint8_t mode = MD_DEF)
          {
            return _colbut.cols(mode);
          }
        void colBut(uint16_t color, uint8_t mode = MD_DEF)
          {
            _colbut.col(color, mode);
          }
        uint16_t colText(uint8_t mode = MD_DEF)
          {
            return _coltxt.cols(mode);
          }
        void colText(uint16_t color, uint8_t mode = MD_DEF)
          {
            _coltxt.col(color, mode);
          }
    };

// --- class tPoint
  class tPoint
    {
    public:
      tPoint(void) {}
      tPoint(int16_t _x, int16_t _y) { set(_x, _y); }

      bool operator==(tPoint p) { return ((p.x == x) && (p.y == y)); }
      bool operator!=(tPoint p) { return ((p.x != x) || (p.y != y)); }
      void operator+=(tPoint p)
        {
          x += p.x;
          y += p.y;
        }
      void operator=(TS_Point p)
        {
          x = p.x;
          y = p.y;
        }

      void set(int16_t _x, int16_t _y)
        {
          x = _x;
          y = _y;
        }
      virtual char *print15(char *buf)
        {
          sprintf(buf, "x %4i y %4i", x, y);
          return buf;
        }

      int16_t x = 0;
      int16_t y = 0;
    };

// --- class tText
  class tText
    {
    protected:
      char *_text = NULL;
      uint16_t _len = 0;
      uint8_t _size = 2;
      uint8_t _bpos = TXT_LEFT; // orientation

    public:
      tText() {}
      ~tText()
        {
          if (_text != NULL)
            delete _text;
        }

      void setText(const char *pTxt);
      void setText(String Text);
      char *getText(char *pText);
      void setLen(uint8_t len)
        {
          _len = len;
        }
      void setTextSize(uint8_t textSize)
        {
          _size = textSize;
        }
      uint8_t txtSize()
        {
          return _size;
        }
      void setOrient(uint8_t orient)
        {
          _bpos = orient;
        }
    };

// --- class tBox
  class tBox
    {
    protected:
      int16_t _x = 0; // position top/left
      int16_t _y = 0; // position top/left
      uint16_t _w = 10;
      uint16_t _h = 10;

    public:
      tBox() {}
      tBox(int16_t x, int16_t y, uint16_t w, uint16_t h)
        {
          setPos(x, y);
          setSize(w, h);
        }

      void setPos(int16_t x, int16_t y)
        {
          _x = x;
          _y = y;
        }
      void setSize(uint16_t w, uint16_t h)
        {
          _w = w;
          _h = h;
        }
      int16_t  x() { return _x; }
      int16_t  y() { return _y; }
      uint16_t w() { return _w; }
      uint16_t h() { return _h; }

      char *printbox(char *p)
        {
          sprintf(p, "%3i %3i %3i %3i", _x, _y, _w, _h);
          return p;
        }

      void operator=(tBox p)
        {
          _x = p.x();
          _y = p.y();
          _w = p.w();
          _h = p.h();
        };
    };

// --- class tButton_def
/* tButtondef is an atomic bit coded value and
 * contains all information for navigation and evaluation
 * Bits:
 *   31-28 screen column
 *         col 1,2   witches for output (20), output (140)
 *         col 3     titel (130 size 2/3)
 *         col 5     main screen, menu
 *         col 9,10  menu control buttons
 *   27-24 screen line in main menu level 3
 *   23-20 screen line in main menu level 2
 *   19-16 screen line in main menu level 1
 *   15-12 screen line in main menu level 0
 *   11- 4 reserved
 *    3- 0 menu status bits
 *          3 reserved
 *          2 maybe selected (used by handleMenu)
 *          1 visible,
 *          0 active for touching (only if visible)
 */
  class tButton_def // 0b cccc LLLL UUUU llll uuuu .... .... ..va
    {
    protected:
      uint32_t _id = 0;

    public:
      tButton_def() {}
      ~tButton_def() {}

      void setID(uint32_t id)
      {
        _id = id;
      }
      void setAct(uint8_t active) //  can be touched
      {
        _id |= (_id & 0xFFFFFFFE) | (active & 0x00000001);
      }
      void setVis(uint8_t visible) // is visible
      {
        _id |= (_id & 0xFFFFFFFD) | (visible & 0x00000002);
      }
      void setCol(uint8_t column) //  column of screen
      {
        _id |= (_id & 0x0FFFFFFF) | (column & 0xF0000000);
      }
      void setMenu(uint8_t menu) //  column of screen
      {
        _id |= (_id & 0xF0000FFF) | (menu & 0x0FFFF000);
      }
      uint32_t id()
      {
        return _id;
      }
      uint8_t isAct()
      {
        return (uint8_t)_id & 0x00000001;
      }
      uint8_t isVis()
      {
        return (uint8_t)_id & 0x00000002;
      }
      uint8_t col()
      {
        return (uint8_t)((_id & 0xF0000000) >> 28);
      }
      uint8_t m0()
      {
        return (uint8_t)((_id & 0x0F000000) >> 24);
      }
      uint8_t m1()
      {
        return (uint8_t)((_id & 0x00F00000) >> 20);
      }
      uint8_t m2()
      {
        return (uint8_t)((_id & 0x000F0000) >> 16);
      }
      uint8_t m3()
      {
        return (uint8_t)((_id & 0x0000F000) >> 12);
      }
      uint16_t menu()
      {
        return (uint16_t)((_id & 0x0FFFF000) >> 12);
      }
    };

// --- class tButton
  class tButton : public tBox, public tBoxColors, public tText, public tButton_def, public md_cell
    {
      protected:
        uint8_t _mode = MD_DEF;
        tBoxColors _cols;

      public:
        tButton() {}
        tButton(int16_t x, int16_t y, uint16_t w, uint16_t h) {}

        void show();
        void show(uint8_t mode)
          {
            _mode = mode;
            show();
          }
        void hide();
        void setBox(int16_t x, int16_t y, uint16_t w, uint16_t h)
          {
            setPos(x, y);
            setSize(w, h);
          }
        // void setText();
    };

// --- class calData
  class calData_t
    {
      public:
        uint16_t xmin = 230;
        uint16_t ymin = 350;
        uint16_t xmax = 3700;
        uint16_t ymax = 3900;
        tPoint x0y0 = {10, 10};
        tPoint x0y1 = {10, 310};
        tPoint x1y0 = {230, 10};
        tPoint x1y1 = {230, 310};

        void setCalib(uint16_t _xmin, uint16_t _ymin, uint16_t _xmax, uint16_t _ymax)
          {
            xmin = _xmin;
            ymin = _ymin, xmax = _xmax, ymax = _ymax;
          }
        virtual char *printPts56(char *pcalData)
          {
            sprintf(pcalData, "points P00 %3i %3i P01 %3i %3i P10 %3i %3i P11 %3i %3i",
                    x0y0.x, x0y0.y, x0y1.x, x0y1.y, x1y0.x, x1y0.y, x1y1.x, x1y1.y);
            return pcalData;
          }
        char *printCal50(char *pcalVals)
          {
            sprintf(pcalVals, "calPos xmin %4i ymin %4i xmax %4i ymax %4i",
                    xmin, ymin, xmax, ymax);
            return pcalVals;
          }
    };

// --- class md_touch
  class md_touch    // public Adafruit_ILI9341, public XPT2046_Touchscreen,
    {
      private:
        int8_t _iscal = MD_UNDEF;
        uint8_t _ledpin = 0;
        uint8_t _LED_ON = 1;
        tButton _statbox;

      public:
        md_touch(uint8_t cspin, uint8_t tft_CS, uint8_t tft_DC, uint8_t tft_RST,
                 uint8_t tft_LED, uint8_t led_ON); //, uint8_t spi_bus = VSPI);
        ~md_touch();

        void start(uint8_t rotation, uint16_t background = MD_BLACK);
        void wrStatus(const char *msg, uint8_t mode = MD_DEF);
        void wrStatus(String msg, uint8_t mode = MD_DEF);
        void setRotation(uint8_t rotation);
        uint8_t rotation();

      private:
        void startTFT();
        void startStatus();
        // calibration
        void loadCalibration();
        void doCalibration();
        void checkCalibration();
        void saveCalibration();
    };
// --- class tList
  class tList : public md_list
    {
      private:
        uint8_t _iscal = false;
        uint32_t _actid = 0;

      public:
        tList() {}
        ~tList() {}

        void append();
        void remove();
    };
#endif

#ifdef PARKEN
  //--- class md_menu
  /* --- menu    brief description ---
      a menu is realized with a linked list structure taking dynamic
      heap memory on startup that containes the menu texts
      no semaphores are used because
      - configuration is done before task is started
      - an 2 atomic uint32_t values are used to
        1) control the menu position and
        2) read the touch value.
        - one side only sets a value when read a 0
        - the other side resets the value when the value was ready
          and the work was done

      atomic 'output', 'control' values
      - both values are designed to be used as 4 uint8_t values
      - each value has 4 bits for both old (=actual) and new (=desired) value
      - this means every value has 15 possible values
        15 pages, 15 columns, 15 entries, 15 actions

      - 'action' type MENU_ACT_t -> UP, DOWN, PREVPAGE, NEXTPAGE, DOIT ...
          new = (uint8_t) ( 'output' & 0x0000 000F       )
          old = (uint8_t) (('output' & 0x0000 00F0) >>  4)
      - 'entry'  uint8_t [0..15]
          new = (uint8_t) ( 'output' & 0x0000 0F00      8)
          old = (uint8_t) (('output' & 0x0000 F000) >> 12)
      - 'page'   uint8_t [0..15]
          new = (uint8_t) ( 'output' & 0x000F 0000     16)
          old = (uint8_t) (('output' & 0x00F0 0000) >> 20)
      - 'level'  uint8_t [0..15]
          new = (uint8_t) (('output' & 0x0F00 0000) >> 24)
          old = (uint8_t) (('output' & 0xF000 0000) >> 28)

      function
      - moving inside menu
        - on exit
          - no exit action define  => done automatically
          - otherwise              => interrupt main program
        - on entry
          - no entry action define => done automatically
          - otherwise              => interrupt main program
    */
  typedef enum MENU_ACT
  {
    ACTNO = 0,
    // PREV,
    // NEXT,
    PREVPAGE,
    NEXTPAGE,
    HIDE,
    SHOW,
    DOIT,
    ACTMAX
  } MENU_ACT_t;

  #define MENU_TXTLEN 25
  #define MENUTYPE_MAIN 0
  #define MENUTYPE_PAGE 1
  #define MENU_MAXLINES 8
  #define MENU_FONT ArialRoundedMTBold_14
#endif // PARKEN