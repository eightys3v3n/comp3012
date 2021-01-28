const int LED_A = 9;
const int LED_B = 10;
const int LED_C = 11;
const int DELAY_WAIT = 500;
const int FADE_INC = 10;
const int FADE_DELAY = 100;
const int BLINK_DELAY = 500;
const int RESTART_DELAY = 4000;

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

void fade_down(int id, int inc=FADE_INC) {
  fade(id, 255, 0, inc=inc);
}

void fade_up(int id, int inc=FADE_INC) {
  fade(id, 0, 255, inc=inc);
}

void blink_off(int id, int wait=BLINK_DELAY) {
  digitalWrite(id, LOW);
  delay(wait);
  digitalWrite(id, HIGH);
  delay(wait);
}


void initial_leds() {
  digitalWrite(LED_A, HIGH);
  digitalWrite(LED_B, HIGH);
  digitalWrite(LED_C, LOW);
}


void setup() {
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_C, OUTPUT);

  initial_leds();
}


void loop() {
  delay(DELAY_WAIT);
  fade_down(LED_B);
  delay(100);
  blink_off(LED_A);
  blink_off(LED_A);
  blink_off(LED_A);
  fade_up(LED_C, 5);

  delay(RESTART_DELAY);
  initial_leds();
}
