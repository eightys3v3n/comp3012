/* Author: Terrence Plunkett
 * Date: 2021-02-04
 *
 * Sets the light up level of a 10 segment LED display utilizing the output from a
 * depth sensor.
 */


// Global Constants
const int NUM_LEDS = 10;
const int LEDS[NUM_LEDS] = {2,3,4,5,6,7,8,9,10,11};
const int DEPTH_PINS[2] = {12,13}; // Echo, Trigger.
const int PING_TIME = 60; // milliseconds to measure distance over. >= 29.

// Predeclare a function so I can put it in any order I wish.
// This isn't done for all functions because the compiler didn't ask me to.
// While it should be, I'm not sure why the compiler disallowed one function
// and not the others.
float getDistance(int delayBetweenPings=100);


// Global Variables
// How many LEDS are lit up [0-100]%?
float percent = 0;
// Are we lighting up (false) or turning off (true)?
bool direction = false;


// Configure pin modes
void setup() {
  /*Serial.begin(115200);
  Serial.print("Starting...       ");*/
  
  for (int i = 0; i < NUM_LEDS; i++)
    pinMode(LEDS[i], OUTPUT);
  // Serial.println("Done");

  pinMode(DEPTH_PINS[0], INPUT);
  pinMode(DEPTH_PINS[1], OUTPUT);
}


// Update the 10 segment LED to match the distance measured.
// Currently maps the range 0-10cm to the display, anything more is just all on.
void loop() {
  float distance = getDistance(PING_TIME);

  percent = map(distance, 0, 10, 0, 100);
  setPercent(percent);
}


// Sets the LED at LEDS[l] to `mode`.
void setLED(unsigned int l, bool mode) {
  if (l >= NUM_LEDS)
    return;
  if (l < 0)
    return;
  
  /*Serial.print(i);
  Serial.print(", ");
  Serial.println(mode);*/
  digitalWrite(LEDS[i], mode);  
}


// Sets that percent of the LEDs to on and the rest to off.
void setPercent(float percent) {
  unsigned int leds = map(percent, 0, 100, 0, NUM_LEDS);
  /*Serial.print("Percent mapped to ");
  Serial.print(percent);
  Serial.print("->");
  Serial.println(leds);*/

  for (unsigned int l=0; l < NUM_LEDS; l++) {
    if (l < leds)
      setLED(l, LOW);
    else
      setLED(l, HIGH);
  }  
}

//-----------------------------------------------
//retunrs a distance reading from an attached 
//HC_SR04 ultrasong sensor
//param: delpayBetweenPings will default to 100
//       if left blank. 
//       Smallest value should be 29.
// Written by Jordan Kidney, borrowed from example code for week 5 of COMP 3012 in Winter of 2021.
float getDistance(int delayBetweenPings=100)
{
  unsigned long pingTime;
  float distance;
 
  //clear trigger pin
  digitalWrite(DEPTH_PINS[1], LOW);
  delayMicroseconds(2);
  
  //make the trig pin output High for 10 microseconds
  //to trigger the HC_SR04
  digitalWrite(DEPTH_PINS[1], HIGH);
  delayMicroseconds(10);
  digitalWrite(DEPTH_PINS[1], LOW);
 
  //wait for the HC_SR04 to return a HIGH level
  //signal and measure how long it took to get a  
  //signal back.
  pingTime = pulseIn(DEPTH_PINS[0], HIGH); 
  
  //calculate distance based upon pingTime based upon
  //known speed of sound
  
  //seed of sound ( v = 340 m/s = 0.034 cm/ microsecond)
  
  distance = pingTime*0.034/2;


  //if distance goes above 400, just assume a large
  //objects is closer than 2cm (the min)

  if(distance > 400) distance = 0;
             
  //wait delayBetweenPings between next
  //reading, a delay is needed to stop problems
  //with trying to read ultrasonic too fast.
  //Min should be 29.
  delay(delayBetweenPings);
  return distance;                  
}
