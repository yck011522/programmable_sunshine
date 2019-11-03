# programmable_sunshine
Programmable alarm clock with light bulb dimmer.

## Hardware - Bedside Module

| Hardware                | Use                 | Communication           |
| ----------------------- | ------------------- | ----------------------- |
| Push Button             | Light On/Off Toggle | Digital Pin             |
| Encoder / Rotary Button | Menu selection      | Digital Pin (Interrupt) |
| LCD Screen              | Menu                |                         |
| NRF24L01                | Radio               | SPI + 2 digital pins    |
| DS1307 Real Time Clock  | Clock               | **A4** and **A5** pins. |

 

## Hardware - Bedside Module



## Programming

### RTC DS1307 Programming

 https://learn.adafruit.com/adafruit-data-logger-shield/using-the-real-time-clock 

### Radio Programming

Reference:

https://create.arduino.cc/projecthub/muhammad-aqib/nrf24l01-interfacing-with-arduino-wireless-communication-0c13d4 



### Alarm Clock Programming

 https://create.arduino.cc/projecthub/Tittiamo/alarm-clock-f61bad 