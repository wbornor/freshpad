//*****************************************************************************
#include <SoftwareSerial.h>   
#include <SmartThings.h>
#include <Adafruit_NeoPixel.h>
#include "Statistic.h" //http://playground.arduino.cc/Main/Statistics

#define PIN_LED_STRIP     6
#define PIN_LED           13
#define PIN_THING_RX      3
#define PIN_THING_TX      2

SmartThingsCallout_t messageCallout;    // call out function forward decalaration
SmartThings smartthing(PIN_THING_RX, PIN_THING_TX, messageCallout);  // constructor

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN_LED_STRIP, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.


bool isDebugEnabled;    // enable or disable debug in this example
int stateLED;           // state to track last set value of LED
int stateNetwork;       // state of the network 


void setNetworkStateLED()
{
  SmartThingsNetworkState_t tempState = smartthing.shieldGetLastNetworkState();
  if (tempState != stateNetwork)
  {
    switch (tempState)
    {
      case STATE_NO_NETWORK:
        if (isDebugEnabled) Serial.println("NO_NETWORK");
        smartthing.shieldSetLED(2, 0, 0); // red
        break;
      case STATE_JOINING:
        if (isDebugEnabled) Serial.println("JOINING");
        smartthing.shieldSetLED(2, 0, 0); // red
        break;
      case STATE_JOINED:
        if (isDebugEnabled) Serial.println("JOINED");
        smartthing.shieldSetLED(0, 0, 0); // off
        break;
      case STATE_JOINED_NOPARENT:
        if (isDebugEnabled) Serial.println("JOINED_NOPARENT");
        smartthing.shieldSetLED(2, 0, 2); // purple
        break;
      case STATE_LEAVING:
        if (isDebugEnabled) Serial.println("LEAVING");
        smartthing.shieldSetLED(2, 0, 0); // red
        break;
      default:
      case STATE_UNKNOWN:
        if (isDebugEnabled) Serial.println("UNKNOWN");
        smartthing.shieldSetLED(0, 2, 0); // green
        break;
    }
    stateNetwork = tempState; 
  }
}


int sensorPin = A1;    // select the input pin for the potentiometer
int sensorPin2 = A0;
int ledPin = 8;      // select the pin for the LED
int LEDbrightness;
int sensorValue = 0;  // variable to store the value coming from the sensor
int sensorValue2 = 0;
Statistic stats;


void setup() {
  // setup default state of global variables
  isDebugEnabled = true;
  
  stateLED = 0;                 // matches state of hardware pin set below
  stateNetwork = STATE_JOINED;  // set to joined to keep state off if off
  
  // declare the ledPin as an OUTPUT:
  pinMode(ledPin, OUTPUT);
  if (isDebugEnabled)
  { // setup debug serial port
    Serial.begin(9600);         // setup serial with a baud rate of 9600
    Serial.println("setup..");  // print out 'setup..' on start
  }
  
  stats.clear();
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  // run smartthing logic
  smartthing.run();
  
  // Code left here to help debut network connections
  setNetworkStateLED();
  
  // read the value from the sensor:
  Serial.print("fsr0: ");
  Serial.print(A0);
  
  
  Serial.print(", fsr1: ");
  Serial.print(sensorValue);
  LEDbrightness = map(sensorValue, 0, 1023, 0, 255);
  
  sensorValue2 = analogRead(A2);
  Serial.print(", fsr2: ");
  Serial.print(sensorValue2);
 
  Serial.print(", fsr3: ");
  Serial.print(analogRead(A3));
  
  Serial.print(", fsr4: ");
  Serial.print(analogRead(A4));
  sensorValue = analogRead(A4);
  
  Serial.print(", fsr5: ");
  Serial.print(analogRead(A5));
  
  //sensorValue = (sensorValue + sensorValue2)/2;
  //Serial.print(", TOTAL: ");
  //Serial.print(sensorValue);
  
  // we'll need to change the range from the analog reading (0-1023) down to the range
  // used by analogWrite (0-255) with map!
  sensorValue = map(sensorValue, 0, 1000, 0, 100);

  //offset  middle tier reading
  //sensorValue = sensorValue >= 90 && sensorValue <= 95 ? map(sensorValue, 90, 95, 10, 99) : sensorValue;
  
  //set caps
  sensorValue = sensorValue > 100 ? 100 : sensorValue;
  sensorValue = sensorValue < 0 ? 0 : sensorValue;
  
  Serial.print(", mapped: ");
  Serial.print(sensorValue);
  
  stats.add(sensorValue);

  Serial.print(", cnt: ");
  Serial.print(stats.count()); 

  Serial.print(", avg: ");
  Serial.print(stats.average(), 4);

  Serial.print(", stddev: ");

  Serial.print(stats.pop_stdev(), 4);
 
  if (stats.pop_stdev() > 5  /*|| stats.count() >= 500*/) {
    stats.clear();
  }
  
  if ( stats.count() < 2 ) {
    
    for(int i = 0; i < 1; i++){
      delay(100);
      announceForce(sensorValue);
      if(sensorValue < 30) {
        
        buildCells(strip.Color(255, 0, 0), 500); // red
        
      } else if(sensorValue < 60) {
          buildCells(strip.Color(0, 0, 255), 500); // blue
      } else {
        buildCells(strip.Color(0, 255, 0), 500); // green
      }
    }
  } else if ( stats.count() == 5) {
   colorWipe(strip.Color(0, 0, 0), 1); // Off 
  }
  else {
    // turn the ledPin off:
    digitalWrite(ledPin, LOW);
  }
  
  //strandTest();
  
  // stop the program for <sensorValue> milliseconds:
  //delay(sensorValue);
  
  Serial.println();
}

void announceForce(int force) {
    Serial.print(", sending force: ");
    Serial.print(sensorValue);
    smartthing.send("fsr1: " + String(sensorValue) );
    networkTrafficLED();
    
    // turn the ledPin on
    digitalWrite(ledPin, LEDbrightness);
    
    //strandBlip();
    
}

void networkTrafficLED() {
    smartthing.shieldSetLED(0, 2, 0); // green
    smartthing.shieldSetLED(1, 0, 0); // red
    smartthing.shieldSetLED(0, 0, 0); // off 
}

void cycleLED() {
    smartthing.shieldSetLED(1, 0, 0); // red
    smartthing.shieldSetLED(0, 1, 0); // green
    smartthing.shieldSetLED(0, 0, 1); // blue
    smartthing.shieldSetLED(1, 0, 0); // red
    smartthing.shieldSetLED(0, 1, 0); // green
    smartthing.shieldSetLED(0, 0, 1); // blue
    smartthing.shieldSetLED(0, 0, 0); // off 
}  

void messageCallout(String message)
{
  // if debug is enabled print out the received message
  if (isDebugEnabled)
  {
    Serial.print("Received message: '");
    Serial.print(message);
    Serial.println("' ");
  }
  
  if (message == "strand-test") {
      strandTest();
  } else if (message == "on") {
      strandOn();
  } else if (message == "off") {
      strandOff();
  } else if (message == "build-cells") {
      buildCells(strip.Color(0, 255, 0), 500);
  } else  {
    Serial.print("unknown message: '");
  }
}

void buildCells(uint32_t color, uint8_t wait) {
  uint16_t i;
  uint8_t third = 4;
  for(i=0; i< third; i++) {
      strip.setPixelColor(i, color);
  }
  strip.show();
  delay(wait);
  
  for(i=third; i< third*2; i++) {
      strip.setPixelColor(i, color);
  }
  strip.show();
  delay(wait);
  
  for(i=third*2; i< third*3; i++) {
      strip.setPixelColor(i, color);
  }
  strip.show();
  delay(wait*10);
  colorWipe(strip.Color(0, 0, 0), 3); // Off
}

void strandOn(){
  colorWipe(strip.Color(255, 255, 255), 1); // White
}

void strandOff(){
  colorWipe(strip.Color(0, 0, 0), 1); // Off 
}

void strandBlip(){
  
  colorWipe(strip.Color(0, 255, 0), 0.5); // Green
  colorWipe(strip.Color(255, 0, 0), 0.1); // Red
  colorWipe(strip.Color(0, 0, 0), 1); // Off
}

void strandTest(){

  // Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
  // Send a theater pixel chase in...
  theaterChase(strip.Color(127, 127, 127), 50); // White
  theaterChase(strip.Color(127,   0,   0), 50); // Red
  theaterChase(strip.Color(  0,   0, 127), 50); // Blue

  rainbow(20);
  rainbowCycle(20);
  theaterChaseRainbow(50);
  
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();
     
      delay(wait);
     
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        strip.show();
       
        delay(wait);
       
        for (int i=0; i < strip.numPixels(); i=i+3) {
          strip.setPixelColor(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

