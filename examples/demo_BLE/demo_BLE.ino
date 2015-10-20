#include <RFduinoBLE.h>
#include <Ai_RFD_WS2812.h>

#define PIN     2
#define NUM_LED 1

RFD_WS2812 blinker = RFD_WS2812(NUM_LED, PIN);

void setup() {
  // this is the data we want to appear in the advertisement
  // (if the deviceName and advertisementData are too long to fix into the 31 byte
  // ble advertisement packet, then the advertisementData is truncated first down to
  // a single byte, then it will truncate the deviceName)
  RFduinoBLE.advertisementData = "rgb";
  
  // start the BLE stack
  RFduinoBLE.begin();

  // Initialize WS2812 modules with all LEDs turned off
  blinker.initialize();

  // Because of the high brightness of the LEDs, we might want to reduce it.
  // The range is between 0 to 255.
  // blinker.setBrightness(100); 

  // For debugging purposes use the Serial object.
  Serial.begin(9600);
}

void loop() {
  // Switch to lower power mode
  RFduino_ULPDelay(INFINITE);
}

void RFduinoBLE_onConnect() {
  // Debug message printed to Serial interface
  Serial.println("RFduino connected");
}

void RFduinoBLE_onDisconnect() {
  // Debug message printed to Serial interface
  Serial.println("RFduino disconnected");
}

void RFduinoBLE_onReceive(char *data, int len) {
  // Debug message printed to Serial interface
  Serial.print("Data received: ");
  Serial.println(data);
  // Each transmission should contain an RGB triple
  if (len >= 3)
  {
    // get the RGB values
    uint8_t r = data[0];
    uint8_t g = data[1];
    uint8_t b = data[2];
    colorWipe(blinker.packRGB(r,g,b),25);
  }
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
