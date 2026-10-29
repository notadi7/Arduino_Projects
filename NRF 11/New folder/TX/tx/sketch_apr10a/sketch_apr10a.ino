#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

#define buttonPin 2
#define redLedPin 3
#define greenLedPin 4

void setup() {
  Serial.begin(9600);
  
  pinMode(buttonPin, INPUT_PULLUP); // Button connected to GND and pin 2
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening(); // TX mode
}

void loop() {
  char command;
  int buttonState = digitalRead(buttonPin);

  if (buttonState == LOW) {
    // Button pressed → STOP
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);
    command = 'S';
  } else {
    // Button released → GO
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
    command = 'G';
  }

  bool success = radio.write(&command, sizeof(command));
  Serial.print("Sending: ");
  Serial.println(command);
  if (!success) Serial.println("Send failed");

  delay(300);
}
