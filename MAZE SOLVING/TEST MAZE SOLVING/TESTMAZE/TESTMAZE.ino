#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);

// INPUTS
#define MAZE 4
#define BONUS1 5
#define BONUS2 6
#define BONUS3 7
#define BONUS4 8
#define BONUS5 9
#define ENDPOINT A1
#define TIMER_BTN A2



#define START_BTN A3

// OUTPUTS
#define BUZZER 3
#define ALERT 2
#define LED1 10
#define LED2 11
#define LED3 12
#define LED4 13
#define LED5 A0

int errorCount=0;

bool mazeTouched=false;

bool gameStart=false;
bool timerEnable=false;

bool bonus1Taken=false;
bool bonus2Taken=false;
bool bonus3Taken=false;
bool bonus4Taken=false;
bool bonus5Taken=false;

int bonusCount=0;

bool lastStartState=HIGH;
bool lastTimerState=HIGH;

bool timerSelected = false;

unsigned long startTime;
unsigned long timerLimit=300000;

void beep()
{
digitalWrite(BUZZER,HIGH);
delay(80);
digitalWrite(BUZZER,LOW);
}

void longBeep()
{
digitalWrite(BUZZER,HIGH);
delay(300);
digitalWrite(BUZZER,LOW);
}

void coinBeep()
{
  tone(BUZZER, 1200); delay(300);
  tone(BUZZER, 1400); delay(150);
  tone(BUZZER, 1600); delay(350);
  noTone(BUZZER);
}



void winSound()
{
tone(BUZZER,900,120);
delay(150);

tone(BUZZER,1100,120);
delay(150);

tone(BUZZER,1300,200);
delay(200);

tone(BUZZER,1700,350);
delay(400);

noTone(BUZZER);
}

void startBeep()
{

for(int i=0;i<3;i++)
{

tone(BUZZER,500,80);
delay(90);

tone(BUZZER,800,80);
delay(90);

tone(BUZZER,1100,80);
delay(90);

tone(BUZZER,1400,80);
delay(90);

tone(BUZZER,1700,80);
delay(90);

}

tone(BUZZER,2000,600);   // final strong beep
delay(650);

noTone(BUZZER);

}

void setup()
{

pinMode(MAZE,INPUT_PULLUP);
pinMode(BONUS1,INPUT_PULLUP);
pinMode(BONUS2,INPUT_PULLUP);
pinMode(BONUS3,INPUT_PULLUP);
pinMode(BONUS4,INPUT_PULLUP);
pinMode(BONUS5,INPUT_PULLUP);
pinMode(ENDPOINT,INPUT_PULLUP);

pinMode(TIMER_BTN,INPUT_PULLUP);
pinMode(START_BTN,INPUT_PULLUP);

pinMode(BUZZER,OUTPUT);
pinMode(ALERT,OUTPUT);

pinMode(LED1,OUTPUT);
pinMode(LED2,OUTPUT);
pinMode(LED3,OUTPUT);
pinMode(LED4,OUTPUT);
pinMode(LED5,OUTPUT);

lcd.init();
lcd.backlight();

/* STARTUP SCREEN */

lcd.setCursor(0,0);
lcd.print("TBM Finishing School");

lcd.setCursor(4,1);
lcd.print("Empowering");

lcd.setCursor(3,2);
lcd.print("Generations !");

startBeep();

delay(3000);

lcd.clear();

lcd.setCursor(0,0);
lcd.print("MIRROR MAZE SOLVING");

lcd.setCursor(0,2);
lcd.print("SET TIMER:");

}

void loop()
{

bool startState=digitalRead(START_BTN);
bool timerState=digitalRead(TIMER_BTN);

/* TIMER BUTTON */

if(!gameStart && timerState==LOW && lastTimerState==HIGH)
{

beep();

timerEnable = !timerEnable;
timerSelected = true;   // USER HAS CHOSEN TIMER

lcd.clear();
lcd.setCursor(0,0);
lcd.print("MIRROR MAZE SOLVING");

lcd.setCursor(0,2);
lcd.print("TIMER : ");

if(timerEnable)
lcd.print("5 MIN");
else
lcd.print("OFF");

delay(300);
}


// BONUS POINTS

if(gameStart && digitalRead(BONUS1)==LOW && !bonus1Taken)
{
  digitalWrite(LED1,HIGH);
  bonus1Taken=true;
  bonusCount++;
 coinBeep();
}

if(gameStart && digitalRead(BONUS2)==LOW && !bonus2Taken)
{
  digitalWrite(LED2,HIGH);
  bonus2Taken=true;
  bonusCount++;
  coinBeep();
}

if(gameStart && digitalRead(BONUS3)==LOW && !bonus3Taken)
{
  digitalWrite(LED3,HIGH);
  bonus3Taken=true;
  bonusCount++;
  coinBeep();
}

if(gameStart && digitalRead(BONUS4)==LOW && !bonus4Taken)
{
  digitalWrite(LED4,HIGH);
  bonus4Taken=true;
  bonusCount++;
  coinBeep();
}

if(gameStart && digitalRead(BONUS5)==LOW && !bonus5Taken)
{
  digitalWrite(LED5,HIGH);
  bonus5Taken=true;
  bonusCount++;
  coinBeep();
}

/* START BUTTON */

if(startState==LOW && lastStartState==HIGH)
{

beep();

// -------- START GAME --------
if(!gameStart)
{

// CHECK TIMER FIRST
if(!timerSelected)
{

lcd.clear();
lcd.setCursor(2,1);
lcd.print("SET TIMER FIRST");

longBeep();

delay(1500);

lcd.clear();
lcd.setCursor(0,0);
lcd.print("MIRROR MAZE SOLVING");
lcd.setCursor(0,2);
lcd.print("SET TIMER:");

return;

}


// RESET GAME DATA
errorCount=0;
bonusCount=0;

bonus1Taken=false;
bonus2Taken=false;
bonus3Taken=false;
bonus4Taken=false;
bonus5Taken=false;

digitalWrite(LED1,LOW);
digitalWrite(LED2,LOW);
digitalWrite(LED3,LOW);
digitalWrite(LED4,LOW);
digitalWrite(LED5,LOW);


// START SOUND


// COUNTDOWN
countdown();

gameStart=true;
startTime=millis();

lcd.clear();

}

// -------- STOP GAME --------
else
{

gameStart=false;
showResult();

}

delay(300);
}

lastStartState=startState;
lastTimerState=timerState;

/* GAME RUNNING */

if(gameStart)
{

if(digitalRead(MAZE)==LOW)
{

digitalWrite(BUZZER,HIGH);
digitalWrite(ALERT,HIGH);

if(!mazeTouched)
{
errorCount++;
mazeTouched=true;
}

}
else
{

digitalWrite(BUZZER,LOW);
digitalWrite(ALERT,LOW);
mazeTouched=false;

}

lcd.setCursor(0,0);
lcd.print("MIRROR MAZE SOLVING");

lcd.setCursor(0,1);
lcd.print("TIMER : ");

if(timerEnable)
{

unsigned long currentTime=millis()-startTime;

int sec=(timerLimit-currentTime)/1000;

int min=sec/60;
sec=sec%60;

lcd.print(min);
lcd.print(":");

if(sec<10) lcd.print("0");
lcd.print(sec);

}
else
lcd.print("OFF ");

lcd.setCursor(0,2);
lcd.print("ERROR : ");
lcd.print(errorCount);

lcd.setCursor(0,3);
lcd.print("BONUS : ");
lcd.print(bonusCount);
lcd.print("/5");

//timer out function 

if(timerEnable && millis()-startTime>=timerLimit)
{

gameStart=false;
winSound();
lcd.clear();
lcd.setCursor(6,0);
lcd.print("RESULT");


lcd.setCursor(0,1);
lcd.print("TIME : TIME OUT");

lcd.setCursor(0,2);
lcd.print("ERROR : ");
lcd.print(errorCount);

lcd.setCursor(0,3);
lcd.print("BONUS : ");
lcd.print(bonusCount);
lcd.print("/5");

}

if(digitalRead(ENDPOINT)==LOW)
{

gameStart=false;

winSound();   // special victory sound

showResult();

}

}

}

/* COUNTDOWN */

void countdown()
{

lcd.clear();


lcd.setCursor(0,1);
lcd.print("   GAME STARTS IN   ");

for(int i=3;i>0;i--)
{

lcd.setCursor(0,2);
lcd.print("      ");
lcd.setCursor(9,3);
lcd.print(i);

beep();

delay(1000);

}

lcd.clear();

lcd.setCursor(7,2);
lcd.print("READY");

longBeep();

delay(1000);

}

/* RESULT SCREEN */

void showResult()
{

unsigned long finishTime=millis()-startTime;

int sec=finishTime/1000;

int min=sec/60;
sec=sec%60;

lcd.clear();

lcd.setCursor(4,0);
lcd.print("RESULT :");

lcd.setCursor(0,1);
lcd.print("TIME : ");

lcd.print(min);
lcd.print(":");

if(sec<10) lcd.print("0");
lcd.print(sec);

lcd.setCursor(0,2);
lcd.print("ERROR : ");
lcd.print(errorCount);

lcd.setCursor(0,3);
lcd.print("BONUS : ");
lcd.print(bonusCount);
lcd.print("/5");


}
