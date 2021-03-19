/* This code simulates a microwave.
 * Terrence Plunkett 2021 <eightys3v3n@gmail.com>
 *
 * The motor and red LED represent the plate spin and magnetron.
 * The yellow LED represents the door interlock.
 * The left button is the Start/Pause.
 * The right button is the Stop.
 * The switch is the door interlock (it would be a button actuated by the door closing).
 * The light ring represents the current state of the microwave.
 * The time is set using the potentiometer.
 */

#include <Adafruit_NeoPixel.h>


// Global Variables
enum State {
  WAITING,
  RUNNING,
  PAUSED,
  FINISHED,
};
char* state_names[] {
  "Waiting",
  "Running",
  "Paused",
  "Finished",
};

// Pins to various devices
#define DEBUG             // print debug messages if enabled
#define LOOP_DELAY 20     // delay the loop speed for simulations
#define POT_PIN A5        // potentiometer
#define STARTPAUSE_PIN 2  // start/pause button
#define STOP_PIN 3        // stop button
#define MICROWAVE_PIN 5   // the microwave relay used to control the motor and magnetron
#define INTERLOCK_PIN 4   // the door interlock that stops the microwave if door is opened
#define LED_RING_PIN 6    // controls the LED ring
#define LED_RING_SIZE 12  // number of lights on the LED ring

#define MAX_DURATION 5000         // in milliseconds
#define MIN_DURATION 500          // in milliseconds
#define BUTTON_DEBOUNCE_DELAY 200 // ms to wait until another button can be pressed
#define FINISHED_TIME 4000        // time to stay on finished state after timer runs out
#define FLASH_TIME 500            // time to flash all lights on
#define LIGHT_SPIN_LENGTH 4       // how many LEDs are lit when spinning
#define SPIN_TIME 100             // move the spin around every this many ms
#define FINISH_FLASH_COLOUR strip.Color(127, 127, 0)  // flash this color when done
#define SPIN_COLOUR strip.Color(0, 0, 255)            // the spinning is this color
#define PAUSED_FLASH_COLOUR strip.Color(0, 255, 0)    // flash this color when paused


State curr_state = WAITING;     // current state
State last_state = PAUSED;      // the state immediately before the current runStateMachine cycle
long timer = 0;                 // time to run for once started
long start_time = 0;            // when a state started running.
long led_start_time = 0;        // when the lights made their last state change.
long last_button_press = 0;     // ms time the last button was pressed.
int pot = 0;                    // potentiometer value at beginning of loop
int last_pot = 0;               // last potentiometer value for printing out changes
int curr_led = 0;               // progress when spinning the light wheel.
bool startpause_button = false; // whether the startpause_button is pressed.
bool stop_button = false;       // whether the stop button is pressed.
bool interlock = false;         // whether the interlock switch is pressed.
bool finished_flashing = false; // whether the finished animation is all on or all off.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(
  LED_RING_SIZE,
  LED_RING_PIN,
  NEO_GRB + NEO_KHZ800
);

// Helper functions for everyone
// print a debug message if DEBUG is defined, optionally append data to the message
void debugMsg(String msg, String data="") {
  #ifdef DEBUG

  Serial.print("(");
  Serial.print(state_names[curr_state]);
  Serial.print(") ");

  if (data != "") {
    Serial.print(msg);
    Serial.println(data);
  } else {
    Serial.println(msg);
  }
  #endif // DEBUG
}

// this fixes the new Arduino build system declaration bug and allows enum arguments
void changeState(State n);
// change to another state
void changeState(State n) {
  curr_state = n;
  debugMsg("Changing to state: ", String(state_names[n]));
}

// update the potentiometer value
void readPot() {
  pot = analogRead(POT_PIN);

  if (pot != last_pot) {
    debugMsg("Potentiometer state: ", String(pot));
    last_pot = pot;
  }
}

// update button states
void readButtons() {
  bool state;
  long now = millis();

  if (now-last_button_press >= BUTTON_DEBOUNCE_DELAY) {
    // start/pause button
    state = digitalRead(STARTPAUSE_PIN);
    if (state != startpause_button) {
      debugMsg("Updated start/pause button to ", String(state));
      startpause_button = state;
    }

    // stop button
    state = digitalRead(STOP_PIN);
    if (state != stop_button) {
      debugMsg("Updated stop button to ", String(state));
      stop_button = state;
    }

    // interlock button
    state = digitalRead(INTERLOCK_PIN);
    if (state != interlock) {
      debugMsg("Updated interlock button to ", String(state));
      interlock = state;
    }
  }
}

// set the microwave relay state (motor and magnetron)
void microwaveState(bool state) {
  debugMsg("Changing microwave state: ", String(state));
  if (state) {
    digitalWrite(MICROWAVE_PIN, HIGH);
  } else {
    digitalWrite(MICROWAVE_PIN, LOW);
  }
}

// set all of the lights in the LED ring
void setAllLights(int colour) {
  for (int i = 0; i < LED_RING_SIZE; i++) {
    strip.setPixelColor(i, colour);
  }
  strip.show();
}

// start spinning the LED ring lights
void spinLights() {
  int on[LIGHT_SPIN_LENGTH] = {};

  for (int i = 0; i < LIGHT_SPIN_LENGTH; i++) {
    on[i] = (curr_led + i) % LED_RING_SIZE;
  }

  for (int l = 0; l < LED_RING_SIZE; l++) {
    bool set_led = false;

    for (int i = 0; i < LIGHT_SPIN_LENGTH; i++) {
      if (l == on[i]) {
        strip.setPixelColor(l, SPIN_COLOUR);
        set_led = true;
      }
    }

    if (!set_led) {
      strip.setPixelColor(l, 0);
    }
  }
  strip.show();

  curr_led = (curr_led+1) % LED_RING_SIZE;
}


// Main functions
void setup() {
  Serial.begin(115200);
  debugMsg("Starting up");

  changeState(WAITING);

  pinMode(POT_PIN, INPUT);
  pinMode(STARTPAUSE_PIN, INPUT);
  pinMode(STOP_PIN, INPUT);
  pinMode(INTERLOCK_PIN, INPUT);
  pinMode(MICROWAVE_PIN, OUTPUT);
  pinMode(LED_RING_PIN, OUTPUT);

  strip.begin();
  strip.setBrightness(255);
}

void loop() {
  runStateMachine();
  delay(LOOP_DELAY);
}

void runStateMachine() {
  State before_change = curr_state;
  readPot();
  readButtons();

  switch(curr_state) {
    case WAITING:
      waiting();
      break;
    case RUNNING:
      running();
      break;
    case PAUSED:
      paused();
      break;
    case FINISHED:
      finished();
      break;
    default:
      debugMsg("Invalid state? Try restarting the device?");
      break;
  }

  last_state = before_change;
}


// State functions
void waiting() {
  // if we just switched to this state
  if (last_state != WAITING) {
    debugMsg("Waiting...");
    setAllLights(0);
  }

  // don't ever leave this state when door open
  if (startpause_button && interlock) {
    startpause_button = false; // prevent the next state from thinking the button is pressed.

    // set the timer
    timer = map(pot, 0, 1023, MIN_DURATION, MAX_DURATION);
    debugMsg("Set timer for ", String(timer));
    changeState(RUNNING);
  }
}

void running() {
  long curr_time = millis();

  // if we just switched to this state
  if (last_state != RUNNING) {
    debugMsg("Running...");
    debugMsg("Timer T-", String(timer));
    start_time = millis();
  }
  else {
    debugMsg("Timer T-", String(timer-(curr_time-start_time)));
  }

  if (!interlock) {
    microwaveState(false);
    changeState(PAUSED);
  }
  else if (timer-(curr_time-start_time) <= 0) { // timer is finished
    microwaveState(false);
    changeState(FINISHED);
  }
  else if (startpause_button) {
    startpause_button = false; // prevent the next state from thinking the button is pressed.

    timer -= curr_time-start_time; // set timer to remaining time.
    microwaveState(false);
    changeState(PAUSED);
  }
  else if (stop_button) {
    stop_button = false;

    microwaveState(false);
    changeState(WAITING);
  }
  else { // microwave is still running
    microwaveState(true);

    // update light spinning
    if (curr_time-led_start_time >= SPIN_TIME) {
      debugMsg("Spinning lights");
      spinLights();
    }
  }
}

void paused() {
  long curr_time = millis();

  // if we just switched to this state
  if (last_state != PAUSED) {
    debugMsg("Paused...");
  }

  if (startpause_button) {
    debugMsg("Resuming...");
    startpause_button = false; // prevent the next state from thinking the button is pressed.

    setAllLights(0);
    changeState(RUNNING);
  }
  else if (stop_button) {
    stop_button = false;

    setAllLights(0);
    changeState(WAITING);
  }
  else if (curr_time-led_start_time >= FLASH_TIME) { // flash lights
    finished_flashing = !finished_flashing;
    led_start_time = curr_time;

    if (finished_flashing) {
      debugMsg("Flashing on");
      setAllLights(PAUSED_FLASH_COLOUR);
    } else {
      setAllLights(0);
      debugMsg("Flashing off");
    }
  }
}

void finished() {
  long curr_time = millis();

  // if we just switched to this state
  if (last_state != FINISHED) {
    debugMsg("Finished...");
    start_time = curr_time;
    led_start_time = curr_time;
  }

  if (!interlock) {
    changeState(WAITING);
  }
  else if (curr_time-start_time >= FINISHED_TIME) {
    changeState(WAITING);
    setAllLights(0);
  }
  else if (curr_time-led_start_time >= FLASH_TIME) { // flash lights
    finished_flashing = !finished_flashing;
    led_start_time = curr_time;

    if (finished_flashing) {
      debugMsg("Flashing on");
      setAllLights(FINISH_FLASH_COLOUR);
    } else {
      setAllLights(0);
      debugMsg("Flashing off");
    }
  }
}
