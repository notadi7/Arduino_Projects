//smart helmet code by ELECTROGENIX! (HELMET CODE)
#include <SoftwareSerial.h>
#include <Wire.h>
#include <TinyGPSPlus.h>

/************ PINS ************/
#define IR_PIN     2
#define RAIN_PIN   3
#define BLINK_PIN  4
#define MQ_PIN     A0
#define MPU_ADDR   0x68

/************ SERIAL ************/
SoftwareSerial bt(8, 9);       // HC-05
SoftwareSerial gsm(10, 11);    // SIM800L
SoftwareSerial gpsSerial(6, 5);// GPS

TinyGPSPlus gps;

/************ SETTINGS ************/
int alcoholThreshold = 500;
#define FALL_G_THRESHOLD 2.0
#define FALL_TIME        5000
#define SLEEP_TIME       5000

/************ MPU ************/
int16_t ax, ay, az;
float Ax, Ay, Az, accMagnitude;

/************ STATES ************/
bool fallState = false;
bool sleepState = false;
unsigned long fallTime = 0;
unsigned long sleepTime = 0;

bool smsAlcohol=false, smsRain=false, smsFall=false, smsSleep=false;
unsigned long lastHeartbeat = 0;

void setup() {
  pinMode(IR_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT);
  pinMode(BLINK_PIN, INPUT);

  Serial.begin(9600);
  bt.begin(9600);
  gsm.begin(9600);
  gpsSerial.begin(9600);
  Wire.begin();

  // Wake MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();

  Serial.println("MQ Warming...");
  delay(20000);   // MQ warm-up
  Serial.println("Helmet Ready");
}

void loop() {

  // ---------- GPS UPDATE ----------
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  // ---------- HEARTBEAT ----------
  if (millis() - lastHeartbeat > 1000) {
    bt.print("C ");
    lastHeartbeat = millis();
  }

  // ---------- HELMET ----------
  char h = digitalRead(IR_PIN) ? 'N' : 'H';

  // ---------- MQ ----------
  int mq = readMQ();
  char a = (mq > alcoholThreshold) ? 'A' : 'S';

  if (a=='A' && !smsAlcohol) {
    sendSMS("ALERT! Alcohol detected");
    smsAlcohol = true;
  }
  if (a=='S') smsAlcohol = false;

  // ---------- RAIN ----------
  char r = digitalRead(RAIN_PIN) ? 'R' : 'D';

  if (r=='R' && !smsRain) {
    sendSMS("ALERT! Rain detected");
    smsRain = true;
  }
  if (r=='D') smsRain = false;

  // ---------- FALL ----------
  readAccel();
  detectFall();
  char f = fallState ? 'F' : 'O';

  if (fallState && !smsFall) {
    sendSMS("EMERGENCY! FALL DETECTED");
    smsFall = true;
  }
  if (!fallState) smsFall = false;

  // ---------- SLEEP ----------
  detectSleep();
  char l = sleepState ? 'L' : 'O';

  if (sleepState && !smsSleep) {
    sendSMS("ALERT! DRIVER SLEEPING");
    smsSleep = true;
  }
  if (!sleepState) smsSleep = false;

  // ---------- SEND DATA ----------
  bt.print(h); bt.print(" ");
  bt.print(a); bt.print(" ");
  bt.print(r); bt.print(" ");
  bt.print(f); bt.print(" ");
  bt.print(l); bt.print(" ");
  bt.println(mq);

  delay(500);
}

/************ MQ ************/
int readMQ() {
  int sum = 0;
  for (int i=0;i<10;i++) {
    sum += analogRead(MQ_PIN);
    delay(5);
  }
  return sum / 10;
}

/************ MPU READ ************/
void readAccel() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR,6,true);

  ax = Wire.read()<<8 | Wire.read();
  ay = Wire.read()<<8 | Wire.read();
  az = Wire.read()<<8 | Wire.read();

  Ax = ax / 16384.0;
  Ay = ay / 16384.0;
  Az = az / 16384.0;

  accMagnitude = sqrt(Ax*Ax + Ay*Ay + Az*Az);
}

/************ FALL LOGIC ************/
void detectFall() {
  if (accMagnitude > FALL_G_THRESHOLD && !fallState) {
    fallState = true;
    fallTime = millis();
  }
  if (fallState && millis() - fallTime > FALL_TIME) {
    fallState = false;
  }
}

/************ SLEEP LOGIC ************/
void detectSleep() {
  if (digitalRead(BLINK_PIN)==LOW && !sleepState) {
    sleepState = true;
    sleepTime = millis();
  }
  if (sleepState && millis() - sleepTime > SLEEP_TIME) {
    sleepState = false;
  }
}

/************ GSM ************/
void sendSMS(String msg) {

  String smsText = msg;

  if (gps.location.isValid()) {

    float lat = gps.location.lat();
    float lon = gps.location.lng();

    smsText += "\nLAT:" + String(lat,6);
    smsText += " LNG:" + String(lon,6);

    smsText += "\nMAP:https://maps.google.com/?q=";
    smsText += String(lat,6);
    smsText += ",";
    smsText += String(lon,6);

  } else {
    smsText += "\nLAT:NA LNG:NA";
    smsText += "\nMAP:NOT AVAILABLE";
  }

  gsm.println("AT+CMGF=1");
  delay(500);
  gsm.println("AT+CMGS=\"+919404411360\"");
  delay(500);
  gsm.print(smsText);
  delay(300);
  gsm.write(26);
}
