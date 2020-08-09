/*  
Arduino Nano script for an experimental lightning simulator for the AS3935 Lightning Detector.   
Prototype uses an si5351 breakout to gebnerate bursts of 500kHz signal.
          also uses a TM1637 4-digit 7 segment display.  
          
Written by Paul Taylor, VK3HN (https://vk3hn.wordpress.com/) 
V1.0, 9 Aug 2020 - first version 
*/

// common libraries
#include <si5351.h>     // Etherkit si3531  V2.0.1   https://github.com/etherkit/Si5351Arduino 
#include <Wire.h>
#include <TM1637Display.h>

#define CLK 2  // TM1637 clock
#define DIO 3  // data   

#define LIGHTNING_FREQ1_KHZ 500   // clock 0
#define LIGHTNING_FREQ2_KHZ 250   // clock 1
#define LIGHTNING_FREQ3_KHZ 1000  // clock 2

const int bolt_energy[] = { LIGHTNING_FREQ1_KHZ, LIGHTNING_FREQ2_KHZ, LIGHTNING_FREQ3_KHZ };
byte j; 

// Arduino Nano analogue pins
//                        A4    SDA
//                        A5    SCL

Si5351 si5351;                   // triple programmable clock generator, I2C at x60 
TM1637Display display(CLK, DIO); // 4-digit 7 segment display 

#define DURATION_RAND_PART 500
long duration_mS;   // randomised part of the duration of each lightning bolt (mS)

#define INTERVAL_RAND_PART 4000
long interval_mS;   // randomised part of the interval between each lightning bolt (mS)

// CONSTANTS for the display library
const uint8_t OFF[] = {0, 0, 0, 0};
// In this library, the byte order is .GFEDCBA
const uint8_t PLAY[] = {B01110011, B00111000, B01011111, B01101110};
const uint8_t DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

const uint8_t BOLT[] = {
  SEG_F | SEG_E | SEG_G | SEG_D | SEG_C,           // b
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_F | SEG_E | SEG_D,                           // L
  SEG_F | SEG_E | SEG_G | SEG_D                    // t
};

const uint8_t test1[] = {    // test structure to light up each display segment, A-D
  SEG_A,           
  SEG_B, 
  SEG_C, 
  SEG_D
};

const uint8_t test2[] = {  // test structure to light up each display segment, E-G
  SEG_E,           
  SEG_F, 
  SEG_G, 
  0
};




void setup(){
  // Set brightness of the LED
  display.setBrightness(4);
  // Clear the display
  display.setSegments(OFF);

  Serial.begin(9600);  
  Wire.begin();
  Serial.println("si5351 Lightning Simulator");

  display.setSegments(PLAY);
  delay(2000);

  Serial.println();
  for(j=0; j<3; j++){
    display.showNumberDec(bolt_energy[j], true, 4, 0);     
    delay(1500);    
  }

// initialise and start the si5351 clock

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0); // If using 27Mhz xtal, put 27000000 instead of 0 (0 is the default xtal freq of 25Mhz)
  si5351.set_correction(25000);    // Library update 26/4/2020: requires destination register address  ... si5351.set_correction(19100, SI5351_PLL_INPUT_XO);                                 
  si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
  
// set the frequency of the clocks
  for(j=0; j<3; j++){
    si5351.set_freq(bolt_energy[j] * 100000, (si5351_clock)j);  
    si5351.drive_strength((si5351_clock)j, SI5351_DRIVE_8MA); 
    si5351.output_enable((si5351_clock)j, 0);
  }

  randomSeed(analogRead(0));  // seed the random generator 
}


void countdown() {   // left over from display example 
  for(int i=20; i>0; i--) {
    // showNumberDec is a function for displaying numeric values,
    display.showNumberDec(i, true, 4, 0);
    delay(80);
  }
}


void loop(){
  // simulate a lightning bolt!
  display.setSegments(BOLT);
  Serial.print(" !");
  duration_mS = 50 + random(DURATION_RAND_PART);
  interval_mS = random(INTERVAL_RAND_PART);

  for(j=0; j<3; j++){
    // turn on each clock at the specified frequencies 
    si5351.output_enable((si5351_clock)j, 1);  
  }
  delay(duration_mS);   // duration of the simulated lightning bolt 
  for(j=0; j<3; j++){
    // turn off each clock  
    si5351.output_enable((si5351_clock)j, 0);
  }
  display.clear(); 
  delay(interval_mS);
}
