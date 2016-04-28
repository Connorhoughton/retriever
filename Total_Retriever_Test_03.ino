//Libraries
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <avr/eeprom.h>
#include <Keypad.h>

//Keypad Setup
const byte ROWS = 4; // Four rows
const byte COLS = 4; // Three columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 22, 24, 26, 28 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 30, 32, 34, 36 };

// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
//End Keypad Setup

//LCD Setup
#define I2C_ADDR    0x27  // Define I2C Address where the PCF8574A is
#define BACKLIGHT_PIN     3
#define En_pin            2
#define Rw_pin            1
#define Rs_pin            0
#define D4_pin            4
#define D5_pin            5
#define D6_pin            6
#define D7_pin            7


LiquidCrystal_I2C  lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);
//End LCD Setup

//More Digital Pin Definitions
#define MagSensor     12
#define LED           13
//Define VFD control pins
#define pwmOut        10
#define pinREV        9
#define pinFWD        8

// Button State Variables (Will probably change when keypad arrives)
byte ForwardState;             // the current reading from the input pin
byte BackwardState;
byte OnState;
byte HomeState;
byte JogState;
byte MagState;
byte lastForwardState   = LOW;   // the previous reading from the input pin
byte lastBackwardState  = LOW;
byte lastOnState        = LOW;
byte lastHomeState      = LOW;
byte lastJogState       = LOW;
byte lastMagState       = LOW;

long lastDebounceTime   = 0;  // the last time the output pin was toggled
long debounceDelay      = 50;    // the debounce time; increase if the output flickers

//Global Button State Variable
byte toggleState;

//Counter Variables
byte pageCounter        = 0;
int MagCounter          = 0;

//HMI Input Variables
char slowDistanceA[3]   = "75";
char val[5];
char slowSpeedA[3]      = "5";
char lastLoc[5]         = "100";
char firstLoc[5]        = "1";

char password[5]        = " ";
int enterLoc            = 1;
char pass[5]            = "0000";
char defaultPassword[5] = "1111";
char newPassword[5]     = " ";
char newLoc[5]          = "";
bool boolNewLocation    = false;

byte i = 0, z = 0, n = 0, k = 0, y = 0;
byte ForwardPressed, BackwardPressed, OnPressed, HomePressed, JogPressed;
byte MagSensed;
byte useQueueVal             = 0;
byte oneTime                 = 0;

//Encoder Variables
int A_SIG = 0, B_SIG = 1;

String LengthString          = "Length: ";
bool boolHome                = false;

//RESET TOTALENGTH TO 0
long TotalLength             = 1000;
long pickupPoint             = 0;
long pulses                  = 0;

//int DistanceA              = 65;
//byte speedA                = 5;
int lastLocation             = 100;
int firstLocation            = 1;
float hookSpacing            = 0;
int newLocation              = 0;
int tempLocation             = 0;
long locationCalculation     = 0;
long lastLocationCalculation = 0;
long totalPulses             = 0;

const int ringsize           = 11;
int head                     = 0;
int tail                     = 0;
int ring[ringsize];
byte q                       = 0;
int queueVal                 = 0;
byte returnHome              = 0;
char queueValue[5]           = "";

//byte  eepromInts[4] = {DistanceA, speedA, lastLocation, firstLocation};
bool boolPickup              = false;
bool boolEmptyQueue          = false;
bool boolTurnOff             = false;
bool boolShortest            = false;
bool boolOrdered             = false;
bool boolUsingQueue          = false;
bool boolRunVFD              = false;
bool isValidNum              = false;
bool boolRepeatVal           = false;
bool boolFullBuffer          = false;

int distA                    = 0;
int distanceA                = 0;
int spdA                     = 0;
int speedA                   = 0;
int moveLocation             = 0;

//temporary
int m = 0;
void setup()
{
  pageCounter = 11;
  pinMode(pwmOut, OUTPUT);
  pinMode(pinFWD, OUTPUT);
  pinMode(pinREV, OUTPUT);
  /*
    //Read settings from memory
    eeprom_read_block((void*)&TotalLength, (void*)0, 3);
    eeprom_read_block((void*)&pickupPoint, (void*)3, 3);
    DistanceA = eeprom_read_byte((unsigned char*)6);
    speedA = eeprom_read_byte((unsigned char*)7);
    lastLocation = eeprom_read_byte((unsigned char*)8);
    firstLocation = eeprom_read_byte((unsigned char*)9); */


  Serial.begin(19200);
  // Serial.println(defaultPassword);

  lcd.begin (20, 4);
  pinMode(LED, OUTPUT);
  //Interrupts for encoder
  attachInterrupt(0, A_RISE, RISING);
  attachInterrupt(1, B_RISE, RISING);

  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();                   // go home
  lcd.setCursor ( 0, 2 );        // go to the 2nd line, 8th row
  lcd.print(F("        OFF         "));
}

void loop() { //In the loop I need to set a flag that will be saved when a user enters setup for the first time. If the flag is set
  //then power up straight to run screen so user does not have to go through all of the pages again.

  //Local Variables
  bool boolJog =  false;

  //Check to see which button is pressed
  LookForPress();
  //If On/Off Button is pressed call a function
  /*
    if (OnPressed == HIGH) {
      eeprom_write_block((const void*)&TotalLength, (void*)0, 3);
      eeprom_write_block((const void*)&pickupPoint, (void*)3, 3);
      eeprom_write_byte((unsigned char*)6, DistanceA);
      eeprom_write_byte((unsigned char*)7, speedA);
      eeprom_write_byte((unsigned char*)8, lastLocation);
      eeprom_write_byte((unsigned char*)9, firstLocation);
      Serial.println(TotalLength);
      Serial.println(pickupPoint);

      //This function returns lcd to off screen
      OnButtonPressed();
    } */
  //Updates the screen depending on which button is pressed
  HMI_Screens();

  ///////////////////////////////////////////////////////Start of Jog Feature////////////////////////////////////////////////////////

  if (pageCounter == 0) {
    char key = kpd.getKey();
    if (key == 'D') // Check for a valid key.
    {
      boolJog = !boolJog;
    }
    while (boolJog == true) {
      //Turn on VFD @ roughly 30Hz
      digitalWrite(pinFWD, HIGH);
      digitalWrite(LED, HIGH);
      analogWrite(pwmOut, 127);  // about 2.5 volts

      // digitalWrite(LED, HIGH);
      char key = kpd.getKey();
      MagSensed = debounceMag(MagSensor, toggleState);
      if (key == 'D') {
        //Turn off VFD
        digitalWrite(pinFWD, LOW);
        digitalWrite(LED, LOW);
        analogWrite(pwmOut, 0);

        digitalWrite(LED, LOW);
        break;
      }
      if (MagSensed == HIGH || pulses == TotalLength) {
        MagCounter ++;
        pulses = 0;
        Serial.println(pulses);
      }
    }
  }
  ////////////////////////////////////////////////////////End of Jog Feature/////////////////////////////////////////////////////////
}//End of Void Loop


///////////////////////////////////////////////////////Start of Functions//////////////////////////////////////////////////////////

void LookForPress() {
  char key = kpd.getKey();
  if (key) {

    if (key == 'A' && pageCounter != 1) {
      pageCounter ++;
      i = 0;
      z = 0;
      k = 0;
    } else if (key == 'B' && pageCounter > 1) {
      pageCounter --;
      i = 0;
      z = 0;
      k = 0;
    } else if (key == 'A' && pageCounter == 1) {
      if (strcmp(pass, defaultPassword) == 0 || strcmp(pass, newPassword) == 0) {
        pageCounter ++;
        i = 0;
      }
    }
  }
}
//Prints "Off" Screen
void OnButtonPressed() {
  //Turn off VFD
  digitalWrite(pinFWD, LOW);
  digitalWrite(LED, LOW);
  analogWrite(pwmOut, 0);

  lcd.setCursor (0, 0);
  lcd.print(F("                    "));
  lcd.setCursor (0, 1);
  lcd.print(F("                    "));
  lcd.setCursor (0, 3);
  lcd.print(F("                    "));
  lcd.setCursor ( 0, 2 );
  lcd.print(F("        OFF         "));
  pageCounter = 0;

  pass[0] = '\0';
}
///////////////////////////////////////////////////////Debounce Functions//////////////////////////////////////////////////////////
//Debounce Jog Button
/*
  int debounceJog(int Pin, int Toggle) {
  long lastDebounceTime = 0;
  int   reading = digitalRead(Pin);

  if (reading != lastJogState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != JogState) {
      JogState = reading;
      if (JogState == HIGH) {
        Toggle = !Toggle;
      }
    }
  }
  lastJogState = reading;
  return Toggle;
  } */
//Debounce Mag Sensor
int debounceMag(int Pin, int Toggle) {
  long lastDebounceTime = 0;
  int   reading = digitalRead(Pin);

  if (reading != lastMagState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != MagState) {
      MagState = reading;
      if (MagState == HIGH) {
        Toggle = !Toggle;
      }
    }
  }
  lastMagState = reading;
  return Toggle;
}
/////////////////////////////////////////////////////End Of Debounce Functions/////////////////////////////////////////////////////
//Home Function
long Home_Function () {
  //Variables
  long lastTime = 0;
  long TrackLength = 0;
  int x;

  x = debounceMag(MagSensor, toggleState);

  if (x == HIGH) {
    MagCounter ++;
    Serial.print("Mag Count: ");
    Serial.println(MagCounter);
  }
  //When the conveyor passes the first time reset the pulse counter then exit the if statement by incrementing counter
  if (MagCounter == 1) {
    pulses = 0;
    digitalWrite(LED, HIGH);
    Serial.println(pulses);
    MagCounter ++;
  }
  //When the conveyor comes around the second time record the tracklength and exit the if statement to preserve the value
  if (MagCounter == 3) {
    digitalWrite(pinFWD, LOW);
    digitalWrite(LED, LOW);
    analogWrite(pwmOut, 0);

    digitalWrite(LED, LOW);
    TrackLength = pulses;
    pulses = 0;
    TrackLength = abs(TrackLength);
    Serial.println(LengthString + TrackLength);
    MagCounter ++;
  }
  return TrackLength;
}
//Sets the User Interface Screens
void HMI_Screens () {
  //Variables
  int directionOne    = 0;
  int directionTwo    = 0;
  int directionOneAbs = 0;
  int directionTwoAbs = 0;

  int revolutions     = 0;
  int UL              = 4;
  int LL              = -1;
  String location     = "0";
  bool isValid        = false;

  switch (pageCounter) {
      LookForPress();

    case 1: {
        if (i == 0) {
          lcd.setCursor(0, 0);
          lcd.print(F("Enter Password and #"));
          lcd.setCursor(0, 1);
          lcd.print(F("Push Setup to exit  "));
          lcd.setCursor(0, 2);
          lcd.print(F("                    "));
          lcd.setCursor(0, 3);
          lcd.print(F("                    "));
          i++;
        }

        char key = kpd.getKey();
        if (key) // Check for a valid key.
        {
          switch (key)
          {
            case '*':
              if (k > (LL + 1)) {
                //Serial.println(k);
                k--;
                password[k] = 0;
                lcd.setCursor(k, 2);
                lcd.print(" ");
                lcd.setCursor(k, 2);
                // Serial.println(k);
              }
              break;
            case '#':
              if (k == UL) {
                lcd.setCursor(0, 3);

                for (int x = 0; x < 4; x++) {
                  pass[x] = password[x];
                  lcd.print(pass[x]);
                  // Serial.println(pass[x]);

                }
                if (strcmp(pass, defaultPassword) == 0 || strcmp(pass, newPassword) == 0) {
                  //  Serial.println(defaultPassword);
                  //  Serial.println(password);
                  i = 0;
                  k = 0;
                  lcd.setCursor(0, 3);
                  lcd.print(F("                    "));
                  pageCounter++;
                  break;
                } else if (pass != defaultPassword) {
                  // Serial.println(pass);
                  // Serial.println(password);
                  lcd.setCursor(0, 0);
                  lcd.print(F("Password Not Valid  "));
                  lcd.setCursor(0, 1);
                  lcd.print(F("Please Try Again    "));
                  lcd.setCursor(0, 2);
                  lcd.print(F("                    "));
                  lcd.setCursor(0, 3);
                  lcd.print(F("                    "));
                  delay(3000);
                  k = 0;
                  i = 0;
                }
                lcd.setCursor((k - 1), 2);
              }
              break;
            case 'A':
            case 'B':
            case 'C':
            case 'D': break;

            default:
              if (k < UL && k > LL) {
                lcd.setCursor(k, 2);
                lcd.print(key);
                password[k] = key;
                k++;
              }
              break;
          }
        }
        break;
      }
    case 2: {
        if (i == 0) {
          lcd.setCursor(0, 0);
          lcd.print(F("Enter Password and #"));
          lcd.setCursor(0, 1);
          lcd.print("        " + String(pass) + "         ");
          lcd.setCursor(0, 2);
          lcd.print(F("                    "));
          // lcd.setCursor(0, 3);
          // lcd.print(F("                    "));
        }
        char key = kpd.getKey();
        if (key) // Check for a valid key.
        {
          switch (key)
          {
            case '*':
              if (k > (LL + 1)) {
                //Serial.println(k);
                k--;
                password[k] = 0;
                lcd.setCursor((k + 8), 3);
                lcd.print(" ");
                lcd.setCursor((k + 8), 3);
                // Serial.println(k);
              }
              break;
            case '#':
              if (k == UL) {
                //lcd.setCursor(8, 3);

                for (int x = 0; x < 4; x++) {
                  pass[x] = password[x];
                  //lcd.print(pass[x]);
                  Serial.print(pass[x]);

                }
                i++;
                for (int x = 0; x < 4; x++) {
                  newPassword[x] = pass[x];
                }
                lcd.setCursor(0, 0);
                lcd.print("Password Changed    ");
                lcd.setCursor(0, 1);
                lcd.print(F("                    "));
                lcd.setCursor(0, 3);
                lcd.print(F("                    "));
                delay(2500);
                k = 0;
                i = 0;
              } else if (k != UL) {

                lcd.setCursor(0, 0);
                lcd.print("Password Not Valid  ");
                lcd.setCursor(0, 1);
                lcd.print(F("                    "));
                delay(2500);
                i = 0;
              }

              break;

            case 'A':
              pageCounter++;
              k = 0;
              break;
            case 'B':
              k = 0;
              pageCounter --;
              k = 0;
              break;

            case 'C':
            case 'D': break;

            default:
              if (k < UL && k > LL) {
                lcd.setCursor((k + 8), 3);
                lcd.print(key);
                password[k] = key;
                k++;
                //Serial.println(k);
                for (int x = 0; x < 4; x++) {
                  Serial.println(password[x]);
                }
              }
              break;
          }
        }
        break;
      }
    case 3:
      i = 0;
      //k = UL;
      lcd.setCursor(0, 0);
      lcd.print(F("Version 1.0         "));
      lcd.setCursor(0, 1);
      lcd.print(F("                    "));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
      break;

    case 4: {
        if (i == 0) {
          lcd.setCursor(0, 0);
          lcd.print(F("Auto Calibrate      "));
          lcd.setCursor(0, 1);
          lcd.print(F("Push C to Begin     "));
          lcd.setCursor(0, 2);
          lcd.print(F("                    "));
          i++;
        }
        ///////////////////////////////////////////////////////Start of Home Feature///////////////////////////////////////////////////////
        char key = kpd.getKey();
        if (key == 'C') // Check for a valid key.
        {
          lcd.setCursor(0, 0);
          lcd.print(F("Calibrating         "));
          lcd.setCursor(0, 1);
          lcd.print(F("                    "));

          //digitalWrite(LED,HIGH);
          MagCounter = 0;
          boolHome = !boolHome;
          Serial.print("Bool: ");
          Serial.println(boolHome);
        }
        //Calibrating Conveyor, will leave if jog or stop button is pressed.
        while (boolHome == true) {
          //Turn on VFD @ roughly 30Hz
          digitalWrite(pinFWD, HIGH);
          digitalWrite(LED, HIGH);
          analogWrite(pwmOut, 127);  // about 2.5 volts

          digitalWrite(LED, HIGH);
          TotalLength = Home_Function();


          if (TotalLength != 0) {
            //Turn off VFD
            digitalWrite(pinFWD, LOW);
            digitalWrite(LED, LOW);
            analogWrite(pwmOut, 0);

            n++;
            lcd.setCursor(0, 0);
            lcd.print(F("Calibration Complete"));
            lcd.setCursor(0, 1);
            lcd.print(F("                    "));
            digitalWrite(LED, LOW);
            pulses = 0;
            boolHome = !boolHome;
            Serial.print("Bool: ");
            Serial.println(boolHome);
            Serial.println(pulses);
            Serial.print("MagCounter: ");
            Serial.println(MagCounter);
            break;
          }
          char exit_Calibration = kpd.getKey();
          if (exit_Calibration == 'C') // Check for a valid key.
          {
            //Turn off VFD
            digitalWrite(pinFWD, LOW);
            digitalWrite(LED, LOW);
            analogWrite(pwmOut, 0);

            lcd.setCursor(0, 0);
            lcd.print(F("Calibration Exited  "));
            lcd.setCursor(0, 1);
            lcd.print(F("                    "));
            digitalWrite(LED, LOW);
            pulses = 0;
            TotalLength = 0;
            boolHome = !boolHome;
            break;
          }
        }
        ////////////////////////////////////////////////////////End of Home Feature////////////////////////////////////////////////////////
        break;
      }

    case 5: {
        totalPulses = abs(pulses);
        i = 0;
        lcd.setCursor(0, 0);
        lcd.print(F("Move First Location "));
        lcd.setCursor(0, 1);
        lcd.print(F("to Pickup Point     "));
        lcd.setCursor(0, 2);
        lcd.print(F("Press 1 to Enter    "));


        //Add jog function to look for button press
        char key = kpd.getKey();
        if (key == 'C') // Check for a valid key.
        {
          boolPickup = !boolPickup;
        }
        while (boolPickup == true) {
          //Turn on VFD @ roughly 30Hz
          digitalWrite(pinFWD, HIGH);
          digitalWrite(LED, HIGH);
          analogWrite(pwmOut, 127);  // about 2.5 volts

          digitalWrite(LED, HIGH);
          //          JogPressed = debounceJog(JogButtonPin, toggleState);
          char keyPress = kpd.getKey();
          if (keyPress == 'C') // Check for a valid key.
          {
            //Turn off VFD
            digitalWrite(pinFWD, LOW);
            digitalWrite(LED, LOW);
            analogWrite(pwmOut, 0);
            boolPickup = !boolPickup;

            break;
          }
        }
        if (totalPulses >= TotalLength) {
          pulses = 1;
        } else if (totalPulses != TotalLength) {
          if (pulses <= 0) {
            pulses = TotalLength - 1;
          }
          if (key == '1') // Check for a valid key.
          {
            pickupPoint = pulses;
            Serial.println(pickupPoint);
            pulses = 0;
          }
          if (key == 'A') {
            pageCounter ++;
            i = 0;
            z = 0;
            k = 0;
          } else if (key == 'B') {
            pageCounter --;
            i = 0;
            z = 0;
            k = 0;
          }
        }
        break;
      }

    case 6:
      getChar(slowDistanceA, 4, -1);

      if (i == 0) {
        lcd.setCursor(0, 0);
        lcd.print(F("Slow Down Distance A"));
        lcd.setCursor(0, 1);
        lcd.print(String(slowDistanceA) + "                 ");
        lcd.setCursor(0, 2);
        lcd.print(F("                    "));
        i++;
        distanceA = atoi(slowDistanceA) ;
        //distanceA = 255 * (distA / 100);
        Serial.println("distanceA: " + String(distanceA));
      }
      break;

    case 7:

      getChar(slowSpeedA, 2, -1);
      if (i == 0) {
        lcd.setCursor(0, 0);
        lcd.print(F("Slow Speed A        "));
        lcd.setCursor(0, 1);
        lcd.print(String(slowSpeedA) + "                 ");
        lcd.setCursor(0, 2);
        lcd.print(F("                    "));
        i++;
        //Convert speed to percentage
        spdA = atoi(slowSpeedA);
        //Convert max value of 255 to proper value based on percent
        speedA = 255 * spdA / 100;
        Serial.println ("SpeedA: " + String(speedA));
      }

      break;

    case 8:
      getChar(lastLoc, 5, -1);
      lastLocation =  atoi(lastLoc);
      firstLocation = atoi(firstLoc);


      if (lastLocation > firstLocation && lastLocation > 0) {
        if (i == 0) {
          lcd.setCursor(0, 0);
          lcd.print(F("Last Loc Tier 1     "));
          lcd.setCursor(0, 1);
          lcd.print(String(lastLoc) + "                ");
          lcd.setCursor(0, 2);
          lcd.print(F("                    "));
          i++;
        }
      } else {
        lcd.setCursor(0, 0);
        lcd.print(F("Location Not Valid  "));
        lcd.setCursor(0, 1);
        lcd.print(F("Please Try Again    "));
        lcd.setCursor(0, 2);
        lcd.print(F("                    "));
        delay(2500);
        i = 0;
        strcpy(lastLoc, "100");
        Serial.println(lastLoc);
      }
      break;

    case 9:
      getChar(firstLoc, 5, -1);
      lastLocation =  atoi(lastLoc);
      firstLocation = atoi(firstLoc);


      if (firstLocation > 0 && firstLocation < lastLocation) {
        if (i == 0) {
          lcd.setCursor(0, 0);
          lcd.print(F("First Loc Tier 1    "));
          lcd.setCursor(0, 1);
          lcd.print(String(firstLocation) + "                ");
          lcd.setCursor(0, 2);
          lcd.print(F("                    "));
          i++;
        }
      } else {
        lcd.setCursor(0, 0);
        lcd.print(F("Location Not Valid  "));
        lcd.setCursor(0, 1);
        lcd.print(F("Please Try Again    "));
        lcd.setCursor(0, 2);
        lcd.print(F("                    "));
        delay(2500);
        n = 0;
        strcpy(firstLoc, "1");
        Serial.println(lastLoc);
      }
      break;

    case 10: {
        z = 0;
        i = 0;
        int sensed = digitalRead(MagSensor);
        String magHere = "";
        if (sensed == HIGH) {
          magHere = "Yes";
        } else {
          magHere = "No";
        }
        lcd.setCursor(0, 0);
        lcd.print("Magnet? " + magHere + "          ");
        lcd.setCursor(0, 1);
        lcd.print("Proxy?              ");
        lcd.setCursor(0, 2);
        lcd.print(F("                    "));
        break;
      }

    case 11: {
        lcd.setCursor(0, 0);
        lcd.print(F("1: ORDERED          "));
        lcd.setCursor(0, 1);
        lcd.print(F("2: SHORTEST         "));
        lcd.setCursor(0, 2);
        lcd.print(F("                    "));

        char keyPress = kpd.getKey();
        if (keyPress == '1') {
          boolOrdered = true;
          boolShortest = false;
        } else if (keyPress == '2') {
          boolShortest = true;
          boolOrdered = false;
        } else if (keyPress == 'A') {
          pageCounter ++;
          i = 0;
          z = 0;
        } else if (keyPress == 'B') {
          pageCounter --;
          i = 0;
          z = 0;
        }
        break;

      }

    case 12: {
        int tempLocation = locationCalculation;
        int tempVal = 0;
        update_Location();
        q = queuelevel();
        //The purpose of case 12 is to control the conveyor. This includes calculating current location, saving values to a queue
        //going to entered values, finding shortest path from current value to entered value



        //if there are values in the queue
        if (q > 0 && oneTime == 0 && useQueueVal > 0) {
          Serial.println("Using queue");
          int eraseQueue = head - 1;
          newLocation = dequeue();
          ring[eraseQueue] = 0;
          lcd.setCursor(0, 2);
          lcd.print(String(newLocation) + "                 ");
          oneTime ++;
          useQueueVal = 0;
          boolUsingQueue = true;
          Serial.println("Ring: ");
          for (int y = 0; y < 10; y++) {
            Serial.println(ring[y]);
          }
        }


        //Start with gathering chracters:
        char key = kpd.getKey();
        if (key) {
          switch (key) {
              Serial.println("backspace");
            case '*': //backspace
              //As long as counter is above the lower limit + 1, we can move the cursor backwards and then print a
              //blank character.
              if (k > (LL + 1)) {
                //Serial.println(k);
                k--;
                newLoc[k] = 0;
                lcd.setCursor(k, 2);
                lcd.print(" ");
                lcd.setCursor(k, 2);
                // Serial.println(k);
              }
              break;

            case '#': //save to queue
              tempLocation = atoi(newLoc); //Get location value from array
              Serial.println("tempLocation: " + String(tempLocation));
              testNewVal(tempLocation);

              //Call isValid function
              //if valid: save to queue
              if (isValidNum) {
                //Add new value to queue
                newLocation = tempLocation;
                enqueue(newLocation);
                if (boolFullBuffer) {
                  lcd.setCursor(0, 2);
                  lcd.print(String(newLocation) + "                 ");
                }
                isValidNum = false;
                useQueueVal = 0;
                i = 0;
              }
              break;

            case 'D':  //enter
              Serial.println("oneTime: " + String(oneTime));
              Serial.println("queuelevel: " + String(q));
              //if there are values in the queue
              tempVal = atoi(newLoc);
              Serial.println("tempVal: " + String(tempVal));
              Serial.println("boolRepeatVal: " + String(boolRepeatVal));

              if (boolRepeatVal == true && tempVal != 0) {
                boolRepeatVal = false;
              }

              //Option 1: is there are values in the queue and a new value hasn't been entered, use queued value
              //delete duplicate requests & delete head value from queue if going to value immediately after saving.
              if ((q > 0 && tempVal == 0 && queueVal == 0) || (boolUsingQueue == true && tempVal == 0)) {
                Serial.println("Using queued value");
                boolNewLocation = !boolNewLocation;
              }
              //Option 2: if user enters a value, use that instead of queued value
              else if ((q == 0 && tempVal != 0) || (q > 0 && tempVal != 0)) {
                Serial.println("using new location");
                testNewVal(tempVal);
                if (isValidNum) {
                  newLocation = tempVal;
                  //enqueue(newLocation);
                  boolNewLocation = !boolNewLocation;
                  isValidNum = false;
                }
              }
              //Option 3: If user cancels retriever request, they should be able to go back to the same value
              else if (boolRepeatVal == true && tempVal == 0) {
                Serial.println("Using repeat");
                boolNewLocation = !boolNewLocation;
              }

              Serial.println("boolNewLocation: " + String (boolNewLocation));
              if (boolNewLocation) {
                Serial.println("In boolNewLocation");



                
                if (newLocation - locationCalculation > 0) {

                  directionOne = newLocation - locationCalculation;
                  directionTwo = (locationCalculation - firstLocation + 1) + (lastLocation - newLocation);
                } else if (newLocation - locationCalculation < 0) {

                  directionOne = (lastLocation - locationCalculation) + (newLocation - firstLocation + 1);
                  directionTwo = newLocation - locationCalculation;
                }







                
                directionOneAbs = abs(directionOne);
                directionTwoAbs = abs(directionTwo);
                lcd.setCursor(0, 2);
                lcd.print(String(newLocation) + "                 ");
                boolRunVFD = true;
                boolNewLocation = false;
                boolNewLocation = !boolNewLocation;
                boolTurnOff = false;
              }
              Serial.println("boolRunVFD: " + String(boolRunVFD));

              while (boolRunVFD == true) {
                //Serial.println("boolTurnOff: " + String(boolTurnOff));
                update_Location();
                //While running a user can enter new values into the queue
                getChar(queueValue, 5, -1);
                queueVal = atoi(queueValue);
                if (queueVal > 0) {
                  testNewVal(queueVal);
                }
                if (isValidNum) {
                  //Add new value to queue
                  tempLocation = queueVal;
                  enqueue(tempLocation);
                  //lcd.setCursor(0, 2);
                  //lcd.print(String(newLocation) + "                 ");
                  isValidNum = false;
                }
                if (boolTurnOff == true) {
                  for (int x = 0; x < sizeof(newLoc); x++) {
                    newLoc[x] = 0;
                  }
                  Serial.println("turning off LED");
                  digitalWrite(LED, LOW);
                  break;
                }
                //Add if statement: ORDERED VS SHORTEST

                moveLocation = newLocation * hookSpacing; //desired hook * spacing = new pulses. So we can compare pulse vals
                if (m == 0) {
                  Serial.println("moveLocation: " + String(moveLocation));
                  Serial.println("distanceA: " + String(distanceA));
                  m++;
                }
                //Serial.println("moveLocation: " + String(moveLocation));
                if (directionOneAbs > directionTwoAbs) { //If direction two is shorter, travel in reverse

                  //  Serial.println("Reverse");
                  //Turn on VFD @ roughly 30Hz
                  digitalWrite(pinREV, HIGH);
                  digitalWrite(LED, HIGH);

                  if (abs(totalPulses - moveLocation) <= distanceA) {
                    //if (abs(locationCalculation - newLocation) <= 7) {
                    Serial.println("Running at 50");
                    analogWrite(pwmOut, speedA);
                  }
                  else if (abs(totalPulses - moveLocation) >= distanceA) {
                    //else if (abs(locationCalculation - newLocation) >= 7) {
                    //Serial.println("Running at 127");
                    analogWrite(pwmOut, 127);  // about 2.5 volts
                  }


                  turnOffVFD();
                }  else if (directionOneAbs <= directionTwoAbs) { //If direction one is shorter or equal, travel forward

                  //Serial.println("Forward");
                  //Turn on VFD @ roughly 30Hz
                  digitalWrite(pinFWD, HIGH);
                  digitalWrite(LED, HIGH);

                  if (abs(totalPulses - moveLocation) <= distanceA) {
                    //if (abs(locationCalculation - newLocation) <= 7) {
                    Serial.println("Running at 50");
                    analogWrite(pwmOut, speedA );
                  }
                  else if (abs(totalPulses - moveLocation) >= distanceA) {
                    //else if (abs(locationCalculation - newLocation) >= 7) {
                    //Serial.println("Running at 127");
                    analogWrite(pwmOut, 127);  // about 2.5 volts
                  }
                  turnOffVFD();
                }

              }
              break;

            case 'A':
              pageCounter ++;
              i = 0;
              z = 0;
              break;
            case 'B':
              pageCounter --;
              i = 0;
              z = 0;
              break;
            case 'C': //do nothing
              break;

            default: //if a number is entered
              //If counter is less than upper limit and above lower limit print the value on the screen
              //then move cursor one position over. save entered value to array
              if (k < UL && k > LL) {
                lcd.setCursor(k, 2);
                lcd.print(key);
                newLoc[k] = key;
                k++;
                Serial.println(newLoc);
              }
              break;
          }
        }
        break;
      }
    case 13:
      z = 0;
      i = 0;
      pageCounter = 1;
      lcd.clear();
      break;

    default:
      break;
  }
}
//Try to make this function work! would save a lot of room. Can send back an isValid bool.
//then if (isValid == true) either: { save to queue or go to location} *depending on if it was called in '#' or 'D'
void testNewVal (int temporaryLocation) {
  Serial.println("testing");
  if (temporaryLocation != locationCalculation && temporaryLocation >= firstLocation && temporaryLocation <= lastLocation) {
    //Check if newLocation is same as previous value in the queue
    // if (temporaryLocation != ring[tail - 1]) {
    for (int y = 0; y < 10; y++) {
      if (temporaryLocation == ring[y]) {
        ring[y] = '\0';
        k = 0;
        lcd.setCursor(0, 2);
        lcd.print(F("                    "));
        lcd.setCursor(0, 3);
        lcd.print(F("Duplicate Value    "));
        delay(600);
        lcd.setCursor(0, 3);
        lcd.print(F("                    "));
        break;
      }
    }
    for (int m = 0; m < 10; m++) {
      Serial.println(ring[m]);
    }
    // }
    //Clear the array so we can accept fresh inputs
    for (int x = 0; x < sizeof(newLoc); x++) {
      newLoc[x] = 0;
    }   
    //clear lcd for new inputs
    lcd.setCursor(0, 2);
    lcd.print(F("                    "));
    //reset counter so lcd cursor starts a beginning
    k = 0;
    isValidNum = true;
  } 
  /*
  else { //if new location is same as last entry
    temporaryLocation = 0;
    
      k = 0;
      lcd.setCursor(0, 2);
      lcd.print(F("                    "));
      lcd.setCursor(0, 3);
      lcd.print(F("Same as last value  "));
      delay(600);
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
    */
 else { //if not valid clear array and temp value for new inputs
  for (int x = 0; x < sizeof(newLoc); x++) {
    newLoc[x] = 0;
  }
  temporaryLocation = atoi(newLoc);
  //Display error message, then reset for new input
  lcd.setCursor(0, 0);
  lcd.print(F("Location Not Valid  "));
  lcd.setCursor(0, 1);
  lcd.print(F("Please Try Again    "));
  lcd.setCursor(0, 2);
  lcd.print(F("                    "));
  delay(2500);
  lcd.setCursor(0, 2);
  k = 0;
}
}
char* getChar(char* returnValue, int upperLim, int lowerLim) {
  //Serial.println("In get char");
  char key = kpd.getKey();

  if (key) { // Check for a valid key.
    Serial.println("Val:");
    Serial.println(val);

    switch (key)  {
      case '*':
        if (y > (lowerLim + 1)) {
          //Serial.println(k);

          y--;
          val[y] = 0;
          lcd.setCursor(y, 2);
          lcd.print(" ");
          lcd.setCursor(y, 2);
        }
        break;
      case '#':
        if (y <= upperLim) {
          i = 0;
          returnValue[0] = '\0';
          for (int x = 0; x < upperLim; x++) {
            returnValue[x] = val[x];
            val[x] = 0;
          }
          z = 0;
          k = 0;

          lcd.setCursor(0, 2);
        }
        break;

      case 'A':

      case 'B':

      case 'C':
        LookForPress();
        break;

      case 'D':
        if (pageCounter == 12) {
          Serial.println("Turning Off");
          //Turn off VFD
          digitalWrite(pinFWD, LOW);
          digitalWrite(LED, LOW);
          analogWrite(pwmOut, 0);
          // for (int x = 0; x < sizeof(newLoc); x++) {
          //   newLoc[x] = 0;
          // }
          k = 0;
          boolTurnOff = true;
          boolNewLocation = false;
          boolUsingQueue = false;
          boolRepeatVal = true;
          useQueueVal = 0;
          oneTime = 0;
        }
        break;

      default:
        if (y < upperLim && y > lowerLim) {
          lcd.setCursor(y, 2);
          lcd.print(key);
          val[y] = key;
          y++;
          break;
        }
    }
  }
}
void turnOffVFD () {
  if (abs(totalPulses - moveLocation) <= 5) { //The 5 will be replaced by a user value called: precision.
    //if (locationCalculation == newLocation) {
    Serial.println("turning off VFD");
    i = 0;
    newLocation = 0;
    //Turn off VFD
    i = 0;
    lcd.setCursor(0, 2);
    lcd.print(F("                    "));
    digitalWrite(pinFWD, LOW);
    digitalWrite(pinREV, LOW);
    digitalWrite(LED, LOW);
    analogWrite(pwmOut, 0);
    for (int x = 0; x < sizeof(newLoc); x++) {
      newLoc[x] = 0;
    }

    Serial.println("q: " + String(q));
    boolRunVFD = false;
    boolNewLocation = false;
    boolUsingQueue = false;
    useQueueVal++;
    oneTime = 0;
  }
}
// Put something into the buffer. Returns 0 when the buffer is full,
// Returns 1 when the stuff was put sucessfully into the buffer
int enqueue (int val) {
  Serial.println("In enqueue");
  int newtail = (tail + 1) % ringsize;

  if (newtail == head) {
    // Buffer is full, do nothing
    lcd.setCursor(0, 3);
    lcd.print(F("Full buffer         "));
    delay(500);
    lcd.setCursor(0, 3);
    lcd.print(F("                    "));
    boolFullBuffer = true;
    return 0;
  }
  else {
    lcd.setCursor(0, 3);
    lcd.print(F("Saved               "));
    delay(500);
    lcd.setCursor(0, 3);
    lcd.print(F("                    "));

    ring[tail] = val;
    tail = newtail;
    return 1;
  }
}

// Return number of elements in the queue.
int queuelevel () {
  return tail - head + (head > tail ? ringsize : 0);
}
// Get something from the queue. 0 will be returned if the queue is empty
int dequeue () {
  if (head == tail) {
    return 0;
  }
  else {
    int val = ring[head];
    head  = (head + 1) % ringsize;
    return val;
  }
}

void update_Location() {
  totalPulses = abs(pulses);
  if (i == 0) {
    hookSpacing = float(TotalLength) / float((lastLocation + 1) - (firstLocation));
    i++;
  }
  if (pulses > TotalLength) {
    pulses = 1;
    Serial.println(pulses);
  }
  if (pulses < 0) {
    pulses = TotalLength;
    Serial.println(pulses);
  }
  // totalPulses = abs(pulses);

  //locationCalculation = round(lastLocation - (totalPulses / hookSpacing));
  //testing:
  totalPulses = abs(pulses);

  locationCalculation = round(totalPulses / hookSpacing) + 0.5;
  if (locationCalculation == 0) {
    locationCalculation = firstLocation;
  }

  lcd.setCursor(0, 0);
  lcd.print("At " + String(locationCalculation) + "              ");
  lcd.setCursor(0, 1);
  lcd.print(F("Enter New Location: "));
}

//Encoder Functions
void A_RISE() {
  detachInterrupt(0);
  A_SIG = 1;

  if (B_SIG == 0)
    pulses++;//moving forward
  if (B_SIG == 1)
    pulses--;//moving reverse
  Serial.println(pulses);
  attachInterrupt(0, A_FALL, FALLING);
}

void A_FALL() {
  detachInterrupt(0);
  A_SIG = 0;

  if (B_SIG == 1)
    pulses++;//moving forward
  if (B_SIG == 0)
    pulses--;//moving reverse
  Serial.println(pulses);
  attachInterrupt(0, A_RISE, RISING);
}

void B_RISE() {
  detachInterrupt(1);
  B_SIG = 1;

  if (A_SIG == 1)
    pulses++;//moving forward
  if (A_SIG == 0)
    pulses--;//moving reverse
  Serial.println(pulses);
  attachInterrupt(1, B_FALL, FALLING);
}

void B_FALL() {
  detachInterrupt(1);
  B_SIG = 0;

  if (A_SIG == 0)
    pulses++;//moving forward
  if (A_SIG == 1)
    pulses--;//moving reverse
  Serial.println(pulses);
  attachInterrupt(1, B_RISE, RISING);
}


