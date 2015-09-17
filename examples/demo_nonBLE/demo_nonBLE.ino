#include <Ai_RFD_WS2812.h>

#define PIN     2
#define NUM     24
#define ANIMATE true
#define Pixels 24
#define BG 1

RFD_WS2812 blinker = RFD_WS2812(NUM, PIN);

void setup() 
{
  // Initialize WS2812 modules with all LEDs turned off
  blinker.initialize();
  blinker.setBrightness(100);  
}

void loop() 
{

  //rainbow(20);
  //for(int i=0;i<25;i*=5)
  //  colorWipe(blinker.packRGB(200, 0, 255),26-i);
  // ripple();
  //theaterChase(blinker.packRGB(200, 255, 255), 50); // White
  for(int k=0;k<blinker.getNumPixels();k++)
  {
    blinker.setPixel(k, blinker.packRGB(255,0,100));    
    for(int j=0;j<blinker.getNumPixels();j++)
    {
      if( j!=k )
        blinker.setPixel(j, blinker.packRGB(120,255,0));
        
      if(j>0)
      {
        if( (j-1)!=k )
          blinker.setPixel(j-1, blinker.packRGB(0,0,0));
      }
      else
        blinker.setPixel(blinker.getNumPixels()-1, blinker.packRGB(0,0,0));          
        
        
      for(int i=0;i<blinker.getNumPixels();i++)
      {
        if( ( i!=j ) && ( i!=k ) )
          blinker.setPixel(i, blinker.packRGB(0,100,255));
        if(i>0)
        {
          if( ( (i-1)!=j )&&(i-1)!=k )
            blinker.setPixel(i-1, blinker.packRGB(0,0,0));
        }
        else
          blinker.setPixel(blinker.getNumPixels()-1, blinker.packRGB(0,0,0));          
        blinker.render();
        delay(15);
      }
    }
  } 
}

void spin(uint32_t c)
{
  
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

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < blinker.getNumPixels(); i=i+3) {
        blinker.setPixel(i+q, c);    //turn every third pixel on
      }
      blinker.render();
     
      delay(wait);
     
      for (int i=0; i < blinker.getNumPixels(); i=i+3) {
        blinker.setPixel(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

int color;
int center = 0;
int step = -1;
int maxSteps = 16;
float fadeRate = 0.6;
int diff;

//background color
uint32_t currentBg = random(256);
uint32_t nextBg = currentBg;

void ripple() {
  if (BG){
    if (currentBg == nextBg) {
      nextBg = random(256);
    } 
    else if (nextBg > currentBg) {
      currentBg++;
    } else {
      currentBg--;
    }
    for(uint16_t l = 0; l < Pixels; l++) {
      blinker.setPixel(l, WheelR(currentBg, 0.1));
    }
  } else {
    for(uint16_t l = 0; l < Pixels; l++) {
      blinker.setPixel(l, 0, 0, 0);
    }
  }

  if (step == -1) {
    center = random(Pixels);
    color = random(256);
    step = 0;
  }

  if (step == 0) {
    blinker.setPixel(center, WheelR(color, 1));
    step ++;
  } 
  else {
    if (step < maxSteps) {
      blinker.setPixel(wrap(center + step), WheelR(color, pow(fadeRate, step)));
      blinker.setPixel(wrap(center - step), WheelR(color, pow(fadeRate, step)));
      if (step > 3) {
        blinker.setPixel(wrap(center + step - 3), WheelR(color, pow(fadeRate, step - 2)));
        blinker.setPixel(wrap(center - step + 3), WheelR(color, pow(fadeRate, step - 2)));
      }
      step ++;
    } 
    else {
      step = -1;
    }
  }
  
  blinker.render();
  delay(50);
}

int wrap(int step) {
  if(step < 0) return Pixels + step;
  if(step > Pixels - 1) return step - Pixels;
  return step;
}



// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t WheelR(byte WheelPos, float opacity) {
  
  if(WheelPos < 85) {
    return blinker.packRGB((WheelPos * 3) * opacity, (255 - WheelPos * 3) * opacity, 0);
  } 
  else if(WheelPos < 170) {
    WheelPos -= 85;
    return blinker.packRGB((255 - WheelPos * 3) * opacity, 0, (WheelPos * 3) * opacity);
  } 
  else {
    WheelPos -= 170;
    return blinker.packRGB(0, (WheelPos * 3) * opacity, (255 - WheelPos * 3) * opacity);
  }
}
