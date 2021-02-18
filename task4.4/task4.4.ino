/* Author: Terrence Plunkett
 * Date: 2021-02-18
 *
 * Utilizes a DHT11 humidity sensor, an LCD display, an IR receiver and remote, and a 10K potentiometer.
 * Potentiometer is used to control LCD contrast, LCD displays DHT11 readings, and IR remote turns on and off the LCD.
 */

// automated timer based events
#include <Automaton.h>

// Usage of DHT sensor
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>

// IR Receiver
#include <IRremote.h>

// LCD Display
#include <Adafruit_LiquidCrystal.h>

// Servo
//#include "libraries/Atm_servo.h"


// Global Variables
// DHT Sensor
#define DHT_PIN 2
#define DHTTYPE DHT11
DHT_Unified DHTDevice(DHT_PIN, DHTTYPE);
sensor_t HumidSensor, TempSensor;
Atm_timer DHTTimer;
double humidity, temperature;

// IR Receiver
#define IR_PIN 3
#define IR_CHECK_INTERVAL 600 // check for received codes every this many ms
Atm_timer IRTimer;
long last_press_time = 0; // time when button was last pressed

// LCD Display
#define LCD_RS 13
#define LCD_E  12
#define LCD_D4 11
#define LCD_D5 10
#define LCD_D6 9
#define LCD_D7 8
#define LCD_CONTRAST 6
#define LCD_CONTRAST_INPUT A0
#define LCD_UPDATE_INTERVAL 2000
#define LCD_CONTRAST_UPDATE_INTERVAL 400 // contrast updating
Adafruit_LiquidCrystal LCD(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
bool lcd_on = true;
byte lcd_contrast = 0;
Atm_timer LCDTimer, ContrastTimer;


// Configure pin modes and schedule callbacks
void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  LCDSetup();
  IRSetup();
  DHTSetup();

  Serial.println("Done");
}

// do call backs via automaton
void loop() {
  automaton.run();
}


// Start the LCD display and associated timer based callbacks
void LCDSetup() {
  pinMode(LCD_CONTRAST, OUTPUT);
  pinMode(LCD_CONTRAST_INPUT, INPUT);

  LCD.begin(16,2);
  LCD.noBlink();
  LCD.noCursor();

  Serial.print("  Updating LCD every "); Serial.print(LCD_UPDATE_INTERVAL); Serial.println("ms");
  LCDTimer.begin(LCD_UPDATE_INTERVAL)
    .repeat(-1)
    .onTimer(LCDUpdate)
    .start();
  Serial.print("  Updating LCD Contrast every "); Serial.print(LCD_CONTRAST_UPDATE_INTERVAL); Serial.println("ms");
  ContrastTimer.begin(LCD_CONTRAST_UPDATE_INTERVAL)
    .repeat(-1)
    .onTimer(ContrastUpdate)
    .start();
}

void ContrastUpdate() {
  // Set contrast
  lcd_contrast = map(analogRead(LCD_CONTRAST_INPUT), 0, 1023, 0, 255);
  analogWrite(LCD_CONTRAST, lcd_contrast);
}

// Writes the temperature and humidity out to the display
void LCDUpdate() {
  LCD.setCursor(0, 0);
  LCD.print("                ");
  LCD.setCursor(0, 0);
  LCD.print("Humid ");
  LCD.print(String(humidity));
  LCD.setCursor(0, 1);
  LCD.print("                ");
  LCD.setCursor(0, 1);
  LCD.print("Temp ");
  LCD.print(String(temperature));
  LCD.setCursor(15,1);
}

// turns on or off the LCD display (doesn't turn off the backlight)
void toggleLCD() {
  if (lcd_on) {
    LCD.noDisplay();
    lcd_on = false;
  } else {
    LCD.display();
    lcd_on = true;
  }
}


// Start the IR receiver and schedule timed callback
void IRSetup() {
  IrReceiver.begin(IR_PIN);

  Serial.print("  Checking IR Sensor every "); Serial.print(IR_CHECK_INTERVAL); Serial.println("ms");
  IRTimer.begin(IR_CHECK_INTERVAL)
    .repeat(-1)
    .onTimer(IRUpdate)
    .start();
}

// Check for IR data and act on it
void IRUpdate(int idx, int v, int up) {
  if (IrReceiver.decode()) {
    switch (IrReceiver.decodedIRData.command) {
    case 0x44: // test
      Serial.println("Pressed test");
      break;
    
    case 69: // power
      toggleLCD();
      break;

    default:
      Serial.print("Pressed unused button "); Serial.println(IrReceiver.decodedIRData.command);
    }

    IrReceiver.resume();
  }
}


// Starts the DHT sensor and schedules a callback to save the value
void DHTSetup() {
  pinMode(DHT_PIN, INPUT);
  
  DHTDevice.begin();
  DHTDevice.humidity().getSensor(&HumidSensor);

  Serial.print("  Updating DHT Sensor every "); Serial.print(HumidSensor.min_delay / 1000); Serial.println("ms");
  DHTTimer.begin(HumidSensor.min_delay / 1000)
    .repeat(-1)
    .onTimer(DHTUpdate)
    .start();
}

// Retrieves the humitity and temperature data from the DHTDevice and saves to global variables
void DHTUpdate(int idx, int v, int up) {
  sensors_event_t event;
  DHTDevice.humidity().getEvent(&event);

  if (!isnan(event.relative_humidity)) {
    humidity = event.relative_humidity;
    //Serial.print("Updated humidity sensor "); Serial.println(humidity);
  } else {
    //Serial.println("Failed to read humidity sensor");
  }

  DHTDevice.temperature().getEvent(&event);

  if (!isnan(event.temperature)) {
    temperature = event.temperature;
    // Serial.print("Updated temperature sensor "); Serial.println(humidity);
  } else {
    // Serial.println("Failed to read temperature sensor");
  }
}