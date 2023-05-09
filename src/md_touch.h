//#ifdef _MD_TOUCH_H_
#ifndef _MD_TOUCH_H_
  #define _MD_TOUCH_H_

  /* --- touch & menu - a brief description ---
   *  touch menu task
   *  - checks touch event and delivers position in 2 variables
   *    - _tactRaw = raw value: hword = x, lword = y
   *    - _tactXY  = pix value: hword = x, lword = y
   *  no semaphores are used because
   *  - configuration is done before task is started
   *  - an 2 atomic uint32_t values are used to
   *    1) control the menu position and
   *    2) read the touch value.
   *    - one side only sets a value when read a 0
   *    - the other side resets the value when the value was ready
   *      and the work was done
   *
   *  - this means every value has 15 possible values
   *    atomic 'output', 'control' values
   *  - both values are designed to be used as 4 uint8_t values
   *  - each value has 4 bits for both old (=actual) and new (=desired) value
   *    15 pages, 15 columns, 15 entries, 15 actions
   *
   *  - 'action' type MENU_ACT_t -> UP, DOWN, PREVPAGE, NEXTPAGE, DOIT ...
   *      new = (uint8_t) ( 'output' & 0x0000 000F       )
   *      old = (uint8_t) (('output' & 0x0000 00F0) >>  4)
   *  - 'entry'  uint8_t [0..15]
   *      new = (uint8_t) ( 'output' & 0x0000 0F00      8)
   *      old = (uint8_t) (('output' & 0x0000 F000) >> 12)
   *  - 'page'   uint8_t [0..15]
   *      new = (uint8_t) ( 'output' & 0x000F 0000     16)
   *      old = (uint8_t) (('output' & 0x00F0 0000) >> 20)
   *  - 'level'  uint8_t [0..15]
   *       new = (uint8_t) (('output' & 0x0F00 0000) >> 24)
   *       old = (uint8_t) (('output' & 0xF000 0000) >> 28)
   *
   *  function
   *  - moving inside menu
   *    - on exit
   *      - no exit action define  => done automatically
   *      - otherwise              => interrupt main program
   *    - on entry
   *      - no entry action define => done automatically
   *      - otherwise              => interrupt main program
   */


  #ifndef DEBUG_MODE
      #define DEBUG_MODE CFG_DEBUG_STARTUP
    #endif
  // includes
    //#include "FS.h"
    #include <SPI.h>
    #include <md_defines.h>
    #include <md_util.h>
    #include <md_spiffs.h>
    #include <XPT2046_Touchscreen.h>
    #include <Adafruit_GFX.h>     // Grafik Bibliothek
    #include <Adafruit_ILI9341.h> // Display Treiber
    #include <md_TouchEvent.h>
    #include <AT_Display.h>
    #include <linked_list.hpp>
    #include <esp_task_wdt.h>
    #include <fonts\AT_Bold12pt7b.h>
    #include <fonts\AT_BoldOblique9pt7b.h>
    #include <fonts\AT_Oblique9pt7b.h>
    #include <fonts\AT_Standard9pt7b.h>
    #include <md_leds.h>          // color conversions to use
  // --- system
    #define WDT_TTIMEOUT 3
    #define TFT_FREQU    2000000
  // --- 2-dim point
    // --- class tPoint - 2 dimensions
      typedef struct POINT_t
        {
         	int16_t x;
         	int16_t y;
          bool operator==(POINT_t p)  { return ((p.x == x) && (p.y == y)); }
          void operator= (POINT_t p)  { x = p.x; y = p.y; }
          bool operator==(TS_Point p) { return ((p.x == x) && (p.y == y)); }
          void operator= (TS_Point p) { x = p.x; y = p.y; }
          virtual char *print2char15(char *buf)
            {
              sprintf(buf, "x %4i y %4i", x, y);
              return buf;
            }
        } POINT_t;
  // --- color definitions
    // --- basic colors
      typedef enum md_colors : uint16_t
        {
          MD_BLACK          = COL16_BLACK,
          //MD_OLIVE          = 0x7BE0u, /* 128, 128,   0 */
          //MD_GREENYELLOW    = 0xAFE5u, /* 173, 255,  47 */
          MD_OLIVE          = COL16_OLIVE,
          MD_GREENYELLOW    = COL16_GREENYELLOW,
          MD_RED_VERYLOW    = COL16_RED_VERYLOW,
          MD_RED_LOW        = COL16_RED_LOW,
          MD_RED_MEDIUM     = COL16_RED_MEDIUM,
          MD_RED_HIGH       = COL16_RED_HIGH,
          MD_GREEN_VERYLOW  = COL16_GREEN_VERYLOW,
          MD_GREEN_LOW      = COL16_GREEN_LOW,
          MD_GREEN_MEDIUM   = COL16_GREEN_MEDIUM,
          MD_GREEN_HIGH     = COL16_GREEN_HIGH,
          MD_BLUE_VERYLOW   = COL16_BLUE_VERYLOW,
          MD_BLUE_LOW       = COL16_BLUE_LOW,
          MD_BLUE_MEDIUM    = COL16_BLUE_MEDIUM,
          MD_BLUE_HIGH      = COL16_BLUE_HIGH,
          MD_YELLOW_VERYLOW = COL16_YELLOW_VERYLOW,
          MD_YELLOW_LOW     = COL16_YELLOW_LOW,
          MD_YELLOW_MEDIUM  = COL16_YELLOW_Medium,
          MD_YELLOW_HIGH    = COL16_YELLOW_HIGH,
          MD_ORANGE_VERYLOW = COL16_ORANGE_VERYLOW,
          MD_ORANGE_LOW		  = COL16_ORANGE_LOW,
          MD_ORANGE_MEDIUM  = COL16_ORANGE_MEDIUM,
          MD_PURPLE_VERYLOW = COL16_PURPLE_VERYLOW,
          MD_PURPLE_LOW		  = COL16_PURPLE_LOW,
          MD_PURPLE_MEDIUM	= COL16_PURPLE_MEDIUM,
          MD_PURPLE_HIGH		= COL16_PURPLE_HIGH,
          MD_CYAN_VERYLOW	  = COL16_CYAN_VERYLOW,
          MD_CYAN_LOW		    = COL16_CYAN_LOW,
          MD_CYAN_MEDIUM		= COL16_CYAN_MEDIUM,
          MD_CYAN_HIGH		  = COL16_CYAN_HIGH,
          MD_GREY_VERYLOW	  = COL16_WHITE_VERYLOW,
          MD_GREY_LOW		    = COL16_WHITE_LOW,
          MD_GREY_MEDIUM	  = COL16_WHITE_MEDIUM,
          MD_GREY_HIGH		  = COL16_WHITE_HIGH
        } MD_COLORS_t;
  // --- touchscreen
    // basics
      #define MINPRESSURE 10 // pressure to detect touch
      //typedef enum : uint8_t
      enum : uint8_t
        {
          TOUCHED_NOTOUCH = 0,
          TOUCHED_SINGLE,
          TOUCHED_LONG,
          TOUCHED_DOUBLE
        };
    // touch timing
      #define TOUCH_TASK_SLEEP_US 10000ul
      #define TOUCH_CLICK_TIME_MS 20
      #define TOUCH_LONG_TIME_MS  800
    // screen
      // orientation 0
        #define DEF_0_TFTW 240  // width in pixel
        #define DEF_0_TFTH 320  // hight in pixel
        #define DEF_0_MINX 366  // minimal x return value
        #define DEF_0_MINY 228  // minimal y return value
        #define DEF_0_MAXX 3677 // maximal x return value
        #define DEF_0_MAXY 3527 // maximal y return value
      // orientation 1
        #define DEF_1_TFTW 320  // width in pixel
        #define DEF_1_TFTH 240  // hight in pixel
        #define DEF_1_MINX 260  // minimal x return value
        #define DEF_1_MINY 409  // minimal y return value
        #define DEF_1_MAXX 3546 // maximal x return value
        #define DEF_1_MAXY 3794 // maximal y return value
      // orientation 2
        #define DEF_2_TFTW 240  // width in pixel
        #define DEF_2_TFTH 320  // hight in pixel
        #define DEF_2_MINX 421  // minimal x return value
        #define DEF_2_MINY 570  // minimal y return value
        #define DEF_2_MAXX 3762 // maximal x return value
        #define DEF_2_MAXY 3849 // maximal y return value
      // orientation 3
        #define DEF_3_TFTW 320  // width in pixel
        #define DEF_3_TFTH 240  // hight in pixel
        #define DEF_3_MINX 525  // minimal x return value
        #define DEF_3_MINY 345  // minimal y return value
        #define DEF_3_MAXX 3877 // maximal x return value
        #define DEF_3_MAXY 3738 // maximal y return value
    // handshake format
      #define TOUCH_HRAW_BITS     16
      #define TOUCH_HRAW_SHIFT    TOUCH_HRAW_BITS
      #define TOUCH_HRAW_MASK     0x0000FFFFul
      #define TOUCH_HPOINT_BITS   14
      #define TOUCH_HPOINT_SHIFT  TOUCH_HPOINT_BITS
      #define TOUCH_HPOINT_MASK   0x00003FFFul
      #define TOUCHED_TYPE_BITS   4
      #define TOUCHED_TYPE_SHIFT  28
      #define TOUCHED_TYPE_NMASK  0x0FFFFFFFul  // 0FFFFFFF
    // click types (max 15)
      #define TOUCH_TYPE_CLICK    1
      #define TOUCH_TYPE_LONG     2
      #define TOUCH_TYPE_DOUBLE   3  // TODO to implement
      #define TOUCH_TYPE_MOVE     4  // TODO to implement
      #define TOUCH_TYPE_MAX      15 // mask = max type no.
    // --- class md_touch
      class md_touch    // public Adafruit_ILI9341, public XPT2046_Touchscreen,
        {
          private:
            int16_t   _tsxmax   = 240; // = rotation 0, 2
            int16_t   _tsymax   = 320;
            int16_t   _rot      = 2;
            uint8_t   _ledpin   = 0;
            uint8_t   _LED_ON   = 1;
            uint8_t   _isinit   = FALSE;
            uint8_t   _mode     = MD_DEF;

          public:
            md_touch(Adafruit_ILI9341*    pTFT,
                     XPT2046_Touchscreen* ptouch,
                     md_TouchEvent*       ptouchev,
                     uint8_t tft_LED, uint8_t led_ON, md_spiffs *pflash); //, uint8_t spi_bus = VSPI);
            ~md_touch();

            void start      (uint8_t rotation, uint8_t doTask = TRUE);
            void wrText     (const char* msg, uint8_t spalte, uint8_t zeile, uint8_t len); // write to text area
            void wrText     (String msg, uint8_t spalte, uint8_t zeile, uint8_t len); // write to text area
            void setRotation(uint8_t rotation);
            void run        ();
            void mapXY      (POINT_t* pPt);
            void doCalibration();
            uint8_t rotation();
            void suspend();
            void resume();

          private:
            void startTFT();
            // calibration
            void loadCalibration();
            void checkCalibration();
            void saveCalibration();
        };
  // --- menu control   // menu timing
    // basics
      #define MENU_TASK_SLEEP_US  10000ul
      #define MENU4BIT_MASK       0x0000000Fu
    // menu format
      #define COL_BACK       COL16_BLACK
      #define BOX_GRID       20
      #define BOX_X_MAX      12
      #define BOX_Y_MAX      16
      #define VAL_L_POS      1
      #define VAL_L_SIZE     6
      #define VAL_V_POS      VAL_L_POS + VAL_L_SIZE
      #define VAL_V_SIZE     4
      #define MENU_MAXLINES  BOX_Y_MAX - 2
      #define MENU_TXTMAX    25
      #define MENU_VALMAX    8
      #define MENU_FONT      (&AT_BoldOblique9pt7b) // ArialRoundedMTBold_14
      typedef enum MENU_ACTION : uint8_t
        {
          ACTNO = 0,
          PREVPAGE,
          NEXTPAGE,
          HIDE,
          SHOW,
          DOIT,
          ACTMAX
        } MENU_ACTION_t;
      typedef enum BOXCOMMODE_t : uint8_t
        {
          COLMODE_BOXDEF = 0,
          COLMODE_BOXSEL,
          COLMODE_BOXWRN,
          COLMODE_BOXPAS,
          COLMODE_BOXMAX,
          COLMODE_BOXRDY
        } BOXCOLMODE_t;
    // menu colors
      typedef struct COLORS_t
        {
          md_colors coldef;
          md_colors colsel;
          md_colors colwrn;
          md_colors colpas;

          md_colors col(uint8_t mode)
            {
              switch (mode % 4)
                {
                  case 0: return coldef;
                  case 1: return colsel;
                  case 2: return colwrn;
                  default: return colpas;
                }
              return coldef;
            }
        } COLORS_t;
    // menu colors
      const static COLORS_t  defBoxCols     = { MD_BLACK,
                                                MD_BLACK,
                                                MD_RED_MEDIUM,
                                                MD_BLACK };
      const static COLORS_t  statitBoxCols  = { MD_BLACK,
                                                MD_BLACK ,
                                                MD_RED_MEDIUM,
                                                MD_BLACK };
      const static COLORS_t  curBoxCols     = { MD_BLACK,
                                                MD_BLACK ,
                                                MD_RED_MEDIUM,
                                                MD_BLACK };
      const static COLORS_t  bmpBoxCols     = { MD_BLACK,
                                                MD_BLACK ,
                                                MD_RED_MEDIUM,
                                                MD_BLACK };
      const static COLORS_t  defTxtCols     = { MD_GREEN_MEDIUM,
                                                MD_CYAN_MEDIUM,
                                                MD_GREENYELLOW,
                                                MD_OLIVE };
      const static COLORS_t  statitTxtCols  = { MD_GREEN_LOW,
                                                MD_CYAN_MEDIUM,
                                                MD_GREENYELLOW,
                                                MD_OLIVE };
      const static COLORS_t  curTxtCols     = { MD_CYAN_MEDIUM,
                                                MD_YELLOW_HIGH,
                                                MD_GREENYELLOW,
                                                MD_OLIVE };
      const static COLORS_t  bmpTxtCols     = { MD_GREEN_MEDIUM,
                                                MD_CYAN_MEDIUM,
                                                MD_GREENYELLOW,
                                                MD_GREY_LOW};
      const static COLORS_t* pdefBoxCols    = &defBoxCols;
      const static COLORS_t* pdefTxtCols    = &defTxtCols;
      const static COLORS_t* pstatitBoxCols = &statitBoxCols;
      const static COLORS_t* pstatitTxtCols = &statitTxtCols;
      const static COLORS_t* pcurBoxCols    = &curBoxCols;
      const static COLORS_t* pcurTxtCols    = &curTxtCols;
      const static COLORS_t* pbmpBoxCols    = &bmpBoxCols;
      const static COLORS_t* pbmpTxtCols    = &bmpTxtCols;
  // --- text defines
    // --- fonts
      #define TXT_CHAR1_W 6
      #define TXT_CHAR1_H 8
    // --- alignment
      #define TXT_LEFT    0
      #define TXT_CENTER  1
      #define TXT_RIGHT   2
    // --- menu text
      #define ITEM_TXT_MAXLEN_240 20
      #define ITEM_TXT_MAXLEN_320 26
    // --- menu size
      #define MENU_ROW1   3
      #define MENU_ROWMAX 12
      typedef struct MENUTEXT_t
        {
          char*     ptext   = NULL;
          uint16_t  txtlen  = 0;
          int16_t   tx = 1,  ty = 1;  // pos text  [pixels abs]
          uint16_t  tw = 10, th = 10; // size text [pixels rel]
          uint8_t   txtsize = 2;      // font scale
          uint8_t   align   = TXT_CENTER; // left - center - right
          uint8_t   tlenmax = 0;
          COLORS_t* ptxtcol = (COLORS_t*) pdefTxtCols;

          char* getText() { return ptext; }
          void operator= (MENUTEXT_t t)
            {
              ptext   = t.ptext;
              txtlen  = t.txtlen;
              txtsize = t.txtsize;
              align   = t.align;
              ptxtcol = t.ptxtcol;
            }
        } MENUTEXT_t;
  // --- struct ITEMID
    // ItemID
      /* ITEMID_t is an atomic bit coded value and
       * contains all information for navigation and evaluation
       * - atomic because it is used for inter task communucation
       * Bits:
       *   31-27 5 bits - 1. row    [unit grid]
       *   26-22 5 bits - 1. column [unit grid]
       *   21-19 3 bits - menu lev
       *   18-14 5 bits - width [unit grid]
       *   13-11 3 bits - it type
       *        =7: reserved
       *        =6: reserved
       *        =5: reserved
       *        =4: link
       *        =3: input  value with edit link
       *        =2: output value (type = valtype)
       *        =1: in/out label
       *        =0: text
       *   10- 7 reserved
       *    6- 2 menu status
       *       6 req draw    0x10u
       *       5 is selected 0x08u
       *       4 has link    0x04u
       *       3 selectable  0x02u
       *       2 visable     0x01u
       *    1- 0 color mode
       *        =0: def   default
       *        =1: sel   select
       *        =2: wrn   warning
       *        =3: eme   emergency
       */
      typedef enum CODE_MASK : uint32_t
        {
          // screen position    - static => no clr
            MENUSCR_MASK_ROW     = 0xF8000000u, // 5 bits 31-27 row [grid]
            MENUSCR_MASK_COL     = 0x07C00000u, // 5 bits 26-22 column [grid]
            MENUSCR_MASK_PAGE    = 0x00380000u, // 3 bits 21-19 screen line all levels
                MENPAGE_ALL      = 255,
            MENUSCR_MASK_COLS    = 0x0007C000u, // 5 bits 18-14 width [grid]
            MENUSCR_SHIFT_ROW    = 27,
            MENUSCR_SHIFT_COL    = 22,
            MENUSCR_SHIFT_PAGE   = 19,
            MENUSCR_SHIFT_COLS   = 14,
          // it type
            MENUTYP_MASK_ITEM    = 0x00003800u, // bits  13-11
            MENUSCR_SHIFT_TYP    = 11,
                MENTYP_RES7      = 7, // reserved
                MENTYP_CTRL      = 6, // menu control button
                MENTYP_BITMAP    = 5, // bitmap
                MENTYP_LINK      = 4, // link
                MENTYP_EDVAL     = 3, // input & link (type VAL)
                MENTYP_OUTVAL    = 2, // out value (type VAL)
                MENTYP_TEXT      = 1, // I/O label
                MENTYP_EDTEXT    = 0, // text
          // value type
            MENUTYP_MASK_VAL     = 0x0000060u, // bits  10-9
            MENUSCR_SHIFT_VAL    = 9,
                MENVAL_BOOL      = 3, // not used
                MENVAL_INT       = 2, // integer
                MENVAL_FLOAT     = 1, // float
                MENVAL_TEXT      = 0, // text
          // reserved
            MENUMOD_MASK_RES     = 0x00000100u, // reserved bit 8
            MENUSCR_SHIFT_RES    = 8,
          // menu status
            MENUMOD_MASK_STAT    = 0x000000FBu, // bits  6- 3 screen line in main menu level 3
            MENUMOD_SHIFT_MENU   = 2,
                MENBIT_DOUBLE    = 32,// bit 7  is double clicked
                MENBIT_TODRAW    = 16,// bit 6  is to be displayed
                MENBIT_ISSEL     = 8, // bit 5  selected - provided for decoding
                MENBIT_HASLINK   = 4, // bit 4  active   - provided for decoding
                MENBIT_TOSEL     = 2, // bit 3  selectable provided for decoding
                MENBIT_RES1      = 1, // bit 2  visible
                MENBIT_NO        = 0,
          // colmode
            MENUMOD_MASK_COLOR   = 0x00000003u, // bits  1-0 screen line in main menu level 3
            MENUMOD_SHIFT_COLOR  = 0,
                MENMOD_COLPAS    = 3,
                MENMOD_COLWRN    = 2,
                MENMOD_COLSEL    = 1,
                MENMOD_COLDEF    = 0
        } CODE_MASK_t;
                                                // screen define masks
      typedef struct ITEMID_t
        {
          uint8_t scrrow  = 1; // 5 bits - screen row    grid [0 - SCRROW_MAX]
          uint8_t scrcol  = 1; // 5 bits - screen column grid [0 - SCRCOL_MAX]
          uint8_t menpage = 0; // 3 bits - menu level
          uint8_t itw     = BOX_X_MAX; // 5 bits - it width    grid [0 - SCRCOL_MAX]
          uint8_t ith     = 1; // 5 bits - it width    grid [0 - SCRCOL_MAX]
          uint8_t bmpwh   = 16;
          uint8_t ittyp   = 0; // 3 bits - it type
          uint8_t valtyp  = 0; // 2 bits - value type
          uint8_t menstat = MENBIT_TODRAW; // 8 bits - menu act status
          uint8_t colmod  = 0; // 2 bits - color work mode [0..3]

          uint32_t itID()
            {
              return (  (scrcol<<MENUSCR_SHIFT_COL)   + (scrrow<<MENUSCR_SHIFT_ROW)
                      + (itw<<MENUSCR_SHIFT_COLS)   + (menpage<<MENUSCR_SHIFT_PAGE)
                      + (ittyp<<MENUSCR_SHIFT_TYP)  + (valtyp<<MENUSCR_SHIFT_VAL)
                      + (menstat<<MENUMOD_SHIFT_MENU) + (colmod<<MENUMOD_SHIFT_COLOR)
                     );
            }
          void operator=(uint32_t id)
            {
              S2HEXVAL(" = colmod itid ", colmod, itID());
              colmod  = (uint8_t) (id & MENUMOD_MASK_COLOR) >> MENUMOD_SHIFT_COLOR;
              S2HEXVAL(" = colmod itid ", colmod, itID());
              menstat = (uint8_t) (id & MENUMOD_MASK_STAT)  >> MENUMOD_SHIFT_MENU;
              menpage = (uint8_t) (id & MENUSCR_MASK_PAGE)  >> MENUSCR_SHIFT_PAGE;
              valtyp  = (uint8_t) (id & MENUTYP_MASK_VAL)   >> MENUSCR_SHIFT_VAL;
              ittyp   = (uint8_t) (id & MENUTYP_MASK_ITEM)  >> MENUSCR_SHIFT_TYP;
              itw     = (uint8_t) (id & MENUSCR_MASK_COLS)  >> MENUSCR_SHIFT_COLS;
              scrcol  = (uint8_t) (id & MENUSCR_MASK_COL)   >> MENUSCR_SHIFT_COL;
              scrrow  = (uint8_t) (id & MENUSCR_MASK_ROW)   >> MENUSCR_SHIFT_ROW;
            }
        } ITEMID_t;
  // --- class md_menu
    // menu it
      class MENUITEM_t : public md_cell //: public tBox, public tBoxColors, public tText, public cItemID //, public md_cell
        {
          public:
            POINT_t    boxpos;
            POINT_t    boxsize;
            COLORS_t*  pboxcols;
            MENUTEXT_t boxtext;
            ITEMID_t   menuid;
            uint8_t    maxtxtlen;
            void*      pobj = NULL; // pointer to bitmap, function, menu it

            char*   printbox(char *p)
              {
                sprintf(p, "%3i %3i %3i %3i", boxpos.x, boxpos.y, boxsize.x, boxsize.y);
                return p;
              }
            void    wrText()
              {
                //SVAL(" .. wrText ", boxtext.ptext);
                menuid.menstat |= MENBIT_TODRAW;
                //SHEXVAL("    wrText menstat ", menuid.menstat);
              }
            void    operator=(MENUITEM_t b)
              {
                boxpos    = b.boxpos;
                boxsize   = b.boxsize;
                pboxcols  = b.pboxcols;
                boxtext   = b.boxtext;
                menuid    = b.menuid;
                maxtxtlen = b.maxtxtlen;
              };
            uint8_t isItem(POINT_t *p)
              {
                if (   (p->x < boxpos.x) || (p->x > (boxpos.x + boxsize.x))
                    || (p->y < boxpos.y) || (p->y > (boxpos.y + boxsize.y)))
                  {
                    return FALSE;
                  }
                return TRUE;
              }
        } ;
      typedef uint32_t ITEMCNT_t ; // 4 bit m3: <<12, m2: <<8, m1: <<4, m0
    // class md_menu
      class md_menu     // home made
        {
          private:
            md_list*     _pMenu      = NULL;
            POINT_t      _backpos;
            POINT_t      _backsize;
            uint8_t      _pages      = 0;
            uint8_t      _rows[8]    = {0,0,0,0,0,0,0,0};
            uint8_t      _actPage    = 0; // def main menu
            uint8_t      _newPage    = 0; // def main menu
            uint8_t      _actRow     = 0; // def main menu
            uint8_t      _newRow     = 0; // def main menu
            uint8_t      _isInit     = FALSE;
          public:
            md_menu();
            ~md_menu();

            void init     (md_list* pMenu, MENUITEM_t* pTitle, MENUITEM_t* pStatus);
            void begin    (uint8_t doTask = TRUE);
            void wrStatus (const char *msg = NULL, uint16_t msTOut = 1, uint8_t colormode = MD_DEF);
            void wrStatus (String msg, uint16_t msTOut = 1, uint8_t colormode = MD_DEF)
              { wrStatus(msg.c_str(), msTOut, colormode); }
            void wrTitle  (const char *msg = NULL, uint8_t colormode = MD_DEF);
            void wrTitle  (String msg, uint8_t colormode = MD_DEF)
              { wrTitle(msg.c_str(), colormode); }
            void run();
            void show(MENUITEM_t* pit = NULL, uint8_t maxWidth = FALSE);
            void wrback();
            MENUITEM_t* getItem(POINT_t* ptouchP);


            void drawEntry();
            void suspend();
            void resume();
        };
  // --- calibration
    // basics
      #define TCAL_FILE_0   "/Touchtest_0.dat"
      #define TCAL_FILE_1   "/Touchtest_1.dat"
      #define TCAL_FILE_2   "/Touchtest_2.dat"
      #define TCAL_FILE_3   "/Touchtest_3.dat"
      #define TCAL_FILE_DEF TCAL_FILE_2
      #define TCAL_ROT_DEF  2   // SD right hand side
    // --- class calData
      class md_tcalData
        {
          private:
            uint16_t _xmin = 230;
            uint16_t _ymin = 350;
            uint16_t _xmax = 3700;
            uint16_t _ymax = 3900;
          public:
            /*
                tPoint x0y0 = {10, 10};
                tPoint x0y1 = {10, 310};
                tPoint x1y0 = {230, 10};
                tPoint x1y1 = {230, 310};
             */
            md_tcalData()  {}
            ~md_tcalData() {}

            void setCalib(uint16_t xmin, uint16_t ymin, uint16_t xmax, uint16_t ymax)
              { _xmin = xmin; _xmax = xmax; _ymin = ymin; _ymax = ymax; }
            void  setxmin(uint16_t xmin) {
              SVAL(" setxmin ", xmin);
              _xmin = xmin; }
            void  setymin(uint16_t ymin) { _xmin = ymin; }
            void  setxmax(uint16_t xmax) { _xmax = xmax; }
            void  setymax(uint16_t ymax) { _ymax = ymax; }
            uint16_t xmin()  { return _xmin; }
            uint16_t ymin()  { return _ymin; }
            uint16_t xmax()  { return _xmax; }
            uint16_t ymax()  { return _ymax; }
            /*
                virtual char *printPts56(char *pcalData)
                  {
                    sprintf(pcalData, "points P00 %3i %3i P01 %3i %3i P10 %3i %3i P11 %3i %3i",
                            x0y0.x(), x0y0.y(), x0y1.x(), x0y1.y(),
                            x1y0.x(), x1y0.y(), x1y1.x(), x1y1.y());
                    return pcalData;
              }
             */
            char* printCal50(char *pcalVals)
              {
                sprintf(pcalVals, "calPos xmin %4i ymin %4i xmax %4i ymax %4i",
                        _xmin, _ymin, _xmax, _ymax);
                return pcalVals;
              }
        };
  // --- tasks
    // task control
      #define TTASK_ON_RUN  0
      #define TTASK_HSHAKE  1
      #define TTASK_ON_HOLD 2
      #define TTASK_NEWVAL TTASK_HSHAKE + TTASK_ON_HOLD
    // --- event handlers for touch & menu
      void IRAM_ATTR touchTask (void *pvParameters);
      void IRAM_ATTR menuTask  (void * pvParameters);

  // --- extern references
    extern MENUITEM_t* pitTitle;
    extern MENUITEM_t* pitStatus;
    extern MENUITEM_t* pitBMPleft;
    extern MENUITEM_t* pitBMPup;
    extern MENUITEM_t* pitBMPdown;
    extern MENUITEM_t* pitBMPright;
    extern MENUITEM_t* pitSmily;

#endif // _MD_TOUCH_H_
