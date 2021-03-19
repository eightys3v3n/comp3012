/*===============================================================================
NOTE: Keys must be pressed with a slight delay, do not press the keys fast or 
the device int he simulator will nto work properly

Oprn serial monitor to see output

Starts in the Locked state

  (1) RED led will be on in locked state (Servo will be at 90 deg)
  (2) GREEN led will be on in unlocked state (Servo will be at 0 deg)
  (3) all three leds will cycle in hypnotic state
  (4) When locked if 1324 has been enetred then the machine will
      go inot unlocked mode. (Servo will be at previouse state from transition)
  
  (5) When unlocked if 4231 has been enetred then the machine will
      go into locked mode
  
  (6) At any time if 1234 is entered the machine will go into hypnotic state
  (7) in hypnotic state if 2 is enetred machine will go into locked state
  (8) in hypnotic state if 3 is enetred machine will go into locked state 
 
 
 Keypad library: https://playground.arduino.cc/Code/Keypad/
 ===============================================================================*/
#include <Keypad.h>
#include <Servo.h>

//----------------------------------------------------------------
//Keypad info
//Map for key press to codes for out program
//in this case it matches to the actual key pad
char keys[4][4] = {
  {'1', '2', '3','A'},  
  {'4', '5', '6','B'},  
  {'7', '8', '9','C'},  
  {'*', '0', '#','D'}
};
//note can not have a row or col on pin 13 for this to work
byte rowPins[4] = {12,11,10,9}; //first 4 lines from left side
byte colPins[4] = {8,7,6,5}; // next 4 lines 
byte potPin = A5;

Keypad myKeypad = Keypad(makeKeymap(keys), rowPins, colPins,4,4);

const int BUFFER_SIZE = 20; //max number of key presses that 
                             //can be stored at one time

char keyBuffer[BUFFER_SIZE]; //array used to store queue of keys pressed
int front = -1, rear = -1;
int numKeysInQueue = 0;


//---------------------------------------------------
//State machine info
//All states
enum State 
{ 
 LOCKED, 
 UNLOCKED,
 HYPNOTIC,
 SWEEP
};
//For debugging and easier readign states
//Enter the string that should be printed for a state
//in the sanme order as the ones from above
char* stateNames[] = 
{
 "Locked",
 "Unlocked",
 "Hypnotic",
 "Sweep"
};
boolean debug = true; // set to false to not show debug messages

boolean printKeyBuffer = true; //false to not show key buffer 

State currentState = LOCKED; // Set start state
//----------------------------------------------------------------
//Servo info
const int SERVO_PIN = 3;
Servo lockServo; 
//----------------------------------------------------------------
//Lights Info
const int LED_r = 2;
const int LED_g = 4;
const int LED_w = 13;

//----------------------------------------------------------------
void setup()
{
  Serial.begin(9600);
  printDebugMessage("Machine starting up");
  
  pinMode(LED_r, OUTPUT);
  pinMode(LED_g, OUTPUT);
  pinMode(LED_w, OUTPUT);
  pinMode(potPin, INPUT);
  
  lockServo.attach( SERVO_PIN );
}
//----------------------------------------------------------------
void loop()
{ 
  saveAnyKeyPress();
  
  //run one step of current state
  runStateMachine();
  
  delay(100); //here justy to mae sure simulation does nto go crazy 
             //if things are runing too fast
}
//---------------------------------------------------
void printDebugMessage(char* msg)
{
  if(debug)
  {
    unsigned long myTime = millis();
    Serial.print(myTime);
    Serial.print(": (");
    Serial.print( stateNames[ currentState ] );
    Serial.print(") ");
    Serial.println(msg);
  }
}
void printDebugMessage(char msg) //overloaded function
{
  if(debug)
  {
    unsigned long myTime = millis();
    Serial.print(myTime);
    Serial.print(": (");
    Serial.print( stateNames[ currentState ] );
    Serial.print(") ");
    Serial.println(msg);
  }
}
//---------------------------------------------------
void runStateMachine()
{
  switch(currentState)
  {
   case LOCKED: 
    runLockedState();
    break; 
   case UNLOCKED: 
    runUnlockedState();
    break;
   case HYPNOTIC: 
    runHypnoticState();
    break;
   case SWEEP:
    runSweepState();
    break;
  }
}
//----------------------------------
boolean locked_first_run = false;

void runLockedState()
{
  //to not waste calls
  //this code is only run the first time the state is enetred
  if(locked_first_run == false)
  {
    lockDevice();
    digitalWrite(LED_r, HIGH);
    digitalWrite(LED_g, LOW);
    digitalWrite(LED_w, LOW);
    locked_first_run = true;
  }
  
  //check for transitions
  String sequence = getSequenceofKeys(4);
  short pot = analogRead(potPin);
  
  //check to see if we have 4 keypresses yet for code
  if(sequence.length() > 0)
  {
  
    //check for proper code 
    if(sequence == String("1234")) //hypnotic state
    {
      printDebugMessage("Changing to HYPNOTIC state");
      currentState = HYPNOTIC;
      locked_first_run = false; //reset for next time locked state is run
    }
    else if(sequence == String("1324") && pot <= 44 ) //unlock state
    {
      printDebugMessage("Changing to UNLOCKED state");
      currentState = UNLOCKED;
      locked_first_run = false; //reset for next time locked state is run
    }
    else 
    {
      //unkown code, do nothing. Code ehre just for debugging
      printDebugMessage("Unknown code and potentiometer reading");
      printDebugMessage((char*) sequence.c_str());
      printDebugMessage((char*) String(pot).c_str());
    }
  } else if(91 <= pot && pot <= 110 ) //sweep state
  {
    printDebugMessage("Changing to SWEEP state");
    currentState = SWEEP;
    locked_first_run = false; //reset for next time locked state is run
  }
}
//----------------------------------
boolean unlocked_first_run = false;
void runUnlockedState()
{
   //to not waste calls
  //this code is only run the first time the state is enetred
  if(unlocked_first_run == false)
  {
    unLockDevice();
    digitalWrite(LED_r, LOW);
    digitalWrite(LED_g, HIGH);
    digitalWrite(LED_w, LOW);
    unlocked_first_run = true;
  }
  
  
  //check for transitions
  String sequence = getSequenceofKeys(4);
  short pot = analogRead(potPin);
  
  //check to see if we have 4 keypresses yet for code
  if(sequence.length() > 0)
  {
    //check for proper code 
    if(sequence == String("1234")) //hypnotic state
    {
      printDebugMessage("Changing to HYPNOTIC state");
      currentState = HYPNOTIC;
      unlocked_first_run = false; //reset for next time unlocked state is run
    }
    else if(sequence == String("4231") && 45 <= pot && pot <= 90) //lock state
    {
      printDebugMessage("Changing to LOCKED state");
      currentState = LOCKED;
      unlocked_first_run = false; //reset for next time unlocked state is run
    }
    else 
    {
      //unkown code, do nothing. Code ehre just for debugging
      printDebugMessage("Unknown code and potentiometer reading");
      printDebugMessage((char*) sequence.c_str());
      printDebugMessage((char*) String(pot).c_str());
    }
  }  
}
//----------------------------------
int currentLED = 0;

void runHypnoticState()
{
  
  switch(currentLED)
  {
    case 0:  //RED LED ON
     digitalWrite(LED_r, HIGH);
     digitalWrite(LED_g, LOW);
     digitalWrite(LED_w, LOW);
     break;
    case 1:  //GREEN LED ON
     digitalWrite(LED_r, LOW);
     digitalWrite(LED_g, HIGH);
     digitalWrite(LED_w, LOW);
     break;
    case 2:  //WHITE LED ON    
     digitalWrite(LED_r, LOW);
     digitalWrite(LED_g, LOW);
     digitalWrite(LED_w, HIGH);
     break; 
  }
  
  currentLED = (currentLED + 1) % 3;
  delay(100);
  
    //check for transitions
  String sequence = getSequenceofKeys(1);
  
  //check to see if we have 4 keypresses yet for code
  if(sequence.length() > 0)
  {
    //check for proper code 
    if(sequence == String("2")) //lock state
    {
      printDebugMessage("Changing to LOCKED state");
      currentState = LOCKED;
      currentLED = 0; //reset for next entry of hypnotic state
    }
    else if(sequence == String("3")) //unlock state
     {
      printDebugMessage("Changing to UNLOCKED state");
      currentState = UNLOCKED;
      currentLED = 0; //reset for next entry of hypnotic state
    }
    else if(sequence == String("4")) //sweep state
     {
      printDebugMessage("Changing to SWEEP state");
      currentState = SWEEP;
      currentLED = 0; //reset for next entry of hypnotic state
    }
    else 
    {
      //unkown code, do nothing. Code ehre just for debugging
      printDebugMessage("Unknown code");
      printDebugMessage((char*) sequence.c_str());
    }
    
  }  
}
//----------------------------------------------------------------
// I assume that "bliinking at the state time continuously" means they
// blink on every state check loop rather than slower like hypnotic mode.
// If this was not the intention I would add a lastToggle variable that
// would store the time the lights mode was switched. Then if the difference
// between that and the current time was > a constant defined at the top,
// toggle the light states.
bool ledState = false;

void runSweepState() {
  if (ledState) {
    digitalWrite(LED_r, LOW);
    digitalWrite(LED_g, LOW);
  } else {
    digitalWrite(LED_r, HIGH);
    digitalWrite(LED_g, HIGH);
  }
  ledState = !ledState;
  
  //check for transitions
  String sequence = getSequenceofKeys(4);
  
  //check to see if we have 4 keypresses yet for code
  if(sequence.length() > 0)
  {
    //check for proper code 
    if(sequence == String("4231")) //lock state
    {
      printDebugMessage("Changing to LOCKED state");
      currentState = LOCKED;
    }
    else if(sequence == String("4321")) //hypnotic state
     {
      printDebugMessage("Changing to HYPNOTIC state");
      currentState = HYPNOTIC;
    }
    else 
    {
      //unkown code, do nothing. Code here just for debugging
      printDebugMessage("Unknown code");
      printDebugMessage((char*) sequence.c_str());
    }
  } 
}
//----------------------------------------------------------------
//Lock mecanism code, just a basic servo here
void lockDevice()
{
 lockServo.write(90); 
 delay(15);
}
//----------------------------
void unLockDevice()
{
 lockServo.write(0); 
 delay(15);
}

//----------------------------------------------------------------
//Keypad functions
//----------------------------------------------------------------
void saveAnyKeyPress()
{
   char key = myKeypad.getKey();
   if(key)
   {
      String msg = "Adding key: ";
      msg = msg + key;
      printDebugMessage((char*) msg.c_str());
      enQueueKeyPress(key);
     
      if(printKeyBuffer)
          printOutCurrentQueue();
   }
}
//----------------------------------------------------------------
//Returns numkeys seequence of keypresses form the internall keyBuffer
//if there are nto enough keyprssses yet, it will return a string of length 
//zero
String getSequenceofKeys(int numKeys)
{
  String sequence = "";
  
  if(numKeys > 0 && numKeys <= numKeysInQueue)
  {
     for(int keys = 0; keys < numKeys; keys++)
       sequence.concat( deQueueKeyPress() );
    
  }
  
  return sequence;
}
//----------------------------------------------------------------
//Functions for keypad buffer that stores presses in a 
//circular queue
//for more info see: https://www.programiz.com/dsa/circular-queue
// the following code was refereced from the website from above
// Check if the queue is full
//------------------------------------------
int isFull() {
  if ( (front == rear + 1) 
       || (front == 0 && rear == BUFFER_SIZE - 1)
     ) 
     return 1;
  
  return 0;
}
//------------------------------------------
// Check if the queue is empty
int isEmpty() {
  if (front == -1) return 1;
  return 0;
}
//------------------------------------------
// Adding an element
void enQueueKeyPress(char element) {
  if (isFull())
    Serial.println("\n Keypad Queue is full!! \n");
  else {
    if (front == -1) front = 0;
    rear = (rear + 1) % BUFFER_SIZE;
    keyBuffer[rear] = element;
    numKeysInQueue++;
  }
}
//------------------------------------------
// Removing an element
char deQueueKeyPress() {
  char element;
  if (isEmpty()) return (-1);
  else 
  {
    element = keyBuffer[front];
   
    if (front == rear) { //Queue is now empty
      front = -1;
      rear = -1;
    } 
    else 
      front = (front + 1) % BUFFER_SIZE;
   
    numKeysInQueue--;
    return (element);
  }
}
//----------------------------------------------------------------
void printOutCurrentQueue()
{
  if(debug)
  {
    String msg = "keyBuffer";
  
    if(isEmpty())
      msg = "Queue is empty";
    else
    {
      int i =0;
      for (i = front; i != rear; i = (i + 1) % BUFFER_SIZE) 
          msg = msg + "[" +  keyBuffer[i] + "]"; 
      
      msg = msg + "[" +  keyBuffer[i] + "]"; //last element
    } 
    
    printDebugMessage((char*) msg.c_str() );
  }
}
