#ifndef NOTREADY
  // --- includes
    #include <md_touch.h>
    #include <md_spiffs.h>
    #include <string>
    #include <ArialRounded.h>
    #include <md_defines.h>

  // --- globals
    static XPT2046_Touchscreen *_ptouch   = NULL;
    static Adafruit_ILI9341    *_pTFT     = NULL;
    static MENUITEM_t          *_pTitle   = NULL;
    static MENUITEM_t          *_pStatus  = NULL;
    static md_touch            *_pmdtouch = NULL;
    static md_TouchEvent       *_ptouchev = NULL;
    static md_menu             *_pmdmenu  = NULL;
    static md_spiffs           *_pflash   = NULL;
    static md_tcalData         _calData;
    static uint8_t             _rotation  = TCAL_ROT_DEF;
    static uint8_t             _back_row  = 1;
    static uint8_t             _back_col  = 2;
    static uint8_t             _ROWS_BACK = BOX_X_MAX;
    static uint8_t             _COLS_BACK = BOX_Y_MAX;
    static char                _tCalFile[]= TCAL_FILE_DEF;
    static uint8_t             _iscal     = MD_UNDEF;
           uint16_t a;
           uint16_t b;
           uint16_t c;
           uint16_t d;
    static msTimer             _TStat     = msTimer(5);
  // specials
    // touch task internal variables
      static uint32_t     _tlastT    = 0;
      static uint32_t     _tactT     = 0;
      static uint32_t     _tdiffT    = 0;
      static TaskHandle_t _ptouchTask = NULL;
      static TS_Point     _touchRaw;
      static POINT_t      _touchP;
      static uint8_t      _isclicked = 0;
      static uint8_t      _touch     = 0;
      static uint8_t      _touchlast = 0;
      static uint32_t     _touchTmp  = 0;
      static uint32_t     _rawTmp    = 0;
    // menu task variables
      static uint32_t     _mlastT    = 0;
      static uint32_t     _mactT     = 0;
      static uint32_t     _mdiffT    = 0;
      static TaskHandle_t _pmenuTask = NULL;
      static POINT_t      _menuP;
      static uint32_t     _menuID    = 0;
      static uint32_t     _menuTmp   = 0;
    // atomic communication with mdtouch task
      // valid _tactRaw value cannot be 0 => handshake free
      // _tactXY value is not allowed     => handshake free
      // 0: wr: touch task / >0 rd->wr: menu task (!! _tactXY = 0; _tactRaw = 0)
        IRAM_ATTR uint32_t    _tactRaw      = 0; // last touched point (raw type)
        IRAM_ATTR uint32_t    _tactXY       = 0; // last touched point (TFT converted)
      // 0: wr: menu task  / >0 rd->wr: poll function
        IRAM_ATTR MENUITEM_t *xtouchedItem   = NULL; // menu info      -> execute menu
        IRAM_ATTR uint32_t    xtouchedType = 0;
    // control values
        int8_t                _tCtrlTouch   = TTASK_ON_RUN; // ext  TTASK_ON_RUN:  handshake off
                                                          // ext  TTASK_HSHAKE:  handshake , value treated, waiting for touch
                                                          // ext  TTASK_ON_HOLD: touch on hold
                                                          // task TTASK_NEWVAL:  handshake , value new, waiting for treatment
        int8_t                _tCtrlMenu    = TTASK_ON_RUN; // ext  TTASK_ON_RUN:  handshake off
                                                          // ext  TTASK_HSHAKE:  handshake , value treated, waiting for touch
                                                          // ext  TTASK_ON_HOLD: touch on hold
                                                          // task TTASK_NEWVAL:  handshake , value new, waiting for treatment
        //const uint32_t    _tWait_Touch_us = 10000; // task time tick (minimum = 1000 ~ 1 msec)
        //const uint32_t    _tWait_Menu_us  = 10000; // task time tick (minimum = 1000 ~ 1 msec)
    //static TaskHandle_t menuTask_  = NULL;
  // task touchTask
    void IRAM_ATTR touchTask(void * pvParameters)
      {
        //unsigned long lastWD, actWD, diff;

        //esp_task_wdt_init(WDT_TTIMEOUT, true); //enable panic so ESP32 restarts
        //esp_task_wdt_add(NULL);                //add current thread to WDT watch

        disableLoopWDT();
        _tlastT = millis();
        while(TRUE)  // endless loop
          {
            //esp_task_wdt_reset();
            if (_tCtrlTouch < TTASK_ON_HOLD) // only poll, if touch is released from extern
              {
                _tactT  = millis();
                _tdiffT = _tactT - _tlastT;
                  //_touch = _ptouchev->getTouchPos(&_touchP, &_touchRaw);
                _touchRaw = _ptouch->getPoint();
                _touch = FALSE;
                if (_touchRaw.z > MINPRESSURE)
                  {
                    if (   (_touchRaw.x > 10) && (_touchRaw.x < 4000)
                        && (_touchRaw.y > 10) && (_touchRaw.y < 4000))
                    _touch = TRUE;
                  }
                if (_touch) // touched
                  {
                    if (!_touchlast) // new touched
                      {
                        _tlastT    = _tactT;         // start touchtime
                        _touchlast = TRUE;
                              //_touchP.x = map(_touchRaw.x, DEF_2_MINX, DEF_2_MAXX, 0, DEF_2_TFTW);
                              //_touchP.y = map(_touchRaw.y, DEF_2_MINY, DEF_2_MAXY, 0, DEF_2_TFTH);
                              //S2VAL(" new touch x y ", _touchP.x,   _touchP.y);
                              //S2VAL(" new raw x y ",   _touchRaw.x, _touchRaw.y);
                        usleep(TOUCH_CLICK_TIME_MS);
                      }
                    else // still touched
                      {
                        if (_tdiffT > TOUCH_CLICK_TIME_MS)
                          {
                            _isclicked = TRUE;
                                //SVAL(" touch time ms ", _tdiffT);
                            if (_tdiffT > TOUCH_LONG_TIME_MS) // long click
                              {
                                if ( (_touchTmp & TOUCH_TYPE_MAX) != TOUCH_TYPE_LONG ) // only once
                                  {
                                    _touchTmp = TOUCH_TYPE_LONG;  // write type
                                          //S2VAL   (" long touch x y ", _touchP.x,   _touchP.y);
                                          //S2HEXVAL(" long raw x y ",   _touchRaw.x, _touchRaw.y);
                                          //S2HEXVAL(" long _rawTmp _touchTmp ", _rawTmp, _touchTmp);
                                  }
                              }
                            else // normal click
                              {
                                if ( _touchTmp == 0 ) // only once
                                  {
                                    _touchTmp  =  TOUCH_TYPE_CLICK;
                                        //_touchTmp +=  (_touchP.x << TOUCH_HPOINT_SHIFT) + _touchP.y;
                                    _rawTmp    =  (_touchRaw.x << TOUCH_HRAW_SHIFT) + _touchRaw.y;
                                          //S2VAL   (" norm touch x y ", _touchP.x,   _touchP.y);
                                          //S2HEXVAL(" norm raw x y ",   _touchRaw.x, _touchRaw.y);
                                          //S2HEXVAL(" norm _rawTmp _touchTmp ", _rawTmp,     _touchTmp);
                                  }
                              }
                          }
                        usleep(TOUCH_TASK_SLEEP_US);
                      }
                  }
                else // not touched
                  {
                    if (!_touchlast) // untouched
                      {
                      }
                    else // released
                      {
                        if (_isclicked > FALSE) // touch evaluated
                          {
                            while (_tactRaw > 0) { usleep(20); }// handshake blocked
                            _tactXY   = _touchTmp;
                            _tactRaw  = _rawTmp;
                                  //S2HEXVAL(" output _rawTmp _touchTmp ", _rawTmp,     _touchTmp);
                                  //S2VAL   (" _rawTmp x y ",             (_rawTmp >> TOUCH_HRAW_SHIFT) & TOUCH_HRAW_MASK , _rawTmp & TOUCH_HRAW_MASK);
                                  //S3VAL   (" _touchTmp x y type",       (_touchTmp >> TOUCH_HPOINT_SHIFT) & TOUCH_HPOINT_MASK,
                                  //                                           _touchTmp  & TOUCH_HPOINT_MASK,
                                  //                                          (_touchTmp  & TOUCH_TYPE_MAX) >> TOUCHED_TYPE_SHIFT);
                            // reset everything
                            _touchlast = FALSE;
                            _touchTmp  = 0;
                            _rawTmp    = 0;
                            _isclicked = FALSE;
                          }
                      }
                    usleep(TOUCH_TASK_SLEEP_US);
                  }
              }
            else
              {
                usleep(TOUCH_TASK_SLEEP_US);
              }
          }
                 /*if (pts_->isTouching())
                    {
                      SOUTLN("screen touched ");
                      pts_->getPosition(x, y);
                      SOUT("position x"); SOUT(x); SOUT(", y "); SOUTLN(y);
                        //delay(30);
                        //pts->readData(&x, &y, &z);
                        //SOUT("x, y, z "); SOUT(x); SOUT(", "); SOUT(y); SOUT(", "); SOUTLN(z);
                      //pts->isrWake = false;
                    }
                   */
      }
    void startTouchTask()
      {
        xTaskCreatePinnedToCore(
                            touchTask,             /* Task function. */
                            "touchTask",           /* name of task. */
                            10000,                 /* Stack size of task */
                            NULL,                  /* parameter of the task */
                            4 | portPRIVILEGE_BIT, /* priority of the task */
                            &_ptouchTask,          /* Task handle to keep track of created task */
                            0              );      /* pin task to core 0 */
        SOUTLN("touchTask started on core 0");
      }
  // task menuTask
    void IRAM_ATTR menuTask(void * pvParameters)
      {
        //unsigned long lastWD, actWD, diff;
        //esp_task_wdt_init(WDT_TTIMEOUT, true); //enable panic so ESP32 restarts
        //esp_task_wdt_add(NULL);                //add current thread to WDT watch
        //STXT(" menuTask started ");
        char        _out[MENU_TXTMAX];
        MENUITEM_t* _pactItem = NULL;
        _mlastT = millis();
              //STXT(" vor while ");
        while(true)  // endless loop
          {
            _mactT  = millis();
                //esp_task_wdt_reset();
                //STXT(" while 1 ");
            _mdiffT = _mactT - _mlastT;
            if (_tCtrlMenu < TTASK_ON_HOLD) // only poll, if touch is released from extern
               {
                if (_tactRaw > 0)
                  {
                    _menuTmp = _tactRaw;
                        //STXT(" menuTask 2 ");
                    _menuP.x = (_menuTmp >> TOUCH_HRAW_BITS) & TOUCH_HRAW_MASK;
                    _menuP.y = _menuTmp & TOUCH_HRAW_MASK;
                    _pmdtouch->mapXY(&_menuP);
                          //S3VAL("       _menuP x y rot ", _menuP.x, _menuP.y, _rotation );
                    if (_pactItem == NULL)
                      { // handshake = free
                        _pactItem = _pmdmenu->getItem(&_menuP);
                            if (_pactItem == NULL)
                              {
                                if (_tactXY == TOUCHED_LONG)
                                  {
                                    sprintf(_out, "tchL %d %d", _menuP.x, _menuP.y);
                                    //S2VAL("  touch long field x y long ", _menuP.x, _menuP.y);
                                  }
                                else
                                  {
                                    sprintf(_out, "tch %d %d", _menuP.x, _menuP.y);
                                    //S2VAL("  touch      field x y long ", _menuP.x, _menuP.y);
                                  }
                                _pmdmenu->wrStatus(_out, 2000);
                              }
                            else
                              {
                                while (xtouchedType > TOUCHED_NOTOUCH) { usleep(20); }
                                  { // handshake xtouchedType is free
                                    xtouchedItem = _pactItem;
                                    xtouchedType = _tactXY;
                                          if (_tactXY == 2)
                                            {
                                              sprintf(_out, "tchL %d %d idx %d ", _menuP.x, _menuP.y, _pactItem->index());
                                              //S3VAL("  touch long  x y idx ", _menuP.x, _menuP.y, _pactItem->index());
                                            }
                                          else
                                            {
                                              sprintf(_out, "tch %d %d idx %d ", _menuP.x, _menuP.y, _pactItem->index());
                                              //S3VAL("  touch       x y idx ", _menuP.x, _menuP.y, _pactItem->index());
                                            }
                                          _pmdmenu->wrStatus(_out, 2000);
                                    _pactItem = NULL;
                                    _tactXY   = TOUCHED_NOTOUCH;
                                  }
                              }
                      }
                    _tactXY  = 0;
                    _tactRaw = 0;
                  }
                //if (_pactItem != NULL)
              }
            //STXT(" end while ");
            usleep(MENU_TASK_SLEEP_US);
          }
      }
    void startMenuTask()
      {
        xTaskCreatePinnedToCore(
                            menuTask,              /* Task function. */
                            "menuTask",            /* name of task. */
                            10000,                 /* Stack size of task */
                            NULL,                  /* parameter of the task */
                            4 | portPRIVILEGE_BIT, /* priority of the task */
                            &_pmenuTask,            /* Task handle to keep track of created task */
                            0                                                                                                                                                                                                                                                                                                         );      /* pin task to core 0 */
        STXT("menuTask started on core 1");
      }
  // --- class md_touch
    // construtors
    	//md_touch::md_touch(uint8_t cspin,   uint8_t tft_CS, uint8_t tft_DC, uint8_t tft_RST,
      //                   uint8_t tft_LED, uint8_t led_ON, md_spiffs *pflash) //, uint8_t spi_bus)
    	md_touch::md_touch(Adafruit_ILI9341*    pTFT,
                         XPT2046_Touchscreen* ptouch,
                         md_TouchEvent*       ptouchev,
                         uint8_t tft_LED, uint8_t led_ON, md_spiffs *pflash) //, uint8_t spi_bus)
        {
          _pmdtouch = this;
          _pTFT     = pTFT;     // TFT   library Adafruit_ILI9341
          _ptouch   = ptouch;   // touch library XPT2046_Touchscreen
          _ptouchev = ptouchev; // event library md_TouchEvent
          _pflash   = pflash;   // internal flash
          _ledpin   = tft_LED;  // backlight switch
          _LED_ON   = led_ON;   // ON state for backlight
        }
      md_touch::~md_touch()
        {
          _pmdtouch = NULL;
        }
    // public implementation
      // initialising
        void md_touch::setRotation(uint8_t rotation)
          {
            _rotation = rotation %4;
          }
        uint8_t md_touch::rotation()
          {
            return _rotation;
          }
    // private implementation
      // initialising
        void md_touch::start(uint8_t rotation, uint8_t doTask)
          {
            if (!_isinit)
              {
                _rotation = rotation %4;
                switch (_rotation) // set default calibration values
                  {
                    case 0:
                        strcpy(_tCalFile,"/Touchtest_0.dat");
                        _calData.setxmin(DEF_0_MINX);
                        _calData.setymin(DEF_0_MINY);
                        _calData.setxmax(DEF_0_MAXX);
                        _calData.setymax(DEF_0_MAXY);
                      break;
                    case 1:
                        strcpy(_tCalFile,"/Touchtest_1.dat");
                        _calData.setxmin(DEF_1_MINX);
                        _calData.setymin(DEF_1_MINY);
                        _calData.setxmax(DEF_1_MAXX);
                        _calData.setymax(DEF_1_MAXY);
                      break;
                    case 2:
                        strcpy(_tCalFile,"/Touchtest_2.dat");
                        _calData.setxmin(DEF_2_MINX);
                        _calData.setymin(DEF_2_MINY);
                        _calData.setxmax(DEF_2_MAXX);
                        _calData.setymax(DEF_2_MAXY);
                      break;
                    default:
                        strcpy(_tCalFile,"/Touchtest_3.dat");
                        _calData.setxmin(DEF_3_MINX);
                        _calData.setymin(DEF_3_MINY);
                        _calData.setxmax(DEF_3_MAXX);
                        _calData.setymax(DEF_3_MAXY);
                      break;
                  }
                char buf[50];
                // start TFT
                startTFT();
                // start XPT2046 touch
                     #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                        SOUTLN(" .. md_touch .. start XPT2046 touch");
                      #endif
                _ptouch->begin();
                _ptouch->setRotation(_rotation);
                //_ptouchev->setResolution(_pTFT->width(),_pTFT->height());
                //_ptouchev->setDrawMode(false);
                    #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                        SOUTLN("             .. load calibration .. ");
                      #endif
                // load and check calibration
                loadCalibration();
          //doCalibration();
                switch (_iscal)
                  {
                    case MD_RDY:
                        //wrStatus("Kalibrierung gefunden", MD_SEL);
                        //sleep(1);
                        #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                            SOUTLN("             .. load calibration ok ");
                          #endif
                      break;
                    case MD_SEL:
                        #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                            SOUTLN("             .. check and save calib ");
                          #endif
                        doCalibration();
                      break;
                    default:
                        #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                            SOUTLN("             .. is to be calibrated");
                          #endif
                        doCalibration();
                      break;
                  }
                if (doTask)
                  {
                    startTouchTask();
                    //startMenuTask();
                  }
                //doCalibration();
                _isinit   = TRUE;
              }
          }
        void md_touch::startTFT()
          {
            if (!_isinit)
              {
                    #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                        STXT(" .. md_touch .. start tft");
                      #endif
                pinMode(_ledpin, OUTPUT);
                digitalWrite(_ledpin, _LED_ON); // switch on backlight
                _pTFT->begin(TFT_FREQU);
                _pTFT->setRotation(_rotation);
                _pTFT->fillScreen(MD_GREEN_MEDIUM);
                usleep(20000);
                _pTFT->fillScreen(MD_OLIVE);
                //_pTFT->setFont(MENU_FONT);
                    STXT(" TFT run ");
              }
          }
        void md_touch::mapXY(POINT_t* pPt)
          {
            TS_Point p1;
            switch (_rotation)
              {
                case 0:
                    p1.x = map(pPt->x, DEF_0_MINX, DEF_0_MAXX, 10, DEF_0_TFTW - 10);
                    p1.y = map(pPt->y, DEF_0_MINY, DEF_0_MAXY, 10, DEF_0_TFTH - 10);
                  break;
                case 1:
                    p1.x = map(pPt->x, DEF_1_MINX, DEF_1_MAXX, 10, DEF_1_TFTW - 10);
                    p1.y = map(pPt->y, DEF_1_MINY, DEF_1_MAXY, 10, DEF_1_TFTH - 10);
                  break;
                case 2:
                    p1.x = map(pPt->x, DEF_2_MINX, DEF_2_MAXX, 10, DEF_2_TFTW - 10);
                    p1.y = map(pPt->y, DEF_2_MINY, DEF_2_MAXY, 10, DEF_2_TFTH - 10);
                  break;
                case 3:
                    p1.x = map(pPt->x, DEF_3_MINX, DEF_3_MAXX, 10, DEF_3_TFTW - 10);
                    p1.y = map(pPt->y, DEF_3_MINY, DEF_3_MAXY, 10, DEF_3_TFTH - 10);
                  break;
              }
            pPt->x = p1.x;
            pPt->y = p1.y;
          }

        void md_touch::loadCalibration()
          {
            uint8_t  res;
            char buf[50];
            _pflash->init(_pflash);
            _iscal = MD_UNDEF;
                SVAL(" .. read ", _tCalFile);
            if (_pflash->exists(_tCalFile))
              {
                res = _pflash->readFile(_tCalFile, 40, buf);
                if (res == ESP_OK)
                  {
                    S3VAL(" .. found ",_tCalFile, buf, strlen(buf));
                    //sscanf((char*) buf, "%3i %3i %4i %4i", a, b, c, d);
                    STXT("scan ok");
                    //_calData.setxmin(a);
                    //_calData.setymin(b);
                    //_calData.setxmax(c);
                    //_calData.setymax(d);
                    _iscal = MD_RDY;
                  }
              }
                  #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                      SVAL(" val int rot ", _rotation );
                      S2VAL(" val int xmin ymin", _calData.xmin(), _calData.ymin());
                      S2VAL(" val int xmax ymax", _calData.xmax(), _calData.ymax());
                    #endif
            _pflash->end();
            //_ptouchev->calibrate(_calData.xmin(), _calData.ymin(), _calData.xmax(), _calData.ymax());
          }
        void md_touch::checkCalibration()
          {
            //if (!pCal) { loadCalibration(); }
          }
        void md_touch::doCalibration()
          {
            //#ifdef NOTUSED
                //calData_t rawP, calP;
                int16_t   xpos = 10;
                int16_t   ypos = 10;
                int16_t   xmax, ymax;
                boolean   tch;
                int16_t   wx, wy;
                uint16_t  ww, wh;
                POINT_t    raw;
                POINT_t   p;
                int16_t   butCal[4]  = {70,  160, 70, 32};
                int16_t   butExit[4] = {145, 160, 55, 32};
                bool      doExit     = false;
                char      text[60];
                uint8_t   i = 0;

                // stop menu task (uses same touch interface)
                _pmdmenu->suspend();
                _pTFT->setTextSize(1);
                switch (_rotation) // set max limits
                  {
                    case 0:
                    case 2:  xmax = 240; ymax = 320; break;
                    case 1:
                    case 3:  xmax = 320; ymax = 240; break;
                  }
                  // _pTFT->setRotation(i);

                for ( i = 0; i < 4 ; i++ )  // draw calibrations points and text
                  {
                    switch (i)
                      {
                        case 0: xpos = 10;       ypos = 10;       wx =  10; wy =  5;  break;
                        case 1: xpos = xmax -10; ypos = 10;       wx = -60; wy =  5;  break;
                        case 2: xpos = 10;       ypos = ymax -10; wx =  10; wy = -10; break;
                        case 3: xpos = xmax -10; ypos = ymax -10; wx = -60; wy = -10; break;
                      }
                    _pTFT->drawRect( xpos, ypos, 3, 3, ILI9341_YELLOW); // draw point
                    // set and draw text
                      sprintf(text,"%i / %i", xpos, ypos);
                      xpos += wx; ypos += wy;
                      _pTFT->setCursor(xpos, ypos);
                      _pTFT->getTextBounds(&text[0], xpos, ypos, &wx, &wy, &ww, &wh);
                      _pTFT->fillRect(wx, wy, ww, wh, ILI9341_BLACK);
                      _pTFT->print(text);
                  }
                    // draw buttons
                      STXT(" draw buttons ");
                      _pTFT->setTextSize(2);
                      _pTFT->fillRoundRect(butCal[0], butCal[1], butCal[2], butCal[3], 2, MD_RED_MEDIUM);
                      _pTFT->drawRoundRect(butCal[0], butCal[1], butCal[2], butCal[3], 2, MD_RED_MEDIUM);
                      _pTFT->setCursor(butCal[0] + 6, butCal[1] + 8);
                      _pTFT->setTextColor(MD_YELLOW_MEDIUM);
                      _pTFT->print("Calib");
                      _pTFT->fillRoundRect(butExit[0], butExit[1], butExit[2], butExit[3], 2, MD_GREEN_MEDIUM);
                      _pTFT->drawRoundRect(butExit[0], butExit[1], butExit[2], butExit[3], 2, MD_RED_MEDIUM);
                      _pTFT->setCursor(butExit[0] + 4, butExit[1] + 8);
                      _pTFT->setTextColor(MD_BLUE_MEDIUM);
                      _pTFT->print("Done");
                    // clear interface
                      _tactXY  = 0;
                      _tactRaw = 0;
                    //get position and check if touched
                     while (!doExit)
                      {
                        // menu interface to touch task
                        _menuTmp = _tactRaw;

                        if (_menuTmp == 0)
                          {
                            usleep(30000);
                          }
                        else
                          {
                            SHEXVAL(" Calib _tactRaw ", _menuTmp);
                                //STXT(" menuTask 2 ");
                            //_menuP.x = (_menuTmp >> TOUCH_HPOINT_BITS) & TOUCH_HPOINT_MASK;
                            raw.x = (_menuTmp >> TOUCH_HRAW_BITS) & TOUCH_HRAW_MASK;
                            //_menuP.y = _menuTmp & TOUCH_HPOINT_MASK;
                            raw.y = _menuTmp & TOUCH_HRAW_MASK;
                            //_menuTmp &= TOUCH_TYPE_MAX;
                            p=raw;
                            _pmdtouch->mapXY(&p);
                            _tactXY  = 0;
                            _tactRaw = 0;
                            S3VAL(" scalP x y rot ", p.x, p.y, _rotation );
                            sprintf(text," x %3i y %3i rx %4i ry %4i", p.x, p.y, raw.x, raw.y);
                            STXT(text);
                            xpos = 25;
                            ypos = 110;
                            _pTFT->setCursor(xpos,ypos);
                            _pTFT->setTextSize(2);
                            _pTFT->getTextBounds(&text[0], xpos, ypos, &wx, &wy, &ww, &wh);
                            _pTFT->fillRect(wx, wy, ww, wh, ILI9341_BLACK);
                            _pTFT->print(text);
                            if (   (p.x > butExit[0]) && (p.x < butExit[0] + butExit[2])
                                && (p.y > butExit[1]) && (p.y < butExit[1] + butExit[3])
                               )
                              {  // exit calibration routine
                                STXT(" Calib done");
                                doExit = TRUE;
                                _pTFT->fillRoundRect(butCal[0], butCal[1], butCal[2], butCal[3], 2, COL16_BLACK);
                                _pTFT->fillRoundRect(butExit[0], butExit[1], butExit[2], butExit[3], 2, COL16_BLACK);
                              }
                            usleep(500000);
                          }
                       }
                _pmdmenu->resume();
          }
        void md_touch::saveCalibration()
          {
                _pflash->init(_pflash);
                if (_pflash->exists(_tCalFile))
                  {
                    _pflash->remove(_tCalFile);
                  }
                char buf[20];
                sprintf(buf,"%3i %3i %4i %4i", _calData.xmin(), _calData.ymin(), _calData.xmax(), _calData.ymax());
                S2VAL("new calib file ", _tCalFile ,buf);
                _pflash->writeFile((char*) _tCalFile, &buf[0]);
                _pflash->end();
          }

    // public implementation
        void md_touch::wrText(const char* msg, uint8_t spalte, uint8_t zeile, uint8_t len)
          {
          }
        void md_touch::wrText(String msg, uint8_t spalte, uint8_t zeile, uint8_t len)
          {
            wrText(msg.c_str(), spalte, zeile, len);
          }
        void md_touch::run()
          {

          }
        void md_touch::suspend()
          { if (_pmdtouch != NULL) { vTaskSuspend( _pmdtouch ); } }
        void md_touch::resume()
          { if (_pmdtouch != NULL) { vTaskResume( _pmdtouch ); } }
  // md_menu
      md_menu::md_menu()
        {
          _pmdmenu = this;
          _TStat.startT(1);
        }
      md_menu::~md_menu() {}
      // init assumes that _pTFT is already initialized
      void md_menu::init(md_list* pMenu, MENUITEM_t* pTitle, MENUITEM_t* pStatus)
        {
          MENUITEM_t*  _ptmp = NULL;
          STXT(" .. init menu .. ");
          // init menu basics
            //_itCnt  = itCnt;
            _pMenu    = pMenu;
            _pTitle   = pTitle;
            _pStatus  = pStatus;
          // init & draw backscreen
            STXT(" .. init backscreen .. ");
            _backpos.x  = 0;
            _backpos.y  = BOX_GRID;
            _backsize.x = _pTFT->width();
            _backsize.y = _pTFT->height() - (2 * BOX_GRID);
            wrback();
              //S4VAL("      backpos pos.x .y .w .h ", _backpos.x, _backpos.y, _backsize.x, _backsize.y);
          // init & show title
            STXT(" .. init menu title ");
            // x y pos
              _pTitle->boxpos.x = (_pTitle->menuid.scrcol - 1) * BOX_GRID;
              if (_pTitle->boxpos.x  < 0) { _pTitle->boxpos.x = 0; }
              _pTitle->boxpos.y = (_pTitle->menuid.scrrow - 1) * BOX_GRID;
              if (_pTitle->boxpos.y  < 0) { _pTitle->boxpos.y = 0; }
            // w h size
              _pTitle->boxsize.x  = (_pTitle->menuid.itw) * BOX_GRID;
              if (_pTitle->boxsize.x < BOX_GRID) { _pTitle->boxsize.x  = BOX_GRID; }
              _pTitle->boxsize.y = BOX_GRID - 1;
                //S3VAL("      title grid.x .y .w .h ", _pTitle->menuid.scrcol, _pTitle->menuid.scrrow, _pTitle->menuid.itw);
                //S4VAL("      title  pos.x .y .w .h ", _pTitle->boxpos.x,  _pTitle->boxpos.y,  _pTitle->boxsize.x,  _pTitle->boxsize.y);
                //S2HEXVAL("    init title      itID menstat ", _pTitle->menuid.itID(), _pTitle->menuid.menstat);
            // menu ID
              _pTitle->menuid.menstat |= MENBIT_TODRAW;
              //_pTitle->menuid.menstat |= MENBIT_ISVIS;
              _pTitle->index(255);
                  //S2HEXVAL(" init title nach itID ISVIS ", _pTitle->menuid.menstat, MENBIT_ISVIS);
            show(_pTitle);
          // init & show status box
            // x/y pos
              STXT(" .. init menu status ");
              _pStatus->boxpos.x = (_pStatus->menuid.scrcol - 1) * BOX_GRID;
              if (_pStatus->boxpos.x < 0) { _pStatus->boxpos.x = 0; }

              _pStatus->boxpos.y  = ((_pStatus->menuid.scrrow - 1) * BOX_GRID) + 1;
              if (_pStatus->boxpos.y  < 0) { _pStatus->boxpos.y  = 0; }
            // w/h size
              _pStatus->boxsize.x  = (_pStatus->menuid.itw) * BOX_GRID;
              if (_pStatus->boxsize.x < BOX_GRID) { _pStatus->boxsize.x  = BOX_GRID; }

              _pStatus->boxsize.y  = BOX_GRID - 1;
                  //S3VAL("      status grid.x .y .w .h ", _pStatus->menuid.scrcol, _pStatus->menuid.scrrow, _pStatus->menuid.itw);
                  //S4VAL("      status  pos.x .y .w .h ", _pStatus->boxpos.x,  _pStatus->boxpos.y,  _pStatus->boxsize.x,  _pStatus->boxsize.y);
                  //S2HEXVAL("    init status      itID menstat ", _pStatus->menuid.itID(), _pStatus->menuid.menstat);
              // menu ID
              _pStatus->menuid.menstat |= MENBIT_TODRAW; //+ MENBIT_ISVIS;
              _pStatus->index(254);
              show(_pStatus);
          // init menu items
            if (_pMenu->count() > 0)
              {
                _ptmp = (MENUITEM_t*) pMenu->pFirst();
                //do
                for (uint8_t i=0 ; i < _pMenu->count() ; i++)
                  {
                      S4HEXVAL("    init menu .. idx page _pages _rows[page]", _ptmp->index(), _ptmp->menuid.menpage, _pages, _rows[_ptmp->menuid.menpage]);
                    // x/y pos
                      _ptmp->boxpos.x = (_ptmp->menuid.scrcol - 1) * BOX_GRID;
                      if (_ptmp->boxpos.x < 0)  { _ptmp->boxpos.x = 0; }

                      _ptmp->boxpos.y  = ((_ptmp->menuid.scrrow - 1) * BOX_GRID) + 1;
                      if (_ptmp->boxpos.y  < 0) { _ptmp->boxpos.y  = 0; }
                    // w/h size
                      if ((_ptmp->menuid.ittyp == MENTYP_CTRL) || (_ptmp->menuid.ittyp == MENTYP_BITMAP))
                        {
                          _ptmp->boxsize.x  = (_ptmp->menuid.itw) * BOX_GRID;
                          if (_ptmp->boxsize.x < BOX_GRID) { _ptmp->boxsize.x  = BOX_GRID; }

                          _ptmp->boxsize.y  = (_ptmp->menuid.ith) * BOX_GRID;
                          if (_ptmp->boxsize.y < BOX_GRID) { _ptmp->boxsize.y  = BOX_GRID; }
                        }
                      else
                        {
                          _ptmp->boxsize.x  = (_ptmp->menuid.itw) * BOX_GRID - 2;
                          if (_ptmp->boxsize.x < BOX_GRID) { _ptmp->boxsize.x  = BOX_GRID; }

                          _ptmp->boxsize.y  = (_ptmp->menuid.ith) * BOX_GRID - 2;
                          if (_ptmp->boxsize.y < BOX_GRID) { _ptmp->boxsize.y  = BOX_GRID; }
                        }
                          /*
                            S4HEXVAL(" ptmp grid.x .y .w ",         (uint32_t) _ptmp,
                                                                    _ptmp->menuid.scrcol,
                                                                    _ptmp->menuid.scrrow,
                                                                    _ptmp->menuid.itw);
                            S4HEXVAL("       pos.x .y .w .h      ", _ptmp->boxpos.x,
                                                                    _ptmp->boxpos.y,
                                                                    _ptmp->boxsize.x,
                                                                    _ptmp->boxsize.y);
                           */
                    // inc page and row conters (exept bitmaps and controls)
                      if (_ptmp->menuid.menpage != MENPAGE_ALL)
                        {
                          if (_ptmp->menuid.menpage >= _pages)
                              { _pages = _ptmp->menuid.menpage + 1; }
                          if ( (_ptmp->menuid.scrrow - MENU_ROW1) >= _rows[_ptmp->menuid.menpage] )
                              { _rows[_ptmp->menuid.menpage]++; }
                        }
                    // menu ID
                      _ptmp->menuid.menstat |= MENBIT_TODRAW;
                      S4HEXVAL("    init menu .. idx page _pages _rows[page]", _ptmp->index(), _ptmp->menuid.menpage, _pages, _rows[_ptmp->menuid.menpage]);
                        //if ((_ptmp->menuid.menpage == 0) || (_ptmp->menuid.menpage == MENPAGE_ALL))
                        //  {
                        //    show(_ptmp);
                        //  }
                              //if (_ptmp != _pStatus)
                                {
                                  /*
                                    S4HEXVAL(" init menu _ptmp index itID menstat ",  (uint32_t) _ptmp,
                                                                                        _ptmp->index(),
                                                                                        _ptmp->menuid.itID(),
                                                                                        _ptmp->menuid.menstat);
                                    S3VAL   ("           gridx gridy gridw          ",  _ptmp->menuid.scrcol,
                                                                                        _ptmp->menuid.scrrow,
                                                                                        _ptmp->menuid.itw);
                                    S4VAL   ("           posx  posy  sizew  sizeh   ",  _ptmp->boxpos.x,
                                                                                        _ptmp->boxpos.y,
                                                                                        _ptmp->boxsize.x,
                                                                                        _ptmp->boxsize.y);
                                   */
                                }
                    // next it
                      //S2HEXVAL(" init menu vor pNext   ptmp pNext", (uint32_t) _ptmp, (uint32_t) _ptmp->pNext());
                      _ptmp = (MENUITEM_t*) _ptmp->pNext();
                  }
              }
            _isInit = TRUE;
            //STXT("    init menu ready ");
        }
      void md_menu::begin(uint8_t doTask)
        {
          // start task
            if (doTask)
              {
                startMenuTask();
              }
        }
      // standard its
      void md_menu::wrStatus(const char* msg, uint16_t msTOut, uint8_t colormode)
        {
          //S2VAL(" wrStatus .. pStatus text isInit ", msg, _isInit );
          //SHEXVAL(" wrStatus pStatus text", (uint32_t) _pStatus);
          if (_isInit)
            {
              if (msg != NULL)
                {
                  //STXT("       vor ptext ");
                  _pStatus->boxtext.ptext = (char*) msg;
                  _pStatus->menuid.menstat |= MENBIT_TODRAW;
                  if (_TStat.TOut())
                    {
                      _pStatus->menuid.colmod = colormode;
                          //STXT("       vor show ");
                      show(_pStatus);
                      //_TStat.startT(msTOut);
                    }
                }
            }
          //STXT(" wrStatus end ");
        }
      void md_menu::wrTitle(const char *msg, uint8_t colormode)
        {
          STXT(" wrTitle ");
          if (msg != NULL)
            {
              _pTitle->boxtext.ptext = (char*) msg;
            }
          _pTitle->menuid.colmod = colormode;
          _pTitle->menuid.menstat |= MENBIT_TODRAW;
          show(_pTitle);
        }
      void md_menu::run()
        {
          MENUITEM_t* _ptmp = NULL;
          uint8_t     _new  = FALSE;
          String      _tmp;
          if (_isInit == FALSE) { return; }
          // evaluate touch event to process
            if (xtouchedType > TOUCHED_NOTOUCH)
              { // touchevent to process
                if (xtouchedItem != NULL) // menu item touched
                  {
                    switch (xtouchedType)
                      {
                        case TOUCHED_SINGLE: // short touched
                            if (xtouchedItem == pitTitle)
                              {
                                _newPage = 0;
                                _newRow  = 0;
                              }
                            else
                              {
                                switch (xtouchedItem->menuid.ittyp)
                                  {
                                    case MENTYP_BITMAP:
                                        //STXT(" SINGLE CTRL");
                                        if (xtouchedItem->menuid.colmod == COLMODE_BOXDEF)
                                          {
                                            xtouchedItem->pobj = (void*) pbmp32_notsmil;
                                            xtouchedItem->menuid.colmod = COLMODE_BOXSEL;
                                          }
                                        else if (xtouchedItem->menuid.colmod == COLMODE_BOXSEL)
                                          {
                                            xtouchedItem->pobj = (void*) pbmp32_frowny;
                                            xtouchedItem->menuid.colmod = COLMODE_BOXWRN;
                                          }
                                        else if (xtouchedItem->menuid.colmod == COLMODE_BOXWRN)
                                          {
                                            xtouchedItem->pobj = (void*) pbmp32_smiley;
                                            xtouchedItem->menuid.colmod = COLMODE_BOXPAS;
                                          }
                                        else
                                          {
                                            xtouchedItem->pobj = (void*) pbmp32_smiley;
                                            xtouchedItem->menuid.colmod = COLMODE_BOXDEF;
                                          }
                                      break;
                                    case MENTYP_CTRL:
                                        //STXT(" SINGLE CTRL");
                                        if (xtouchedItem == pitBMPdown)   // cursor down
                                          {
                                            //S4VAL(" cursor down touched _actPage _actRrow _rows[_actPage] newRow", _actPage, _actRow, _rows[_actPage], _newRow);
                                            if ( _actRow < MENU_ROW1 )
                                              { _newRow = MENU_ROW1; }
                                            else if ( (_actRow - MENU_ROW1) < (_rows[_actPage] - 1))
                                              { _newRow = _actRow + 1; }
                                            //S4VAL(" cursor down touched _actPage _actRrow _rows[_actPage] _newRow", _actPage, _actRow, _rows[_actPage], _newRow);
                                          }
                                        else if (xtouchedItem == pitBMPup) // cursor up
                                          {
                                            //S4VAL(" cursor up touched _actPage _actRrow _rows[_actPage] _newRow", _actPage, _actRow, _rows[_actPage], _newRow);
                                            if ((_actRow > MENU_ROW1) )
                                              { _newRow = _actRow - 1; }
                                            else
                                              { _newRow = 0; }
                                            //S4VAL(" cursor up touched _actPage _actRrow _rows[_actPage] _newRow", _actPage, _actRow, _rows[_actPage], _newRow);
                                          }
                                        else if (xtouchedItem == pitBMPleft)            // cursor left
                                          {
                                            //S4VAL(" cursor left touched _actPage _actRrow _rows[_actPage] _newPage", _actPage, _pages, _rows[_actPage], _newPage);
                                            if ( _actPage > 0)
                                              { _newPage = _actPage - 1; _newRow = 0;}
                                            //S4VAL(" cursor left touched _actPage _actRrow _rows[_actPage] _newPage", _actPage, _pages, _rows[_actPage], _newPage);
                                          }
                                        else if (xtouchedItem == pitBMPright)    // cursor right
                                          {
                                            //S4VAL(" cursor right touched _actPage _actRrow _rows[_actPage] _newPage", _actPage, _pages, _rows[_actPage], _newPage);
                                            if ( _actPage < (_pages - 1))
                                              { _newPage = _actPage + 1;  _newRow = 0;}
                                            //S4VAL(" cursor right touched _actPage _actRrow _rows[_actPage] _newPage", _actPage, _pages, _rows[_actPage], _newPage);
                                          }
                                        //S4VAL(" cursor touched _actPage _newPage _actRow _newRow ", _actPage, _newPage, _actRow, _newRow);
                                        _new == TRUE;
                                      break;
                                    case MENTYP_EDTEXT:
                                      break;
                                    case MENTYP_EDVAL:
                                      break;
                                    case MENTYP_LINK:
                                      break;
                                    case MENTYP_OUTVAL:
                                      // break;
                                    case MENTYP_TEXT:
                                        if (_actRow == _newRow) // only once
                                          {
                                            if (xtouchedItem->menuid.scrrow == _actRow)  // already selected
                                              { _newRow = 0; }
                                            else
                                              { _newRow = xtouchedItem->menuid.scrrow; }
                                          }
                                        xtouchedItem->menuid.menstat |= MENBIT_TODRAW;
                                      break;
                                  }
                              }
                          break;
                        case TOUCHED_LONG:
                            switch (xtouchedItem->menuid.ittyp)
                              {
                                case MENTYP_EDTEXT:
                                  break;
                                case MENTYP_OUTVAL:
                                case MENTYP_TEXT:
                                    switch (xtouchedItem->menuid.colmod)
                                      {
                                        case COLMODE_BOXDEF:
                                            xtouchedItem->menuid.colmod = COLMODE_BOXWRN;
                                          break;
                                        default:
                                            xtouchedItem->menuid.colmod = COLMODE_BOXDEF;
                                          break;
                                      }
                                  break;
                                case MENTYP_BITMAP:
                                      xtouchedItem->pobj = (void*) pbmp32_smiley;
                                      xtouchedItem->menuid.colmod == COLMODE_BOXDEF;
                                  break;
                                case MENTYP_CTRL:
                                    if (xtouchedItem->menuid.colmod < COLMODE_BOXPAS)
                                      {
                                        xtouchedItem->menuid.colmod = COLMODE_BOXPAS;
                                      }
                                    else
                                      {
                                        xtouchedItem->menuid.colmod = COLMODE_BOXDEF;
                                      }
                                  break;
                              }
                            xtouchedItem->menuid.menstat |= MENBIT_TODRAW;
                          break;
                      }
                  }
                else // no menu item touched
                  {
                    switch (xtouchedType)
                      {
                        case TOUCHED_SINGLE: _tmp = "tchs ", _menuP.x, "/", _menuP.y; break;
                        case TOUCHED_LONG:   _tmp = "tchl ", _menuP.x, "/", _menuP.y; break;
                      }
                    wrStatus(_tmp);
                  }
                xtouchedItem = NULL;
                xtouchedType = TOUCHED_NOTOUCH;
              }
          // run screen
             if (_newPage != _actPage)     // page changed wanted
              {
                wrback();
                _new = TRUE;
                _newRow = 0;
              }
            show(_pTitle);
            show(_pStatus);
            _ptmp = (MENUITEM_t*) _pMenu->pFirst();
            if ( _new || (_newRow != _actRow) )    // page change
              {
                for (uint8_t i = 0 ; i < _pMenu->count() ; i++)
                  {
                    //S4HEXVAL(" .. run    ptmp idx lev stat  ", (uint32_t) _ptmp, _ptmp->index(), _ptmp->menuid.menpage, _ptmp->menuid.menstat);
                    if (   (_ptmp->menuid.menpage == _newPage)
                        || (_ptmp->menuid.menpage == MENPAGE_ALL) )
                      {
                        _ptmp->menuid.menstat |= MENBIT_TODRAW;
                      }
                    //if (_newRow == 0)
                    //  {
                        // nothing to do
                    //  }
                    if ( _newRow != _actRow)
                      {
                        if ( _ptmp->menuid.scrrow == _newRow )  // selected
                          {
                            _ptmp->menuid.colmod = MENMOD_COLSEL;
                            //S2VAL("  run set SEL idx colmod ", _ptmp->index(), _ptmp->menuid.colmod);
                          }
                        else
                          {
                            _ptmp->menuid.colmod = MENMOD_COLDEF;
                            //S2VAL("  run set DEF idx colmod ", _ptmp->index(), _ptmp->menuid.colmod);
                          }
                        _ptmp->menuid.menstat |= MENBIT_TODRAW;
                      }
                    _ptmp = (MENUITEM_t*) _ptmp->pNext();
                  }
              }
            _actPage = _newPage;
            _actRow = _newRow;
            _ptmp = (MENUITEM_t*) _pMenu->pFirst();
            for (uint8_t i = 0 ; i < _pMenu->count() ; i++)
              {
                if ((_ptmp->menuid.menstat & MENBIT_TODRAW) == MENBIT_TODRAW)
                  {
                    if (   (_ptmp->menuid.menpage == _actPage)
                        || (_ptmp->menuid.menpage == MENPAGE_ALL) )
                      {
                        //_ptmp->menuid.menstat |= MENBIT_ISVIS;
                        show(_ptmp);
                      }
                  }
                _ptmp = (MENUITEM_t*) _ptmp->pNext();
              }
        }
      void md_menu::show(MENUITEM_t* pit, uint8_t maxWidth)
        {
          int16_t      _tx,   _ty; // txtpos    [pixel]
          uint16_t     _tw,   _th; // txtsize   [pixel]
          uint8_t      _txtcol = 0, _boxcol = 0;
          uint8_t      _tlen, _maxlen = 0;   // txtlength [char count]
          char         _ttmp[MENU_TXTMAX + 1];
                  //S2HEXVAL(" .. show .. pit isInit",(uint32_t) pit, _isInit);
          if ( (_isInit == FALSE) || (pit == NULL)) { return; }

          if      (pit == pitBMPleft)
            {
              //S2VAL(" show pit BMPleft ", _actRow, _actPage );
              if ( _actPage > 0)
                { _txtcol = COLMODE_BOXDEF; }
              else
                { _txtcol = COLMODE_BOXPAS;}
              pit->menuid.menstat |= MENBIT_TODRAW;
            }
          else if (pit == pitBMPright)
            {
              //S2VAL(" show pit BMPright ", _actRow, _actPage );
              if ( _actPage < (_pages - 1))
                { _txtcol = COLMODE_BOXDEF; }
              else
                { _txtcol = COLMODE_BOXPAS;}
              pit->menuid.menstat |= MENBIT_TODRAW;
            }
          else if (pit == pitBMPdown)
            {
              //S2VAL(" show pit BMPdown ", _actRow, _actPage );
              if (   _rows[_actPage] > 0
                  && ((_actRow) < (_rows[_actPage] + MENU_ROW1 - 1)))
                { _txtcol = COLMODE_BOXDEF; }
              else
                { _txtcol = COLMODE_BOXPAS;}
              pit->menuid.menstat |= MENBIT_TODRAW;
            }
          else if (pit == pitBMPup)
            {
              //S2VAL(" show pit BMPup ", _actRow, _actPage );
              if ( _actRow >= MENU_ROW1)
                { _txtcol = COLMODE_BOXDEF; }
              else
                { _txtcol = COLMODE_BOXPAS;}
              pit->menuid.menstat |= MENBIT_TODRAW;
            }
          else if (pit == pitSmily)
            {
              pit->menuid.menstat |= MENBIT_TODRAW;
            }
          else
            {
              _boxcol = pit->pboxcols->col(pit->menuid.colmod);
              _txtcol = pit->boxtext.ptxtcol->col(pit->menuid.colmod);
            }
            //if (   (   (pit->menuid.menstat & (MENBIT_ISVIS + MENBIT_TODRAW))
            //        == (MENBIT_ISVIS + MENBIT_TODRAW))
            //    || ((pit->menuid.ittyp   & MENTYP_CTRL)   == MENTYP_CTRL  ))
          if (   ((pit->menuid.menstat & MENBIT_TODRAW) == MENBIT_TODRAW)
              || ((pit->menuid.ittyp   & MENTYP_CTRL)   == MENTYP_CTRL  ))
            {
              _tw = pit->boxtext.txtsize * TXT_CHAR1_W;
              _th = pit->boxtext.txtsize * TXT_CHAR1_H;
              strncpy(_ttmp,pit->boxtext.ptext, MENU_TXTMAX);
              switch (pit->menuid.ittyp)
                {
                  case MENTYP_EDTEXT: // break;
                  case MENTYP_TEXT:   // break;
                  case MENTYP_OUTVAL:
                      // calculate text x
                        if (_tw > 0) { _maxlen = (uint8_t) ((pit->boxsize.x - 2) / _tw); }
                        if (_maxlen >= strlen(pit->boxtext.ptext)) // text smaler than box width
                          {
                            _tlen = (uint8_t) strlen(pit->boxtext.ptext);
                          }
                        else
                          { // shrink text to maxlen
                            _tlen = _maxlen;
                                  //S4VAL("    show _tlen _maxlen _tw strlen ", _tlen, _maxlen, _tw, (uint8_t) strlen(pit->boxtext.ptext));
                                  //STXT(" show  vor  ptxt[tlen] = 0");
                            _ttmp[_tlen] = 0;
                                  //STXT(" show  nach ptxt[tlen] = 0");
                          }
                        _tw *= _tlen;
                            //S4VAL("    show _tlen _maxlen _tw strlen ", _tlen, _maxlen, _tw, (uint8_t) strlen(pit->boxtext.ptext));
                        switch(pit->boxtext.align)
                          {
                            case TXT_CENTER:
                                _tx = pit->boxpos.x + ((pit->boxsize.x - _tw) / 2);
                              break;
                            case TXT_RIGHT:
                                _tx = pit->boxpos.x + (pit->boxsize.x - _tw) - 1;
                              break;
                            default: // TXT_LEFT
                                _tx = pit->boxpos.x + 1;
                              break;
                          }
                      // calculate text y - only single row text
                        _ty = pit->boxpos.y + ((pit->boxsize.y - _th) / 2);
                        _pTFT->setCursor(_tx, _ty);
                      // write box
                        //S3VAL("  show idx txtcol _actRow ", pit->index(), pit->menuid.colmod, _actRow);
                        _pTFT->fillRect(pit->boxpos.x, pit->boxpos.y, pit->boxsize.x, pit->boxsize.y,
                                            (uint16_t) (pit->pboxcols->col(pit->menuid.colmod)));
                        _pTFT->setTextSize(pit->boxtext.txtsize);
                        _pTFT->setTextColor((uint16_t) pit->boxtext.ptxtcol->col(pit->menuid.colmod));
                        _pTFT->print(_ttmp);
                            //S2HEXVAL("   show  pit menstat ", (uint32_t) pit, pit->menuid.menstat);
                    break;
                  case MENTYP_BITMAP:
                      _tw = BOX_GRID * 2;
                      _th = BOX_GRID * 2;
                    // break;
                  case MENTYP_CTRL:
                      if (pit->menuid.ittyp == MENTYP_CTRL)
                        {
                          _tw = BOX_GRID;
                          _th = BOX_GRID;
                        }
                      _boxcol = 0;
                      //_pTFT->fillRect  (pit->boxpos.x, pit->boxpos.y, pit->boxsize.x, pit->boxsize.y,
                      _pTFT->fillRect  (pit->boxpos.x, pit->boxpos.y - 1, _tw, _th,
                                        (uint16_t) (pit->pboxcols->col(_boxcol)));
                      _pTFT->drawBitmap(pit->boxpos.x + 2,  pit->boxpos.y + 2, (uint8_t*) pit->pobj,
                                        pit->menuid.bmpwh, pit->menuid.bmpwh,
                                        pit->boxtext.ptxtcol->col(_txtcol),
                                        pit->pboxcols->col(_boxcol));
                    break;
                }
              // reset flags
                pit->menuid.menstat = pit->menuid.menstat - MENBIT_TODRAW;
            }
        }
                //STXT("  .. end ");
      void md_menu::wrback()
        {
          _pTFT->fillRect( _backpos.x, _backpos.y, _backsize.x, _backsize.y, COL_BACK );
        }
      MENUITEM_t* md_menu::getItem(POINT_t* ptouchP)
        {
          MENUITEM_t* _pmy = (MENUITEM_t*) _pMenu->pFirst();
          for (uint8_t i = 0; (i < _pMenu->count()) && (_pmy != NULL); i++ )
            {
              if ( _pmy->isItem(ptouchP) == TRUE)
                {
                  return _pmy;
                }
              _pmy = (MENUITEM_t*) _pmy->pNext();
            }
          if (_pStatus->isItem(ptouchP) == TRUE)
            {
              return _pStatus;
            }
          if (_pTitle->isItem(ptouchP) == TRUE)
            {
              return _pTitle;
            }
          return NULL;
        }
      void md_menu::drawEntry()
        {
          /*
              void md_menu::drawEntry(const char* txt, uint8_t line, uint16_t color)
                {
                  //pgfx_->setColor(MD4_BLACK);
                  pgfx_->drawRect(_x, _y[line], _ym[line], 16);
                  pgfx_->commit();
                  pgfx_->setFont(MENU_FONT);
                  pgfx_->setColor(color);
                  pgfx_->drawString(_x + 1, _y[line] + 1, txt);
                  pgfx_->commit();
            */
        }
      void md_menu::suspend()
        { if (_pmenuTask != NULL)
          {
            disableLoopWDT();
            vTaskSuspend( _pmenuTask );
          }
        }
      void md_menu::resume()
        {
          if (_pmenuTask != NULL) { vTaskResume( _pmenuTask ); }
        }
#endif // NOTREADY
