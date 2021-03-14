#include "md_touch.h"

#ifdef USE_TOUCHSCREEN

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
      tft.fillRect(STATUS_XLI, STATUS_YOB, STATUS_XRE, STATUS_YUN, STATUS_BCOL);
      tft.setTextPadding(240);
      //tft.setCursor(STATUS_X, STATUS_Y);
      tft.setTextColor(STATUS_FCOL, STATUS_BCOL);
      tft.setTextDatum(MC_DATUM);

      tft.setTextFont(1);
      tft.setTextSize(2);
      tft.drawString(outStat, STATUS_XCENT, STATUS_YCENT);
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