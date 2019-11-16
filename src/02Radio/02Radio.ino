/*
 Name:		_02Radio.ino
 Created:	11/16/2019 6:26:54 PM
 Author:	Victor
*/

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9, 10); // CE, CSN         
const byte address[6] = "00001";     //Byte of array representing the address. This is the address where we will send the data. This should be same on the receiving side.

void setup() {
	radio.begin();                  //Starting the Wireless communication
	radio.openWritingPipe(address); //Setting the address where we will send the data
	radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
	radio.stopListening();          //This sets the module as transmitter

	Serial.begin(115200);
	Serial.println(F("Two way serial repeater example"));

	const char text[] = "I am a radio that just wokeup.";
	radio.write(&text, sizeof(text));                  //Sending the message to receiver
	radio.startListening();

}
void loop() {
	if (Serial.available()) {
		String text;
		text = Serial.readStringUntil('\n');
		radio.stopListening();          //This sets the module as transmitter
		radio.write(text.c_str(), text.length());
		radio.startListening();
		Serial.print("Sent:");
		Serial.println(text);
	}

	if (radio.available()) {
		char buffer[100];
		while (radio.available()) {
			radio.read(&buffer, sizeof(buffer));
		}

		Serial.println(buffer);
	}

	delay(1);
}