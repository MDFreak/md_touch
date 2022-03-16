/*
 * Copyright (c) 2015-2016  Spiros Papadimitriou
 *
 * This file is part of github.com/spapadim/XPT2046 and is released
 * under the MIT License: https://opensource.org/licenses/MIT
 *
 * This software is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied.
 */

#include <Arduino.h>
#include <md_SPI.h>

#include <md_XPT2046.h>
#include <md_defines.h>

#define Z_THRESHOLD_      400
#define Z_THRESHOLD_INT_	75
#define MSEC_THRESHOLD_   3
#define SPI_SETTING     SPISettings(2000000, MSBFIRST, SPI_MODE0)

static mdXPT2046 	*isrPinptr;
void isrPin(void);

inline static void swap(uint16_t &a, uint16_t &b)
  {
    uint16_t tmp = a;
    a = b;
    b = tmp;
  }

/**********************************************************/
#if 0

    // Bisection-based median; will modify vals array
    static uint16_t fastMedian (uint16_t *vals, uint8_t n) {
      uint8_t l = 0, r = n;

      while (l < r) {
        uint16_t pivot = vals[l];  // TODO use middle elt?
        uint8_t i = l+1, j = r-1;
        while (i <= j) {
          while ((i < r) && (vals[i] <= pivot)) {
            ++i;
          }
          while ((j > l) && (vals[j] > pivot)) {
            --j;
          }
          if (i < j) {
            swap(vals[i], vals[j]);
          }
        }
        swap(vals[l], vals[j]);

        // j is final pivot position
        if (j == n/2) {
          return vals[j];
        } else if (n/2 < j) {
          r = i;
        } else { // n/2 > j
          l = j+1;
        }
      }
    }

    static uint16_t mean (const uint16_t *vals, uint8_t n) {
      uint32_t sum = 0;
      for (uint8_t i = 0;  i < n;  i++) {
        sum += vals[i];
      }
      return (uint16_t)(sum/n);
    }

  #endif // 0
/**********************************************************/

  static int16_t besttwoavg( int16_t x , int16_t y , int16_t z )
    {
      int16_t da, db, dc;
      int16_t reta = 0;
      if ( x > y ) da = x - y; else da = y - x;
      if ( x > z ) db = x - z; else db = z - x;
      if ( z > y ) dc = z - y; else dc = y - z;

      if ( da <= db && da <= dc ) reta = (x + y) >> 1;
      else if ( db <= da && db <= dc ) reta = (x + z) >> 1;
      else reta = (y + z) >> 1;   //    else if ( dc <= da && dc <= db ) reta = (x + y) >> 1;

      return (reta);
    }

  /*
  static int16_t besttwoavg( int16_t x , int16_t y , int16_t z )
    {
      int16_t da, db, dc;
      int16_t reta = 0;
      if ( x > y ) da = x - y; else da = y - x;
      if ( x > z ) db = x - z; else db = z - x;
      if ( z > y ) dc = z - y; else dc = y - z;

      if ( da <= db && da <= dc ) reta = (x + y) >> 1;
      else if ( db <= da && db <= dc ) reta = (x + z) >> 1;
      else reta = (y + z) >> 1;   //    else if ( dc <= da && dc <= db ) reta = (x + y) >> 1;

      return (reta);
    }
  */
//--- interrupt servis
  ISR_PREFIX
  void isrPin( void )
    {
    	mdXPT2046 *o = isrPinptr;
    	o->_isrWake = true;
    }

//
//--- class ts_point
ts_point mdXPT2046::getPoint()
  {
  	update();
  	return ts_point(_xraw, _yraw, _zraw);
  }


//
//--- class ts_calibration
ts_calibration::ts_calibration(
    	const ts_point aS, const ts_point aT,
    	const ts_point bS, const ts_point bT,
    	const ts_point cS, const ts_point cT,
    	const uint16_t sW, const uint16_t sH)
  {
  	defined      = true;
  	screenWidth  = sW;
  	screenHeight = sH;

  	int32_t delta =
  	    ( (aT.x - cT.x) * (bT.y - cT.y) )
  	    -
  	    ( (bT.x - cT.x) * (aT.y - cT.y) );

  	alphaX =
  	    (float)
  	      ( ( (aS.x - cS.x) * (bT.y - cT.y) )
  	        -
  	        ( (bS.x - cS.x) * (aT.y - cT.y) ) )
  	    / delta;

  	betaX =
  	    (float)
  	      ( ( (aT.x - cT.x) * (bS.x - cS.x) )
  	        -
  	        ( (bT.x - cT.x) * (aS.x - cS.x) ) )
  	    / delta;

  	deltaX =
  	    ( ( (uint64_t)aS.x
  	          * ( (bT.x * cT.y) - (cT.x * bT.y) ) )
  	      -
  	      ( (uint64_t)bS.x
  	          * ( (aT.x * cT.y) - (cT.x * aT.y) ) )
  	      +
  	      ( (uint64_t)cS.x
  	          * ( (aT.x * bT.y) - (bT.x * aT.y) ) ) )
  	    / delta;

  	alphaY =
  	    (float)
  	      ( ( (aS.y - cS.y) * (bT.y - cT.y) )
  	        -
  	        ( (bS.y - cS.y) * (aT.y - cT.y) ) )
  	    / delta;

  	betaY =
  	    (float)
  	      ( ( (aT.x - cT.x) * (bS.y - cS.y) )
  	        -
  	        ( (bT.x - cT.x) * (aS.y - cS.y) ) )
  	    / delta;

  	deltaY =
  	    ( ( (uint64_t)aS.y
  	          * (bT.x * cT.y - cT.x * bT.y) )
  	      -
  	      ( (uint64_t)bS.y
  	          * (aT.x * cT.y - cT.x * aT.y) )
  	      +
  	      ( (uint64_t)cS.y
  	          * (aT.x * bT.y - bT.x * aT.y) ) )
  	    / delta;
  }


//
//--- class mdXPT2046
  mdXPT2046::mdXPT2046 (uint8_t cs_pin, uint8_t irq_pin, uint8_t spi_bus)
    {
      _cs   = cs_pin;
      _irq  = irq_pin;
      _spi_bus = spi_bus;
    }

  void mdXPT2046::begin(uint16_t width, uint16_t height, uint8_t rot)
    {
      _width  = width;
      _height = height;
      _pmdSPI = pVSPI;
      _rot    = rot % 4;
      SOUT(" md_XPT2046.begin SPI "); SOUTHEX((uint32_t) &SPI);
      SOUT(" width "); SOUT(_width); SOUT(" heigth "); SOUTLN(_height);
      _pSPI = &SPI;
      pinMode(_cs, OUTPUT);
      if (_irq != 255)
        {
          pinMode(_irq, INPUT);
      		attachInterrupt(digitalPinToInterrupt(_irq), isrPin, FALLING);
      		isrPinptr = this;
        }

      //SOUTHEX((uint32_t) millis); SOUT(" XPT2046 begin _cs "); SOUT(_cs); SOUT(" _irq "); SOUTLN(_irq);

      // Default calibration -> to override with stored calibration values (from SPIFFS)
      // calParams_t cal = { 0, 0, 209, 1759, 1775, 273 }; // vj2
            /* TODO(?) Use the following empirical calibration instead? -- Does it depend on VCC??
               touch.setCalibration(209, 1759, 1775, 273);
               SPI.begin(); not allowed here */
      // setCalibration(&cal);
      // powerDown();  // Make sure PENIRQ is enabled
    }

  void mdXPT2046::setRotation(uint8_t rot)
    { _rot = rot % 4; }

bool mdXPT2046::tirqTouched()
  {
  	return (_isrWake);
  }

bool mdXPT2046::touched()
  {
  	update();
  	return (_zraw >= Z_THRESHOLD_);
  }

void mdXPT2046::readData(uint16_t *x, uint16_t *y, uint8_t *z)
  {
  	update();
  	*x = _xraw;
  	*y = _yraw;
  	*z = _zraw;
  }

bool mdXPT2046::bufferEmpty()
  {
  	return ((millis() - msraw) < MSEC_THRESHOLD_);
  }

// TODO: perhaps a future version should offer an option for more oversampling,
//       with the RANSAC algorithm https://en.wikipedia.org/wiki/RANSAC

      /*  void mdXPT2046::getCalibrationPoints(uint16_t &x1, uint16_t &y1, uint16_t &x2, uint16_t &y2)
          {
            x1 = y1 = TS_CAL_POS;
            x2 = _width  - TS_CAL_POS;
            y2 = _height - TS_CAL_POS;
          }

        void mdXPT2046::setCalibration (calParams_t* pcalPars)
          {
            SOUT(" setCalibration _cal "); SOUT(_cal.dx);
            SOUT(" "); SOUT(_cal.dy);
            SOUT(" "); SOUT(_cal.vi1);
            SOUT(" "); SOUT(_cal.vj1);
            SOUT(" "); SOUT(_cal.dvi);
            SOUT(" "); SOUTLN(_cal.dvj);

            if (pcalPars->dx == 0) { pcalPars->dx  = _width  - 2 * TS_CAL_POS; }
            if (pcalPars->dx == 0) { pcalPars->dy  = _height - 2 * TS_CAL_POS; }

            SOUT(" setCalibration pCalPars "); SOUT(pcalPars->dx);
            SOUT(" "); SOUT(pcalPars->dy);
            SOUT(" "); SOUT(pcalPars->vi1);
            SOUT(" "); SOUT(pcalPars->vj1);
            SOUT(" "); SOUT(pcalPars->dvi);
            SOUT(" "); SOUTLN(pcalPars->dvj);
            _cal = *pcalPars;
            SOUT(" setCalibration _cal "); SOUT(_cal.dx);
            SOUT(" "); SOUT(_cal.dy);
            SOUT(" "); SOUT(_cal.vi1);
            SOUT(" "); SOUT(_cal.vj1);
            SOUT(" "); SOUT(_cal.dvi);
            SOUT(" "); SOUTLN(_cal.dvj);
          }

        // TODO: Caveat - MODE_SER is completely untested!!
        bool mdXPT2046::isTouching()
          {
            uint8_t isT = (digitalRead(_irq) == LOW);
            if (isT) SOUTLN("isTouching ");
            return isT;
          }

        //   Need to measure current draw and see if it even makes sense to keep it as an option
        void mdXPT2046::getRaw (uint16_t &vi, uint16_t &vj)
          {
            // Implementation based on TI Technical Note http://www.ti.com/lit/an/sbaa036/sbaa036.pdf
            uint8_t ctrl_lo = TS_MEAS_RUN | TS_MEAS_ADC | TS_MODE_12B;
                #ifdef TEMPLATE
                	      //SPI.beginTransaction(SPI_SETTING);
                	digitalWrite(_cs, LOW);
                	_pSPI->transfer(0xB1); //  Z1
                	int16_t z1 = SPI.transfer16(0xC1) >> 3; // Z2
                	int z = z1 + 4095;
                	int16_t z2 = SPI.transfer16(0x91) >> 3; // X
                	z -= z2;
                	if (z >= Z_THRESHOLD) {
                		_pSPI->transfer16(0x91);  // dummy X measure, 1st is always noisy
                		data[0] = _pSPI->transfer16(0xD1) >> 3;
                		data[1] = _pSPI->transfer16(0x91) >> 3; // make 3 x-y measurements
                		data[2] = _pSPI->transfer16(0xD1) >> 3;
                		data[3] = _pSPI->transfer16(0x91) >> 3;
                	}
                	else data[0] = data[1] = data[2] = data[3] = 0;	// Compiler warns these values may be used unset on early exit.
                	data[4] = _pSPI->transfer16(0xD0) >> 3;	// Last Y touch power down
                	data[5] = _pSPI->transfer16(0) >> 3;
                	digitalWrite(csPin, HIGH);
                	_pSPI->endTransaction();
                #endif // TEMPLATE
            SOUTLN(" getRaw ...");
            startSPI();
            usleep(10000);
            digitalWrite(_cs, LOW);
            uint8_t ctrl = ctrl_lo | TS_MEAS_X;
            SOUT(" getRaw ctrl1 "); SOUTHEXLN(ctrl);
            _pSPI->transfer(ctrl);  // Send first control byte
            vi = _readLoop(ctrl);
            usleep(1000);
            ctrl = ctrl_lo | TS_MEAS_Y;
            SOUT(" getRaw ctrl2 "); SOUTHEXLN(ctrl);
            _pSPI->transfer(ctrl);  // Send first control byte
            vj = _readLoop(ctrl);
            #ifdef PARKEN
              if (mode == MODE_DFR)
                {
                  // Turn off ADC by issuing one more read (throwaway)
                  // This needs to be done, because PD=0b11 (needed for MODE_DFR) will disable PENIRQ
                  _pSPI->transfer(0);  // Maintain 16-clocks/conversion; _readLoop always ends after issuing a control byte
                  _pSPI->transfer(CTRL_HI_Y | CTRL_LO_SER);
                }
              #endif
            //_pSPI->transfer(0);  // Maintain 16-clocks/conversion; _readLoop always ends after issuing a control byte

            digitalWrite(_cs, HIGH);
            stopSPI();
          }

        void mdXPT2046::getPosition (uint16_t &x, uint16_t &y)
          {
            uint16_t vi, vj;
            if (!isTouching())
              {
                x = y = 0xffff;
              }
              else
              {
                startSPI();
                getRaw(vi, vj);
                stopSPI();
                // Map to (un-rotated) display coordinates
                #if defined(SWAP_AXES) && SWAP_AXES
                  x = (uint16_t)(_cal.dx * (vj - _cal.vj1) / _cal.dvj + TS_CAL_POS);
                  y = (uint16_t)(_cal.dy * (vi - _cal.vi1) / _cal.dvi + TS_CAL_POS);
                #else
                  x = (uint16_t)(_cal_dx * (vi - _cal_vi1) / _cal_dvi + TS_CAL_POS);
                  y = (uint16_t)(_cal_dy * (vj - _cal_vj1) / _cal_dvj + TS_CAL_POS);
                #endif

                // Transform based on current rotation setting
                // TODO: Is it worth to do this by tweaking _cal_* values instead?
                switch (_rot)
                  {  // TODO double-check
                    case ROT90:
                      x = _width - x;
                      swap(x, y);
                      break;
                    case ROT180:
                      x = _width - x;
                      y = _height - y;
                      break;
                    case ROT270:
                      y = _height - y;
                      swap(x, y);
                      break;
                    case ROT0:
                    default:
                      // Do nothing
                      break;
                  }
              }
              SOUT(" getPosition x "); SOUT(x); SOUT(" y "); SOUTLN(y);
          }
          */

  void mdXPT2046::powerDown()
    {
      startSPI();
      digitalWrite(_cs, LOW);
      // Issue a throw-away read, with power-down enabled (PD{1,0} == 0b00)
      // Otherwise, ADC is disabled
      _pSPI->transfer(TS_MEAS_RUN | TS_MEAS_ADC | TS_MODE_12B);
      _pSPI->transfer16(0);  // Flush, just to be sure
      digitalWrite(_cs, HIGH);
      stopSPI();
    }


  // --- private
  void mdXPT2046::startSPI()
    {
      //SOUT(" mdXPT2046 mount SPI ... _isON "); SOUTLN(_isON);
      if (!_isON)
        {
          uint8_t ok = TRUE;
          //SOUT(" mdXPT2046 SPI mount "); SOUTHEXLN((uint32_t) _pmdSPI);
          while (!_pmdSPI->mount((void*)this, _cs, SPI_SETTING))
            {
              if (ok)
                {
                  SOUTLN(" mdXPT2046 SPI nicht verfuegbar -> reset! ");
                  ok = FALSE;
                }
            }
          _isON = ON;
          SOUTLN(" mdXPT2046 SPI mounted");
        }
      //SPI.begin(_sck, _miso, _mosi, _cs); already done in mount
    }

    /*
      uint16_t mdXPT2046::_readLoop(uint8_t ctrl)
        {
          //uint16_t prev = 0xffff, cur = 0xffff;
          uint16_t max = 0, val = 0;
          uint16_t cur;
          uint8_t  anz;

          SOUT("readLoop ctrl "); SOUTHEXLN(ctrl);
          if (_isON)
            {
              for (uint8_t i = 0; i < TS_MAX_DAT ; i++)
                {
                  cur = _pSPI->transfer(0);
                  cur = (cur << 4) | (_pSPI->transfer(ctrl) >> 4);  // 16 clocks -> 12-bits (zero-padded at end)
                  SOUT(cur); SOUT(" ");
                  if ((i > 1) && (cur > max))
                      {
                        max = cur;
                        anz = 0;
                        SOUT(" max "); SOUTLN(max);
                      }
                    else
                      {
                        if (cur > val)
                          {
                            val = cur;
                            SOUT(" val "); SOUTLN(val);
                          }
                        if (++anz > 5) break;
                      }
                }
                  #ifdef PARKEN
                      do
                        {
                          prev = cur;
                          cur = _pSPI->transfer(0);
                          SOUTHEX(cur); SOUT("=");
                          cur = (cur << 4) | (_pSPI->transfer(ctrl) >> 4);  // 16 clocks -> 12-bits (zero-padded at end)
                          SOUTHEX(cur); SOUT(" ");
                        }
                      while ((prev != cur) && (++i < max_samples));
                    #endif
                    //Serial.print("RL i: "); Serial.println(i); Serial.flush();  // DEBUG
                    //cur = _pSPI->transfer(0);
              SOUT(" val "); SOUTLN(val);
            }
          return val;
        }
        */

  void     mdXPT2046::stopSPI()
    {
      if (_isON)
        {
          _pmdSPI->unmount();
          _isON = OFF;
          SOUTLN(" mdXPT2046 SPI released");
        }
    }

