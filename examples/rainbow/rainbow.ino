#include <Ai_RFD_WS2812.h>

#define PIN     2
#define NUM     16
#define ANIMATE true

RFD_WS2812 blinker = RFD_WS2812(NUM, PIN);

void setup() 
{
  // Initialize WS2812 modules with all LEDs turned off
  blinker.initialize();
}

void loop() 
{
  colorWipe(blinker.packRGB(255, 0, 0), 50); // Red
  colorWipe(blinker.packRGB(0, 255, 0), 50); // Green
  colorWipe(blinker.packRGB(0, 0, 255), 50); // Blue
  rainbow(20);
  rainbowCycle(20);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) 
{
  for(uint16_t i=0; i<blinker.getNumPixels(); i++) 
  {
      blinker.setPixel(i, c);
      blinker.render();
      delay(wait);
  }
}

void rainbow(uint8_t wait) 
{
  uint16_t i, j;

  for(j=0; j<256; j++) 
  {
    for(i=0; i<blinker.getNumPixels(); i++) 
    {
      blinker.setPixel(i, Wheel((i+j) & 255));
    }
    blinker.render();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) 
{
  uint16_t i, j;

  for(j=0; j<256*5; j++) 
  { // 5 cycles of all colors on wheel
    for(i=0; i< blinker.getNumPixels(); i++) 
    {
      blinker.setPixel(i, Wheel(((i * 256 / blinker.getNumPixels()) + j) & 255));
    }
    blinker.render();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) 
  {
  if(WheelPos < 85) 
  {
   return blinker.packRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) 
  {
   WheelPos -= 85;
   return blinker.packRGB(255 - WheelPos * 3, 0, WheelPos * 3);
  } else 
  {
   WheelPos -= 170;
   return blinker.packRGB(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
