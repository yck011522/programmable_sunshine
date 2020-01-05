
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"

RTC_DS1307 rtc;
DateTime timeNow;

// OLED Display functions using Adafruit SSD1306 Library

#include "SPI.h"
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Encoder Connection
// CLK = Pin 2 (Interrupt)
// DT  = Pin 3 (Interrupt)
// SW  = Pin 4
// V+  = Pin 5
// GND = Pin 6

#include <Encoder.h>
Encoder dial(2, 3);
#define BUTTON_PIN 4

// Radio
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(9, 10); // CE, CSN         
const byte address[6] = "00001";     //Byte of array representing the address. This is the address where we will send the data. This should be same on the receiving side.


// Menu variables
unsigned int SettingsMode = 0;

// Alarm / Sunrise Variables
#include <EEPROM.h>
int alarmHours = EEPROM.read(0) % 24; // 8;
int alarmMinutes = EEPROM.read(1) % 60; // 15;
int alarmDuration = EEPROM.read(2) % 91; // 15;
double brightnessRateOfChange = 0.0;
double brightnessPercentage = 0.0;    // how bright the output is

// Single Press
// 0 - Toggle Light On Off

// Turn Dial
// - Initial Turn doesnt do anything to avoid accidential touch
// - Adjust Brightness
// - Cooldown will reset the 

// Long Press = Settings Mode
// 1 - Change Sunrise Time (Hour)
// 2 - Change Sunrise Time (Min)
// 3 - Change Sunrise Duration (Min)
// 4 - Change Current Time (Hour)
// 5 - Change Current Time (Min)

// Idle for a while

void setup() {
	Serial.begin(115200);

	// OLED Display Setup
	// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
		Serial.println(F("SSD1306 allocation failed"));
		for (;;); // Don't proceed, loop forever
	}
	// Real Time Clock Setup
	if (!rtc.begin()) {
		Serial.println("Couldn't find RTC");
		while (1);
	}

	if (!rtc.isrunning()) {
		Serial.println("RTC is NOT running!");
	}

	// Radio Setup
	radio.begin();                  //Starting the Wireless communication
	radio.openWritingPipe(address); //Setting the address where we will send the data
	radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
	radio.stopListening();          //This sets the module as transmitter


	// Button and Encoder input setup
	pinMode(4, INPUT_PULLUP);
	pinMode(5, OUTPUT);
	pinMode(6, OUTPUT);
	digitalWrite(5, HIGH);
	digitalWrite(6, LOW);

	// Show initial display buffer contents on the screen --
	// the library initializes this with an Adafruit splash screen.
	display.display();
	delay(1000); // Pause for 2 seconds

	// Clear the buffer
	display.clearDisplay();

}

void getTime() {

	timeNow = rtc.now();

}

void redraw() {
	display.clearDisplay();
	//Draw Time on Screen

	display.setTextSize(5);      // Normal 1:1 pixel scale
	display.setTextColor(WHITE); // Draw white text
	display.cp437(true);         // Use full 256 char 'Code Page 437' font

	// Format Hour and Minutes String
	String hour, minute, second;
	if (SettingsMode == 0 || SettingsMode == 4 || SettingsMode == 5) {
		hour = String(timeNow.hour(), DEC);
		minute = String(timeNow.minute(), DEC);
		second = String(timeNow.second(), DEC);
	}
	if (SettingsMode == 1 || SettingsMode == 2) { // Alarm Time
		hour = String(alarmHours, DEC);
		minute = String(alarmMinutes, DEC);
	}
	if (SettingsMode == 3 ) { // Duration
		minute = String(alarmDuration, DEC);
	}
	if (hour.length() == 1) hour = "0" + hour;
	if (minute.length() == 1) minute = "0" + minute;
	if (second.length() == 1) second = "0" + second;

	// Draw Hour
	display.setCursor(0, 11);
	for (int16_t i = 0; i < hour.length(); i++) {
		display.write(hour[i]);
	}

	// Draw Separater
	display.fillRect(61, 26, 2, 2, WHITE);
	display.fillRect(61, 31, 2, 2, WHITE);
	//display.write(':');

	// Draw Minutes
	display.setCursor(68, 11);
	for (int16_t i = 0; i < minute.length(); i++) {
		display.write(minute[i]);
	}

	// Draw Seconds
	//display.setCursor(58, 30);
	//display.setTextSize(1);      // Normal 1:1 pixel scale
	//for (int16_t i = 0; i < second.length(); i++) {
	//	display.write(second[i]);
	//}

	// Draw Menu Line
	display.setCursor(0, 0);
	display.setTextSize(1);
	String message;
	if (SettingsMode == 0) {
		//message = String("Current Time");
		message = String(" ");
	}
	if (SettingsMode == 1) {
		message = String("Set Sunrise Time (HH)");
	}
	if (SettingsMode == 2) {
		message = String("Set Sunrise Time (MM)");
	}
	if (SettingsMode == 3) {
		message = String("Set Sunrise Duration (Mins)");
	}
	if (SettingsMode == 4) {
		message = String("Set Current Time (HH)");
	}
	if (SettingsMode == 5) {
		message = String("Set Current Time (MM)");
	}

	for (int16_t i = 0; i < message.length(); i++) {
		display.write(message[i]);
	}

	// Draw Settings Changing Cursor
	if (SettingsMode == 1 || SettingsMode == 4) {
		display.fillRect(0, 51, 56, 2, WHITE);
	}
	if (SettingsMode == 2 || SettingsMode == 3 || SettingsMode == 5) {
		display.fillRect(68, 51, 56, 2, WHITE);
	}

	// Draw Bottom Text Line when lights are ON
	if (SettingsMode == 0) {
		if (brightnessPercentage > 0.01) {
			display.setCursor(0, 56);
			display.setTextSize(1);
			String debugLine = "Light ON: ";
			debugLine += String((int) brightnessPercentage, DEC);
			debugLine += "%";
			for (int16_t i = 0; i < debugLine.length(); i++) {
				display.write(debugLine[i]);
			}
		}
	}

	// Display everything
	display.display();
	delay(10);
}

void readButtons() {
	// Handle Button Press
	static int previousButtonState = HIGH;
	static long ButtonDownTime = 0;
	static long ButtonUpTime = 0;
	static int longPressFired = 0; //Long press must be longer then this amount

	const long shortPressThreshold = 100; //Short press must be longer then this amount
	const long longPressThreshold = 1500; //Long press must be longer then this amount

	int currentButtonState = digitalRead(BUTTON_PIN);
	if (currentButtonState == LOW && millis() - ButtonUpTime > 10) { //Pressed Down
		// On Key Down
		if (previousButtonState == HIGH) { 
			previousButtonState = LOW;
			ButtonDownTime = millis();
			longPressFired = 0;
			//handleKeyDown();
		}
		// Handle Long Press Timeout
		if (longPressFired == 0 && millis() - ButtonDownTime > longPressThreshold) {
			handleLongPress();
			longPressFired += 1;
			delay(100);
			return;
		}

	}

	if (currentButtonState == HIGH) { //Button Release
		// On Key Up
		if (previousButtonState == LOW) {
			previousButtonState = HIGH;
			ButtonUpTime = millis();
			if (millis() - ButtonDownTime > shortPressThreshold) {
				if (longPressFired == 0) { // Fire short press only if long press had not been fired.
					handleShortPress();
				}
			}
			return;
		}
	}

	// Handle Dial Rotation

}

void handleShortPress() {
	if (SettingsMode == 0) { // If not in settings mode, toggle light
		toggleLightOnOff();	
	} else {				 // If in settings mode, cycle between modes
		SettingsMode += 1;
		if (SettingsMode > 5) SettingsMode = 1;
		delay(200);			// Delay to avoid rotary dial bouncing after button release
	}
}

void handleLongPress() {
	if (SettingsMode == 0) { // If not in settings mode, go into settings mode:
		SettingsMode = 1;
	} else {				 // If in settings mode, get out
		SettingsMode = 0;
	}

}

void toggleLightOnOff() {
	if (brightnessPercentage > 0.01) {
		brightnessRateOfChange = -0.1;
	}
	else {
		brightnessRateOfChange = 0.1;
	}

}

void readDial() {
	static int32_t previousDialPos = 0;
	int32_t difference = (dial.read() - previousDialPos) * -0.5;
	if (difference != 0) {
		handleDial(difference);
		previousDialPos = dial.read();
	}
}

void handleDial(int steps) {
	//Adjust brightness
	if (SettingsMode == 0) {
		static long recentAdjustmentTime = 0;
		static long accumulatedMoves = 0;
		// Reset cooldown if last adjustment is long time ago
		if (millis() - recentAdjustmentTime > 5000) {
			accumulatedMoves = 0;
		}
		if (accumulatedMoves > 20) { 
			//Allow Adjustments
			brightnessPercentage += steps;
			if (brightnessPercentage > 100.0) brightnessPercentage = 100.0;
			if (brightnessPercentage < 0.0) brightnessPercentage = 0.0;
		}
		accumulatedMoves += abs(steps);
		recentAdjustmentTime = millis();
	}

	if (SettingsMode == 1) {
		alarmHours += steps;
		if (alarmHours > 23) alarmHours = 0;
		if (alarmHours < 0) alarmHours = 23;
		EEPROM.update(0, alarmHours);
	}
	if (SettingsMode == 2) {
		alarmMinutes += steps;
		if (alarmMinutes > 59) alarmMinutes = 0;
		if (alarmMinutes < 0) alarmMinutes = 23;
		EEPROM.update(1, alarmMinutes);
	}
	if (SettingsMode == 3) {
		alarmDuration += steps;
		if (alarmDuration > 99) alarmDuration = 0;
		if (alarmDuration < 0) alarmDuration = 99;
		EEPROM.update(2, alarmDuration);
	}
	if (SettingsMode == 4) {
		timeNow = timeNow + TimeSpan(0, steps, 0, 0);
		rtc.adjust(timeNow);
	}
	if (SettingsMode == 5) {
		timeNow = timeNow + TimeSpan(0, 0, steps, 0);
		rtc.adjust(timeNow);
	}

}

void computeBrightness() {
	static long lastComputeTime = 0;
	long deltaTime = millis() - lastComputeTime;
	lastComputeTime = millis();
	if (brightnessRateOfChange > 1.0E-10) { //Increasing
		brightnessPercentage += deltaTime * brightnessRateOfChange;
		if (brightnessPercentage > 100.0) { // Increased past Max Brightness
			brightnessPercentage = 100.0;
			brightnessRateOfChange = 0.0;
		}
	} else if (brightnessRateOfChange < -1.0E-10) {
		brightnessPercentage += deltaTime * brightnessRateOfChange;
		if (brightnessPercentage < 0.0) { // Decreased past Min Brightness
			brightnessPercentage = 0.0;
			brightnessRateOfChange = 0.0;
		}
	} else {

	}


}

void checkSunRiseTime() {
	static long lastSunRise = 0;
	if (millis() - lastSunRise > 70000) {
		if (timeNow.hour() == alarmHours && timeNow.minute() == alarmMinutes) {
			brightnessRateOfChange = 100.0 / 60 / 1000 / (double)alarmDuration;
		}
	}
}

void updateRemoteBrightness() {
	// Return if last transmission was not long ago
	// TBD


	// The percentage value is * 10 and converted to integer for transmission
	static String previousString = "";
	String transmissionString = "";
	//transmissionString = String((int)(brightnessPercentage * 10));
	
	// Return incase the value is the same
	if (transmissionString == previousString) return;
	previousString = String(transmissionString.c_str());
	

	// Send to radio
	//radio.write(transmissionString.c_str(), transmissionString.length());
}

void loop() {
	readButtons();
	readDial();
	getTime();
	redraw();
	computeBrightness();
	checkSunRiseTime();
	//updateRemoteBrightness();
}