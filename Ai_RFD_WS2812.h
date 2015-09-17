#ifndef AI_RFD_WS2812_H
#define AI_RFD_WS2812_H
   
#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

#define port_set ((uint32_t*)&NRF_GPIO->OUTSET)
#define port_clr ((uint32_t*)&NRF_GPIO->OUTCLR)

enum { RAINBOW };

class RFD_WS2812
{
  public:
    RFD_WS2812(uint16_t l,uint8_t p);
    ~RFD_WS2812();
    void initialize();
    void setPixel(uint16_t idx, uint32_t c);
    void setPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b);
    void setPixels(uint8_t r, uint8_t g, uint8_t b);
    void setNumPixels(uint16_t n);
    void setBrightness(uint8_t val);
    void clear();
    void render();
    uint32_t packRGB(uint8_t r, uint8_t g, uint8_t b); 
    uint16_t getNumPixels();
  
  private:
    bool isPaused;
    const uint8_t nbits;
    uint16_t
      length,
      nbytes;
    
    uint8_t 
      *rgb_arr,
      brightness,
      pin;

    uint32_t t_f;
};

#endif
