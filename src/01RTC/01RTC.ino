/*
 Name:		_01RTC.ino
 Created:	11/3/2019 10:40:24 PM
 Author:	Victor
*/

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"

RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
unsigned int alarmHours = 8; // 8;
unsigned int alarmMinutes = 0; // 15;

// Light Control
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX (Only pin 11 matters becuase the dimmer is write only)
const int minimum_brightness = 0;
const int maximum_brightness = 220;
int brightness = minimum_brightness;    // how bright the output is
int fadeDelay = 2727;    // 2727*220/1000/60 = 10 mins


// the setup function runs once when you press reset or power the board
void setup() {


	while (!Serial); // for Leonardo/Micro/Zero

	Serial.begin(115200);

	if (!rtc.begin()) {
		Serial.println("Couldn't find RTC");
		while (1);
	}

	if (!rtc.isrunning()) {
		Serial.println("RTC is NOT running!");
		// following line sets the RTC to the date & time this sketch was compiled
		// rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
		// This line sets the RTC with an explicit date & time, for example to set
		// January 21, 2014 at 3am you would call:
	}

	mySerial.begin(9600);
	mySerial.print("A050V");
	delay(1000);
	mySerial.print("A000V");


}

void sunRise() {

	// change the brightness for next time through the loop:
	for (int i = 0; i <= maximum_brightness; i++) {

		brightness = i;

		// Transmit Dinner control command.
		String str = "A";
		if (brightness < 100) str += "0";
		if (brightness < 10) str += "0";
		str += String(brightness);
		str += "V";
		mySerial.print(str);
		Serial.println(str);

		// delay and repeat
		delay(fadeDelay);

	}
}


// the loop function runs over and over again until power down or reset
void loop() {
	DateTime now = rtc.now();

	Serial.print(now.year(), DEC);
	Serial.print('/');
	Serial.print(now.month(), DEC);
	Serial.print('/');
	Serial.print(now.day(), DEC);
	Serial.print(" (");
	Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
	Serial.print(") ");
	Serial.print(now.hour(), DEC);
	Serial.print(':');
	Serial.print(now.minute(), DEC);
	Serial.print(':');
	Serial.print(now.second(), DEC);
	Serial.println();

	delay(3000);

	if (now.hour() == alarmHours && now.minute() == alarmMinutes) {
		sunRise();
	}
}
