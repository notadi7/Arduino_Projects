//smart helmet code by ELECTROGENIX! (BIKE CODE)

#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

SoftwareSerial bt(8, 9);
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define RELAY   10
#define BUZZER  12
#define LIGHT   13

// States
#define SHOW_PROJECT   0
#define BT_CONNECTING  1
#define BT_CONNECTED   2
#define RUNNING        3
#define BT_FAILED      4

int systemState = SHOW_PROJECT;

// Data
bool helmet=false, alcohol=false, rain=false, fall=false, sleep=false;
int mqValue=0;

// Timers
unsigned long stateTimer=0, buzzerTimer=0, rainBuzzTimer=0, displayTimer=0;
bool buzzerState=false, rainBuzzActive=false, altDisplay=false;

void setup() {
  bt.begin(9600);
  Serial.begin(9600);

  pinMode(RELAY, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LIGHT, OUTPUT);

  lcd.init();
  lcd.backlight();

  lcd.print("SMART HELMET");
  lcd.setCursor(0,1);
  lcd.print("SAFETY SYSTEM");

  stateTimer = millis();
}

void loop() {

  if (bt.available()) {
    String d = bt.readStringUntil('\n');
    parseData(d);

    if (systemState==BT_CONNECTING) {
      systemState=BT_CONNECTED;
      lcd.clear();
      lcd.print("Bluetooth OK");
      stateTimer=millis();
    }
  }

  switch(systemState) {

    case SHOW_PROJECT:
      if (millis()-stateTimer>3000) {
        lcd.clear();
        lcd.print("Connecting BT");
        systemState=BT_CONNECTING;
        stateTimer=millis();
      }
      break;

    case BT_CONNECTING:
      blinkBuzzer(400);
      engineOff();
      if (millis()-stateTimer>20000) {
        lcd.clear();
        lcd.print("BT FAILED");
        digitalWrite(BUZZER,HIGH);
        systemState=BT_FAILED;
      }
      break;

    case BT_CONNECTED:
      if (millis()-stateTimer>2000) {
        lcd.clear();
        systemState=RUNNING;
      }
      break;

    case RUNNING:
      handleRain();
      handleEngine();
      cycleDisplay();
      break;

    case BT_FAILED:
      engineOff();
      break;
  }
}

/************ PARSE ************/
void parseData(String d) {
  char h,a,r,f,l;
  int mq;
  if (sscanf(d.c_str(),"%c %c %c %c %c %d",&h,&a,&r,&f,&l,&mq)==6) {
    helmet=(h=='H');
    alcohol=(a=='A');
    rain=(r=='R');
    fall=(f=='F');
    sleep=(l=='L');
    mqValue=mq;
  }
}

/************ DISPLAY ************/
void cycleDisplay() {
  if (millis()-displayTimer>2000) {
    altDisplay=!altDisplay;
    displayTimer=millis();
  }

  if (!altDisplay) showMainLCD();
  else showSleepLCD();
}

void showMainLCD() {
  lcd.setCursor(0,0);
  lcd.print("H:");
  lcd.print(helmet?"Y":"N");
  lcd.print(" A:");
  lcd.print(alcohol?"Y":"N");
  lcd.print(" ");
  lcd.print(mqValue);

  lcd.setCursor(0,1);
  lcd.print("R:");
  lcd.print(rain?"Y":"N");
  lcd.print(" F:");
  lcd.print(fall?"Y":"N");
  lcd.print(" EN:");
  lcd.print((helmet&&!alcohol&&!fall&&!sleep)?"OK":"FL");
}

void showSleepLCD() {
  lcd.setCursor(0,0);
  lcd.print("SLEEP:");
  lcd.print(sleep?"YES                     ":"NO                ");
  lcd.setCursor(0,1);
  lcd.print("ENGINE:");
  lcd.print((helmet&&!alcohol&&!fall&&!sleep)?"OK            ":"FL                  ");
}

/************ ENGINE ************/
void handleEngine() {
  if (fall || alcohol || sleep) {
    digitalWrite(BUZZER,HIGH);
    engineOff();
    return;
  }
  if (!helmet) {
    blinkBuzzer(600);
    engineOff();
    return;
  }
  digitalWrite(BUZZER,LOW);
  digitalWrite(RELAY,HIGH);
}

/************ RAIN ************/
void handleRain() {
  if (rain && !rainBuzzActive) {
    rainBuzzActive=true;
    rainBuzzTimer=millis();
    digitalWrite(LIGHT,HIGH);
  }
  if (rainBuzzActive) {
    if (millis()-rainBuzzTimer<5000)
      digitalWrite(BUZZER,(millis()/300)%2);
    else rainBuzzActive=false;
  }
  if (!rain) digitalWrite(LIGHT,LOW);
}

void engineOff() {
  digitalWrite(RELAY,LOW);
}

void blinkBuzzer(unsigned long t) {
  if (millis()-buzzerTimer>t) {
    buzzerTimer=millis();
    buzzerState=!buzzerState;
    digitalWrite(BUZZER,buzzerState);
  }
}
