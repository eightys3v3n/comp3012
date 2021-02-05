/* Author: Terrence Plunkett
 * Date: 2021-02-04
 *
 * Should improve upon by utilizing a game loop structure for state changes.
 * That is, the loop should not wait while doing an action on one LED. This
 * limits the number of concurrent changes to one LED and its inefficient.
 * But with other classes I didn't get past designing a better loop in time.
 */


// Pins for the various LEDs
const int LED_A = 9;
const int LED_B = 10;
const int LED_C = 11;

// How long to wait in the initial state before commensing the pattern.
const int DELAY_START = 1000;

// Increase/decrease LED brightness by this amount (0-255) when fading.
const int FADE_INC = 10;

// Wait for this many milliseconds in between FADE_INCs.
const int FADE_DELAY = 100;

// Wait for this long between state changes in a blink.
const int BLINK_DELAY = 500;

// Wait for how long before restarting the pattern.
const int RESTART_DELAY = 4000;


/* Fade an LED at PWM pin `id` from `start` to `end` brightness in `inc` increments with `wait` milliseconds
 * between each increment.
 */
void fade(int id, int start, int end, int inc=FADE_INC, int wait=FADE_DELAY) {
  int brightness = start;

  // fade up or down depending on start and end.
  if (end > start) {
    inc = abs(inc);
  } else if (end < start) {
    inc = -abs(inc);
  } else {
    return;
  }

  // delay, increment, write
  while (brightness != end) {
    delay(wait);

    // don't increment past the end value.
    if (abs(brightness - end) < abs(inc)) {
      brightness = end;
    } else {
      brightness += inc;
    }
    
    analogWrite(id, brightness);
  }

  if (inc < 0)
    delay(wait);
}

// Fade an LED down from 255 to 0 brightness.
void fade_down(int id, int inc=FADE_INC) {
  fade(id, 255, 0, inc=inc);
}

// Fade an LED up from 0 to 255 brightness.
void fade_up(int id, int inc=FADE_INC) {
  fade(id, 0, 255, inc=inc);
}

// Blink an ON LED to OFF for wait milliseconds.
void blink_off(int id, int wait=BLINK_DELAY) {
  digitalWrite(id, LOW);
  delay(wait);
  digitalWrite(id, HIGH);
  delay(wait);
}

// Setup the initial LED state.
void initial_leds() {
  digitalWrite(LED_A, HIGH);
  digitalWrite(LED_B, HIGH);
  digitalWrite(LED_C, LOW);
}

// Configure LED pins and set their initial state.
void setup() {
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_C, OUTPUT);

  initial_leds();
}

void loop() {
  delay(DELAY_START);
  fade_down(LED_B);
  delay(100);
  blink_off(LED_A);
  blink_off(LED_A);
  blink_off(LED_A);
  fade_up(LED_C, 5);

  initial_leds();
  delay(RESTART_DELAY);
}
