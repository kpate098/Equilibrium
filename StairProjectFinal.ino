// Import third part libraries

#include <Adafruit_NeoPixel.h>
#include <timer.h>    // from https://github.com/brunocalou/Timer

// Variables required for
// the NeoPixel Strip

#define STRIP_PIN  3               // The pin to which the NeoPixel strip is connected
#define NUM_PIXELS 900             // The number of NeoPixels in the strip

const uint16_t zones[14][2] = {    // The different segments the strip is divided into
  //Zone  {startPixel, endPixel}
  /*0*/   {0, 99},
  /*1*/   {100, 132},
  /*2*/   {133, 187},
  /*3*/   {188, 239},
  /*4*/   {240, 288},
  /*5*/   {289, 338},
  /*6*/   {339, 370},
  /*7*/   {371, 420},
  /*8*/   {421, 452},
  /*9*/   {453, 506},
  /*10*/  {507, 555},
  /*11*/  {556, 601},
  /*12*/  {602, 697},
  /*13*/  {698, 802}
};

int activeColor;               // The color currently in use
int r, g, b;                   // The rbg values of activeColor
const byte colors[7][3] = {    // Preset color values
  // {r, g, b}
  {255,255 ,255 }, /*White*/ 
  {255, 0, 255},   /*Purple*/
  {0, 0, 255},     /*Blue*/
  {0, 255, 0},   /*Green*/
  {255, 255, 0},     /*Yellow*/
  {255, 153, 0},   /*Orange*/  
  {255, 0, 0}      /*Red*/  
};

bool litZones[14];                // Stores a 1 at the index of zone that is currently lit
                                  // and a 0 at the index of a zone that is currently off

/**
 * PIR Sensor Helper Class
 * 
 * This class is used to create PIR sensor objects. These objects 
 * initialise the PIR sensor at a specified pin and provide a 
 * method to determine if they have been triggered.
 */
class PIR
{
  private:
    uint16_t _pin;             // The pin to which the sensor is connected
    bool _currentState;        // The senor's current state, HIGH or LOW
    bool _previousState;       // The sensor's previous state, HIGH or LOW
    
  public:
    uint16_t zone;             // The segment of the NeoPixel strip the sensor is associated with

  /**
   * Constructor
   * Initialises a PIR sensor at a given pin and  
   * with a specified zone along the NeoPixel strip
   */
  PIR(uint16_t pin,  uint16_t Zone)
  {
    pinMode(pin, INPUT);
    zone = Zone;
    _pin = pin;
    _currentState = 0;
    _previousState = 0;
  }

  /**
   * Returns true if the sensor's state changes from
   * LOW to HIGH (it has been triggered) and false otherwise
   */
  bool isTriggered()
  {
    bool triggered = false;

    // Read the sensor's current state
    _currentState = digitalRead(_pin);
    
    // Check for changes between the current and previous states
    if (_currentState != _previousState) {
      if (_currentState == HIGH) {
        // State changed to high -> sensor has been triggered
        triggered = true;
      } else {
        // Stated changed to low -> sensor has reset
        triggered = false;
      }
    }

    // Set current state to previous state,
    // for next iteration of the check
    _previousState = _currentState;
  
    return triggered;
  }
};

// Create PIR sensor objects
// pir0 is at the bottom of the stairs, pir14 is at the top
//         PIR(PIN, ZONE)           
PIR pir0 = PIR(16, 0);
PIR pir1 = PIR(15, 1);
PIR pir2 = PIR(14, 2);
PIR pir3 = PIR(13, 4);
PIR pir4 = PIR(12, 5);
PIR pir5 = PIR(2, 3);
PIR pir6 = PIR(4, 6);
PIR pir7 = PIR(5, 7);
PIR pir8 = PIR(10, 8);
PIR pir9 = PIR(11, 10);
PIR pir10 = PIR(8, 9);
PIR pir11 = PIR(9, 11);
PIR pir12 = PIR(7, 12);
PIR pir13 = PIR(6, 13);

// Create a NeoPixel strip object
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, STRIP_PIN, NEO_GRB + NEO_KHZ800);

// Create a Timer object
Timer timer;

/**
 * Runs once when the reset button is pressed 
 * or the when the board is powered up
 */
void setup() {

  // Prepare the NeoPixel strip for data 
  // output and clear any active pixels
  activeColor = random(7);
  r = colors[activeColor][0]; g = colors[activeColor][1]; b = colors[activeColor][2];
  strip.begin();
  strip.setBrightness(75);
  strip.clear();
  strip.show();

  // Set the timer interval and its
  // callback function (called when the time is up)
  timer.setInterval(3000);
  timer.setCallback(timerComplete);

  // Begin serial monitor for debugging
  Serial.begin(115200);
}

/**
 * Runs over and over, forever
 */
void loop() {

  // Check if the a PIR sensor has been triggered
  // and store its associated zone number
  int activeZone = checkPIRs();

  // If the zone number is valid
  if (activeZone >= 0) {
    // Start the timer
    timer.start();                             
    // Select a random animation for the zone
    randomAnimation(r, g, b, activeZone);
    // Set the active zone as "lit", that is, 1
    litZones[activeZone] = 1;
  }
  
  // update the timer
  timer.update();  
}

/**
 * Timer callback function
 * Called when the timer's time interval has elapsed
 */
void timerComplete() {

  // Reset and pause the timer
  timer.pause();                   

  // If all the sensors have been triggered
  // and the entire strip is lit
  if (isStripLit()) {   
    // Run a fancy animation on the whole strip
    randomFancyAnimation(r, g, b);
  }

  // Else, turn off the strip and
  // set all the zones to "not lit", that is, 0
  strip.clear();
  strip.show();
  resetLitZones();

  // Generate a new rgb color
  activeColor = random(7);
  r = colors[activeColor][0]; g = colors[activeColor][1]; b = colors[activeColor][2];
}


/** Set all the zones to "not lit", that is, 0 */
void resetLitZones() {

  for (int i = 0; i < 14; i++) {
    litZones[i] = 0;
  }
}

/**
 * Returns true if all the zones on the strip are "lit"
 * and false otherwise
 */
bool isStripLit() {

  // Checks if all the zones are in the "lit" 
  // state by summing the ones in the litZones array
  int sum = 0;
  for (int i = 0; i < 14; i++) {
    // IMPORTANT: REMOVE THE IF STATEMENT BELOW
    // ONCE TO THE BROKEN PIR SENSOR IS FIXED
    if (i == 11) {
      sum = sum + 1;
    }
    sum = sum + litZones[i];
  }
  return sum >= 14;             // CHANGE TO == 14
}

/**
 * Checks if a PIR sensor has been triggered and returns
 * the zone number associated with the sensor. Returns
 * -1 if no sensor has been triggered.
 */
int checkPIRs() {

  if (pir0.isTriggered()) { return pir0.zone; }
  if (pir1.isTriggered()) { return pir1.zone; }
  if (pir2.isTriggered()) { return pir2.zone; }
  if (pir3.isTriggered()) { return pir3.zone; }
  if (pir4.isTriggered()) { return pir4.zone; }
  if (pir5.isTriggered()) { return pir5.zone; }
  if (pir6.isTriggered()) { return pir6.zone; }
  if (pir7.isTriggered()) { return pir7.zone; }
  if (pir8.isTriggered()) { return pir8.zone; }
  if (pir9.isTriggered()) { return pir9.zone; }
  if (pir10.isTriggered()) { return pir10.zone; }
  if (pir11.isTriggered()) { return pir11.zone; }
  if (pir12.isTriggered()) { return pir12.zone; }
  if (pir13.isTriggered()) { return pir13.zone; }

  return -1;
}


/**
 * Sets all the pixels in the specified zone to the provide (r, g, b) color
 */
void setZone(byte red, byte green, byte blue, uint16_t zone) {

  //  zone startPixe, zone endPixel
  for(int i = zones[zone][0]; i < zones[zone][1]; i++ ) {
    strip.setPixelColor(i, strip.Color(red, green, blue));
  }
  strip.show();
}


/**
 * Runs a random animation on a specified zone of the strip
 */
void randomAnimation(byte red, byte green, byte blue, uint16_t activeZone) {

  int rnd = random(2);

  switch (rnd) {
    case 0:
      fadeInZone(red, green, blue, activeZone);
      break;
    case 1:
      fadeInZone(red, green, blue, activeZone);
      break;
    default:
      break;
  }
}

/**
 * Runs a random "fancy" animation on the whole strip
 */
void randomFancyAnimation(byte red, byte green, byte blue) {
  
  int rnd = random(3);

  switch (rnd) {
    case 0:
      theaterChase(red, green, blue, 50);
      break;
    case 1:
      theaterChaseRainbow(25);
      break;
    case 2:
      colorWipe(red, green, blue, 0);
      break;
    default:
      break;
  }
}
 
/*
 * Creates a fades in animation of the provided 
 * (r, g, b) color in the zone that is specified
 */
void fadeInZone(byte red, byte green, byte blue, uint16_t activeZone){
  
  float r, g, b;
  for(int k = 0; k < 20; k=k+1) {
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setZone(r,g,b,activeZone);
  }
}


/**
 * Produces a theatre chase animation on the entire strip
 * See what it looks like here: https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectTheatreChase
 */
void theaterChase(byte red, byte green, byte blue, int SpeedDelay) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < NUM_PIXELS; i=i+3) {
        strip.setPixelColor(i+q, strip.Color(red, green, blue));    //turn every third pixel on
      }
      strip.show();
     
      delay(SpeedDelay);
     
      for (int i=0; i < NUM_PIXELS; i=i+3) {
        strip.setPixelColor(i+q, strip.Color(0,0,0));        //turn every third pixel off
      }
    }
  }
}


/**
 * Produces a theatre chase rainbow animation on the entire strip
 * See what it looks like here: https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectTheatreChaseRainbow
 */
void theaterChaseRainbow(int SpeedDelay) {
  byte *c;
  
  for (int j=0; j < 64; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < NUM_PIXELS; i=i+3) {
          c = Wheel( (i+j) % 255);
          strip.setPixelColor(i+q, strip.Color(*c, *(c+1), *(c+2)));    //turn every third pixel on
        }
        strip.show();
       
        delay(SpeedDelay);
       
        for (int i=0; i < NUM_PIXELS; i=i+3) {
          strip.setPixelColor(i+q, strip.Color(0,0,0));        //turn every third pixel off
        }
    }
  }
}

/** Helper function for theatreChaseRaibow animation*/
byte * Wheel(byte WheelPos) {
  static byte c[3];
  
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }

  return c;
}

/**
 * Produces the color wipe animation on the entire strio
 * See what it looks like here: https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectColorWipe
 */
void colorWipe(byte red, byte green, byte blue, int SpeedDelay) {

  strip.clear();
  for(uint16_t i=0; i<NUM_PIXELS; i++) {
      strip.fill(strip.Color(red, green, blue), i, 200);
      strip.show();
      delay(SpeedDelay);
  }
}


void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay, uint16_t activeZone){

 int NUM_LEDS  = zones[activeZone][1] - zones[activeZone][0];
  for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
    setZone(0,0,0,activeZone);
    strip.setPixelColor(i, strip.Color(red/10, green/10, blue/10));
    for(int j = 1; j <= EyeSize; j++) {
      strip.setPixelColor(i+j, strip.Color(red,green,blue));
    }
    strip.setPixelColor(i+EyeSize+1, strip.Color(red/10,green/10,blue/10));
    strip.show();
    delay(SpeedDelay);
  }

  delay(ReturnDelay);

  for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
    setZone(0,0,0, activeZone);
    strip.setPixelColor(i, strip.Color(red/10, green/10, blue/10));
    for(int j = 1; j <= EyeSize; j++) {
      strip.setPixelColor(i+j, strip.Color(red,green,blue));
    }
    strip.setPixelColor(i+EyeSize+1, strip.Color(red/10,green/10,blue/10));
    strip.show();
    delay(SpeedDelay);
  }
  
  delay(ReturnDelay);
}



///////////////////// Not used, for now //////////////////////

void fadeOutStrip(byte red, byte green, byte blue) {
  float r, g, b;
    
  for(int k = 40; k >= 0; k=k-2) {
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
  }
}

void setAll(byte red, byte green, byte blue) {
 
  for(int i = 0; i < NUM_PIXELS; i++ ) {
    strip.setPixelColor(i, strip.Color(red, green, blue));
  }
  strip.show();
} 




///**
// * Produces the fire animation
// * See what it looks like here: https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectFire
// */
//void Fire(int Cooling, int Sparking, int SpeedDelay, uint16_t activeZone) {
//
//  int cooldown;
//  int NUM_LEDS = zones[activeZone][1] - zones[activeZone][0];
//  static int heat[NUM_LEDS];
//  
//  // Step 1.  Cool down every cell a little
//  for( int i = 0; i < NUM_LEDS; i++) {
//    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);
//    
//    if(cooldown>heat[i]) {
//      heat[i]=0;
//    } else {
//      heat[i]=heat[i]-cooldown;
//    }
//  }
//  
//  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
//  for( int k= NUM_LEDS - 1; k >= 2; k--) {
//    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
//  }
//    
//  // Step 3.  Randomly ignite new 'sparks' near the bottom
//  if( random(255) < Sparking ) {
//    int y = random(7);
//    heat[y] = heat[y] + random(160,255);
//    //heat[y] = random(160,255);
//  }
//
//  // Step 4.  Convert heat to LED colors
//  for( int j = zones[activeZone][0]; j < NUM_LEDS; j++) {
//    setPixelHeatColor(j, heat[j] );
//  }
//
//  strip.show();
//  delay(SpeedDelay);
//}
//
//void setPixelHeatColor (int Pixel, byte temperature) {
//  // Scale 'heat' down from 0-255 to 0-191
//  byte t192 = round((temperature/255.0)*191);
// 
//  // calculate ramp up from
//  byte heatramp = t192 & 0x3F; // 0..63
//  heatramp <<= 2; // scale up to 0..252
// 
//  // figure out which third of the spectrum we're in:
//  if( t192 > 0x80) {                     // hottest
//    strip.setPixelColor(Pixel, strip.Color( 255, 255, heatramp));
//  } else if( t192 > 0x40 ) {             // middle
//    strip.setPixelColor(Pixel, strip.Color(255, heatramp, 0));
//  } else {                               // coolest
//    strip.setPixelColor(Pixel, strip.Color(heatramp, 0, 0));
//  }
//}
