// =====================================================
// UIelements.cpp

#include "Display.h"
#include "UIelements.h"

// Button constructor
Button::Button(uint16_t x, 
          uint16_t      y, 
          uint8_t       width, 
          uint8_t       height,
          uint16_t      colorActive,
          uint16_t      colorNotActive,
          uint16_t      colorBorder,
          uint8_t       fontCharWidth,
          uint8_t       fontCharHeight,
          const char*   label,
          bool          stateMonitor) {           
  b_x = x;
  b_y = y;
  b_width = width;
  b_height = height;
  b_colorActive = colorActive;
  b_colorNotActive = colorNotActive;
  b_colorBorder = colorBorder;
  b_fontCharWidth = fontCharWidth;
  b_fontCharHeight = fontCharHeight;      
  b_label = label;
  b_stateMonitor = stateMonitor;
}

// Draw a single button, both x and y can change
void Button::draw(uint16_t x, uint16_t y, const char* label, bool active, bool stateChange) {
  int buttonRadius = 7;
  int len = strlen(label);

  // font width is the value used for alphabetic charachters only, no others
  uint16_t xTextOffset = (b_width  - b_fontCharWidth*len)/2;

  // the y text offset is not an obvious calculation....
  // the GFX default font uses the lower left corner for the origin
  // for Custom fonts, used here, the upper left is the origin
  // Adafruit_GFX::setFont(const GFXfont *f) does an offset of +/- 6 pixels based on default vs. custom font
  // hence, that is why this calculation looks weird, standards would be good :-/
  uint16_t yTextOffset = ((b_height - b_fontCharHeight)/2) + b_fontCharHeight-6;

  if (!stateChange) { // not waiting for a State change, so draw button and label
    if (active) { // show active background
      tft.fillRoundRect(x, y, b_width, b_height, buttonRadius, b_colorActive);
    } else {
      tft.fillRoundRect(x, y, b_width, b_height, buttonRadius, b_colorNotActive);
    }
    tft.drawRoundRect(x, y, b_width, b_height, buttonRadius, b_colorBorder);
    tft.setCursor(x+xTextOffset, y+yTextOffset);
    tft.print(label);
  } else { // waiting for State change, so only draw the text since button graphics already there
    tft.setCursor(x+xTextOffset, y+yTextOffset);
    tft.print(label);   
  }
}

// Draw a single button overload, no x axis changes involved
void Button::draw(uint16_t y, const char* label, bool active, bool stateChange) {
  int buttonRadius = 7;
  int len = strlen(label);
  uint16_t xTextOffset = abs((b_width  - b_fontCharWidth*len)/2); // no negative values
  uint16_t yTextOffset = abs(((b_height - b_fontCharHeight)/2) + b_fontCharHeight-6);
  if (!stateChange) { // not waiting for a State change, so draw button and label
    if (active) {
      tft.fillRoundRect(b_x, y, b_width, b_height, buttonRadius, b_colorActive);
    } else {
      tft.fillRoundRect(b_x, y, b_width, b_height, buttonRadius, b_colorNotActive);
    }
    tft.drawRoundRect(b_x, y, b_width, b_height, buttonRadius, b_colorBorder);
    tft.setCursor(b_x+xTextOffset, y+yTextOffset);
    tft.print(label);
  } else { // waiting for State change, so only draw the text since button graphics already there
    tft.setCursor(b_x+xTextOffset, y+yTextOffset);
    tft.print(label);   
  }
}


