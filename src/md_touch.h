//#ifdef _MD_TOUCH_H_
#ifndef _MD_TOUCH_H_
  #define _MD_TOUCH_H_

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
     //public Adafruit_ILI9341, public XPT2046_Touchscreen, public md_TouchEvent
      {
        public:
        	md_touch(uint8_t cspin, uint8_t tirq = 255); //, uint8_t spi_bus = VSPI);
          ~md_touch();

          //constexpr md_touch(uint8_t cspin, uint8_t tirq=255)
        	//	: _csPin(cspin), _tirqPin(tirq) { }
          void start(Adafruit_ILI9341* ptft, uint8_t rotation = 0, );
          //void calibratePoint(uint16_t x, uint16_t y, uint16_t* vi, uint16_t* vj);
          bool loadCalibration();
          void doCalibration();
          void calibrate();
          //void calibratePoint(uint16_t x, uint16_t y, uint16_t &vi, uint16_t &vj);
          bool saveCalibration(char* text, size_t len);
          //bool isTouched();
          //TS_Point getPoint();

        private:
          uint8_t _csPin    = 0;
          uint8_t _rotation = 1;
          long    _lastStateChange = 0;
          long    _lastTouched = 0;

          uint8_t _tirqPin  = 255;
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