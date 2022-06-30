// =====================================================
// UIelements.h
// User Interface elements (Button, CanvasPrint)
//
// Author: Richard Benear 6/22

#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H

#include <Arduino.h>

#define BUTTON_RADIUS 7

typedef struct Boundry{
  int x;
  int y;
  uint16_t width;
  uint16_t height;
} Boundry;

//----------------------------------------------------------
// Button element
//----------------------------------------------------------
class Button{
  Boundry       boundry;
  int           b_x;
  int           b_y;
  uint16_t      b_width; 
  uint16_t      b_height;
  uint16_t      colorActive;
  uint16_t      b_colorActive;
  uint16_t      b_colorNotActive;
  uint16_t      b_colorBorder;
  uint8_t       b_fontCharWidth;
  uint8_t       b_fontCharHeight;      
  const char*   b_label;

	public:
    // constructor
    Button(
      int           x,
      int           y,
      uint16_t      width,
      uint16_t      height,
      uint16_t      colorActive,
      uint16_t      colorNotActive,
      uint16_t      colorBorder,
      uint8_t       fontCharWidth,
      uint8_t       fontCharHeight,
      const char*   label);

  
    void draw(int x, int y, uint16_t width, uint16_t height, const char* label, bool active);
    void draw(int x, int y,                                  const char* label, bool active);
    void draw(       int y,                                  const char* label, bool active);

  private:
};

//----------------------------------------------------------
// Canvas Text element
//----------------------------------------------------------
class CanvasPrint{
	public:
    // constructor
    CanvasPrint(
      int           x, 
      int           y, 
      uint16_t      width, 
      uint16_t      height,
      uint16_t      colorActive,
      uint16_t      colorNotActive,
      uint16_t      colorBorder,
      int           fontCharWidth,
      int           fontCharHeight,
      const char*   label);

    void canvPrint(int x, int y, uint16_t width, uint16_t height, const char* label, bool warning);
    void canvPrint(int x, int y,                                  const char* label, bool warning);
    void canvPrint(       int y,                                  const char* label, bool warning);

  private:
};

extern Button button;

#endif