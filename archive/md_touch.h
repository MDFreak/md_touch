//#define _MD_TOUCH_H_
#ifndef _MD_TOUCH_H_
  #define _MD_TOUCH_H_

  #include "FS.h"
  #include <SPI.h>
  #include <md_defines.h>
  #include <md_util.h>
  #include <md_grafx.h>
  #include <md_spiffs.h>
  #include <md_XPT2046.h>
  //#include <XPT2046A.h>
  //#include <XPT2046A_calib.h>
  //#include "XPT2046_Calibrated.h"
  //#include <XPT2046_Touchscreen.h>
  //#include <TouchControllerWS.h>
  #include <linked_list.hpp>
/*
  //--- point
    class TS_Point
      {
        public:
        	TS_Point(void)
                : x(0), y(0), z(0) {}
        	TS_Point(int16_t x, int16_t y)
                : x(x), y(y), z(0) {}
        	TS_Point(int16_t x, int16_t y, int16_t z)
                : x(x), y(y), z(z) {}

          bool operator==(TS_Point p)
                { return ((p.x == x) && (p.y == y) && (p.z == z)); }
        	bool operator!=(TS_Point p)
                { return ((p.x != x) || (p.y != y) || (p.z != z)); }

          int16_t x, y, z;
      };

  //--- calibration
    class TS_Calibration
      {
        public:
        	constexpr TS_Calibration(void):
        		defined(false),
        		screenWidth(0U), screenHeight(0U),
        		deltaX(0), deltaY(0),
        		alphaX(0.0F), betaX(0.0F), alphaY(0.0F), betaY(0.0F) {}
        	constexpr TS_Calibration(float aX, float bX, int32_t dX, float aY, float bY, int32_t dY, uint16_t sW, uint16_t sH):
        		defined(true),
        		screenWidth(sW), screenHeight(sH),
        		deltaX(dX), deltaY(dY),
        		alphaX(aX), betaX(bX), alphaY(aY), betaY(bY) {}
        	TS_Calibration(
        		const TS_Point aS, const TS_Point aT,
        		const TS_Point bS, const TS_Point bT,
        		const TS_Point cS, const TS_Point cT,
        		const uint16_t sW, const uint16_t sH);
        	bool defined;
        	uint16_t screenWidth, screenHeight;
        	int32_t deltaX, deltaY;
        	float alphaX, betaX, alphaY, betaY;
      };

    class XPT2046_Calibrated
      {
        public:
        	constexpr XPT2046_Calibrated(uint8_t cspin, uint8_t tirq=255)
        		: csPin(cspin), tirqPin(tirq) { }
        	bool begin();
        	TS_Point getPoint();
        	bool tirqTouched();
        	bool touched();
        	void readData(uint16_t *x, uint16_t *y, uint8_t *z);
        	bool bufferEmpty();
        	uint8_t bufferSize() { return 1; }
        	void setRotation(uint8_t n) { rotation = n % 4; }
        	void calibrate(TS_Calibration c) { cal = c; }
        // protected:
        	volatile bool isrWake=true;

        private:
        	void update();
        	uint8_t csPin, tirqPin, rotation=1;
        	int16_t xraw=0, yraw=0, zraw=0;
        	uint32_t msraw=0x80000000;
        	TS_Calibration cal;
      };

    #ifndef ISR_PREFIX
      #if defined(ESP8266)
        #define ISR_PREFIX ICACHE_RAM_ATTR
      #elif defined(ESP32)
        // TODO: should this also be ICACHE_RAM_ATTR ??
        #define ISR_PREFIX IRAM_ATTR
      #else
        #define ISR_PREFIX
      #endif
    #endif
*/
    void    actLev(uint8_t level);
    uint8_t actLev(void);
    void    oldLev(uint8_t level);
    uint8_t oldLev(void);
    void    actLev(uint8_t level);
    uint8_t actLev(void);
    void    oldLev(uint8_t level);
    uint8_t oldLev(void);

    // --- tasks
    void handleTouch(void * pvParameters);
    void handleMenu(void * pvParameters);

  //--- class md_touch
    /*____Calibrate TFT LCD_____*/
    #define TS_MINX 325
    #define TS_MINY 200
    #define TS_MAXX 3850
    #define TS_MAXY 3700

    #define TS_CAL_FILE   "touchcal.dat"
    #define TS_CAL_MAXLEN 50

    class md_touch
      {
        public:
        	md_touch(uint8_t cspin, uint8_t tirq = 255, uint8_t spi_bus = VSPI);
          ~md_touch();

          //constexpr md_touch(uint8_t cspin, uint8_t tirq=255)
        	//	: _csPin(cspin), _tirqPin(tirq) { }
          void begin(mdGrafx* pgfx, uint8_t rotation = 0);
          //void calibratePoint(uint16_t x, uint16_t y, uint16_t* vi, uint16_t* vj);
          bool loadCalibration();
          void calibrate();
          void calibratePoint(uint16_t x, uint16_t y, uint16_t &vi, uint16_t &vj);
          bool saveCalibration(char* text, size_t len);
          bool isTouched();
              #ifdef ALT
                        bool isCalibrationFinished();
                        void continueCalibration();
                        bool isTouched(int16_t debounceMillis);
              #endif
          //TS_Point getPoint();

        private:
          uint8_t _csPin    = 0;
          uint8_t _rotation = 1;
          long    _lastStateChange = 0;
          long    _lastTouched = 0;

          uint8_t _tirqPin  = 255;
          //float   dx = 0.0;
          //float   dy = 0.0;
          //int     ax = 0;
          //int     ay = 0;
          //int     state = 0;
          //TS_Point p1, p2;
      };

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

    class md_menu
      {
        private:
          md_spiffs* _pffs  = NULL;
          md_list*   _phead = NULL;
          md_list*   _ppages[MENU_MAXLINES]= { NULL };

          int16_t    _x     = 0;  // pos left edge of header text
          int16_t    _y[MENU_MAXLINES + 1]; // pos upper edge of header text
          int16_t    _wline  = 0;
          int16_t    _ym[MENU_MAXLINES + 1];
          int8_t     _hline = 16;

          uint8_t    _pages = 1;
          //uint8_t    _levs  = 1;
          uint8_t    _lines = 3;
          uint8_t    _on    = OFF;

        public:
          md_menu ();
          ~md_menu();

          void begin(mdGrafx* pgf,  // no SPIFFS
                     const int16_t x, const int16_t y, const int16_t w,
                     const uint8_t pages, const uint8_t lines, const uint8_t line_h);
          void begin(mdGrafx* pgfx, md_spiffs* pffs, // with storage in SPIFFS \spiffs\touchcal
                     const int16_t x, const int16_t y, const int16_t w,
                     const uint8_t pages, const uint8_t lines, const uint8_t line_h);
          void load (const char* entries, const uint8_t cnt, uint8_t mtype = MENUTYPE_MAIN);
          void show ();
          void hide ();

        protected:
          void    drawEntry(const char* txt, uint8_t line, uint16_t color = MD4_WHITE);
      };

  //--- bitmap handler ----------------------
    class bmp_pgm
      {
        private:
          BMPHeader_t   _head;
          const char*   _pbmp = NULL;
          mdGrafx*      _ptft = NULL;
          int16_t       _posx = NN;
          int16_t       _posy = NN;

        public:
      	  bmp_pgm(mdGrafx *ptft);
      	  ~bmp_pgm();

          void draw(const char *palBmp, int16_t x = 0, int16_t y = 0);  // left upper position
      };

#endif // _MD_TOUCH_H_
#ifdef PARKEN
    #include <SPIFFS.h>
    #include <XPT2046_Touchscreen.h>

    #ifndef _TOUCH_CONTROLLERWSH_
    #define _TOUCH_CONTROLLERWSH_

    /*____Calibrate TFT LCD_____*/
    #define TS_MINX 370
    #define TS_MINY 470
    #define TS_MAXX 3700
    #define TS_MAXY 3600
    /*______End of Calibration______*/

    typedef void (*CalibrationCallback)(int16_t x, int16_t y);

    class TouchControllerWS {
      public:
        TouchControllerWS(XPT2046_Touchscreen *touchScreen);
        bool loadCalibration();
        bool saveCalibration();
        void startCalibration(CalibrationCallback *callback);
        void continueCalibration();
        bool isCalibrationFinished();
        bool isTouched();
        bool isTouched(int16_t debounceMillis);
        TS_Point getPoint();

      private:
        XPT2046_Touchscreen *touchScreen;
        float dx = 0.0;
        float dy = 0.0;
        int ax = 0;
        int ay = 0;
        int state = 0;
        long lastStateChange = 0;
        long lastTouched = 0;
        CalibrationCallback *calibrationCallback;
        TS_Point p1, p2;

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
    #define STAT_X 120 // Centred on this
    #define STAT_Y 65

    // Number length, buffer for storing it and character index
    #define NUM_LEN 12
  #endif

