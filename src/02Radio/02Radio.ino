/*
The radio expects to receive a value that ranges from 0 to 1000
The received value will be mapped to the 0 to 220V range and sent to the dimmer via serial
The command to the dimmer is in format "AxxxV" where xxx is a three digit number padded with 0
*/

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

// Radio (NRF24L01)
RF24 radio(9, 10); // Radio CE, CSN pins
const byte address[6] = "pgsun";    // Address for receiving. Must match with sender

// Dimmer Control
#include <SoftwareSerial.h>
SoftwareSerial mySerial(6,7);		// RX, TX (Only pin 7 TX matters becuase the dimmer is write only)
const int minBrightness = 0;	// Minimal Value that will make the lightblub glow
const int maxBrightness = 220; // Maximum Value
int currentBrightness = 0;    // how bright the output is


void setup() {
	// Radio Setup
	radio.begin();                  //Starting the Wireless communication
	radio.setPALevel(RF24_PA_LOW);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
	radio.openReadingPipe(1, address);
	radio.startListening();

	// Serial Connection Setup with Dimmer
	mySerial.begin(9600);
	mySerial.print("A050V");
	delay(1000);
	mySerial.print("A000V");

	// Debug Setup
	Serial.begin(115200);
	Serial.println(F("Radio Listener and Dimmer Controller"));

}
void loop() {

	if (radio.available()) {
		// Read String from radio
		char buffer[20];
		while (radio.available()) {
			radio.read(&buffer, sizeof(buffer));
		}

		// String to Value Convertion
		int value = String(buffer).toInt();
		if (value == 0) currentBrightness = 0;
		else currentBrightness = (double) value / 1000.0 * (maxBrightness - minBrightness) + minBrightness;

		// Format Dimmer Control Command.
		String str = "A";
		if (currentBrightness < 100) str += "0";
		if (currentBrightness < 10) str += "0";
		str += String(currentBrightness);
		str += "V";

		// Send String to Dimmer
		mySerial.print(str);

		// Debug Messages via USB Serial
		Serial.print(F("Received:"));
		Serial.print(buffer);
		Serial.print(F(" Dimmer:"));
		Serial.println(str);
	}

	delay(1);
}