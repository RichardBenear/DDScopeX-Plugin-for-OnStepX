// =====================================================
// Adafruit_ILI9486_Teensy.cpp
//
//See rights and use declaration in License.h
//
//based on Adafruit ili9341 library @ Dec 2016
//modified for the Maple Mini (STM32) by Steve Strong 2017 
// -- https://github.com/stevstrong/Adafruit_ILI9486_STM32
//
//Note: SPI pins are set in Library .h file, rather than using the constructor
//
//modified for Teensy 3.1 by Richard Palmer 2017
//DMA transfers have been crippled by RP 
//
// **** Modified by Richard Benear 3/21/2025 ****
// Cleaned up some of the extraneous comments and added a faster drawRect().
// Added ability to capture the pixels going to each screen of the TFT into
// a RAM buffer and then store each screen file on the SD Flash. The
// SD flash can then be read by a python script "TftCapture/display_sd_image.py" 
// which converts the color format to viewable .png files. These screen images
// are much more readable and representative of what is actually seen on the TFT. 
// This was done primarily for documentation because the ones that were generated by
// taking photos of the handheld TFT display had too much backlight glow.

#include "Adafruit_ILI9486_Teensy.h"
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <SD.h>
#include "../display/Display.h"

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 480
#define COLOR_DEPTH   2  // 2 bytes per pixel (RGB565)
#define BUFFER_SIZE   (SCREEN_WIDTH * SCREEN_HEIGHT * COLOR_DEPTH)

DMAMEM static uint8_t tftBuffer[BUFFER_SIZE];  // Use Teensy RAM2
static int windowX0, windowY0, windowX1, windowY1;

uint8_t useDMA;
uint16_t lineBuffer[1];
GFXfont *gfxFont;     ///< Pointer to special font

/*****************************************************************************/
// Constructor uses hardware SPI, the pins being specific to each device
/*****************************************************************************/
Adafruit_ILI9486_Teensy::Adafruit_ILI9486_Teensy(void) : Adafruit_GFX(TFTWIDTH, TFTHEIGHT){}
/*****************************************************************************/

// ******* Functions to capture a screen to SD *******
void Adafruit_ILI9486_Teensy::enableLogging(bool enable) {
	if (enable == 1) {
		Serial.println("Logging=true"); 
	} else { 
		Serial.println("Logging=false");
	}
    loggingEnabled = enable;
}

// ==================== Set Address Window (Modified) ====================
void Adafruit_ILI9486_Teensy::captureSetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    windowX0 = x0;
    windowY0 = y0;
    windowX1 = x1;
    windowY1 = y1;
}

void Adafruit_ILI9486_Teensy::bufferedTransfer(uint16_t color) {
    if (!loggingEnabled) return;  // Skip capture if logging is disabled

    static uint16_t lastColor = 0xFFFF;  // Store last color written
    static int lastX = -1, lastY = -1;   // Store last position written

    if (lastColor == color && lastX == windowX0 && lastY == windowY0) {
        // The same pixel data is being written, ignore to prevent duplication
        return;
    }

    // Update last known color and position
    lastColor = color;
    lastX = windowX0;
    lastY = windowY0;

    if (windowX0 <= windowX1 && windowY0 <= windowY1) {
        for (int y = windowY0; y <= windowY1; y++) {
            for (int x = windowX0; x <= windowX1; x++) {
                if (x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
                    int index = (y * SCREEN_WIDTH + x) * COLOR_DEPTH;
                    tftBuffer[index] = color >> 8;     // High byte
                    tftBuffer[index + 1] = color & 0xFF; // Low byte
                }
            }
        }
    }
}

// ==================== Save Buffer to SD Card ====================
void Adafruit_ILI9486_Teensy::saveBufferToSD(const char* screenName) {
    // Build the file name based on the screen name
    char fileName[64];
    snprintf(fileName, sizeof(fileName), "/tft_%s_log.bin", screenName);

	 // Remove the file if it already exists
	 if (SD.exists(fileName)) {
        SD.remove(fileName);   // Delete the file to ensure it's completely erased
    }

    // Open the file for writing (overwrite existing file)
    File logFile = SD.open(fileName, (uint8_t)(O_WRITE | O_CREAT)); 
    if (logFile) {
        logFile.write(tftBuffer, BUFFER_SIZE);
		Serial.print("File size: ");
    	Serial.println(logFile.size());
        logFile.close();
        Serial.print("Buffer saved to SD card as: ");
        Serial.println(fileName);
    } else {
        Serial.println("Failed to open file for writing.");
    }
	// Clear the buffer (Overwrite with zeros)
    memset(tftBuffer, 0, BUFFER_SIZE);
}

//*********************** End Capture Functions ******************************/

/*****************************************************************************/
void Adafruit_ILI9486_Teensy::writedata16(uint16_t c)
{
	CD_DATA;
	CS_ACTIVE;
#ifdef ENABLE_TFT_CAPTURE
    // Capture this single pixel into the buffer
    if (loggingEnabled) bufferedTransfer(c);  // Capture only if logging is enabled
#endif
    SPI.transfer(c>>8);
    SPI.transfer(c&0xFF);
	CS_IDLE;
}

void Adafruit_ILI9486_Teensy::writedata16(uint16_t c, uint32_t num)
{ 
    int counter = 0;
    CD_DATA;
    CS_ACTIVE;
    useDMA = 0;

#ifdef ENABLE_TFT_CAPTURE
    if (loggingEnabled) {
        uint32_t pixelCount = 0;
        int x = windowX0;
        int y = windowY0;

        while (pixelCount < num) 
        {
            if (x >= SCREEN_WIDTH) {
                x = 0;
                y++;
            }
            if (y >= SCREEN_HEIGHT) break;  // Prevent writing outside the buffer

            int index = (y * SCREEN_WIDTH + x) * COLOR_DEPTH;
            tftBuffer[index] = c >> 8;
            tftBuffer[index + 1] = c & 0xFF;

            pixelCount++;
            x++;  // Move to the next pixel horizontally

            if (x > windowX1) {  // Move to the next line if beyond window boundary
                x = windowX0;
                y++;
            }
        }
    }
#endif

    while (num-- > 0) 
    {
        SPI.transfer16(c);
        if (counter++ > SPIBLOCKMAX) {
            SPI.endTransaction();
            SPI.beginTransaction(SPISET);
            counter = 0;
        }
    }

    CS_IDLE;
}

/*****************************************************************************/
void Adafruit_ILI9486_Teensy::writecommand(uint8_t c)
{
	CD_COMMAND;
	CS_ACTIVE;
    SPI.transfer(c);
	CS_IDLE;
}

/*****************************************************************************/
void Adafruit_ILI9486_Teensy::writedata(uint8_t c)
{
	CD_DATA;
	CS_ACTIVE;
    SPI.transfer(c);
	CS_IDLE;
}

/*****************************************************************************/
// https://github.com/adafruit/adafruit-rpi-fbtft/blob/35890c52f9e3eef3237b76acc295585dd93fc8cd/fb_ili9486.c
#define DELAY 0x80
/*****************************************************************************/
uint8_t ili9486_init_sequence[] =
{
  //	2, 0xb0, 0x0,	// Interface Mode Control
  //	1, 0x11,		// Sleep OUT
  //	DELAY, 150,
	2, 0x3A, 0x55,	// use 16 bits per pixel color
	2, 0x36, 0x48,	// MX, BGR == rotation 0
  //	2, 0xC2, 0x44,	// Power Control 3
	//  VCOM Control 1
  //	5, 0xC5, 0x00, 0x00, 0x00, 0x00,
	//  PGAMCTRL(Positive Gamma Control)
	16, 0xE0, 0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98,
	          0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00,
	//  NGAMCTRL(Negative Gamma Control)
	16, 0xE1, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
	          0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00,
	//  Digital Gamma Control 1
	16, 0xE2, 0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
	          0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00,
	1, 0x11,	// Sleep OUT
	DELAY, 150, 	// wait some time
	1, 0x29,	// Display ON
	0			// end marker
};

/*****************************************************************************/
// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
/*****************************************************************************/
void Adafruit_ILI9486_Teensy::commandList(uint8_t *addr)
{
	uint8_t  numBytes, tmp;

	while ( (numBytes=(*addr++))>0 ) { // end marker == 0
		if ( numBytes&DELAY ) {
			//Serial.print("delay ");
			tmp = *addr++;
			//Serial.println(tmp);
			delay(tmp); // up to 255 millis
		} else {
			//Serial.print(numBytes); Serial.print("byte(s): ");
			tmp = *addr++;
			//Serial.write('<'); Serial.print(tmp, HEX); Serial.write('>');
			writecommand(tmp); // first byte is command
			//Serial.print("writecommand");
			while (--numBytes) { //   For each argument...
				tmp = *addr++;
				//Serial.print(tmp, HEX); Serial.write('.');
				writedata(tmp); // all consecutive bytes are data
			}
			//Serial.write('\n');
		}
	}
}

/*****************************************************************************/
void Adafruit_ILI9486_Teensy::begin(void)
{
	useDMA = 0; // disable DMA
	pinMode(TFT_RS, OUTPUT);
	CD_DATA;
	pinMode(TFT_CS, OUTPUT);
	CS_IDLE;

	// toggle RST low to reset
	if (TFT_RST > 0) {
		//Serial.println("resetting display...");
		pinMode(TFT_RST, OUTPUT);
		digitalWrite(TFT_RST, HIGH);
		delay(2);
		digitalWrite(TFT_RST, LOW);
		delay(2);
		digitalWrite(TFT_RST, HIGH);
		delay(20);	
	}
	Serial.println(F("MSG: Reset tft"));

  	SPI.beginTransaction(SPISET); //SPISettings(36000000,MSBFIRST,MODE0))

	// init registers
	commandList(ili9486_init_sequence);
	SPI.endTransaction();
}

/*****************************************************************************/
void Adafruit_ILI9486_Teensy::setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	SPI.beginTransaction(SPISET);
#ifdef ENABLE_TFT_CAPTURE
    captureSetAddrWindow(x0, y0, x1, y1);  // Store window area for capture
#endif
	writecommand(ILI9486_CASET); // Column addr set
	writedata(x0 >> 8);
	writedata(x0 & 0xFF);     // XSTART
	writedata(x1 >> 8);
	writedata(x1 & 0xFF);     // XEND

	writecommand(ILI9486_PASET); // Row addr set
	writedata(y0 >> 8);
	writedata(y0);     // YSTART
	writedata(y1 >> 8);
	writedata(y1);     // YEND

	writecommand(ILI9486_RAMWR); // write to RAM
	SPI.endTransaction();
}

/*****************************************************************************/
void Adafruit_ILI9486_Teensy::drawPixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

	SPI.beginTransaction(SPISET);  // Only one transaction for the entire operation
    setAddrWindow(x, y, x, y);     // Set window for a single pixel
    writedata16(color);            // Log and write pixel data via bufferedTransfer()
    SPI.endTransaction();
}

/*****************************************************************************/
void Adafruit_ILI9486_Teensy::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	// Rudimentary clipping
	if ((x >= _width) || (y >= _height || h < 1)) return;
	if ((y + h - 1) >= _height)	{ h = _height - y; }
	if (h < 2 ) { drawPixel(x, y, color); return; }

	setAddrWindow(x, y, x, y + h - 1);
	SPI.beginTransaction(SPISET);
	writedata16(color, h);
	SPI.endTransaction();
}

/*****************************************************************************/
void Adafruit_ILI9486_Teensy::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	// Rudimentary clipping
	if ((x >= _width) || (y >= _height || w < 1)) return;
	if ((x + w - 1) >= _width) { w = _width - x; }
	if (w < 2 ) { drawPixel(x, y, color); return; }

	setAddrWindow(x, y, x + w - 1, y);
	SPI.beginTransaction(SPISET);
	writedata16(color, w);
	SPI.endTransaction();
}

/*****************************************************************************/
void Adafruit_ILI9486_Teensy::fillScreen(uint16_t color)
{	
	setAddrWindow(0, 0,  _width, _height);
	SPI.beginTransaction(SPISET);
	writedata16(color, (_width*_height));
	SPI.endTransaction();
}

// fill a rectangle
void Adafruit_ILI9486_Teensy::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _width) || (y >= _height)) return;
	if(x < 0) {	w += x; x = 0; 	}
	if(y < 0) {	h += y; y = 0; 	}
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;
	if (w == 1 && h == 1) {
		drawPixel(x, y, color);
		return;
	}
	
	setAddrWindow(x, y, x+w-1, y+h-1);
	SPI.beginTransaction(SPISET);
	writecommand(ILI9486_RAMWR);
	writedata16(color, (w*h));
	SPI.endTransaction();
}

/*
* Draw lines faster by calculating straight sections and drawing them with fastVline and fastHline.
*/
/*****************************************************************************/
void Adafruit_ILI9486_Teensy::drawLine(int16_t x0, int16_t y0,int16_t x1, int16_t y1, uint16_t color)
{
	if ((y0 < 0 && y1 <0) || (y0 > _height && y1 > _height)) return;
	if ((x0 < 0 && x1 <0) || (x0 > _width && x1 > _width)) return;
	if (x0 < 0) x0 = 0;
	if (x1 < 0) x1 = 0;
	if (y0 < 0) y0 = 0;
	if (y1 < 0) y1 = 0;

	if (y0 == y1) {
		if (x1 > x0) {
			drawFastHLine(x0, y0, x1 - x0 + 1, color);
		}
		else if (x1 < x0) {
			drawFastHLine(x1, y0, x0 - x1 + 1, color);
		}
		else {
			drawPixel(x0, y0, color);
		}
		return;
	}
	else if (x0 == x1) {
		if (y1 > y0) {
			drawFastVLine(x0, y0, y1 - y0 + 1, color);
		}
		else {
			drawFastVLine(x0, y1, y0 - y1 + 1, color);
		}
		return;
	}

	bool steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap16(x0, y0);
		swap16(x1, y1);
	}
	if (x0 > x1) {
		swap16(x0, x1);
		swap16(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	}
	else {
		ystep = -1;
	}

	int16_t xbegin = x0;
	if (steep) {
		for (; x0 <= x1; x0++) {
			err -= dy;
			if (err < 0) {
				int16_t len = x0 - xbegin;
				if (len) {
					drawFastVLine (y0, xbegin, len + 1, color);
					//writeVLine_cont_noCS_noFill(y0, xbegin, len + 1);
				}
				else {
					drawPixel(y0, x0, color);
					//writePixel_cont_noCS(y0, x0, color);
				}
				xbegin = x0 + 1;
				y0 += ystep;
				err += dx;
			}
		}
		if (x0 > xbegin + 1) {
			//writeVLine_cont_noCS_noFill(y0, xbegin, x0 - xbegin);
			drawFastVLine(y0, xbegin, x0 - xbegin, color);
		}

	}
	else {
		for (; x0 <= x1; x0++) {
			err -= dy;
			if (err < 0) {
				int16_t len = x0 - xbegin;
				if (len) {
					drawFastHLine(xbegin, y0, len + 1, color);
					//writeHLine_cont_noCS_noFill(xbegin, y0, len + 1);
				}
				else {
					drawPixel(x0, y0, color);
					//writePixel_cont_noCS(x0, y0, color);
				}
				xbegin = x0 + 1;
				y0 += ystep;
				err += dx;
			}
		}
		if (x0 > xbegin + 1) {
			//writeHLine_cont_noCS_noFill(xbegin, y0, x0 - xbegin);
			drawFastHLine(xbegin, y0, x0 - xbegin, color);
		}
	}
}

/*****************************************************************************/
// Pass 8-bit (each) R,G,B, get back 16-bit packed color
/*****************************************************************************/
uint16_t Adafruit_ILI9486_Teensy::color565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/*****************************************************************************/
void Adafruit_ILI9486_Teensy::setRotation(uint8_t m)
{
	writecommand(ILI9486_MADCTL);
	rotation = m & 3; // can't be higher than 3
	switch (rotation) {
		case 0:
			writedata(MADCTL_MX |MADCTL_BGR);
			_width  = TFTWIDTH;
			_height = TFTHEIGHT;
			break;
		case 1:
			writedata(MADCTL_MV | MADCTL_BGR);
			_width  = TFTHEIGHT;
			_height = TFTWIDTH;
			break;
		case 2:
			writedata(MADCTL_MY | MADCTL_BGR);
			_width  = TFTWIDTH;
			_height = TFTHEIGHT;
			break;
		case 3:
			writedata(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
			_width  = TFTHEIGHT;
			_height = TFTWIDTH;
			break;
	}
}

/*****************************************************************************/
void Adafruit_ILI9486_Teensy::invertDisplay(boolean i)
{
	SPI.beginTransaction(SPISET);
	writecommand(i ? ILI9486_INVON : ILI9486_INVOFF);
	SPI.endTransaction();
}
	
