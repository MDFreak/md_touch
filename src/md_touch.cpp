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
  static md_touch            *_pmdtouch = NULL;
  static md_TouchEvent       *_ptouchev = NULL;
  static md_spiffs           *pConf     = new md_spiffs();
  static calData_t           *pCal      = NULL;
  static int8_t              _rotation  = 3;
  static uint16_t            _COL_BACK  = MD_BLACK;
  static TaskHandle_t        _touchTask = NULL;
  static uint32_t            _hTcount   = 0;

  // atomic communication with touch task
    // task wr    / extern rd
      static uint32_t        _tactRaw   = 0;    // last touched point (raw type)
      static uint32_t        _tactXY    = 0;    // last touched point (TFT converted)
    // task rd+wr / extern rd+wr  =  handshake
      static int8_t          _tCtrl     = TTASK_ON_RUN;    // ext  TTASK_ON_RUN:  handshake off
                                                           // ext  TTASK_HSHAKE:  handshake , value treated, waiting for touch
                                                           // ext  TTASK_ON_HOLD: touch on hold
                                                           // task TTASK_NEWVAL:  handshake , value new, waiting for treatment
    // extern wr  / task rd
      static uint32_t        _tWait_us  = 10000; // task time tick (minimum = 1000 ~ 1 msec)


  //static TaskHandle_t menuTask_  = NULL;

// task touchTask
  void IRAM_ATTR handleTouch(void * pvParameters)
    {
      boolean   tch;
      TS_Point  raw;
      TS_Point  p;
      unsigned long lastWD, actWD, diff;

      //esp_task_wdt_init(WDT_TTIMEOUT, true); //enable panic so ESP32 restarts
      //esp_task_wdt_add(NULL); //add current thread to WDT watch
      lastWD = millis();

      while(true)  // endless loop
        {
          actWD = millis();
          diff  = actWD - lastWD;
          if (diff > 1000)
            {
              lastWD = actWD;
              //esp_task_wdt_reset();
                //SOUT("cycle "); SOUT((uint32_t) actWD*1000/count); SOUT(" "); SOUTLN(count);
                //count = 0;
            }
          if (_tCtrl < TTASK_ON_HOLD)
            {
              tch = _ptouchev->getTouchPos(&p, &raw);
              if (tch)
                {
                  if (_hTcount < 1)
                    {
                      _hTcount++;
                      if (raw.x < 0) raw.x = 0;
                      if (raw.y < 0) raw.y = 0;
                      if (p.x < 0) p.x = 0;
                      if (p.y < 0) p.y = 0;
                      _tactRaw = (raw.x << 16) + raw.y;
                      _tactXY  = (p.x << 16) + p.y;
                      SOUT(actWD); SOUT(" "); SOUT(_hTcount); SOUT(" ttask "); SOUT(p.x); SOUT("/"); SOUT(p.y); SOUT(" "); SOUTHEXLN(_tactXY);
                    }
                  else
                    {
                      if (_hTcount < 25) {_hTcount++;  }
                      else            {_hTcount = 0;}
                      //SOUT(actWD); SOUT(" "); SOUTLN(_hTcount);
                    }
                }
            }
          //count++;
          usleep(_tWait_us);
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
                          handleTouch,           /* Task function. */
                          "touchTask",           /* name of task. */
                          10000,                 /* Stack size of task */
                          NULL,                  /* parameter of the task */
                          4 | portPRIVILEGE_BIT, /* priority of the task */
                          &_touchTask,            /* Task handle to keep track of created task */
                          0              );      /* pin task to core 0 */
      SOUTLN("touchTask started on core 0");
    }

// task menuTask

// --- class tColors
// --- class tBoxColors
// --- class tPoint
// --- class tText
  void tText::setText(const char* pTxt)
    {
      //SOUT(" setText .. "); SOUT(pTxt); SOUT(" "); SOUT(_len);
      if (_text != NULL) delete _text;
      _text=new char[_len+1];
      memcpy( _text, (char*) pTxt, _len);
      _text[_len]=0;
      //SOUT(" neu "); SOUTLN(_text);
    }

  void  tText::setText(String Text)
    {
        setText(Text.c_str());
    }

  char* tText::getText(char* pText)
    {
      memcpy(pText, _text, sizeof(_text));
      return pText;
    }

// --- class tBox
// --- class tButton
  void tButton::show()
    {
      uint8_t  len;
      int16_t  x = _x;
      int16_t  y = _y;
      uint16_t w, h;
      _pTFT->fillRoundRect(_x, _y, _w, _h, 2, _cols.colBut(_mode));
      _pTFT->setTextSize(_size);
      h = _size * TXT_CHAR1_H;  // hight of 1 char
      w = _size * TXT_CHAR1_W;  // width of 1 char
      len = _w / w;      // max count of char
      if (strlen(_text) < len)
        {
          len = strlen(_text);
        }
      else
        {
          _text[len] = 0;
        }
      w = w * len;
      y++;
      if (h < _h) { y = _y + (_h - h) / 2; }
      x++; // pos +1 in window
      switch(_bpos)
        {
          case TXT_CENTER:
            if (_w > w) { x += (_w - w) / 2; }
            break;
          case TXT_RIGHT:
            if (_w > w) { x += (_w - w) - 2; }
            break;
          default: // TXT_LEFT
            break;
        }
      _pTFT->setCursor(x, y);
      _pTFT->setTextColor(_cols.colText(_mode));
            //SOUT(" show "); SOUTLN(_text);
      _pTFT->print(_text);
    }

  void tButton::hide()
    {
      _pTFT->fillRoundRect(_x, _y, _w, _h, 2, _COL_BACK);
      this->setVis(false);
    }

// --- class calData
// --- class md_touch
  // construtors
  	md_touch::md_touch(uint8_t cspin, uint8_t tft_CS, uint8_t tft_DC,  uint8_t tft_RST,
                                                      uint8_t tft_LED, uint8_t led_ON) //, uint8_t spi_bus)
      {
        _pmdtouch = this;
        _pTFT     = new Adafruit_ILI9341(tft_CS, tft_DC, tft_RST);
        _ptouch   = new XPT2046_Touchscreen(cspin);
        _ptouchev = new md_TouchEvent(_ptouch);
        _ledpin  = tft_LED;
        _LED_ON   = led_ON;
      }

    md_touch::~md_touch()
      {
        _pmdtouch = NULL;
      }

  // public implementation
    void md_touch::wrStatus(const char* msg, uint8_t mode)
      {
        _statbox.setText(msg);
        usleep(200000);
        _statbox.show(mode);
        usleep(200000);
      }

    void md_touch::wrStatus(String msg, uint8_t mode)
      {
        wrStatus(msg.c_str(), mode);
      }

    void md_touch::wrText(const char* msg, uint8_t spalte, uint8_t zeile, uint8_t len)
      {
      }

    void md_touch::wrText(String msg, uint8_t spalte, uint8_t zeile, uint8_t len)
      {
        wrText(msg.c_str(), spalte, zeile, len);
      }

    void md_touch::setRotation(uint8_t rotation)
      {
        _rotation = rotation;
      }

    uint8_t md_touch::rotation()
      {
        return _rotation;
      }

    void md_touch::run() {}
  // protected implementation
    void md_touch::start(uint8_t rotation, uint16_t background)
      {
        _rotation = rotation;
        _COL_BACK = background;
        char buf[50];
        // start TFT
        startTFT();
        // install status window
        startStatus();
        // start touch
             #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                SOUTLN(" .. md_touch .. start touch");
              #endif
        _ptouch->begin();
        _ptouch->setRotation(_rotation);
        _ptouchev->setResolution(_pTFT->width(),_pTFT->height());
        _ptouchev->setDrawMode(false);
            #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                SOUTLN("             .. load calibration .. ");
              #endif
        // load and check calibration
        loadCalibration();
        switch (_iscal)
          {
            case MD_RDY:
                wrStatus("Kalibrierung gefunden", MD_SEL);
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
        startTouchTask();
      }

    void md_touch::startTFT()
      {
            #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                SOUTLN(" .. md_touch .. start tft");
              #endif
        pinMode(_ledpin, OUTPUT);
        digitalWrite(_ledpin, _LED_ON); // switch on backlight
        _pTFT->begin();
        _pTFT->setRotation(_rotation);
        _pTFT->fillScreen(MD_GREEN);
        usleep(200000);
        _pTFT->fillScreen(_COL_BACK);
      }

    void md_touch::startStatus()
      {
            #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                SOUTLN(" .. md_touch .. start status window");
              #endif
        _statbox.setBox(STAT_X, STAT_Y, STAT_W, STAT_H);
        _statbox.setTextSize(STAT_SIZE);
        //_statbox.setTextSize(1);
        _statbox.setOrient(STAT_ORIENT);
        _statbox.setLen(44);
        _statbox.setText("Herzlich Willkommen");
        _statbox.show();
        sleep(1);
        _statbox.hide();
      }

    void md_touch::loadCalibration()
      {
        uint16_t a,b,c,d;
        uint8_t  res;
        char buf[60];
        if (pCal != NULL) { delete pCal; pCal = NULL;}
        pCal = new calData_t(); // to be deleted after calibration
        pConf->init(pConf);
        _iscal = MD_UNDEF;
        SOUTLN(" .. read /tcalib.dat or /conf.dat");

        if (pConf->exists("/tcalib.dat"))
          {
            res = pConf->readFile("/tcalib.dat", 40, buf);
            if (res == ESP_OK)
              {
                SOUT(" .. '/tcalib' found "); SOUTLN(buf);
                sscanf(&buf[0], "%3i %3i %4i %4i", &a, &b, &c, &d);
                _iscal = MD_RDY;
              }
          }
        else if ((_iscal == MD_UNDEF) && (pConf->exists("/conf.dat")))
          {
            res = pConf->readFile("/conf.dat", 40, buf);
            if (res == ESP_OK)
              {
                SOUT(" .. '/conf' found "); SOUTLN(buf);
                sscanf(&buf[0], "%3i %3i %4i %4i", &a, &b, &c, &d);
                _iscal = MD_SEL;
              }
          }
        else
          {
            _iscal = MD_DEF;
          }

        if (_iscal > MD_DEF)
          {
            pCal->setCalib(a, b, c, d);
          }

              #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
                  SOUTLN(pCal->printCal50(buf));
                #endif
        _ptouchev->calibrate(pCal->xmin, pCal->ymin, pCal->xmax, pCal->ymax);
        pConf->end();
      }

    void md_touch::checkCalibration()
      {
        if (!pCal) { loadCalibration(); }
      }

    void md_touch::doCalibration()
      {
        calData_t rawP, calP;
        int16_t   xpos = 1;
        int16_t   ypos = 10;
        int16_t   xmax, ymax;
        boolean   tch;
        int16_t   wx, wy;
        uint16_t  ww, wh;
        TS_Point  raw;
        TS_Point  p;
        int16_t   butCal[4]  = {70,  160, 70, 32};
        int16_t   butExit[4] = {145, 160, 55, 32};
        bool      doExit     = false;
        char      text[60];

        if (!pCal)
          {
            SOUTLN(" doCal  !!! no cal struct !!!");
            loadCalibration();
          }

        _pTFT->setTextSize(1);
        uint8_t i = _rotation % 4;
        switch (i) // set max limits
          {
            case 0:  xmax = 240; ymax = 320; break;
            case 1:  xmax = 320; ymax = 240; break;
            case 2:  xmax = 240; ymax = 320; break;
            default: xmax = 320; ymax = 240; break;
          }
        _pTFT->setRotation(i);

        for ( i = 0; i < 4 ; i++ )  // draw calibrations points
          {
            switch (i)
              {
                case 0:
                  xpos = 10; //
                  ypos = 10;
                  wx   = 10;
                  wy   = 5;
                  break;
                case 1:
                  xpos = xmax -10;
                  ypos = 10;
                  wx   = -60;
                  wy   = 5;
                  break;
                case 2:
                  xpos = 10;
                  ypos = ymax -10;
                  wx   = 10;
                  wy   = -10;
                  break;
                default:
                  xpos = xmax - 10;
                  ypos = ymax - 10;
                  wx   = -60;
                  wy   = -10;
                  break;
              }
            _pTFT->drawRoundRect( xpos, ypos, 3, 3, 1, ILI9341_YELLOW);
            sprintf(text,"%i / %i", xpos, ypos);
            xpos += wx;
            ypos += wy;
            _pTFT->setCursor(xpos, ypos);
            _pTFT->getTextBounds(&text[0], xpos, ypos, &wx, &wy, &ww, &wh);
            _pTFT->fillRect(wx, wy, ww, wh, ILI9341_BLACK);
            _pTFT->print(text);
          }

              SOUTLN("draw buttons");
          _pTFT->setTextSize(2);
          _pTFT->fillRoundRect(butCal[0], butCal[1], butCal[2], butCal[3], 2, MD_RED);
          //_pTFT->drawRoundRect(butCal[0], butCal[1], butCal[2], butCal[3], 2, MD_RED);
          _pTFT->setCursor(butCal[0] + 6, butCal[1] + 8);
          _pTFT->setTextColor(MD_YELLOW);
          _pTFT->print("Calib");
          _pTFT->fillRoundRect(butExit[0], butExit[1], butExit[2], butExit[3], 2, MD_GREEN);
          //_pTFT->drawRoundRect(butExit[0], butExit[1], butExit[2], butExit[3], 2, MD_RED);
          _pTFT->setCursor(butExit[0] + 4, butExit[1] + 8);
          _pTFT->setTextColor(MD_BLUE);
          _pTFT->print("Done");
        //get position and check if touched
        while (1)
          {
            tch = _ptouchev->getTouchPos(&p, &raw);
            if (tch)
              {
                sprintf(text,"x %3i y %3i rx %5i ry %5i", p.x, p.y, raw.x, raw.y);
                SOUT(text);
                xpos = 25;
                ypos = 110;
                if (   (p.x > butExit[0]) && (p.x < butExit[0] + butExit[2])
                    && (p.y > butExit[1]) && (p.y < butExit[1] + butExit[3])
                   )
                  {  // exit calibration routine
                    SOUTLN(" Calib done");
                    doExit = true;
                    _pTFT->fillRoundRect(butCal[0], butCal[1], butCal[2], butCal[3], 2, MD_BLACK);
                    _pTFT->fillRoundRect(butExit[0], butExit[1], butExit[2], butExit[3], 2, MD_BLACK);
                    break;
                  }
                else if (   (p.x > butCal[0]) && (p.x < butCal[0] + butCal[2])
                         && (p.y > butCal[1]) && (p.y < butCal[1] + butCal[3])
                        )
                  { // execute calibration and write to file in flash
                    SOUTLN(" do Calib");
                        /*
                          SOUTLN("Rechnung");
                          SOUT("x0y0/1 "); SOUT(calP.x0y0.x); SOUT("/"); SOUTLN(calP.x0y1.x);
                          SOUT("x0/1y0 "); SOUT(calP.x0y0.y); SOUT("/"); SOUTLN(calP.x1y0.y);
                          SOUT("x1y0/1 "); SOUT(calP.x1y0.x); SOUT("/"); SOUTLN(calP.x1y1.x);
                          SOUT("x0/1y1 "); SOUT(calP.x0y1.y); SOUT("/"); SOUTLN(calP.x1y1.y);
                         */
                    calP.xmin = (calP.x0y0.x + calP.x0y1.x)/2;
                    calP.ymin = (calP.x0y0.y + calP.x1y0.y)/2;
                    calP.xmax = (calP.x1y0.x + calP.x1y1.x)/2;
                    calP.ymax = (calP.x0y1.y + calP.x1y1.y)/2;
                    int16_t dRawX = calP.xmax - calP.xmin;
                    int16_t dRawY = calP.ymax - calP.ymin;
                    calP.xmin -= 10 * dRawX / (xmax - 20);
                    calP.xmax += 10 * dRawX / (xmax - 20);
                    calP.ymin -= 10 * dRawY / (ymax - 20);
                    calP.ymax += 10 * dRawY / (ymax - 20);
                    _ptouchev->calibrate(calP.xmin, calP.ymin, calP.xmax, calP.ymax);
                    saveCalibration();
                  }
                else if ((raw.x < 1000) && (raw.y < 1000))
                  {  // touched upper left
                    ypos = 30;
                        SOUTLN(" P00");
                    calP.x0y0 = raw;
                  }
                else if ((raw.x > 3000) && (raw.y < 1000))
                  {  // touched upper right
                    ypos = 50;
                        SOUTLN(" P10");
                    calP.x1y0 = raw;
                  }
                else if ((raw.x < 1000) && (raw.y > 3000))
                  {  // touched lower left
                    ypos = 70;
                        SOUTLN(" P01");
                    calP.x0y1 = raw;
                  }
                else if ((raw.x > 3000) && (raw.y > 3000))
                  {  // touched lower right
                    ypos = 90;
                        SOUTLN(" P11");
                    calP.x1y1 = raw;
                  }

                _pTFT->setCursor(xpos,ypos);
                _pTFT->setTextSize(1);
                _pTFT->getTextBounds(&text[0], xpos, ypos, &wx, &wy, &ww, &wh);
                _pTFT->fillRect(wx, wy, ww, wh, ILI9341_BLACK);
                _pTFT->print(text);
                usleep(500000);
              }
          }
      }

    void md_touch::saveCalibration()
      {
        if (pCal != NULL)
          {
            if (pConf->exists("/tcalib.dat"))
              {
                pConf->remove("/tcalib.dat");
              }
            char buf[20];
            sprintf(buf,"%3i %3i %4i %4i",pCal->xmin, pCal->ymin, pCal->xmax, pCal->ymax);
            SOUT("new calib file '"); SOUT(buf); SOUTLN("'");
            pConf->writeFile("/tcalib.dat", buf);
          }
        else
          {
            wrStatus("ERR pCal not existing");
          }
      }

#ifdef NOT_USED
  // --- includes
    #include <md_touch.h>
    #include <md_spiffs.h>
    #include <string>
    #include <ArialRounded.h>
    #include <md_defines.h>

  // --- globals
    TaskHandle_t touchTask_ = NULL;
    TaskHandle_t menuTask_  = NULL;

    uint32_t     menu_ctrl_ = 0;
    uint32_t     menu_outp_ = 0;
    uint16_t     x_         = 0;
    uint16_t     y_         = 0;
    uint8_t    Z_THRESHOLD_ = 0;

    uint8_t      tcs_       = 0;
    uint8_t      tmosi_     = 0;
    uint8_t      tmiso_     = 0;
    uint8_t      tsck_      = 0;
    uint8_t      tirq_      = 255;

  // --- tasks
    void startTouchTask()
      {
        xTaskCreatePinnedToCore(
                            handleTouch,           /* Task function. */
                            "touchTask",           /* name of task. */
                            10000,                 /* Stack size of task */
                            NULL,                  /* parameter of the task */
                            4 | portPRIVILEGE_BIT, /* priority of the task */
                            &touchTask_,            /* Task handle to keep track of created task */
                            0              );      /* pin task to core 0 */
        SOUTLN("touchTask started on core 0");
      }

    void startMenuTask()
      {
        xTaskCreatePinnedToCore(
                            handleMenu,            /* Task function. */
                            "menuTask",               /* name of task. */
                            10000,                 /* Stack size of task */
                            NULL,                  /* parameter of the task */
                            4 | portPRIVILEGE_BIT, /* priority of the task */
                            &menuTask_,             /* Task handle to keep track of created task */
                            0              );      /* pin task to core 0 */
        SOUTLN("menuTask started on core 0");
      }

  // --- task handle
  void handleTouch(void * pvParameters)
    {
      uint16_t x;
      uint16_t y;

      while(true)  // endless loop
        {
          if (pts_->isTouching())
            {
              SOUTLN("screen touched ");
              pts_->getPosition(x, y);
              SOUT("position x"); SOUT(x); SOUT(", y "); SOUTLN(y);
                //delay(30);
                //pts->readData(&x, &y, &z);
                //SOUT("x, y, z "); SOUT(x); SOUT(", "); SOUT(y); SOUT(", "); SOUTLN(z);
              //pts->isrWake = false;
            }
          sleep(2);
          usleep(2000);
        }
    }

  void handleMenu(void * pvParameters)
    {
      while(true)  // endless loop
        {
          //SOUTLN("! handleMenu called !");

          usleep(2000);
        }
    }

  // --- callback functions
    /*void isrPin( void )
        {
        	XPT2046_Calibrated *o = pts_;
        	o->isrWake = true;
        }
      */

  void    actLev(uint32_t val, uint8_t level)
    {
    }

  uint8_t actLev(uint32_t val)
    {
      return true;
    }

  void    oldLev(uint32_t val, uint8_t level)
    {
    }

  uint8_t oldLev(uint32_t val)
    {
      return true;
    }

  void    actPage(uint32_t val, uint8_t page)
    {
    }

  uint8_t actPage(uint32_t val)
    {
      return true;
    }

  void    oldPage(uint32_t val, uint8_t page)
    {
    }

  uint8_t oldPage(uint32_t val)
    {
      return true;
    }

    /*
      void calibrate()
        {
          uint16_t x1, y1, x2, y2;
          //uint16_t vi1, vj1, vi2, vj2;
          uint8_t  isok = false;
          calParams_t calPars;
          char buf[80];

          SOUTLN("Start calibration ...");
          pts_->getCalibrationPoints(x1, y1, x2, y2);
          SOUT("cal points "); SOUT(x1); SOUT(" "); SOUT(y1); SOUT(" "); SOUT(x2); SOUT(" "); SOUTLN(y2);
          do // calib until plausibility
            {
              calibratePoint(x1, y1, calPars.vi1, calPars.vj1);
              SOUT("cal p(1) "); SOUT(calPars.vi1); SOUT(" "); SOUTLN(calPars.vj1);
              sleep(1);
              //usleep(50000);
              calibratePoint(x2, y2, calPars.dvi, calPars.dvj);
              SOUT("cal p(2) "); SOUT(x2); SOUT(" "); SOUT(y2); SOUT(" "); SOUT(calPars.dvi); SOUT(" "); SOUTLN(calPars.dvj);
              pts_->setCalibration(&calPars);
              //snprintf(buf, sizeof(buf), "%d,%d,%d,%d", (int)vi1, (int)vj1, (int)vi2, (int)vj2);
              SOUT("buf '"); SOUTLN(buf);
              // show result on touch
                pgfx_->fillBuffer(MD4_BLACK);
                pgfx_->setFont(ArialRoundedMTBold_14);
                pgfx_->setColor(MD4_YELLOW);
                pgfx_->drawStringInternal(0, 200, (char*) "setCalibration params:", 22, 240);
                pgfx_->drawStringInternal(0, 220, buf, strlen(buf), 240);
                pgfx_->commit();
            }
          while (!isok);
          snprintf(buf, sizeof(buf), "%d/n%d/n%d/n%d/n%d/n%d/n", calPars.dx, calPars.dy,
                                     calPars.vi1, calPars.vj1, calPars.dvi, calPars.dvj);
          pmdt_->saveCalibration(buf, sizeof(buf));
        }
    */
  // ----------------------------------------------------------------
  // --- class md_menu
    md_menu::md_menu()  {}
    md_menu::~md_menu() {}

    void md_menu::begin(Adafruit_ILI9341* ptft,
                        const int16_t x, const int16_t y, const int16_t w,
                        const uint8_t pages, const uint8_t lines, const uint8_t line_h)
      {
        md_list *plist = NULL;
        SOUTLN("init menu ...");
        // define outline geometry header
        _x     = x;
        _wline = _x + w - 1;
        _hline = line_h;
        _y[0]  = y;
        _ym[0] = _y[0] + _hline - 1;
        // define menu
        _pages = pages;
        _lines = lines;
        // grafic object
        pgfx_ = pgfx;
        // create menu lists
        static md_list list = md_list();
        _phead = &list;
                    //SOUT("  main menu");
                    //SOUT(" y "); SOUT(_y[0]); SOUT(" - "); SOUTLN(_ym[0]);
        for (uint8_t i = 0; i < MENU_MAXLINES ; i++ )
          {
            plist = new md_list();
            _ppages[i] = plist;
                    //SOUT("  menu page "); SOUT(i); SOUT(" "); SOUTHEX((uint32_t) plist);
            _y[i+1]  = _y[i]   + _hline + 2;
            _ym[i+1] = _y[i+1] + _hline - 1;
                    //SOUT(" y "); SOUTLN(_y[i+1]);
          }
        startMenuTask();
        SOUTLN("menu initialized");
      }

    void md_menu::load(const char* entries, const uint8_t cnt, uint8_t mtype)
      {
        if (pgfx_ != NULL)
          {
            md_cell* pcell = NULL;
                    //SOUT("load menu ... ");
            md_list* _plist = NULL;
            char* _ptext    = (char*) entries;
            if (mtype == MENUTYPE_MAIN) // main menue
              {
                _plist = _phead;
              }
            else if (mtype < _pages) // pages menu
              {
                _plist = _ppages[mtype];
              }
              else // ERROR
                {
                  SOUT(" ERRROR !! page "); SOUT(mtype); SOUT(" is to high !"); SOUTLN(_pages);
                }
            // load title
            pcell = new md_cell();
            pcell->setobj((void*) _ptext);
            _plist->add(pcell);
                      //SOUTLN(_ptext);

            for (uint8_t i = 0; i < cnt - 1 ; i++)
              {
                _ptext += MENU_TXTLEN + 1;
                pcell = new md_cell();
                      //SOUT(" load new Entry '"); SOUTHEX((uint32_t) pcell);
                      //SOUT("' "); SOUT(_ptext);
                pcell->setobj((void*) _ptext);
                _plist->add(pcell);
                      //SOUT(" obj '"); SOUT((char*) pcell->getobj()); SOUTLN("'");
              }
                      //SOUTLN("  menu valid");
          }
        else
          { SOUTLN(" ERROR !! menu not initialized !! "); }
      }

    void md_menu::show()
      {
        if (_phead > NULL)
          {
            md_list* plist = NULL;
            md_cell* pcell = NULL;
            uint8_t  lev   = 0;
            uint8_t  page  = 0;

            // check actual pages and action

            // open list
                      //SOUTLN("show openlist");
            if (lev == 0)
              { plist = _phead; }
            else
              { plist = _ppages[page]; }
            // draw title
            for (uint8_t i = 0 ; i <= _pages ; i++ )
              {
                      //SOUT(i); SOUT(" - ");
                pcell = (md_cell*) plist->pIndex(i);
                if ( i == 0 )
                  { //this->drawEntry((char*) pcell->getobj(), i, MD4_YELLOW);
                  }
                else
                  { this->drawEntry((char*) pcell->getobj(), i); }
                      //SOUTLN((char*) pcell->getobj());
              }
          }
          else
            {
              SOUTLN(" ERROR !! show: menu not initialized !!");
            }
      }

    void md_menu::hide()
      {

      }

    void md_menu::drawEntry(const char* txt, uint8_t line, uint16_t color)
      {
        //pgfx_->setColor(MD4_BLACK);
        pgfx_->drawRect(_x, _y[line], _ym[line], 16);
        pgfx_->commit();
        pgfx_->setFont(MENU_FONT);
        pgfx_->setColor(color);
        pgfx_->drawString(_x + 1, _y[line] + 1, txt);
        pgfx_->commit();
      }

  // ----------------------------------------------------------------
  // --- class bmp_pgm
  bmp_pgm::bmp_pgm(Adafruit_ILI9341* ptft) { _ptft = ptft; }
  bmp_pgm::~bmp_pgm() {}

  /*
      void bmp_pgm::init(mdGrafx *ptft, const char *palBmp)
        {
          SOUT(" bmp::init grafx "); SOUTHEX((uint32_t) ptft); SOUT(" bmp "); SOUTHEXLN((uint32_t) palBmp);
          if (palBmp != NULL)
            {
              SOUT(" getBMPHeader ...");
              _ptft = ptft;
              _pbmp = palBmp;
              _ptft->getPalBitmapHeadFromPgm(&_head, _pbmp);
              SOUTLN(" ready");
            }
        }
    */

  void bmp_pgm::draw(const char *palBmp ,int16_t x, int16_t y)  // left upper position
    {
      if ( (x >= 0) && (y >= 0) & (palBmp != NULL))
        {
          if ((_pbmp != NULL) && (_pbmp != palBmp))
            { // clear old logo
              _ptft->setColor(MD16_BLACK);
              _ptft->fillRect(_posx, _posy, _head.width, _head.height);
              _ptft->commit();
            }
          _posx = x;
          _posy = y;
          _pbmp = palBmp;
          _ptft->getPalBitmapHeadFromPgm(&_head, _pbmp);
          _ptft->drawPalettedBitmapFromPgm(_posx, _posy, _pbmp);
            /*
              SOUT("draw icon addr "); SOUTHEX((uint32_t) _pbmp);
              SOUT(" size "); SOUT(_head.width * _head.height + sizeof(BMPHeader_t));
              SOUT(" bmpcolor "); SOUT(_head.bitDepth);
              SOUT(" width "); SOUT(_head.width); SOUT(" height "); SOUTLN(_head.height);
              */
        }
    }

#ifdef PARKENS

  TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

  #define CALIBRATION_FILE "/TouchCalData1" // internal flash drive
  #define REPEAT_CAL false // Set to run calibration every startup
  // Keypad start position, key sizes and spacing
  char numberBuffer[KEY_NUM_LEN + 1] = "";
  uint8_t numberIndex = 0;

  // create  keys for the keypad
  char     keyLabel[KEY_NUM_LEN][5] = {"SET", "DO", "OK"};
  uint16_t keyColor[KEY_NUM_LEN] = {TFT_GREENYELLOW, TFT_GREENYELLOW, TFT_RED};
  uint16_t labelColor[KEY_NUM_LEN] = {TFT_NAVY, TFT_NAVY, TFT_GREENYELLOW};

  // Invoke the TFT_eSPI button class and create all the button objects
  TFT_eSPI_Button key[KEY_NUM_LEN];

  // public implementation
  bool md_touch::startTouch()
    {
          #if (DEBUG_MODE >= CFG_DEBUG_DETAILS)
            Serial.println("md_touch::startTouch .. initTFT .."); delay(100);
          #endif
      _initTFT(TFT_BL);
          #if (DEBUG_MODE >= CFG_DEBUG_DETAILS)
            Serial.println(".. clearTFT .."); delay(100);
          #endif
      _clearTFT();
          #if (DEBUG_MODE >= CFG_DEBUG_DETAILS)
            Serial.println(".. drawKeypad .."); delay(100);
          #endif
      _drawKeypad();
          #if (DEBUG_MODE >= CFG_DEBUG_DETAILS)
            Serial.println(".. wrstatus .."); delay(100);
          #endif
      wrStatus("TFT&Touch started");
          #if (DEBUG_MODE >= CFG_DEBUG_STARTUP)
            Serial.println("md_touch::startTouch ready"); delay(100);
          #endif
      return ISOK;
    }

  bool md_touch::calTouch() // calibrate touch
    {
      uint16_t calData[5];
      uint8_t calDataOK = 0;

      // check file system exists
      if (!SPIFFS.begin())
      {
        Serial.println("Formating file system");
        SPIFFS.format();
        SPIFFS.begin();
      }

      // check if calibration file exists and size is correct
      if (SPIFFS.exists(CALIBRATION_FILE))
      {
        if (REPEAT_CAL)
        {
          // Delete if we want to re-calibrate
          SPIFFS.remove(CALIBRATION_FILE);
        }
        else
        {
          File f = SPIFFS.open(CALIBRATION_FILE, "r");
          if (f)
          {
            if (f.readBytes((char *)calData, 14) == 14)
              calDataOK = 1;
            f.close();
          }
        }
      }

      if (calDataOK && !REPEAT_CAL)
      {
        // calibration data valid
        tft.setTouch(calData);
      }
      else
      {
        // data not valid so recalibrate
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(20, 0);
        tft.setTextFont(2);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.println("Touch corners as indicated");

        tft.setTextFont(1);
        tft.println();

        if (REPEAT_CAL)
        {
          tft.setTextColor(TFT_RED, TFT_BLACK);
          tft.println("Set REPEAT_CAL to false to stop this running again!");
        }

        tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);
        //   tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, TFT_CS);

        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.println("Calibration complete!");

        // store data
        File f = SPIFFS.open(CALIBRATION_FILE, "w");
        if (f)
        {
          f.write((const unsigned char *)calData, 14);
          //      f.write((const unsigned char *)calData, TOUCH_CS);
          f.close();
        }
      }
      return false;
    }

  bool md_touch::wrTouch(const char *msg, uint8_t spalte, uint8_t zeile ) // write to text area
    {
      //Serial.print("Write "); Serial.print(msg); Serial.print(" - "); Serial.print(zeile); Serial.print(" - "); Serial.println(spalte);
      tft.setTextPadding(240);
      tft.setTextColor(DISP_TX_FCOL, DISP_TX_BCOL);
      tft.setTextDatum(L_BASELINE);

      tft.setTextFont(2);
      tft.setTextSize(2);
      //tft.setCursor(spalte, zeile);
      strncpy(outTxt, msg, STAT_LINELEN - spalte + 1);
      //  outTxt[STAT_LINELEN - spalte] = 0;
      tft.drawString(outTxt, spalte * 13, zeile * 29 - 5 );
      return ISOK;
    }

  void md_touch::runTouch(char* pStatus)
    {
        uint16_t t_x = 0, t_y = 0; // To store the touch coordinates
        boolean pressed = false;

        // touch lesen
        if (pressed = tft.getTouch(&t_x, &t_y))
            {
              if (!isAct)
                {
                  sprintf(outTxt," x%d, y%d", t_x, t_y);
                  Serial.println(outTxt);
                  wrTouch(outTxt, 0, 3);
                  wrTouch("", 0, 4);
                  SET(isAct);
                }
            }
          else
            {
              if (isAct)
                {
                  Serial.println(" released ");
                  wrTouch("", 0, 3);
                  RESET(isAct);
                }
            }

        // / Check if any key coordinate boxes contain the touch coordinates
        for (uint8_t b = 0; b < KEY_NUM_LEN; b++)
          {
            if (pressed && key[b].contains(t_x, t_y))
                {
                  key[b].press(true);  // tell the button it is pressed
                }
              else
                {
                  key[b].press(false);  // tell the button it is NOT pressed
                }
          }

        // Check if any key has changed state
        for (uint8_t b = 0; b < KEY_NUM_LEN; b++)
          {
            // set caption font
            if (b < 2)
              {
                //tft.setFreeFont(NORM_FONT);
                tft.setFreeFont(BOLD_FONT);
              }
              else
              {
                tft.setFreeFont(BOLD_FONT);
              }



            // check and handle state
            if (key[b].justReleased())
              {
                key[b].drawButton();     // draw normal
              }

            if (key[b].justPressed())
              {
                key[b].drawButton(true);  // draw invert
                switch (b)
                  {
                    case 0:
                      Serial.println("key 1 pressed");
                      wrTouch(" key 1", 4, 4);
                      break;
                    case 1:
                      wrTouch(" key 2", 4, 4);
                      Serial.println("key 2 pressed");
                      break;
                    case 2:
                      wrTouch(" key 3", 4, 4);
                      Serial.println("key 3 pressed");
                      break;
                    default:
                        break;
                  }

                // Draw the string, the value returned is the width in pixels
                // int xwidth = tft.drawString(numberBuffer, DISP_X + 4, DISP_Y + 12);

                // Now cover up the rest of the line up by drawing a black rectangle.  No flicker this way
                // but it will not work with italic or oblique fonts due to character overlap.
                // tft.fillRect(DISP_X + 4 + xwidth, DISP_Y + 1, DISP_W - xwidth - 5, DISP_H - 2, TFT_BLACK);

                usleep(10000); // UI debouncing
              }
          }
  //        pStatus =  pStatus;  // no warning
    }

  bool md_touch::wrStatus() // write status line
    {
      return wrStatus("");
    }
  bool md_touch::wrStatus(const char *msg)
    {
      return wrStatus(msg,STAT_DELTIME);
    }
  bool md_touch::wrStatus(const char *msg, uint32_t stayTime)
    {
      //unsigned long diffTime = millis() - statWrTime;
      int8_t res = 0; // -1:wait, 0:ok, 1:write , 2:clear
      bool   ret = ISOK;
      if(strlen(msg) == 0)
      {
        if ((isStatus == true) && (clrT.TOut() == true))
        { // status is visible && timeout
          res = 2;
          isStatus = false;
                #if (DEBUG_MODE >= CFG_DEBUG_ACTIONS)
                  Serial.print((uint32_t) millis());
                  Serial.println(" Statuszeile loeschen");
                #endif
        }
      }
      else
      {
        if ((isStatus == true) && (minT.TOut() == false))
        { // visible status has to stay
          res = -1;
        }
        else
        { // write it
          res = 1;
          isStatus = true;
          if (stayTime == 0)
          {
            stayTime = STAT_DELTIME;
          }
                #if (DEBUG_MODE >= CFG_DEBUG_ACTIONS)
                  Serial.print((uint32_t) millis());
                  Serial.println(" Statuszeile schreiben");
                #endif
        }

      }
      if (res > 0)
      {
        memset(outTxt, 0, STAT_LINELEN + 1);
        if (res == 1)
        {
          strncpy(outTxt, msg, STAT_LINELEN);
        }
        ret = _drawStatus(outTxt);
        minT.startT();              // start timer min time
        clrT.startT(stayTime);      // start timer max time
        res = 0;
      }
      return (ret || (res != 0));
    }
  //
  // ------ private implementation ------------
  bool md_touch::_drawScreen()
    {
      // Draw keypad background
      tft.fillRect(0, 0, 240, 320, TFT_DARKGREY);
      // Draw number display area and frame
      tft.fillRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_BLACK);
      tft.drawRect(DISP_X, DISP_Y, DISP_W, DISP_H, TFT_WHITE);
      return ISOK;
    }

  bool md_touch::_drawKeypad()
    {
      // Draw the keys
      for (uint8_t col = 0; col < 3; col++)
      {
        tft.setFreeFont(NORM_FONT);
        tft.setFreeFont(BOLD_FONT);

        key[col].initButton(&tft,
                          KEY_X + col * (KEY_W + KEY_SPACING_X),
                          KEY_Y /* + row * (KEY_H + KEY_SPACING_Y)*/, // x, y, w, h, outline, fill, text
                          KEY_W, KEY_H, TFT_WHITE,
                          keyColor[col], labelColor[col],
                          keyLabel[col], KEY_TEXTSIZE);
        key[col].drawButton();
      }
      return ISOK;
    }

  bool md_touch::_drawStatus(char* outStat)
    {
      tft.fillRect(STAT_XLI, STAT_YOB, STAT_XRE, STAT_YUN, STAT_BCOL);
      tft.setTextPadding(240);
      //tft.setCursor(STAT_X, STAT_Y);
      tft.setTextColor(STAT_FCOL, STAT_BCOL);
      tft.setTextDatum(MC_DATUM);

      tft.setTextFont(1);
      tft.setTextSize(2);
      tft.drawString(outStat, STAT_XCENT, STAT_YCENT);
      return ISOK;
    }

  //
  // Print something in the mini status bar
  bool md_touch::_clearTFT()
    {
      tft.fillScreen(DISP_BCOL);
      return ISOK;
    }

  bool md_touch::_initTFT(const uint8_t csTFT)
    {
      pinMode(csTFT, OUTPUT);
      digitalWrite(csTFT, LOW);
      delay(1);  // Initialise the TFT screen
      tft.init();
      // Set the rotation before we calibrate
      tft.setRotation(DISP_ORIENT);
      // Calibrate the touch screen and retrieve the scaling factors
      calTouch();
      // Clear the screen
      _clearTFT();
      return ISOK;
    }

  //
  #endif
  #endif
  #endif // NOTREADY
