// =====================================================
// UIelements.h

#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H

#include <Arduino.h>

//----------------------------------------------------------
// Class for the boundry dimensions of a UI element
//----------------------------------------------------------
class Boundry{
	public : 
		uint16_t   x      = 0;
    uint16_t   y      = 0;
    uint8_t    width  = 0;
    uint8_t    height = 0;

  private:
};

//----------------------------------------------------------
// Button element
//----------------------------------------------------------
class Button{
  uint16_t b_x;
  uint16_t b_y;
  uint8_t b_width;
  uint8_t b_height;
  uint16_t b_colorActive;
  uint16_t b_colorNotActive;
  uint16_t b_colorBorder;
  uint8_t b_fontCharWidth;
  uint8_t b_fontCharHeight;      
  const char* b_label;
  bool b_stateMonitor;

	public:
    // constructor
    Button(uint16_t     x, 
          uint16_t      y, 
          uint8_t       width, 
          uint8_t       height,
          uint16_t      colorActive,
          uint16_t      colorNotActive,
          uint16_t      colorBorder,
          uint8_t       fontCharWidth,
          uint8_t       fontCharHeight,
          const char*   label,
          bool          stateMonitor);

    void draw(uint16_t x, uint16_t y, const char* label, bool active, bool stateUpdate);
    void draw(uint16_t y, const char* label, bool active, bool stateChange);

  private:
};

//----------------------------------------------------------
// Canvas Text element
//----------------------------------------------------------
class CanvasPrint{
	public:
    CanvasPrint(uint16_t x, uint16_t y, uint8_t width, uint8_t height);
    Boundry boundry;
    uint16_t      colorActive    = 0xF800;
    uint16_t      colorNotActive = 0x0000;
    uint16_t      colorBorder    = 0xFFE0;
    int           fontCharWidth  = 8;
    int           fontCharHeight = 16;
    const char*   label          = "";
    void cPrint();


  private:
};


#endif