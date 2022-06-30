// =====================================================
// UIelements.cpp
// User Interface elements (Button, CanvasPrint)
//
// Author: Richard Benear 6/22

#include "Display.h"
#include "UIelements.h"

// Button constructor
Button::Button(
      int           x,
      int           y,
      uint16_t      width,
      uint16_t      height,
      uint16_t      colorActive,
      uint16_t      colorNotActive,
      uint16_t      colorBorder,
      uint8_t       fontCharWidth,
      uint8_t       fontCharHeight,
      const char*   label)
  {                
    b_x               = x;
    b_y               = y;
    b_width           = width;
    b_height          = height;
    b_colorActive     = colorActive;
    b_colorNotActive  = colorNotActive;
    b_colorBorder     = colorBorder;
    b_fontCharWidth   = fontCharWidth;
    b_fontCharHeight  = fontCharHeight;      
    b_label           = label;
  }

// Draw a single button, assume constructor called to set colors and font size
void Button::draw(int x, int y, uint16_t width, uint16_t height, const char* label, bool active) {
  int buttonRadius = BUTTON_RADIUS;
  int len = strlen(label);

  // font width is a value used for alphabetic characters only, no others
  uint16_t xTextOffset = (width  - b_fontCharWidth*len)/2;

  // the y text offset is not an obvious calculation....explanation follows:
  // the GFX default font uses the lower left corner for the origin
  // for Custom fonts, used here, the upper left is the origin
  // Adafruit_GFX::setFont(const GFXfont *f) does an offset of +/- 6 pixels based on default vs. custom font
  // hence, that is why this calculation looks weird, standards would be nice :-/
  uint16_t yTextOffset = ((height - b_fontCharHeight)/2) + b_fontCharHeight-6;
  if (active) { // show active background
    tft.fillRoundRect(x, y, width, height, buttonRadius, b_colorActive);
  } else {
    tft.fillRoundRect(x, y, width, height, buttonRadius, b_colorNotActive);
  }
  tft.drawRoundRect(x, y, width, height, buttonRadius, b_colorBorder);
  tft.setCursor(x+xTextOffset, y+yTextOffset);
  tft.print(label);
}

// Draw a single button, overload, no font changes from constructor
void Button::draw(int x, int y, const char* label, bool active) {
  draw(x, y, b_width, b_height, label, active);
}

// Draw a single button, overload, no x axis changes involved
void Button::draw(int y, const char* label, bool active) {
  draw(b_x, y, b_width, b_height, label, active);
}
