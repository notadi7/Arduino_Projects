/*
==============================================================
          BINARY CLOCK
--------------------------------------------------------------
Controller : Arduino Nano
RTC        : DS3231
Shift IC   : 74HC595

Display

Hours      : 4 LEDs
Minute MSB : 3 LEDs
Minute LSB : 4 LEDs
Second MSB : 3 LEDs
Second LSB : 4 LEDs
PM LED
Blink LED

LED Logic : ACTIVE LOW
==============================================================
*/

#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;


//==============================================================
// HOURS
//==============================================================

const byte hourPins[4] =
{
  2,
  3,
  4,
  5
};


//==============================================================
// MINUTE TENS
//==============================================================

const byte minuteMSBPins[3] =
{
  6,
  7,
  11
};


//==============================================================
// MINUTE ONES
//==============================================================

const byte minuteLSBPins[4] =
{
  12,
  13,
  A0,
  A1
};


//==============================================================
// STATUS LEDS
//==============================================================

const byte PM_LED = A2;

const byte BLINK_LED = A3;


//==============================================================
// 74HC595
//==============================================================

const byte DATA_PIN  = 8;
const byte CLOCK_PIN = 9;
const byte LATCH_PIN = 10;


//==============================================================
// SHIFT REGISTER BITS
//==============================================================

#define SEC_MSB0 0
#define SEC_MSB1 1
#define SEC_MSB2 2

#define SEC_LSB0 3
#define SEC_LSB1 4
#define SEC_LSB2 5
#define SEC_LSB3 6

#define UNUSED_BIT 7


byte shiftData = 0xFF;


//==============================================================
// TIME VARIABLES
//==============================================================

byte hour12;

byte minuteNow;

byte secondNow;

bool isPM;

byte previousSecond = 255;


//==============================================================
// FUNCTION PROTOTYPES
//==============================================================

void updateDisplay();

void display4Bit(byte value, const byte pins[]);

void display3Bit(byte value, const byte pins[]);

void updateShiftRegister();

void setShiftLED(byte bit,bool state);

void startupAnimation();

void allLEDsON();

void allLEDsOFF();


//==============================================================
// SETUP
//==============================================================

void setup()
{

    Wire.begin();

    Serial.begin(9600);

    if(!rtc.begin())
    {
        while(1);
    }


    //-------------------------------
    // Hour Pins
    //-------------------------------

    for(byte i=0;i<4;i++)
        pinMode(hourPins[i],OUTPUT);


    //-------------------------------
    // Minute Tens
    //-------------------------------

    for(byte i=0;i<3;i++)
        pinMode(minuteMSBPins[i],OUTPUT);


    //-------------------------------
    // Minute Ones
    //-------------------------------

    for(byte i=0;i<4;i++)
        pinMode(minuteLSBPins[i],OUTPUT);


    //-------------------------------
    // Status LEDs
    //-------------------------------

    pinMode(PM_LED,OUTPUT);

    pinMode(BLINK_LED,OUTPUT);


    //-------------------------------
    // Shift Register
    //-------------------------------

    pinMode(DATA_PIN,OUTPUT);

    pinMode(CLOCK_PIN,OUTPUT);

    pinMode(LATCH_PIN,OUTPUT);


    //-------------------------------
    // Uncomment ONLY ONCE
    //-------------------------------

    // rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));


    allLEDsOFF();

    startupAnimation();

}



//==============================================================
// LOOP
//==============================================================

void loop()
{

    DateTime now = rtc.now();

    byte hour24 = now.hour();

    minuteNow = now.minute();

    secondNow = now.second();


    //----------------------------------------------------------
    // Convert 24-Hour to 12-Hour
    //----------------------------------------------------------

    isPM = (hour24 >= 12);

    if(hour24 == 0)
    {
        hour12 = 12;
    }
    else if(hour24 > 12)
    {
        hour12 = hour24 - 12;
    }
    else
    {
        hour12 = hour24;
    }


    //----------------------------------------------------------
    // Update only when second changes
    //----------------------------------------------------------

    if(previousSecond != secondNow)
    {
        previousSecond = secondNow;

        updateDisplay();

        // Print current time to Serial Monitor
        Serial.println("------------------------------------------------");

Serial.print("Time : ");

if(hour12 < 10) Serial.print("0");
Serial.print(hour12);

Serial.print(":");

if(minuteNow < 10) Serial.print("0");
Serial.print(minuteNow);

Serial.print(":");

if(secondNow < 10) Serial.print("0");
Serial.print(secondNow);

Serial.print(isPM ? " PM" : " AM");

Serial.println();


//----------------------------------------------
// Hours
//----------------------------------------------

Serial.print("Hour      : ");

for(int i = 3; i >= 0; i--)
{
    Serial.print(bitRead(hour12, i));
    Serial.print(" ");
}

Serial.println();


//----------------------------------------------
// Minute Tens
//----------------------------------------------

byte minMSB = minuteNow / 10;

Serial.print("Minute10  : ");

for(int i = 2; i >= 0; i--)
{
    Serial.print(bitRead(minMSB, i));
    Serial.print(" ");
}

Serial.println();


//----------------------------------------------
// Minute Ones
//----------------------------------------------

byte minLSB = minuteNow % 10;

Serial.print("Minute1   : ");

for(int i = 3; i >= 0; i--)
{
    Serial.print(bitRead(minLSB, i));
    Serial.print(" ");
}

Serial.println();


//----------------------------------------------
// Second Tens
//----------------------------------------------

byte secMSB = secondNow / 10;

Serial.print("Second10  : ");

for(int i = 2; i >= 0; i--)
{
    Serial.print(bitRead(secMSB, i));
    Serial.print(" ");
}

Serial.println();


//----------------------------------------------
// Second Ones
//----------------------------------------------

byte secLSB = secondNow % 10;

Serial.print("Second1   : ");

for(int i = 3; i >= 0; i--)
{
    Serial.print(bitRead(secLSB, i));
    Serial.print(" ");
}

Serial.println();


//----------------------------------------------
// PM LED
//----------------------------------------------

Serial.print("PM LED    : ");
Serial.println(isPM ? "ON" : "OFF");


//----------------------------------------------
// Blink LED
//----------------------------------------------

Serial.print("Blink LED : ");
Serial.println((secondNow % 2 == 0) ? "ON" : "OFF");

Serial.println();
    }

}


//==============================================================
// UPDATE COMPLETE DISPLAY
//==============================================================

void updateDisplay()
{

    //----------------------------------------------------------
    // Split Minute
    //----------------------------------------------------------

    byte minuteMSB = minuteNow / 10;

    byte minuteLSB = minuteNow % 10;


    //----------------------------------------------------------
    // Split Second
    //----------------------------------------------------------

    byte secondMSB = secondNow / 10;

    byte secondLSB = secondNow % 10;


    //----------------------------------------------------------
    // Display Hours
    //----------------------------------------------------------

    display4Bit(hour12, hourPins);


    //----------------------------------------------------------
    // Display Minutes
    //----------------------------------------------------------

    display3Bit(minuteMSB, minuteMSBPins);

    display4Bit(minuteLSB, minuteLSBPins);


    //----------------------------------------------------------
    // PM LED
    //----------------------------------------------------------

    if(isPM)
        digitalWrite(PM_LED, LOW);      // ON
    else
        digitalWrite(PM_LED, HIGH);     // OFF


    //----------------------------------------------------------
    // Blink LED
    //----------------------------------------------------------

    if(secondNow % 2 == 0)
        digitalWrite(BLINK_LED, LOW);   // ON
    else
        digitalWrite(BLINK_LED, HIGH);  // OFF


    //----------------------------------------------------------
    // Clear Shift Register Data
    //----------------------------------------------------------

    shiftData = 0xFF;


    //----------------------------------------------------------
    // Second Tens (0-5)
    //----------------------------------------------------------

    setShiftLED(SEC_MSB0, bitRead(secondMSB,0));

    setShiftLED(SEC_MSB1, bitRead(secondMSB,1));

    setShiftLED(SEC_MSB2, bitRead(secondMSB,2));


    //----------------------------------------------------------
    // Second Ones (0-9)
    //----------------------------------------------------------

    setShiftLED(SEC_LSB0, bitRead(secondLSB,0));

    setShiftLED(SEC_LSB1, bitRead(secondLSB,1));

    setShiftLED(SEC_LSB2, bitRead(secondLSB,2));

    setShiftLED(SEC_LSB3, bitRead(secondLSB,3));


    //----------------------------------------------------------
    // Update Shift Register
    //----------------------------------------------------------

    updateShiftRegister();

}



//==============================================================
// DISPLAY 4-BIT VALUE
// Used For:
//   Hours
//   Minute Ones
// ACTIVE LOW
//==============================================================

void display4Bit(byte value, const byte pins[])
{
    for(byte i = 0; i < 4; i++)
    {
        if(bitRead(value, i))
            digitalWrite(pins[i], LOW);      // LED ON
        else
            digitalWrite(pins[i], HIGH);     // LED OFF
    }
}


//==============================================================
// DISPLAY 3-BIT VALUE
// Used For:
//   Minute Tens
// ACTIVE LOW
//==============================================================

void display3Bit(byte value, const byte pins[])
{
    for(byte i = 0; i < 3; i++)
    {
        if(bitRead(value, i))
            digitalWrite(pins[i], LOW);
        else
            digitalWrite(pins[i], HIGH);
    }
}


//==============================================================
// SET SHIFT REGISTER LED
//
// state = true  -> LED ON
// state = false -> LED OFF
//
// Since LEDs are ACTIVE LOW:
// ON  -> bit = 0
// OFF -> bit = 1
//==============================================================

void setShiftLED(byte bit, bool state)
{
    if(state)
    {
        bitClear(shiftData, bit);
    }
    else
    {
        bitSet(shiftData, bit);
    }
}


//==============================================================
// UPDATE 74HC595
//==============================================================

void updateShiftRegister()
{
    digitalWrite(LATCH_PIN, LOW);

    shiftOut(DATA_PIN,
             CLOCK_PIN,
             MSBFIRST,
             shiftData);

    digitalWrite(LATCH_PIN, HIGH);
}


//==============================================================
// TURN ALL LEDS ON
//==============================================================

void allLEDsON()
{

    //-----------------------------
    // Hours
    //-----------------------------

    for(byte i=0;i<4;i++)
        digitalWrite(hourPins[i],LOW);


    //-----------------------------
    // Minute Tens
    //-----------------------------

    for(byte i=0;i<3;i++)
        digitalWrite(minuteMSBPins[i],LOW);


    //-----------------------------
    // Minute Ones
    //-----------------------------

    for(byte i=0;i<4;i++)
        digitalWrite(minuteLSBPins[i],LOW);


    //-----------------------------
    // PM LED
    //-----------------------------

    digitalWrite(PM_LED,LOW);


    //-----------------------------
    // Blink LED
    //-----------------------------

    digitalWrite(BLINK_LED,LOW);


    //-----------------------------
    // Shift Register LEDs
    //-----------------------------

    shiftData = 0x00;

    updateShiftRegister();

}


//==============================================================
// TURN ALL LEDS OFF
//==============================================================

void allLEDsOFF()
{

    //-----------------------------
    // Hours
    //-----------------------------

    for(byte i=0;i<4;i++)
        digitalWrite(hourPins[i],HIGH);


    //-----------------------------
    // Minute Tens
    //-----------------------------

    for(byte i=0;i<3;i++)
        digitalWrite(minuteMSBPins[i],HIGH);


    //-----------------------------
    // Minute Ones
    //-----------------------------

    for(byte i=0;i<4;i++)
        digitalWrite(minuteLSBPins[i],HIGH);


    //-----------------------------
    // PM LED
    //-----------------------------

    digitalWrite(PM_LED,HIGH);


    //-----------------------------
    // Blink LED
    //-----------------------------

    digitalWrite(BLINK_LED,HIGH);


    //-----------------------------
    // Shift Register LEDs
    //-----------------------------

    shiftData = 0xFF;

    updateShiftRegister();

}

//==============================================================
// STARTUP ANIMATION
//==============================================================

void startupAnimation()
{
    // Turn everything ON
    allLEDsON();
    delay(500);

    // Turn everything OFF
    allLEDsOFF();
    delay(300);

    // Hours Test
    for(byte i=0;i<4;i++)
    {
        digitalWrite(hourPins[i],LOW);
        delay(120);
    }

    delay(300);

    for(byte i=0;i<4;i++)
    {
        digitalWrite(hourPins[i],HIGH);
    }


    // Minute Tens Test
    for(byte i=0;i<3;i++)
    {
        digitalWrite(minuteMSBPins[i],LOW);
        delay(120);
    }

    delay(300);

    for(byte i=0;i<3;i++)
    {
        digitalWrite(minuteMSBPins[i],HIGH);
    }


    // Minute Ones Test
    for(byte i=0;i<4;i++)
    {
        digitalWrite(minuteLSBPins[i],LOW);
        delay(120);
    }

    delay(300);

    for(byte i=0;i<4;i++)
    {
        digitalWrite(minuteLSBPins[i],HIGH);
    }


    // PM LED Test
    digitalWrite(PM_LED,LOW);
    delay(400);
    digitalWrite(PM_LED,HIGH);


    // Blink LED Test
    digitalWrite(BLINK_LED,LOW);
    delay(400);
    digitalWrite(BLINK_LED,HIGH);


    // Second LEDs Test (74HC595)

    shiftData = 0xFF;

    for(byte i=0;i<7;i++)
    {
        bitClear(shiftData,i);

        updateShiftRegister();

        delay(120);

        bitSet(shiftData,i);
    }

    updateShiftRegister();

    delay(300);

    allLEDsOFF();
}



//==============================================================
// OPTIONAL RTC SETTING
//==============================================================

// Uncomment this line ONLY ONCE
// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));



//==============================================================
// IF RTC IS NOT FOUND
// Replace inside setup()
//==============================================================

/*

if(!rtc.begin())
{

    pinMode(BLINK_LED,OUTPUT);

    while(1)
    {
        digitalWrite(BLINK_LED,LOW);
        delay(250);

        digitalWrite(BLINK_LED,HIGH);
        delay(250);
    }

}

*/
