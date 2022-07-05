// =====================================================
// UIelements.cpp
// User Interface elements (Button, CanvasPrint)
//
// Author: Richard Benear 6/22

#include "Display.h"
#include "UIelements.h"

// =======================================================================
// ======================= Button UI elements ============================
// =======================================================================
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

// =====================================================================
// Draw Button, Center Text, Custom Font
// Draw a single button, assume constructor called to set colors and font size
// Center text in button both x and y
void Button::draw(int x, int y, uint16_t width, uint16_t height, const char* label, bool active) {
  int buttonRadius = BUTTON_RADIUS;
  int len = strlen(label);

  // font width is a value selected for alphabetic characters only, no others
  uint16_t xTextOffset = (width  - b_fontCharWidth*len)/2;

  // the y text offset is not an obvious calculation....explanation follows:
  // the GFX default font uses the lower left corner for the origin
  // for Custom fonts, used here, the upper left is the origin
  // Adafruit_GFX::setFont(const GFXfont *f) does an offset of +/- 6 pixels based on default vs. custom font
  // hence, that is why this calculation looks weird, standards would be nice :-/
  uint16_t yTextOffset = ((height - b_fontCharHeight)/2) + b_fontCharHeight-4;
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

// =====================================================================
// Draw Button, Left Justify Text, Default GFX Font Arial
// Draw a single button, assume constructor called to set colors and font size
// Center y axis only, no centering for x axis
void Button::drawLJ(int x, int y, uint16_t width, uint16_t height, const char* label, bool active) {
  int buttonRadius = BUTTON_RADIUS;
  uint16_t yTextOffset = ((height - b_fontCharHeight)/2) + b_fontCharHeight-6;
  if (active) { // show active background
    tft.fillRoundRect(x, y, width, height, buttonRadius, b_colorActive);
  } else {
    tft.fillRoundRect(x, y, width, height, buttonRadius, b_colorNotActive);
  }
  tft.drawRoundRect(x, y, width, height, buttonRadius, b_colorBorder);
  tft.setCursor(x+2, y+yTextOffset);
  tft.print(label);
}

// =======================================================================
// ================= Canvas print UI elements ============================
// =======================================================================
// Canvas Print constructor
CanvasPrint::CanvasPrint(
      int           x,
      int           y,
      uint16_t      width,
      uint16_t      height,
      uint16_t      colorActive,
      uint16_t      colorNotActive,
      //GFXfont       *font,
      const char*   label)
  {                
    p_x               = x;
    p_y               = y;
    p_width           = width;
    p_height          = height;
    p_colorActive     = colorActive;
    p_colorNotActive  = colorNotActive;
    //p_font            = font;      
    p_label           = label;
  }

// =====================================================================
// Canvas print function using characters
void CanvasPrint::cPrint(int x, int y, uint16_t width, uint16_t height, const char* label, bool warning) {
  char _label[60] = "";
  int y_box_offset = -6; // default font offset
  GFXcanvas1 canvas(width, height);

  canvas.setFont(); // default Arial 6x8 pt font
  canvas.setCursor(0, (height-y_box_offset)/2 + y_box_offset); // offset from top left corner of canvas box
  sprintf(_label, "%9s", label);
  canvas.print(_label);
  if (warning) { // show warning background
    tft.drawBitmap(x, y - y_box_offset, canvas.getBuffer(), width, height, display.textColor, p_colorActive);
  } else {
    tft.drawBitmap(x, y - y_box_offset, canvas.getBuffer(), width, height, display.textColor, p_colorNotActive);
  }
}

  // Overload for double
void CanvasPrint::cPrint(int x, int y, uint16_t width, uint16_t height, double label, bool warning) {
  char charlabel[60];
  sprintf(charlabel, "%5.1f", label);
  cPrint(p_x, p_y, p_width, p_height, charlabel, warning);
 }

