#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 9
#define CSN_PIN 10

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

#define relayPin 5

void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);

  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening(); // RX mode
}

void loop() {
  if (radio.available()) {
    char received;
    radio.read(&received, sizeof(received));

    Serial.print("Received: ");
    Serial.println(received);

    if (received == 'S') {
      digitalWrite(relayPin, HIGH); // Stop motor
    } else if (received == 'G') {
      digitalWrite(relayPin, LOW); // Start motor
    }
  }
}
