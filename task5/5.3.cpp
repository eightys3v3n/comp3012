// Global Variables
enum State {
  A,
  B,
  C,
  D,
  E,
  F
};
char* stateNames[] {
  "A",
  "B",
  "C",
  "D",
  "E",
  "F"
};

boolean debug = true; // false to not show debug messages
State currentState = B; // start state
#define LOOP_DELAY 80

#define POT_PIN A5
int DEPTH_PINS[] = {7,8};

// LEDs
#define RED_PIN 6
#define BLUE_PIN 5
#define GREEN_PIN 3
#define BLINK_TIME 1000 // toggle state every this many milliseconds
#define FADE_TIME 2000 // fade entirely down or up in this amount of time
#define FADE_AMT 20
bool led_state = true; // for all states that utilize and LED to blink or fade
int led_brightness = 0; // for states C, D, F to fade
unsigned long last_change = millis();


// Helper functions for everyone
void debugMsg(char* msg, char* data=NULL) {
  if(debug) {
    unsigned long myTime = millis();
    Serial.print(myTime);
    Serial.print(": (");
    Serial.print(stateNames[currentState]);
    Serial.print(") ");

    if (data != NULL) {
      Serial.print(msg);
      Serial.println(data);
    } else {
      Serial.println(msg);
    }
  }
}

void changeState(State n); // this fixes the new Arduino build system declaration bug and allows enum arguments.
void changeState(State n) {
  currentState = n;
  debugMsg("Changing to state", stateNames[n]);
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
  // delay(delayBetweenPings);
  return distance;
}



// Main functions
void setup() {
  Serial.begin(9600);
  debugMsg("Machine starting up");

  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(POT_PIN, INPUT);
  pinMode(DEPTH_PINS[0], INPUT);
  pinMode(DEPTH_PINS[1], OUTPUT);
}

void loop() {
  runStateMachine();
  delay(LOOP_DELAY);
}

void runStateMachine() {
  debugMsg("Potentiometer state: ", (char*)String(analogRead(POT_PIN)).c_str());
  debugMsg("Distance state: ", (char*)String(getDistance()).c_str());

  switch(currentState) {
    case A:
      runA();
      break;
    case B:
      runB();
      break;
    case C:
      runC();
      break;
    case D:
      runD();
      break;
    case E:
      runE();
      break;
    case F:
      runF();
      break;
  }
}


// State functions
void runA() {
  debugMsg("Runing state A");

  const short pot = analogRead(POT_PIN);

  if (pot > 90) {
    changeState(E);
  }

  unsigned long time = millis();

  if (time-last_change >= BLINK_TIME) {
    last_change = time;
    led_state = !led_state;
    digitalWrite(BLUE_PIN, led_state);
  }
}

void runB() {
  debugMsg("Runing state B");

  const short pot = analogRead(POT_PIN);

  if (pot < 25) {
    changeState(A);
  } else if (25 < pot && pot < 90) {
    changeState(E);
  } else if (90 < pot && pot < 120) {
    changeState(C);
  }

  unsigned long time = millis();

  if (time-last_change >= BLINK_TIME) {
    last_change = time;
    led_state = !led_state;
    digitalWrite(GREEN_PIN, led_state);
  }
}

void runC() {
  debugMsg("Runing state C");

  const short dist = getDistance();
  const short pot = analogRead(POT_PIN);

  if (pot < 90) {
    changeState(B);
  } else if (pot > 120 && dist > 20) {
    changeState(D);
  }

  unsigned long time = millis();

  if (time-last_change >= FADE_TIME/(255/FADE_AMT)) {
    last_change = time;

    if (led_brightness <= 0) {
      led_state = true;
    } else if (led_brightness >= 255) {
      led_state = false;
    }

    if (!led_state) {
      led_brightness = max(0, led_brightness - FADE_AMT);
    } else {
      led_brightness = min(255, led_brightness + FADE_AMT);
    }

    debugMsg("LED brightness: ", (char*)String(led_brightness).c_str());
    analogWrite(RED_PIN, led_brightness);
  }
}

void runD() {
  debugMsg("Runing state D");

  const short dist = getDistance();
  const short pot = analogRead(POT_PIN);

  if (dist < 20) {
    changeState(F);
  }

  unsigned long time = millis();

  if (time-last_change >= FADE_TIME/(255/FADE_AMT)) {
    last_change = time;

    if (led_brightness <= 0) {
      led_state = true;
    } else if (led_brightness >= 255) {
      led_state = false;
    }

    if (!led_state) {
      led_brightness = max(0, led_brightness - FADE_AMT);
    } else {
      led_brightness = min(255, led_brightness + FADE_AMT);
    }

    debugMsg("LED brightness: ", (char*)String(led_brightness).c_str());
    analogWrite(RED_PIN, led_brightness);
  }
}

void runE() {
  debugMsg("Runing state E");

  const short pot = analogRead(POT_PIN);

  if (pot < 25) {
    changeState(B);
  }

  unsigned long time = millis();

  if (time-last_change >= BLINK_TIME) {
    last_change = time;
    led_state = !led_state;
    digitalWrite(RED_PIN, led_state);
  }
}

void runF() {
  debugMsg("Runing state F");

  const short dist = getDistance();
  const short pot = analogRead(POT_PIN);

  if (pot < 20) {
    changeState(C);
  } else if (20 < pot && pot < 40) {
    changeState(D);
  }

  unsigned long time = millis();

  if (time-last_change >= FADE_TIME/(255/FADE_AMT)) {
    last_change = time;

    if (led_brightness <= 0) {
      led_state = true;
    } else if (led_brightness >= 255) {
      led_state = false;
    }

    if (!led_state) {
      led_brightness = max(0, led_brightness - FADE_AMT);
    } else {
      led_brightness = min(255, led_brightness + FADE_AMT);
    }

    debugMsg("LED brightness: ", (char*)String(led_brightness).c_str());
    analogWrite(RED_PIN, led_brightness);
  }
}
