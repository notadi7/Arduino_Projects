#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

/************ MPU6050 ************/
#define MPU_ADDR 0x68

/************ OBJECTS ************/
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial gsm(9, 8);

/************ PINS ************/
#define BUZZER_PIN 11
#define LED_PIN    12
#define BAT_ADC    A0

/************ SETTINGS ************/
#define FALL_G_THRESHOLD 2.5
#define PHONE_NUMBER "+918237043470"

/************ MPU ************/
int16_t ax, ay, az;
float accMagnitude;

/************ STATES ************/
bool fallState = false;

String smsStatus  = "WAIT";
String callStatus = "WAIT";

unsigned long smsTimer;
unsigned long callTimer;

/************ 🔥 EMERGENCY BUZZER ************/
bool buzzerActive = false;
bool buzzerState  = false;
unsigned long buzzerStart;
unsigned long buzzerToggle;

#define BUZZER_DURATION 7000
#define BUZZER_PERIOD   120

/************ CALL STATE MACHINE ************/
int callStage = 0;
unsigned long callStageTimer = 0;

/************ BATTERY ************/
int batteryPercent;


/**************** SETUP ****************/
void setup() {

  Serial.begin(9600);
  gsm.begin(9600);
  Wire.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  lcd.init();
  lcd.backlight();

  // wake MPU
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();
  
  lcd.setCursor(0,0);
  lcd.print("FALL SYSTEM");
  lcd.setCursor(0,1); 
  lcd.print("READY");
  delay(1500);
  lcd.clear();
}


/**************** LOOP ****************/
void loop() {

  readBattery();
  readAccel();
  detectFall();
  updateStatus();

  if (callStatus == "CALL")
    makeCall();        // NON-BLOCKING CALL

  playEmergencyTone();
  updateLCD();
}


/**************** BATTERY ****************/
void readBattery() {
  int adc = analogRead(BAT_ADC);
  float v = (adc * 5.0 / 1023.0) * 2;
  batteryPercent = constrain(map(v * 100, 280, 420, 0, 100), 0, 100);
}


/**************** MPU ****************/
void readAccel() {

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  ax = Wire.read()<<8 | Wire.read();
  ay = Wire.read()<<8 | Wire.read();
  az = Wire.read()<<8 | Wire.read();

  float Ax = ax/16384.0;
  float Ay = ay/16384.0;
  float Az = az/16384.0;

  accMagnitude = sqrt(Ax*Ax + Ay*Ay + Az*Az);
}


/**************** FALL DETECTION ****************/
void detectFall() {

  if (accMagnitude > FALL_G_THRESHOLD && !fallState) {

    fallState = true;

    // start buzzer immediately
    buzzerActive = true;
    buzzerStart = millis();
    buzzerToggle = millis();

    smsStatus = "SEND";
    sendSMS();
    smsTimer = millis();
  }
}


/**************** STATUS ****************/
void updateStatus() {

  unsigned long now = millis();

  // SMS finished
  if (smsStatus == "SEND" && now - smsTimer > 3000) {
    smsStatus = "SENT";

    callStatus = "CALL";
    callStage = 0;         // start call stages
  }

  // call finished
  if (callStatus == "DONE") {
    callStatus = "WAIT";
    smsStatus  = "WAIT";
    fallState  = false;
  }
}


/**************** 🔥 NON-BLOCKING CALL ****************/
void makeCall() {

  unsigned long now = millis();

  switch (callStage) {

    case 0:   // hang old call
      gsm.println("ATH");
      callStageTimer = now;
      callStage = 1;
      break;

    case 1:   // wait 1s → CFUN
      if (now - callStageTimer > 1000) {
        gsm.println("AT+CFUN=1");
        callStageTimer = now;
        callStage = 2;
      }
      break;

    case 2:   // wait 3s → dial
      if (now - callStageTimer > 3000) {
        gsm.print("ATD");
        gsm.print(PHONE_NUMBER);
        gsm.println(";");
        callStageTimer = now;
        callStage = 3;
      }
      break;

    case 3:   // ring 15s → hang
      if (now - callStageTimer > 15000) {
        gsm.println("ATH");
        callStage = 4;
      }
      break;

    case 4:   // finished
      callStatus = "DONE";
      callStage = 0;
      break;
  }
}


/**************** 🔥 EMERGENCY TONE ****************/
void playEmergencyTone() {

  if (!buzzerActive) return;

  unsigned long now = millis();

  if (now - buzzerStart > BUZZER_DURATION) {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    buzzerActive = false;
    return;
  }

  if (now - buzzerToggle > BUZZER_PERIOD) {
    buzzerToggle = now;
    buzzerState = !buzzerState;

    digitalWrite(BUZZER_PIN, buzzerState);
    digitalWrite(LED_PIN, buzzerState);
  }
}


/**************** LCD ****************/
void updateLCD() {

  lcd.setCursor(0,0);
  lcd.print("BAT:");
  lcd.print(batteryPercent);
  lcd.print("% ");

  lcd.print("F:");
  lcd.print(fallState ? "YES   " : "NO    ");

  lcd.setCursor(0,1);
  lcd.print("S:");
  lcd.print(smsStatus);
  lcd.print("   ");

  lcd.print("C:");
  lcd.print(callStatus);
  lcd.print("   ");
}


/**************** GSM ****************/
void sendSMS() {
  gsm.println("AT");
  delay(300);
  gsm.println("AT+CMGF=1");
  delay(300);
  gsm.print("AT+CMGS=\"");
  gsm.print(PHONE_NUMBER);
  gsm.println("\"");
  delay(300);
  gsm.print("FALL DETECTED!");
  gsm.write(26);
}
