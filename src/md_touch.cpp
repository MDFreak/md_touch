  #include <md_touch.h>
  #include <md_spiffs.h>
  #include <string>
  #include <ArialRounded.h>
  #include <md_defines.h>

    static XPT2046_Touchscreen *_ptouch   = NULL;
    static Adafruit_ILI9341    *_pTFT     = NULL;
    static md_touch            *_pmdtouch = NULL;
    //md_TouchEvent     tevent(ptouch);
    md_spiffs         conf = md_spiffs();
    static md_spiffs *pConf = &conf;


// --- class md_touch
	md_touch::md_touch(uint8_t cspin, uint8_t tft_CS, uint8_t tft_DC, uint8_t tft_RST) //, uint8_t spi_bus)
    {
      _pmdtouch = this;
      _ptouch   = new XPT2046_Touchscreen(cspin);
      _pTFT     = new Adafruit_ILI9341(tft_CS, tft_DC, tft_RST);
      _ptouchev = new md_TouchEvent()
    }

  md_touch::~md_touch()
    {
      //pts_ = NULL;
    }

  // public implementation
  bool md_touch::start(uint8_t rotation)
    {
      Adafruit_ILI9341 pTFT = new(Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
    XPT2046_Touchscreen  touch(TOUCH_CS);
    XPT2046_Touchscreen *ptouch = &touch;


//Hintergrundbeleuchtung einschalten
pinMode(TFT_LED,OUTPUT);
digitalWrite(TFT_LED, LED_ON);
//Display initialisieren
Serial.println(" .. tft");
tft.begin();
tft.setRotation(TOUCH_ROTATION);
tft.fillScreen(BACKGROUND);



touch.begin();
touch.setRotation(TOUCH_ROTATION);
tevent.setResolution(tft.width(),tft.height());
tevent.setDrawMode(false);
//Callback Funktion registrieren
tevent.registerOnTouchClick(onClick);
            //tft.fillRect(LEFTMARGIN,TOPMARGIN,xposUMNS*BLOCKSIZE,yposS*BLOCKSIZE,NOPIECE);
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

#ifdef NOT_USED
// --- includes
  #include <md_touch.h>
  #include <md_spiffs.h>
  #include <string>
  #include <ArialRounded.h>
  #include <md_defines.h>

// --- globals
  md_touch*    pmdt_      = NULL;
  mdGrafx*     pgfx_      = NULL;
  md_menu*     menu_      = NULL;

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
// --- class md_touch
	md_touch::md_touch(uint8_t cspin, uint8_t tirq) //, uint8_t spi_bus)
    {
      //pts_ = new XPT2046_Touchscreen(cspin, tirq);
      pmdt_  = this;
      tcs_   = cspin;
      tirq_  = tirq;
      //_pSPI  = p;
      //pts_  = new mdXPT2046(tcs_, tirq_, spi_bus);
    }

  md_touch::~md_touch()
    {
      //pts_ = NULL;
    }

  void md_touch::init(Adafruit_ILI9341* ptft, uint8_t rotation)
    {
      pgfx_ = pgfx;
      _rotation = rotation;
      //pts_->begin(240, 320);
      //if(1)
      if (!loadCalibration())
        {
          SOUT("Calibration necessary rot "); SOUTLN(_rotation);
          calibrate();
        }
        else
        {
          SOUTLN("Calibration data found");
        }
      // check if spiffs is present with calibration data
        //pts_  = new XPT2046_Calibrated(_csPin);
        //pts_  = new XPT2046_Touchscreen(_csPin);
        //pts   = new XPT2046_Touchscreen(_csPin, _tirqPin);
      startTouchTask();
    }

  bool md_touch::loadCalibration()
    {
      // always use this to "mount" the filesystem
      //      SOUTLN("loadCalibration SPIFFS ...");
        //      bool result = SPIFFS.begin();
        //      SOUTLN("SPIFFS is init");
        //
        //      SOUT("SPIFFS opened: ");
        //      SOUTLN(result ? "OK" : "Failed");
        //      // this opens the file in read-mode
        //      if (!SPIFFS.exists("/calibration.txt"))
        //        {
        //          SOUTLN("calibration.txt not present");
        //          return false;
        //        }
        //      File f = SPIFFS.open("/calibration.txt", "r");
        //      SOUT("filename "); SOUTLN(f.name());
        //      if (f)
        //        {
        //          //Lets read line by line from the file
        //          char tmp[9];
        //          tmp[8] = 0;
        //          calParams_t cal;
        //
        //          SOUTLN("load cal params ");
        //          f.readBytesUntil('\n', tmp, 8);
        //          SOUT("dx  '"); SOUT(tmp); SOUT("' ");
        //          cal.dx = atoi(tmp);
        //          SOUTLN(cal.dx);
        //          f.readBytesUntil('\n', tmp, 8);
        //          SOUT("dy  '"); SOUT(tmp); SOUT("' ");
        //          cal.dy = atoi(tmp);
        //          SOUTLN(cal.dy);
        //          f.readBytesUntil('\n', tmp, 8);
        //          SOUT("vi1 '"); SOUT(tmp); SOUT("' ");
        //          cal.vi1 = atoi(tmp);
        //          SOUTLN(cal.vi1);
        //          f.readBytesUntil('\n', tmp, 8);
        //          SOUT("vj1 '"); SOUT(tmp); SOUT("' ");
        //          cal.vj1 = atoi(tmp);
        //          SOUTLN(cal.vj1);
        //          f.readBytesUntil('\n', tmp, 8);
        //          SOUT("dvi  '"); SOUT(tmp); SOUT("' ");
        //          cal.dvi = atoi(tmp);
        //          SOUTLN(cal.dvi);
        //          f.readBytesUntil('\n', tmp, 8);
        //          SOUT("dvj  '"); SOUT(tmp); SOUT("' ");
        //          cal.dvj = atoi(tmp);
        //          SOUTLN(cal.dvj);
        //          //String dxStr  = tmp;
        //          /*
        //            String dxStr  = f.readString();
        //            String dyStr  = f.readString(); //  readStringUntil(';');
        //            String dviStr = f.readString(); //  readStringUntil(';');
        //            String dvjStr = f.readString(); //  readStringUntil(';');
        //            String vi1Str = f.readString(); //  readStringUntil(';');
        //            String vj1Str = f.readString(); //  readStringUntil(';');
        //              SOUT("cal params '"); SOUT(dxStr);
        //              SOUT("' '"); SOUT(dyStr);
        //              SOUT("' '"); SOUT(dviStr);
        //              SOUT("' '"); SOUT(dvjStr);
        //              SOUT("' '"); SOUT(vi1Str);
        //              SOUT("' '"); SOUT(vj1Str);
        //            int32_t cal_dx  = dxStr.toInt();
        //            int32_t cal_dy  = dyStr.toInt();
        //            int32_t cal_dvi = dviStr.toInt();
        //            int32_t cal_dvj = dvjStr.toInt();
        //            int32_t cal_vi1 = vi1Str.toInt();
        //            int32_t cal_vj1 = vj1Str.toInt();
        //              SOUT("' int "); SOUT(cal_dx);
        //              SOUT(" "); SOUT(cal_dy);
        //              SOUT(" "); SOUT(cal_dvi);
        //              SOUT(" "); SOUT(cal_dvj);
        //              SOUT(" "); SOUT(cal_vi1);
        //              SOUT(" "); SOUTLN(cal_vj1);
        //            if (!cal_dx)  return false;
        //            if (!cal_dy)  return false;
        //            if (!cal_dvi) return false;
        //            if (!cal_dvj) return false;
        //            if (!cal_vi1) return false;
        //            if (!cal_vj1) return false;
        //            */
        //          f.close();
        //          pts_->setCalibration(&cal);
        //        }
      return true;
    }

/*void md_touch::calibratePoint(uint16_t x, uint16_t y, uint16_t &vi, uint16_t &vj)
  {
    SOUT(" calibratePoint "); SOUTHEXLN((uint32_t) pgfx_);
    // Draw cross
      pgfx_->fillBuffer(MD4_BLACK);
      pgfx_->commit();
      pgfx_->setColor(MD4_YELLOW);
      pgfx_->drawLine(x - 8, y, x + 8, y);
      pgfx_->drawLine(x, y - 8, x, y + 8);
      pgfx_->commit();

    SOUT("wait for touching ... ");
    while (!pts_->isTouching())
      { usleep(100000); }
    SOUT("touching ... ");
    pts_->getRaw(vi, vj);
    //pgfx_->drawRect(x - 8, y - 8, 17, 17);
    while (pts_->isTouching())
      { usleep(1000); }
    SOUTLN(" touch released");
  }
*/
void md_touch::calibrate()
  {
    //uint16_t x1, y1, x2, y2, x3, y3, x4, y4;
    //uint16_t vi1, vj1, vi2, vj2, vi3, vj3, vi4, vj4;
    ////uint8_t  isok = false;
    //calParams_t cal;
    //char buf[80];
    //
    //SOUTLN("Start calibration ...");
    //pts_->getCalibrationPoints(x1, y1, x2, y2);
    //SOUT("cal point "); SOUT(x1); SOUT(" "); SOUTLN(y1);
    //calibratePoint(x1, y1, vi1, vj1);
    //SOUT("cal p(1) "); SOUT(x1); SOUT(" "); SOUT(y1); SOUT(" "); SOUT(vi1); SOUT(" "); SOUTLN(vj1);
    ////calibratePoint(x1, y1, cal.vi1, cal.vj1);
    ////SOUT("cal p(1) "); SOUT(x1); SOUT(" "); SOUT(y1); SOUT(" "); SOUT(cal.vi1); SOUT(" "); SOUTLN(cal.vj1);
    //sleep(1);
    ////usleep(50000);
    ////calibratePoint(x2, y2, cal.dvi, cal.dvj);
    ////SOUT("cal p(2) "); SOUT(x2); SOUT(" "); SOUT(y2); SOUT(" "); SOUT(cal.dvi); SOUT(" "); SOUTLN(cal.dvj);
    //SOUT("cal point "); SOUT(x2); SOUT(" "); SOUTLN(y2);
    //calibratePoint(x2, y2, vi2, vj2);
    //SOUT("cal p(2) "); SOUT(x2); SOUT(" "); SOUT(y2); SOUT(" "); SOUT(vi2); SOUT(" "); SOUTLN(vj2);
    //sleep(1);
    //SOUT("cal point "); SOUT(x3); SOUT(" "); SOUTLN(y3);
    //x3 = 20;  y3 = 300;
    //x4 = 220; y4 = 20;
    //calibratePoint(x3, y3, vi3, vj3);
    //SOUT("cal p(3) "); SOUT(x3); SOUT(" "); SOUT(y3); SOUT(" "); SOUT(vi3); SOUT(" "); SOUTLN(vj3);
    //sleep(1);
    //SOUT("cal point "); SOUT(x4); SOUT(" "); SOUTLN(y4);
    //calibratePoint(x4, y4, vi4, vj4);
    //SOUT("cal p(4) "); SOUT(x4); SOUT(" "); SOUT(y4); SOUT(" "); SOUT(vi4); SOUT(" "); SOUTLN(vj4);
    //
    //
    //cal.dx = x2 - x1;
    //cal.dy = y2 - y1;
    //cal.vi1 = vi1;
    //cal.vj1 = vj1;
    //cal.dvi -= cal.vi1;
    //cal.dvj -= cal.vj1;
    //
    //
    //pts_->setCalibration(&cal);
    //// show result on touch
    //  snprintf(buf, sizeof(buf), "%u %u %u %u %u %u",      cal.dx,  cal.dy,
    //                             cal.vi1, cal.vj1, cal.dvi, cal.dvj);
    //  pgfx_->fillBuffer(MD4_BLACK);
    //  pgfx_->setFont(ArialRoundedMTBold_14);
    //  pgfx_->setColor(MD4_YELLOW);
    //  pgfx_->drawStringInternal(0, 200, (char*) "setCalibration params:", 22, 240);
    //  pgfx_->drawStringInternal(0, 220, buf, strlen(buf), 240);
    //  pgfx_->commit();
    //
    //snprintf(buf, sizeof(buf), "%u\n%u\n%u\n%u\n%u\n%u\n",      cal.dx,  cal.dy,
    //                           cal.vi1, cal.vj1, cal.dvi, cal.dvj);
    //SOUT("cal "); SOUT(buf); SOUT(" "); SOUTLN(sizeof(buf));
    //saveCalibration(buf, sizeof(buf));
  }

  bool md_touch::saveCalibration(char* text, size_t len)
    {
      bool result = SPIFFS.begin();
    //  if (result)
    //    {
    //      File f = SPIFFS.open("/calibration.txt", "w");
    //      if (!f)
    //        {
    //          SOUTLN("file creation failed");
    //          result = false;
    //        }
    //      else
    //        {
    //          f.close();
    //          SPIFFS.remove("/calibration.txt");
    //          f = SPIFFS.open("/calibration.txt", "w");
    //          f.write((uint8_t*) text, len);
    //          SOUT("file '"); SOUT(text); SOUT("' ("); SOUT(len);SOUTLN(") saved");
    //          f.flush();
    //          f.close();
    //        }
    //    }
      return result;
    }

  bool md_touch::loadCalibration()
    {
      // always use this to "mount" the filesystem
      bool result = SPIFFS.begin();
      Serial.print("SPIFFS opened: ");
      Serial.println(result ? "OK" : "Failed");

      // this opens the file in read-mode
      File f = SPIFFS.open("/calibration.txt", "r");

      if (!f)
        { return false; }
        else
        {
          //Lets read line by line from the file
          String dxStr = f.readStringUntil('\n');
          String dyStr = f.readStringUntil('\n');
          String axStr = f.readStringUntil('\n');
          String ayStr = f.readStringUntil('\n');

          dx = dxStr.toFloat();
          dy = dyStr.toFloat();
          ax = axStr.toInt();
          ay = ayStr.toInt();
        }
      f.close();
      return true;
    }

  bool md_touch::saveCalibration()
    {
      bool result = SPIFFS.begin();

      // open the file in write mode
      File f = SPIFFS.open("/calibration.txt", "w");
      if (!f)
        {
          Serial.println("file creation failed");
          return false;
        }
      // now write two lines in key/value style with  end-of-line characters
      f.println(dx);
      f.println(dy);
      f.println(ax);
      f.println(ay);

      f.close();

      return true;
    }

  void md_touch::doCalibration()
    {
      state = 0;
      this->calibrationCallback = calibrationCallback;
    }
  #ifdef XXXX
    bool md_touch::isTouched()
      {
        return pts_->touched();
      }

    void md_touch::continueCalibration()
      {
        TS_Point p = pts_->getPoint();
        if (state == 0)
          {
            (*calibrationCallback)(10, 10);
            if (pts_->touched())
              {
                p1 = p;
                state++;
                lastStateChange = millis();
              }
          }
        else if (state == 1)
          {
            (*calibrationCallback)(230, 310);
            if (pts_->touched() && (millis() - lastStateChange > 1000))
              {

                p2 = p;
                state++;
                lastStateChange = millis();
                dx = 240.0 / abs(p1.y - p2.y);
                dy = 320.0 / abs(p1.x - p2.x);
                ax = p1.y < p2.y ? p1.y : p2.y;
                ay = p1.x < p2.x ? p1.x : p2.x;
              }
          }
        }
    bool md_touch::isCalibrationFinished()
      {
        return state == 2;
      }

    bool md_touch::isTouched(int16_t debounceMillis)
      {
        if (pts_->touched() && millis() - lastTouched > debounceMillis)
          {
            lastTouched = millis();
            return true;
          }
        return false;
      }

    TS_Point md_touch::getPoint()
      {
        TS_Point p = pts_->getPoint();
        /*
        int x = (p.y - ax) * dx;
        int y = 320 - (p.x - ay) * dy;
        p.x = x;
        p.y = y;
        */
        p.x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
        p.y = map(p.y, TS_MINY, TS_MAXY, 240, 0);

        return p;
      }
  #endif
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

#ifdef PARKEN

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
      return wrStatus(msg,STAT_TIMEDEF);
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
            stayTime = STAT_TIMEDEF;
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