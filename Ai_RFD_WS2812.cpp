#include <Ai_RFD_WS2812.h>

RFD_WS2812::RFD_WS2812
( uint16_t l, 
  uint8_t p
) : length(l),
  nbytes(l * 3),
  nbits(8),
  pin(p),
  brightness(0),
  rgb_arr(NULL)
{
  if((rgb_arr = (uint8_t *)malloc(nbytes))) {
    memset(rgb_arr, 0, nbytes);
  }
}

RFD_WS2812::~RFD_WS2812()
{
  if(rgb_arr) free(rgb_arr);
  pinMode(pin, INPUT);
}

void RFD_WS2812::initialize()
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  setPixels(0,0,0);
  render();
}

void RFD_WS2812::setPixel(uint16_t idx, uint8_t r, uint8_t g, uint8_t b)
{
  if(idx < length)
  {
    if(brightness)
    {
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    uint8_t *p = &rgb_arr[idx*3];
    *p++ = g;
    *p++ = r;
    *p = b;
  }
}

// Set pixel color from 'packed' 32-bit RGB color:
void RFD_WS2812::setPixel(uint16_t idx, uint32_t c) {
  if(idx < length) {
    uint8_t
      r = (uint8_t)(c >> 16),
      g = (uint8_t)(c >>  8),
      b = (uint8_t)c;
    if(brightness)
    {
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
    }
    uint8_t *p = &rgb_arr[idx* 3];
    *p++ = g;
    *p++ = r;
    *p = b;
  }
}

void RFD_WS2812::setPixels(uint8_t r, uint8_t g, uint8_t b)
{
  for(uint16_t idx=0; idx < length; idx++)
  {
    setPixel(idx, r, g, b);
  }
}

// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t RFD_WS2812::packRGB(uint8_t r, uint8_t g, uint8_t b) 
{
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

uint16_t RFD_WS2812::getNumPixels(void) {
  return length;
}

void RFD_WS2812::setNumPixels(uint16_t n) 
{
  length = n;
  nbytes = n*3;
  initialize();
}

void RFD_WS2812::clear()
{
  for(uint16_t idx=0; idx < length; idx++)
  {
    setPixel(idx, 0, 0, 0);
  }
  render();
}

void RFD_WS2812::setBrightness(uint8_t val)
{
  // Stored brightness value is different than what's passed.
  // This simplifies the actual scaling math later, allowing a fast
  // 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
  // adding 1 here may (intentionally) roll over...so 
  // 0 = max brightness              (255+1)
  // 1 = min brightness              (  0+1), 
  // 255 = just below max brightness (254+1).
  uint8_t newBrightness = val + 1;
  if(newBrightness != brightness) 
  { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM
    uint8_t  c,
            *ptr           = rgb_arr,
             oldBrightness = brightness - 1; // De-wrap old brightness value
    uint16_t scale;
    if(oldBrightness == 0) scale = 0; // Avoid /0
    else if(val == 255) scale = 65535 / oldBrightness;
    else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
    for(uint16_t i=0; i < nbytes; i++) {
      c      = *ptr;
      *ptr++ = (c * scale) >> 8;
    }
    brightness = newBrightness;
  }
}

void RFD_WS2812::render()
{
  if(!rgb_arr) return;

  while((micros() - t_f) < 50L);  // wait for 50us (data latch)

  uint32_t maskhi = (1<<pin);
  uint32_t masklo = (1<<pin);
  volatile uint32_t *set = port_set;
  volatile uint32_t *clr = port_clr;

  volatile uint8_t
   *p      =  rgb_arr,   // Copy the start address of our data array
    val    = *p++,       // Get the current byte value & point to next byte
    tmp    =  masklo;    // Swap variable to adjust duty cycle 

  uint16_t 
    n = nbytes,
    b = nbits;
    

  noInterrupts();
  // The volatile attribute is used to tell the compiler not to optimize 
  // this section.  We want every instruction to be left as is.
  asm volatile(
  // RFduino clock rate: 16MHz => 1 clock cycle takes 62.5ns
  //
  // The timing of the WS2812B signal is described here:
  // http://acrobotic.com/datasheets/WS2812B.pdf
  //
  // To generate a value of 1, we need to hold the signal HIGH (maximum)
  // for 0.9us, and then LOW (minimum) for 0.35us.  Since our timing has a
  // resolution of 0.0625us we can only approximate these values. Luckily, 
  // the WS2812B modules were designed to accept a +/- 150ns variance in the 
  // duration of the signal.  Thus, if we hold the signal HIGH for 13 
  // cycles (0.8125us), and LOW for 8 cycles (0.500us), we stay within 
  // the tolerated range.
  //
  // To generate a value of 0, we need to hold the signal HIGH (maximum)
  // for 0.35us, and then LOW (minimum) for 0.85us.  Thus, holding the
  // signal HIGH for 8 cycles (0.500us), and LOW for 13 cycles (0.8125us)
  // stays within the tolerated range.
  //
  // By choosing 1 clock cycle as our time unit we can keep track of 
  // the signal's phase (T) after each instruction is executed.
  //
  // For a full description of each assembly instruction consult the ARM
  // manual here: 
  // infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0432c/CHDCICDF.html
  //
    // Instruction                    CLK     Description                 Phase
    "lsl %[val],#24\n\t"              // 1    << val by 24               

   "nextbit:\n\t"                     // -    label                      
    "str %[maskhi],[%[set]]\n\t"      // 2    signal HIGH                (T =  2) 
    "sub %[nbits],#1\n\t"             // 1    advance bit counter        (T =  3)
    "beq nextbyte\n\t"                // 1,3  if bytecnt !=0->nextbyte   (T =  4)
    "lsl %[val],#1\n\t"               // 1    << val by 1                (T =  5)
    "nop\n\t"                         // 1    keep signal HI             (T =  6)
    "nop\n\t"                         // 1    keep signal HI             (T =  7)
    "bcs hinner\n\t"                  // 1,3  if MSB set                 (T =  ?) 
    "str %[masklo],[%[clr]]\n\t"      // 2    signal LOW                 (T = 10) 

   "hinner:\n\t"                      // -    label 
    "nop\n\t"                         // 1    keep signal LO             (T = 11)
    "nop\n\t"                         // 1    keep signal LO             (T = 12)
    "nop\n\t"                         // 1    keep signal LO             (T = 13)
    "str %[masklo],[%[clr]]\n\t"      // 2    signal LOW                 (T = 15) 
    "nop\n\t"                         // 1    keep signal LO             (T = 16)
    "nop\n\t"                         // 1    keep signal LO             (T = 17)
    "nop\n\t"                         // 1    keep signal LO             (T = 18)
    "b nextbit\n\t"                   // 3    go to next bit             (T = 21)

   "nextbyte:\n\t"                    // -    label 
    "lsl %[val],#1\n\t"               // 1    << val by 1                (T =  7)
    "bcs houter\n\t"                  // 1,3  if MSB set                 (T =  ?) 
    "str %[masklo],[%[clr]]\n\t"      // 2    signal LOW                 (T = 10) 

   "houter:\n\t"                      // -    label
    "ldrb %[val],[%[ptr]]\n\t"        // 2    val = *p                   (T = 12)
    "mov %[nbits],#8\n\t"             // 1    reset bit counter          (T = 13)
    "str %[masklo],[%[clr]]\n\t"      // 2    signal LOW                 (T = 15) 
    "lsl %[val],#24\n\t"              // 1    << val by 24               (T = 16)       
    "add %[ptr],%[ptr],#1\n\t"        // 1    p++                        (T = 17)
    "sub %[nbytes],#1\n\t"            // 1    advance byte counter       (T = 18)
    "bne nextbit\n\t"                 // 1,3  if bytecnt!=0->nextbit     (T =  ?)
    "nop\n\t"                         // 1    keep signal LO             (T = 20)
    "nop\n\t"                         // 1    keep signal LO             (T = 21)
    // Output operands
  : [set]    "+r" (set), 
    [clr]    "+r" (clr),
    [val]    "+r" (val),
    [nbits]  "+r" (b), 
    [nbytes] "+r" (n),
    [ptr]    "+r" (p)
    // Input operands
  : [masklo] "r"  (masklo), 
    [maskhi] "r"  (maskhi)
  );
  interrupts();
  t_f = micros();                 // t_f will be used to measure the 50us 
                                  // latching period in the next call of the 
                                  // function.
}
