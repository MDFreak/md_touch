//#ifdef _MD_TOUCH_H_
#ifndef _MD_TOUCH_H_
  #define _MD_TOUCH_H_

  #ifndef DEBUG_MODE
      #define DEBUG_MODE  CFG_DEBUG_STARTUP
    #endif
  //#include "FS.h"
  #include <SPI.h>
  #include <md_defines.h>
  #include <md_util.h>
  #include <md_spiffs.h>
  #include <XPT2046_Touchscreen.h>
  #include <Adafruit_GFX.h> //Grafik Bibliothek
  #include <Adafruit_ILI9341.h> // Display Treiber
  #include <md_TouchEvent.h> //Auswertung von Touchscreen Ereignissen
  #include <linked_list.hpp>

  // --- color definitions
    #define MD_BLACK       0x0000      /*   0,   0,   0 */
    #define MD_NAVY        0x000F      /*   0,   0, 128 */
    #define MD_DARKGREEN   0x03E0      /*   0, 128,   0 */
    #define MD_DARKCYAN    0x03EF      /*   0, 128, 128 */
    #define MD_MAROON      0x7800      /* 128,   0,   0 */
    #define MD_PURPLE      0x780F      /* 128,   0, 128 */
    #define MD_OLIVE       0x7BE0      /* 128, 128,   0 */
    #define MD_LIGHTGREY   0xC618      /* 192, 192, 192 */
    #define MD_DARKGREY    0x7BEF      /* 128, 128, 128 */
    #define MD_BLUE        0x001F      /*   0,   0, 255 */
    #define MD_GREEN       0x07E0      /*   0, 255,   0 */
    #define MD_CYAN        0x07FF      /*   0, 255, 255 */
    #define MD_RED         0xF800      /* 255,   0,   0 */
    #define MD_MAGENTA     0xF81F      /* 255,   0, 255 */
    #define MD_YELLOW      0xFFE0      /* 255, 255,   0 */
    #define MD_WHITE       0xFFFF      /* 255, 255, 255 */
    #define MD_ORANGE      0xFD20      /* 255, 165,   0 */
    #define MD_GREENYELLOW 0xAFE5      /* 173, 255,  47 */
    #define MD_PINK        0xF81F

  // Parameter für Touchscreen
  #define MINPRESSURE 10 //pressure to detect touch
  #ifndef TS_MINX
      #define TS_MINX 230 //minimal x return value
      #define TS_MINY 350 //minimal y return value
      #define TS_MAXX 3700  //maximal x return value
      #define TS_MAXY 3900 //maximal y return value
    #endif
        //496 //262 //3997 //3925

    typedef struct calData
      {
        uint16_t xmin = TS_MINX;
        uint16_t xmax = TS_MAXX;
        uint16_t ymin = TS_MINY;
        uint16_t ymax = TS_MAXY;
        TS_Point x0y0 = {10,  10,  0};
        TS_Point x0y1 = {10,  310, 0};
        TS_Point x1y0 = {230, 10,  0};
        TS_Point x1y1 = {230, 310, 0};
      } calData_t;

  void    actLev(uint8_t level);
  uint8_t actLev(void);
  void    oldLev(uint8_t level);
  uint8_t oldLev(void);
  void    actLev(uint8_t level);
  uint8_t actLev(void);
  void    oldLev(uint8_t level);
  uint8_t oldLev(void);

  // --- tasks
    //void handleTouch(void * pvParameters);
    //void handleMenu(void * pvParameters);


  //--- class md_touch
    //#define TS_MINX 325
    //#define TS_MINY 200
    //#define TS_MAXX 3850
    //#define TS_MAXY 3700

    //#define TS_CAL_FILE   "touchcal.dat"
    //#define TS_CAL_MAXLEN 50

    class md_touch
     //public Adafruit_ILI9341, public XPT2046_Touchscreen,
      {
        public:
        	md_touch(uint8_t cspin, uint8_t tft_CS, uint8_t tft_DC,  uint8_t tft_RST,
                                                  uint8_t tft_LED, uint8_t led_ON); //, uint8_t spi_bus = VSPI);
          ~md_touch();

          bool    start(uint8_t rotation, uint16_t background = MD_BLACK);
            // calibration
          bool    loadCalibration();
          bool    doCalibration();
          bool    checkCalibration();
          bool    saveCalibration(char* text, size_t len);
            // properties
          void    rotation(uint8_t _rotatio) { _rotation = _rotatio; }
          uint8_t rotation()                 { return _rotation; }
          //bool isTouched();
          //TS_Point getPoint();


        private:
          uint8_t  _rotation = 1;
          uint8_t  _tft_LED  = 0;
          uint8_t  _LED_ON   = 1;
          uint16_t _COL_BACK = MD_BLACK;



          //long    _lastStateChange = 0;
          //long    _lastTouched = 0;

          //uint8_t _tirqPin  = 255;
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
        ACTNO    = 0,
        //PREV,
        //NEXT,
        PREVPAGE,
        NEXTPAGE,
        HIDE,
        SHOW,
        DOIT,
        ACTMAX
      } MENU_ACT_t;

    #define MENU_TXTLEN    25
    #define MENUTYPE_MAIN  0
    #define MENUTYPE_PAGE  1
    #define MENU_MAXLINES  8
    #define MENU_FONT      ArialRoundedMTBold_14
#endif // PARKEN