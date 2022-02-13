/*
 * Copyright (c) 2015-2016  Spiros Papadimitriou
 *
 * This file is part of github.com/spapadim/XPT2046 and is released
 * under the MIT License: https://opensource.org/licenses/MIT
 *
 * This software is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied.
 */

#ifndef _MD_XPT2046_h_
  #define _MD_XPT2046_h_

  #include <Arduino.h>
  #include <md_defines.h>
  #include <md_SPI.h>
  // On my display driver ICs i,j mapped to (width-y),x
  //  Flipping can be handled by order of calibration points, but not swapping
  #if !defined(SWAP_AXES)
      #define SWAP_AXES 1
    #endif

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

  // touch data
    #define TS_ADCMAX8  0x00ff;
    #define TS_ADCMAX12 0x0fff;
  #define TS_ADC_MAX  TS_ADCMAX12
  #define TS_MEAS_PD  0x00   // measure and powerdown PD1/0 = 0/0
  #define TS_MEAS_ADC 0x01   // measure no pd Ref off PD1/0 = 0/1
  #define TS_MEAS_REF 0x02   // measure no pd ADC off PD1/0 = 1/0
  #define TS_MEAS_SYS 0x03   // measure no powerdown  PD1/0 = 1/1
  #define TS_MEAS_RUN 0x80   // measure no powerdown  PD1/0 = 1/1
  #define TS_MAX_DAT  30

  #define TS_ROT0     0
  #define TS_ROT90    1
  #define TS_ROT180   2
  #define TS_ROT270   3

  // calibration data
  #define TS_CAL_POS  20

  // measure in differtial mode - uses bias voltage = ratiometric
  #define TS_MEAS_X   0x50   // measure X  A2/1/0 = 1/0/1, SER-/DFR = 0
  #define TS_MEAS_Y   0x10   // measure Y  A2/1/0 = 0/0/1, SER-/DFR = 0
  #define TS_MEAS_Z1  0x30   // measure Z1 A2/1/0 = 0/1/1, SER-/DFR = 0
  #define TS_MEAS_Z2  0x40   // measure Z2 A2/1/0 = 0/0/1, SER-/DFR = 0
  /*
    // measure in single mode - uses reference voltage
    #define TS_MEAS_T1  0x04   // measure X A2/1/0 = 0/0/0, SER-/DFR = 1
    #define TS_MEAS_T2  0x74   // measure X A2/1/0 = 1/1/1, SER-/DFR = 1
    #define TS_MEAS_BAT 0x24   // measure X A2/1/0 = 0/1/0, SER-/DFR = 1
    #define TS_MEAS_AUX 0x64   // measure X A2/1/0 = 1/1/0, SER-/DFR = 1
    */
  #define TS_MODE_8B  0x08   //  8Bit Mode
  #define TS_MODE_12B 0x00   // 12Bit Mode

  //--- class ts_point
    class ts_point
      {
        public:
        	ts_point(void) : x(0), y(0), z(0) {}
        	ts_point(int16_t x, int16_t y) : x(x), y(y), z(0) {}
        	ts_point(int16_t x, int16_t y, int16_t z) : x(x), y(y), z(z) {}

          bool operator==(ts_point p)
            { return ((p.x == x) && (p.y == y) && (p.z == z)); }
        	bool operator!=(ts_point p)
            { return ((p.x != x) || (p.y != y) || (p.z != z)); }

          int16_t x, y, z;
      };

  //--- ts_calibration
    class ts_calibration
      {
        public:
        	constexpr ts_calibration(void):
                defined(false), screenWidth(0U), screenHeight(0U),
        		    deltaX(0), deltaY(0),
        		    alphaX(0.0F), betaX(0.0F), alphaY(0.0F), betaY(0.0F) {}

          constexpr ts_calibration(float aX, float bX, int32_t dX, float aY, float bY, int32_t dY, uint16_t sW, uint16_t sH):
        		    defined(true), screenWidth(sW), screenHeight(sH),
        		    deltaX(dX), deltaY(dY),
        		    alphaX(aX), betaX(bX), alphaY(aY), betaY(bY) {}

          ts_calibration( const ts_point aS, const ts_point aT,
                      		const ts_point bS, const ts_point bT,
                      		const ts_point cS, const ts_point cT,
                      		const uint16_t sW, const uint16_t sH);

          bool defined;
        	uint16_t screenWidth;
        	uint16_t screenHeight;
        	int32_t  deltaX;
        	int32_t  deltaY;
        	float    alphaX;
          float    betaX;
          float    alphaY;
          float    betaY;
      };

    /*  typedef struct calParams
        {
          uint16_t dx;
          uint16_t dy;
          uint16_t vi1;
          uint16_t vj1;
          uint16_t dvi;
          uint16_t dvj;
        } calParams_t;
      */


  //--- class mdXPT2046
    class mdXPT2046
      {
        public:
          //static const uint16_t CAL_MARGIN = 20;

          //enum rotation_t : uint8_t { ROT0, ROT90, ROT180, ROT270 };
          //enum adc_ref_t : uint8_t { MODE_SER, MODE_DFR };

          mdXPT2046 (uint8_t cs_pin, uint8_t irq_pin, uint8_t spi_bus = VSPI);

          void begin(uint16_t width, uint16_t height, uint8_t rot = TS_ROT0);  // width and height with no rotation!
          void setRotation(uint8_t rot);

          // Calibration needs to be done with no rotation, on both display and touch drivers
          //void getCalibrationPoints(uint16_t &x1, uint16_t &y1, uint16_t &x2, uint16_t &y2);
                //void setCalibration (calParams_t* pcalPars);

        	ts_point getPoint();
        	bool tirqTouched();
        	bool touched();
        	void readData(uint16_t *x, uint16_t *y, uint8_t *z);
        	bool bufferEmpty();
        	uint8_t bufferSize() { return 1; }
        	void calibrate(ts_calibration c) { cal = c; }

          //bool isTouching();

                  //void getRaw(uint16_t &vi, uint16_t &vj, adc_ref_t mode = MODE_DFR, uint8_t max_samples = 0xff);
                  //void getPosition(uint16_t &x, uint16_t &y, adc_ref_t mode = MODE_DFR, uint8_t max_samples = 0xff);
          //void getRaw(uint16_t &vi, uint16_t &vj);
          //void getPosition(uint16_t &x, uint16_t &y);

          void powerDown();

        // protected:
        	volatile bool _isrWake=true;

        private:
        	void update();
          //uint16_t _readLoop(uint8_t ctrl);
          void     startSPI();
          void     stopSPI();

          //static const uint8_t CTRL_LO_DFR = 0b0011;
          //static const uint8_t CTRL_LO_SER = 0b0100;
          //static const uint8_t CTRL_HI_X   = 0b1001  << 4;
          //static const uint8_t CTRL_HI_Y   = 0b1101  << 4;
          //static const uint16_t ADC_MAX    = 0x0fff;  // 12 bits

          mdSPI_class* _pmdSPI  = NULL;
          SPIClass*    _pSPI    = NULL;
          uint8_t      _isON    = OFF;
          uint8_t      _spi_bus = VSPI;
          uint16_t     _width   = 240;
          uint16_t     _height  = 320;
          uint8_t      _rot     = TS_ROT0;
          uint8_t      _cs      = 0;
          uint8_t      _irq     = 255;
          uint8_t      _istouch = OFF;
        	int16_t      _xraw    = 0;
          int16_t      _yraw    = 0;
          int16_t      _zraw    = 0;
          uint32_t msraw=0x80000000;
        	ts_calibration cal;

          //calParams_t  _cal     = {1, 1, 1, 1, 1, 1};
      };

#endif  // _XPT2046_h_
