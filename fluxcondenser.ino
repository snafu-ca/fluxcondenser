//+++
//
// Flux Condenser
// snafu.ca 2020 - Freeware - no rights reserved
//
//---

#include <Adafruit_NeoPixel.h>

#define LED_LENGTH     5     // # of LEDs
#define NUM_SPEED      9     // number of distinct speed/color steps
#define LED_PIN        12    // data pin for WS2812 strings
#define METER1_PIN     5     // data pin for meter (needs to be PWM capable,  3,5,6,9,10 or 11)
#define METER2_PIN     6     // data pin for meter (needs to be PWM capable,  3,5,6,9,10 or 11)
#define BASESPEED      400   // slowest chase speed N mS per LED
#define SPEEDBOOST     35    // faster by N mS for each notch of speed
#define SPEEDUPDATE    600   // time between each speed step mS
#define CHANGEUPDATE   23000 // change target speed every N mS
#define METERUPDATE    150   // update meter (w/ randomness) every N mS

unsigned long  now;                  // used to hold current millis
unsigned long  nextSpeedChange = 0;  // when should we change speed next 
unsigned long  nextSpeedStep = 0;    // when should we step speed next (to go from current to target)
unsigned long  nextMeterUpdate = 0;  // when should we update the meter
unsigned long  nextLedUpdate = 0;    // when we should next update the LED string
unsigned short currentSpeed = 0;     // 0..(NUM_SPEED - 1) 
unsigned short targetSpeed = 0;      // speed we're aiming for
unsigned short ledStep = 0;          // 0..(LEDLENGTH - 1)
unsigned short sweep = 0;            // sweepmode toggle
unsigned long  colormap[NUM_SPEED] = {0xFFF0C0, 0xFFF090, 0xFFE060,  0xFFD030, 0xFFC000, 0xFFA000,   0xFF8000, 0xFF4000, 0xFF0000};   // warmish white -> yellow -> red (RGB format) 

Adafruit_NeoPixel pixels(LED_LENGTH, LED_PIN, NEO_KHZ800 + NEO_GRB);  // my string uses the GREEN-RED-BLUE option (which is quite common)


//
// Setup runs once at the beginning of your program
//
void setup() {
  now = millis();
  nextSpeedChange = now + CHANGEUPDATE;  // schedule the next speed change
  nextSpeedStep = now + SPEEDUPDATE;     // schedule the next led step change
  nextMeterUpdate = now + METERUPDATE;   // schedule the next meter updates
  nextLedUpdate = now; // forces a display update right away
  currentSpeed = 0;    // start at slowest speed
  targetSpeed = 0;     // start at slowest speed
  
  // meter inits
  pinMode(METER1_PIN, OUTPUT);
  pinMode(METER2_PIN, OUTPUT);
  analogWrite(METER1_PIN, 0);
  analogWrite(METER2_PIN, 0);

  // WS2812 init
  pixels.begin();
  pixels.clear(); 
}


//
// Loop does just that, continuously run this code
//
void loop() {
  now = millis();    // get the current tick, used in a lot of places to see if it's time to do things
  
  //
  // should we pick a new overall speed ??
  //
  if (now > nextSpeedChange) {
    targetSpeed = (random(2) > 0) ? random(NUM_SPEED) : 0;      // 50% of the time we want the slowest setting
    nextSpeedChange = now + CHANGEUPDATE;  // and schedule next update
  }
  
  //
  // see if we should ramp up/down the speed/colour
  //
  if ((currentSpeed != targetSpeed) && (now > nextSpeedStep) && (ledStep == 0)) {
    if (currentSpeed > targetSpeed) {
      currentSpeed--;
    } else {
      currentSpeed++;
      sweep = 1;  // toggle used in special top-speed mode
    }
    nextSpeedStep = now + SPEEDUPDATE;  // and schedule next update
  }

  //
  // should update the meter ??
  // by adding some randomness the meter looks more organic
  //
  if (now > nextMeterUpdate) {
    analogWrite(METER1_PIN, 200/(NUM_SPEED - 1) * currentSpeed * (random(100,121) / 100.0) + random(10) + 5);     // random from 0 -> 255, but higher with higher 'speed'
    analogWrite(METER2_PIN, 200/(NUM_SPEED - 1) * currentSpeed * (random(100,121) / 100.0) + random(10) + 5);     
    nextMeterUpdate = now + METERUPDATE;  // and schedule next update
  }

  //
  // and should we update the LED string
  //
  if (now > nextLedUpdate) {
    pixels.clear(); 
    pixels.setBrightness (255);
    for(int i=0; i < LED_LENGTH; i++) {      // foreach LED...
      if (currentSpeed == (NUM_SPEED - 1) ) {
        pixels.setPixelColor(i, ((i > ledStep) ^ sweep) ? colormap[currentSpeed] : 0);    // on highest setting use a slightly different pattern. Note ^ is logical XOR
      } else {
        pixels.setPixelColor(i, (i == ledStep) ? colormap[currentSpeed] : 0);
      }
    }
    pixels.show();   // Send the updated pixel colors to the hardware.
    ledStep = (ledStep+1) % LED_LENGTH;
    if (ledStep == 0) {
      sweep = !sweep;
    }
    nextLedUpdate = now + (BASESPEED - currentSpeed * SPEEDBOOST); // and schedule next update
  }
}
