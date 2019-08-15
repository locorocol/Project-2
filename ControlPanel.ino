///HEADER////////////////////////////////////////////////////////////////////////////////

#include <IRremote.h>
//Servo
#include "ServoTimer2.h"

/////////////////////////////////////////////////////////////////////////////////////////


// VARIABLES AND CONSTANTS DECLARATION/////////////////////////////////////////////////////////////////

//IR Receiver
const int RECV_PIN = 4;
IRrecv irrecv(RECV_PIN);
decode_results results;

const unsigned long stateOne = 000;
const unsigned long stateTwo = 111;
const unsigned long stateThree = 222;


//LED
const int LEDPin = 11;

// TriColor LED
const int redPin = 5;
const int greenPin = 6;
const int bluePin = 9;
//comment this line if using a Common Cathode LED
#define COMMON_ANODE

//Servo
ServoTimer2 myservo;
int pos = 750; // intial position
int Step = 5;
int maxPos = 2250; // max position
const int minPos = 0; // min position (must be 0)

// State
int State = 1;

//Buzzer
int BuzzerPin = 3;
int songLength = 0;
int noteDuration = 0;
int pauseBetweenNotes = 0;

// Note
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

///////SETUP CODE/////////////////////////////////////////////////////////////////////


void setup() {

  Serial.begin(9600);

  //IR
  irrecv.enableIRIn();
  irrecv.blink13(true);

  //LED
  pinMode(LEDPin, OUTPUT);

  //TriColor LED
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  //servo
  myservo.attach(10); // pin 10

}

// Song Arrays ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// The first array contains the list of notes 
// The second array contains the duration for each corresponding note

//Mario main theme 
int melody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,
};

int noteDurations[] = {


6, 6, 6, 6,
6, 6, 6, 6,
6, 6, 6, 6,
6, 6, 6, 6,

6, 6, 6, 6,
6, 6, 6, 6,
6, 6, 6, 6,
6, 6, 6, 6,

4.5, 4.5, 4.5,
6, 6, 6, 6,
6, 6, 6, 6,
6, 6, 6, 6,

6, 6, 6, 6,
6, 6, 6, 6,
6, 6, 6, 6,
6, 6, 6, 6,

4.5, 4.5, 4.5,
6, 6, 6, 6,
6, 6, 6, 6,
6, 6, 6, 6,
};

//Mario Underworld melody
int melody2[] = {
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,
  0,
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0, NOTE_DS4, NOTE_CS4, NOTE_D4,
  NOTE_CS4, NOTE_DS4,
  NOTE_DS4, NOTE_GS3,
  NOTE_G3, NOTE_CS4,
  NOTE_C4, NOTE_FS4, NOTE_F4, NOTE_E3, NOTE_AS4, NOTE_A4,
  NOTE_GS4, NOTE_DS4, NOTE_B3,
  NOTE_AS3, NOTE_A3, NOTE_GS3,
  0, 0, 0
};



int noteDurations2[] = {
  6, 6, 6, 6,
  6, 6, 3,
  1.5,
  6, 6, 6, 6,
  6, 6, 3,
  1.5,
  6, 6, 6, 6,
  6, 6, 3,
  1.5,
  6, 6, 6, 6,
  6, 6, 3,
  3, 9, 9, 9,
  3, 3,
  3, 3,
  3, 3,
  9, 9, 9, 9, 9, 9,
  5, 5, 5,
  5, 5, 5,
  1.5, 1.5, 1.5,
};

//Unravel  ////////////////////////////////////////////////////////////////////////////////////////////////////////
int melody3[] = {
NOTE_AS4, NOTE_C5, NOTE_AS4, NOTE_A4, NOTE_G4,  NOTE_C5, NOTE_AS4, NOTE_A4, NOTE_G4, NOTE_G4, NOTE_F4, 0, 0,
NOTE_DS4, NOTE_DS4, NOTE_F4, NOTE_D4, 0, 0, 0, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D5, NOTE_D5,
0, 0, 0, 0, 0, 0, NOTE_AS4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_AS4, NOTE_AS4, 
0, 0, 0, 0, 0, 0,

NOTE_AS4, NOTE_C5, NOTE_AS4, NOTE_A4, NOTE_G4,  NOTE_C5, NOTE_AS4, NOTE_A4, NOTE_G4, NOTE_G4, NOTE_F4, 0, 0,
NOTE_DS4, NOTE_DS4, NOTE_F4, NOTE_D4, 0, 0, 0, 0, 0, 0, NOTE_F3, 
NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D5, NOTE_D5,
0, 0, 0, 0, 0, 0, NOTE_AS4, NOTE_A4, NOTE_A4, NOTE_A4, NOTE_AS4, NOTE_AS4, 
};


// note durations: 4 = quarter note, 8 = eighth note
int noteDurations3[] = {
  
4.5, 2.25, 2.25, 4.5, 2.25, 2.25, 2.25, 2.25, 2.25, 4.5, 3, 9, 4.5, 
4.5, 2.25, 4.5, 2, 1.125, 2.5, 4.5, 4.5, 2.25, 4.5, 2.25, 4.5, 2, 
4.75, 4.5, 4.5, 2.25, 4.5, 4.5, 4.5, 2.25, 4.5, 2.25, 4.5, 2, 
4.5, 4.5, 4.5, 2.25, 4.5, 4.5,

4.5, 2.25, 2.25, 4.5, 2.25, 2.25, 2.25, 2.25, 2.25, 4.5, 3, 9, 4.5, 
4.5, 2.25, 4.5, 2, 4.75, 4.5, 4.5, 4.5, 3, 3, 4.5, 
2.25, 4.5, 2.25, 4.5, 2, 
4.75, 4.5, 4.5, 4.5, 2.25, 4.5, 4.5, 2.25, 4.5, 2.25, 4.5, 2, 
} ;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////MAIN LOOP///////////////////////////////////////////////////////

void loop() {

  if (irrecv.decode(&results)) {

    //////////////////////////////////////// Part 1: IR Codes & Protocols  ///////////////////////////////////////////////

    // Print the hex codes of the remote signal to serial monitor
    if (results.value != 0xFFFFFFFF) {
      switch (results.decode_type) {
        case NEC: Serial.print("NEC "); break ;
        case SONY: Serial.print("SONY "); break ;
        case RC5: Serial.print("Phillips RC5 "); break ;
        case RC6: Serial.print("Phillips RC6 "); break ;
        case DISH: Serial.print("DISH "); break ;
        case SHARP: Serial.print("SHARP "); break ;
        case JVC: Serial.print("JVC "); break ;
        case SANYO: Serial.print("SANYO "); break ;
        case MITSUBISHI: Serial.print("MITSUBISHI "); break ;
        case SAMSUNG: Serial.print("SAMSUNG "); break ;
        case LG: Serial.print("LG "); break ;
        case WHYNTER: Serial.print("WHYNTER "); break ;
        case AIWA_RC_T501: Serial.print("AIWA_RC_T501 "); break ;
        case PANASONIC: Serial.print("PANASONIC "); break ;
        case DENON: Serial.print("DENON "); break ;
        default:
        case UNKNOWN: Serial.print("UNKNOWN "); break ;
      }
      Serial.println(results.value, HEX);

    }



    if (State == 1) {

      //////////////////////////////////////// Part 2: LED and RGB LED -- State = 1  /////////////////////////////////////////////////

      switch (results.value) {

        /////////// LED ////////////////////

        //turn on LED
        case 0xA10C140B:
          digitalWrite(LEDPin, HIGH);
          break;

        // turn off
        case 0xA10C940B
:
          digitalWrite(LEDPin, LOW);
          break;

        // ON for 500ms and OFF
        case 0xA10C94:
          digitalWrite(LEDPin, HIGH);
          delay(500);
          digitalWrite(LEDPin, LOW);
          break;

        ////////// TriColor LED //////////

        // Sample Code Red
        case 0xFF3:
          setColor(255, 0, 0);
          delay(500);
          setColor(0, 0, 0);
          break;

        // Sample Code Green
        case 0xFF4:
          setColor(0, 255, 0);
          delay(500);
          setColor(0, 0, 0);
          break;

        // Sample Code Blue
        case 0xFF5:
          setColor(0, 0, 255);
          delay(500);
          setColor(0, 0, 0);
          break;


        case 0xFF6:
          break;

        case 0xFF7:
          break;

        case 0xFF8:
          break;

        case 0xFF9:
          break;



        // Go to State = 2
        case stateTwo:
          changeState(2);
          break;

        // Go to State = 3
        case stateThree:
          changeState(3);
          break;


      }
    }

    //////////////////////////////////////// Part 3: Servo Motor  //////////////////////////////////
    else if (State == 2) {

      /////////// Servo Limit /////////////
      // 
      if ( maxPos > 2250) {
        maxPos = 180;
      } else if (Step < 5) {
        Step = 5;
      } else if (Step > 2250) {
        Step = 2250;
      }
      /////////////////////////////////////

      switch (results.value) {

        // Sample Code :  rotate the Servo from minPos to maxPos
        case 0xFF13:
          for (pos = minPos; pos <= maxPos; pos += Step) {
            myservo.write(pos);
            delay(15);
          }
          break;


        // Rotate the Servo from maxPos to minPos
        case 0xFF14:
          for (pos = maxPos; pos >= minPos; pos -= Step) {
            myservo.write(pos);
            delay(15);
          }
          break;

        // Sweep Movement
        case 0xFF15:
          break;

        // Increase Speed
        case 0xFF16:
          Step = Step + 5;
          break;

        // Decrease Speed
        case 0xFF17:
          Step = Step - 5;
          break;


        // Go to State = 3
        case stateThree:
          changeState(3);
          break;

        // Go to State = 1
        case stateOne:
          changeState(1);
          break;


      }
    }


    /////////////////////////////////// PART 4: BUZZER /////////////////////////////////////////////////////////

    else if (State = 3) {

      ////////////////////////////////Individual notes//////////////////////////////////////

      switch (results.value) {

        // SAMPLE CODE (NOTE_C4)
        case 0xFF21:
          tone(BuzzerPin, NOTE_C4, 100);
          break;

        // SAMPLE CODE (NOTE_D4)
        case 0xFF22:
          tone(BuzzerPin, NOTE_D4, 100);
          break;

        // SAMPLE CODE (NOTE_E4)
        case 0xFF23:
          tone(BuzzerPin, NOTE_E4, 100);
          break;

        // SAMPLE CODE (NOTE_F4)
        case 0xFF24:
          tone(BuzzerPin, NOTE_F4, 100);
          break;

        // SAMPLE CODE (NOTE_G4)
        case 0xFF25:
          tone(BuzzerPin, NOTE_G4, 100);
          break;

        // SAMPLE CODE (NOTE_A4)
        case 0xFF26:
          tone(BuzzerPin, NOTE_A4, 100);
          break;

        // SAMPLE CODE (NOTE_B4)
        case 0xFF27:
          tone(BuzzerPin, NOTE_B4, 100);
          break;

        // SAMPLE CODE (NOTE_C5)
        case 0xFF28:
          tone(BuzzerPin, NOTE_C5, 100);
          break;

        ////////////////////////////////////////////////Songs//////////////////////////////////////////////
        
        // SAMPLE CODE (Mario)
        case 0xA10C740B:
          songLength = sizeof(melody) / sizeof(int);
          Song(songLength, melody, noteDurations);
          break;
          
        // SAMPLE CODE (Mario Underworld)
        case 0xA10CF40B:
          songLength = sizeof(melody2) / sizeof(int);
          Song(songLength, melody2, noteDurations2);
          break;

        // SAMPLE CODE (Unravel)
        case 0xA10C0C03:
          songLength = sizeof(melody3) / sizeof(int);
          Song(songLength, melody3, noteDurations3);
          break;

        
        // SAMPLE CODE (Twinkle Twinkle Little Star)
        case 0xFF35:
          tone(BuzzerPin, NOTE_C4, 100);
          delay(110);
          tone(BuzzerPin, NOTE_C4, 100);
          delay(110);
          tone(BuzzerPin, NOTE_G4, 100);
          delay(110);
          tone(BuzzerPin, NOTE_G4, 100);
          delay(110);
          tone(BuzzerPin, NOTE_A4, 100);
          delay(110);
          tone(BuzzerPin, NOTE_A4, 100);
          delay(110);
          tone(BuzzerPin, NOTE_G4, 200);
          delay(210);
          tone(BuzzerPin, NOTE_F4, 100);
          delay(110);
          tone(BuzzerPin, NOTE_F4, 100);
          delay(110);
          tone(BuzzerPin, NOTE_E4, 100);
          delay(110);
          tone(BuzzerPin, NOTE_E4, 100);
          delay(110);
          tone(BuzzerPin, NOTE_D4, 100);
          delay(110);
          tone(BuzzerPin, NOTE_D4, 200);
          delay(210);
          tone(BuzzerPin, NOTE_C4, 200);
          delay(210);
          break;

   
        // Go to State = 1
        case stateOne:
          changeState(1);
          break;

        // Go to State = 2
        case stateTwo:
          changeState(2);
          break;
      }
    }
    ////////////////////////////////////// End ///////////////////////////////////////////////////////

    irrecv.resume();
  }
}

/////////////////////////////////////////// ADDITIONAL METHODS ///////////////////////////////////////////

void setColor(int red, int green, int blue)
{
#ifdef COMMON_ANODE
  red = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
#endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

void Song (int songLength, int melody[], int noteDurations[])
{
  for (int thisNote=0; thisNote < songLength; thisNote++) {
    noteDuration = 600 / noteDurations[thisNote];
    tone(BuzzerPin, melody[thisNote], noteDuration);
    pauseBetweenNotes = noteDuration * 1.50;
    delay(pauseBetweenNotes);
    noTone(11);
  }
}

void changeState(int newState)
{
  State = newState;
  Serial.print("State changed to: ");

  //////////////////// Set RGB LED color to reflect new state ///////////////////////////
  switch(newState)
  {
    
    case 1:
    setColor(255, 255, 0);
    Serial.println(" 1");
    break;

    case 2:
    setColor(0, 255, 255);
    Serial.println(" 2");
    break;

    case 3:
    setColor(255, 0, 255);
    Serial.println(" 3");
    break;

    default:
    Serial.println(" error");
  }

  
  return;
}
