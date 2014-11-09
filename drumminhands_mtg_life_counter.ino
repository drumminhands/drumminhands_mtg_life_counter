/* 
  An arduino life counter for MtG: Magic the Gathering
  Created by chris@drumminhands.com
  
  Press the plus button (button A) to increment the life counter by one. Hold to move faster.
  Press the minus button (button B) to decrease the life counter by one. Hold to move faster.
  Hold the reset button (button C) to reset to 20 life when starting a new game. 
  Hold longer to reset to 40 life for an EDH game.
  
  Uses a four digit display from Adafruit https://learn.adafruit.com/adafruit-led-backpack/0-54-alphanumeric

  Note, I tried to get this small enough for regular Trinket, but I couldn't get the libraries small enough. So it'll work on a Pro Trinket or Arduino Uno
*/

//includes
#include <Button.h> //http://playground.arduino.cc/Main/ImprovedButton
#include <Wire.h>   
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

//constants don't change
#define buttonPin1 12  //12 on the Arduino
#define buttonPin2 8  //8 on the Arduino
#define buttonPin3 4  //4 on the Arduino
// plug D (data) on backpack to pin A4 on arduino uno or trinket pro
// plug C (clock) on the backpack to pin A5 on arduino uno or trinket pro
//note buttons can only be named with one letters, I'm not sure why
Button A(HIGH); // LOW = milliseconds, HIGH = microseconds, default is LOW
Button B(HIGH); // LOW = milliseconds, HIGH = microseconds, default is LOW
Button C(HIGH); // LOW = milliseconds, HIGH = microseconds, default is LOW
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4(); //the four digit display
const uint8_t digits = 4; //how many digits in the display
const uint16_t upperRange = 9999;  //how large can the life get before resetting to zero
const int lowerRange = -999; //how low can the life get before resetting to zero

//variables can change
uint16_t holdTime = 1000; // how long does the button need to be held until the reaction occurs
float resetTimeMedium = 1.5; //the medium wait amount to reset
float resetTimeLong = 3.0; //the long wait amount to reset
uint8_t life = 20; //the life value
uint8_t resetLife = 20; //the life value to reset for a new game
uint8_t edhLife = 40; //the starting life value for EDH games
uint8_t fastDelay = 100; //when holding a plus or minus button, use this value in delay to slow down the increment
String intro = "MTG "; // start as the first message to share, must only be 4 digits
String intro2 = "LIFE"; // second message, must only be 4 digits

void setup(){ 
  alpha4.begin(0x70);  // pass in the address for the digit display
  
  //setup buttons as input
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  
  // Set what the button will be when pressed (default is HIGH)
  // and set the hold time (default is 500)
  // then set all the buttons to functions
  A.SetStateAndTime(LOW, holdTime);
  A.onPressed(plusOne); //when pressed once, increase life by one
  A.onHold(plusOneFast); //when held, increase by one repeatedly until released
  B.SetStateAndTime(LOW, holdTime);
  B.onPressed(minusOne); //when pressed once, decrease life by one
  B.onHold(minusOneFast); //when held, decrease by one repeatedly until released
  C.SetStateAndTime(LOW, holdTime);

  // light up all segments to test that they work
  alpha4.writeDigitRaw(3, 0x0);
  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(0, 0x0);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(1, 0x0);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(2, 0x0);
  alpha4.writeDigitRaw(3, 0xFFFF);
  alpha4.writeDisplay();
  delay(1000);

  // display the intro messages
  messageFour(intro);
  delay(1000);
  messageFour(intro2);
  delay(1000);
  // display life
  messageLife();
}

void loop(){
  //check the buttons
  byte myButton1 = A.checkButton(digitalRead(buttonPin1));
  byte myButton2 = B.checkButton(digitalRead(buttonPin2));
  byte myButton3 = C.checkButton(digitalRead(buttonPin3));
  
  //reset button has more needs than the setup function above
  if (myButton3) // if myButton is anything but 0, it is true
  {
    switch (myButton3)
    {
      case PRESSED:
        //nothing to see here
        break;
      case HELD:
        //the button was held
        if (C.GetHeldTime(SECONDS)>resetTimeLong){ //if it was held a long amount of time
          messageFour("  " + String(edhLife)); //display the long message
        } else if (C.GetHeldTime(SECONDS)>resetTimeMedium){ //if it was held a medium amount of time
          messageFour("  " + String(resetLife)); //display the medium message
        }
        break;
      case RELEASED:
        if (C.GetHeldTime(SECONDS)>resetTimeLong){ //if it was held a long time
          resetEDH(); // reset to the larger value used for EDH
        } else if (C.GetHeldTime(SECONDS)>resetTimeMedium){ //if it was held a medium time
          resetStandard(); //else reset for the standard
        }
        break;
      default: break;
    }
  }
}

void plusOne(){
  life++;
  if (life > upperRange){
    life = 0;
  }
  messageLife(); // display the life total on the digit display
}

void plusOneFast(){
  plusOne();
  delay(fastDelay);
}

void minusOne(){
  life--;
  if (life < lowerRange){
    life = 0;
  }
  messageLife(); // display the life total on the digit display
}


void minusOneFast(){
  minusOne();
  delay(fastDelay);
}

void resetStandard(){
  life = resetLife;
  messageLife(); // display the life total on the digit display
}

void resetEDH(){
  life = edhLife;
  messageLife(); // display the life total on the digit display
}

void messageLife(){
  String lifeStr = String(life); //store the life value in a string
  
  if (lifeStr.length()>digits){
    // something's wrong. It's long. Display it.
    message(lifeStr);
  }else{
    // add spaces to the front of the string to force four digits
    do
    {
     lifeStr = " " + lifeStr;
    } while (lifeStr.length()<digits);
    
    // display life, no scroll in
    messageFour(lifeStr);
  }
}
void message(String str){
   
  alpha4.clear();
  alpha4.writeDisplay();
  
  for (uint8_t i=0; i<str.length()-digits; i++) { // note only works if longer than number of digits
    alpha4.writeDigitAscii(0, str[i]);
    alpha4.writeDigitAscii(1, str[i+1]);
    alpha4.writeDigitAscii(2, str[i+2]);
    alpha4.writeDigitAscii(3, str[i+3]);
    alpha4.writeDisplay();
    
    delay(200);
  }  
}
void messageFour(String str){
  //NOTE only accepts strings with four characters
 
  alpha4.clear();
  alpha4.writeDisplay();
  
  // display, no scroll in
  alpha4.writeDigitAscii(0, str[0]);
  alpha4.writeDigitAscii(1, str[1]);
  alpha4.writeDigitAscii(2, str[2]);
  alpha4.writeDigitAscii(3, str[3]);
  alpha4.writeDisplay();  
}

