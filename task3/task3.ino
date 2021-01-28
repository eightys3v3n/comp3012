const int LED_A = 9;
const int LED_B = 10;
const int LED_C = 11;
const int DELAY_WAIT = 500;
const int FADE_INC = 10;
const int FADE_DELAY = 100;
const int BLINK_DELAY = 500;
const int RESTART_DELAY = 4000;

void fade_down(int id, int inc=FADE_INC, int wait=FADE_DELAY) {
  
}

void fade_up(int id, int inc=FADE_INC, int wait=FADE_DELAY) {

}

void blink_off(int id, int wait=BLINK_DELAY) {
  digitalWrite(id, LOW);
  delay(wait);
  digitalWrite(id, HIGH);
  delay(wait);
}

void setup() {
  pinMode(LED_A, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(LED_C, OUTPUT);

  digitalWrite(LED_A, HIGH);
  digitalWrite(LED_B, HIGH);
  digitalWrite(LED_C, LOW);
}


void loop() {
  delay(DELAY_WAIT);
  fade_down(LED_B);
  blink_off(LED_A);
  blink_off(LED_A);
  blink_off(LED_A);
  fade_up(LED_C);

  delay(RESTART_DELAY);
}
