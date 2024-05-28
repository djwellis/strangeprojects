#include <TM1637Display.h>  // For the 4-digit display
#include <LedControl.h>     // Needed for the 8-digit display


// 4-digit display --------------------------------------------------------------------------------------------------
const int CLK = 13; //Set the CLK pin connection to the display
const int DIO = 12; //Set the DIO pin connection to the display
TM1637Display display(CLK, DIO);  //set up the 4-Digit Display.




// 8-digit display --------------------------------------------------------------------------------------------------
LedControl display1=LedControl(45,47,46,1);
/*
 * This is the command to setup the display we use to INPUT a code into the system
 * Pin 45 = DIO, on my arduino that pin is labelled as 45
 * Pin 47 = CLK, on my arduino that pin is labelled as 47
 * Pin 46 = LOAD/CS, on my arduino that pin is labelled as 46
 * 1 = Not a pin at all, this is the number of MAX72XX boards connected apparently.
 */


// All of the LEDs --------------------------------------------------------------------------------------------------
int Button1LED = 14;  // First sequence of switches - LED
int Button2LED = 15;  // Second sequence of switches - LED
int Button3LED = 16;  // Third sequence of switches - LED
int ConnectorLED = 17;  // First sequence of switches - LED
int Sequence1LED = 21;  // First sequence of switches - LED
int Sequence2LED = 19;  // Second sequence of switches - LED
int Sequence3LED = 20;  // Third sequence of switches - LED


// Analogue inputs --------------------------------------------------------------------------------------------------
int tensPin = A7;       // The potentiometer for the tens
int hundredsPin = A6;   // The potentiometer for the hundreds


// Digital input pins -----------------------------------------------------------------------------------------------
int sequence1Pin = 38;  // First sequence of switches connect to this pin
int sequence2Pin = 39;  // Second sequence of switches connect to this pin
int sequence3Pin = 40;  // Third sequence of switches connect to this pin
int button1Pin = 9;  // Button on the left (green)
int button2Pin = 10;  // Button in the middle (red)
int button3Pin = 11;  // Button on the right (green)
int cat5Pin = 42;        // The rj45 socket connects here (contiuity circuit)
int defusePin = 6;      // The big red defuse button connects here
int piezoPin = 30;       // This is the pin the buzzer connects to



// Input statuses ---------------------------------------------------------------------------------------------------
int Button1=0;
int Button2=0;
int Button3=0;
int Row1=0;
int Row2=0;
int Row3=0;
int theSocket=0;
int defuseButton=0;


// The ABC puzzle ---------------------------------------------------------------------------------------------------
bool stateA=false;
bool stateB=false;
bool stateC=false;


// The big countdown!! -----------------------------------------------------------------------------------------------
long countdown=17500; // With a 50ms delay on the main loop, this provides about 1 minute per 900
bool systemArmed=true;
int countbeep=0; // We will increment this every countdown count,  when it hits 10 we will beep and then reset it to 0
bool systemDetonated=false; // If this is true, then the timer clearly ran out!

// These are all of the calculations and such for the two potentiometers that operate the 4-digit display ------------
long interimValue;
int sensorTens;
int sensorHundreds;
int finalCode;
int sensorValue = 0;
int potsCode = 0;

// And these are for the averaging filter used in the two potentiometers (because they are prone to error!) ----------
int sum=0;
int val=0;


int buttonState = 0;










// SETUP ------------------------------------------------------------------------------------------------------------ SETUP

void setup()
{
  Serial.begin(9600); 
  display.setBrightness(3);  //set the diplay to maximum brightness


  // Setup 8-digit display
  display1.shutdown(0, false);
  display1.setIntensity(0,8);
  displayDashes();
  
  // Setup the LED pins
  pinMode(Sequence1LED, OUTPUT);
  pinMode(Sequence2LED, OUTPUT);
  pinMode(Sequence3LED, OUTPUT);
  pinMode(ConnectorLED, OUTPUT);
  pinMode(Button1LED, OUTPUT);
  pinMode(Button2LED, OUTPUT);
  pinMode(Button3LED, OUTPUT);

  pinMode(sequence1Pin, INPUT_PULLUP);
  pinMode(sequence2Pin, INPUT_PULLUP);
  pinMode(sequence3Pin, INPUT_PULLUP);
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  pinMode(cat5Pin, INPUT_PULLUP);
  pinMode(defusePin, INPUT_PULLUP);
}








// LOOP ------------------------------------------------------------------------------------------------------------- LOOP



void loop(){

  if (systemDetonated==false){
    // If it hasn't detonated yet, then all of the below is true!
    
      readButtons();
      potsCode = potsDisplay();
      
      printNumber(countdown);
      
      if (systemArmed==true) {
        if (countdown==0){
          systemDetonated=true;
          systemArmed=false;
          tone(piezoPin, 400, 3000);
        }
        countdown--;
        countbeep++;
      }
    
      //display.showNumberDec(0);
    
      if(countdown<1000){
          if (countbeep==50){
            countbeep=0;
            tone(piezoPin, 400, 5);
          }
        } else {
          if (countbeep==100){
            countbeep=0;
            tone(piezoPin, 400, 5);
          }
        }
    
    
      // This is the ABC puzzle --------------------------
      // MIDDLE, LEFT, RIGHT, RIGHT
      if (Button1==LOW) {
        if(potsCode==5060)
        {
          stateA = stateA;
          stateB = true;
          stateC = false;
        } else {
          badABC();
        }
        abcPuzzle();
      }
    
      if (Button2== LOW) {
        if(potsCode==5060) 
        {
          stateA = !stateB;
          stateB = stateB;
          stateC = stateC;
        } else {
          badABC();
        }
        abcPuzzle();
      }
    
      if (Button3== LOW) {
        if (potsCode==5060) {
          
          if (stateC==true){
            stateB = true;
          } else {
            stateB = false;
            stateA = stateA;
          }
          stateC = true;
        } else {
          badABC();
        }
          abcPuzzle();
      }
      // End of the ABC puzzle --------------------------
    
      
      // Rows of switches -------------------------------
      // DOWN, DOWN, UP, DOWN, UP
      if (Row1== LOW) {
        digitalWrite(Sequence1LED, HIGH);
      } else {
        digitalWrite(Sequence1LED, LOW);
      }
    
      // UP, DOWN, DOWN, DOWN, DOWN
      if (Row2== LOW) {
        digitalWrite(Sequence2LED, HIGH);
      } else {
        digitalWrite(Sequence2LED, LOW);
      }
    
      // DOWN, UP, UP, UP, DOWN
      if (Row3== LOW) {
        digitalWrite(Sequence3LED, HIGH);
      } else {
        digitalWrite(Sequence3LED, LOW);
      }
      // End rows of switches ---------------------------
    
    
      // Check the RJ45 socket --------------------------
      if (theSocket== LOW) {
        digitalWrite(ConnectorLED, HIGH);
      } else {
        digitalWrite(ConnectorLED, LOW);
      }
      // Done checking the RJ45 socket ------------------
    
    
      // Check the Big red button -----------------------
      if ((defuseButton== LOW) && (theSocket== LOW) && (Row1== LOW) && (Row2== LOW) && (Row3== LOW) && (stateA==true) && (stateB==true) && (stateC==true)  && (potsCode==4560)) {
        // Defused!
        tone(piezoPin, 250, 50);
        systemArmed=false;
      }
      // Done checking the big red button --------------

  // The else statement below relates to "is it detonated" - if it hasn't, then the above is true...  If it *has* detonated then the next bit happens.
  } else {
    displayDetonate();
  }
}

void badABC(){
// This is called when button A, B or C is pressed and the combination is not 5060

display.showNumberDec(404);
delay(500);
stateA=false;
stateB=false;
stateC=false;
}

void readButtons(){
  Button1=digitalRead(button1Pin);
  Button2=digitalRead(button2Pin);
  Button3=digitalRead(button3Pin);
  Row1=digitalRead(sequence1Pin);
  Row2=digitalRead(sequence2Pin);
  Row3=digitalRead(sequence3Pin);
  theSocket=digitalRead(cat5Pin);
  defuseButton=digitalRead(defusePin);
  Serial.println(Button1);
  Serial.println(Button2);
  Serial.println(Button3);
  Serial.println(Row1);
  Serial.println(Row2);
  Serial.println(Row3);
  Serial.println(theSocket);
  Serial.println(defuseButton);
}






int potsDisplay(){
  // This is the function that handles the 4-digit display with the 4 potentiometers

  // First, call the functions that reds the tens and the hundreds
  sensorTens=readTheTens();
  sensorHundreds=readTheHundreds();
  
  // Now, add it all up and put it on display screen
  finalCode=sensorHundreds+sensorTens;
  display.showNumberDec(finalCode);

  // Now, return the number to whatever called this function
  return finalCode;
}

int readTheTens(){
      sensorValue = analogRead(tensPin);
    
      // Averaging filter
      sum = 0;
      for (uint8_t i=0; i< 16; i++) sum += sensorValue;
      val = sum / 16;
      
      return map(sensorValue,0,1023,0,99);
}

int readTheHundreds(){
      sensorValue = analogRead(hundredsPin);
    
      // Averaging filter
      sum = 0;
      for (uint8_t i=0; i< 16; i++) sum += sensorValue;
      val = sum / 16;
      
      sensorHundreds = map(val,0,1023,0,99);
      return sensorHundreds*100;
}


void displayDashes(){
  display1.setRow(0,7,B00000001);
  display1.setRow(0,6,B00000001);
  display1.setRow(0,5,B00000001);
  display1.setRow(0,4,B00000001);
  display1.setRow(0,3,B00000001);
  display1.setRow(0,2,B00000001);
  display1.setRow(0,1,B00000001);
  display1.setRow(0,0,B00000001);
}

void displayDetonate(){
  display1.setRow(0,7,B00111101); //D
  display1.setRow(0,6,B01001111); //E
  display1.setRow(0,5,B00001111); //T
  display1.setRow(0,4,B01111110); //O
  display1.setRow(0,3,B01110110); //N
  display1.setRow(0,2,B01110111); //A
  display1.setRow(0,1,B00001111); //T
  display1.setRow(0,0,B01001111); //E
}


void printNumber(long v){
  int ones;
  int tens;
  int hundreds;
  int thousands;
  int tenthousands;
  int hundredthousands;
  int millions;
  int tenmillions;
  
  if(v < 100000000){
    ones=v%10;
    v=v/10;
    tens=v%10;
    v=v/10;
    hundreds=v%10;
    v=v/10;
    thousands=v%10;
    v=v/10;
    tenthousands=v%10;
    v=v/10;
    hundredthousands=v%10;
    v=v/10;
    millions=v%10;
    v=v/10;
    tenmillions=v;
    

    //Now print the number digit by digit
    
    display1.setRow(0,7,B00001111); //T
    display1.setRow(0,6,B00000001); //-
    display1.setRow(0,5,B00000000); //BLANK
    //display1.setDigit(0,7,(byte)tenmillions,false);
    //display1.setDigit(0,6,(byte)millions,false);
    //display1.setDigit(0,5,(byte)hundredthousands,false);
    display1.setDigit(0,4,(byte)tenthousands,false);
    display1.setDigit(0,3,(byte)thousands,false);
    display1.setDigit(0,2,(byte)hundreds,false);
    display1.setDigit(0,1,(byte)tens,false);
    display1.setDigit(0,0,(byte)ones,false);
  } else {
    display1.setChar(0,0,'E',false);
  }
}



void abcPuzzle(){
  
  delay(150);
  
  if (stateA==true)
  {
    digitalWrite(Button1LED, HIGH);
  } else {
    digitalWrite(Button1LED, LOW);
  }


  if (stateB==true)
  {
    digitalWrite(Button2LED, HIGH);
  } else {
    digitalWrite(Button2LED, LOW);
  }

  
  if (stateC==true)
  {
    digitalWrite(Button3LED, HIGH);
  } else {
    digitalWrite(Button3LED, LOW);
  }
  
}
