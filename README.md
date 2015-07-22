# UW_datalogger
A datalogger project for underwater deployment for logging temperature and other data

* Gets time from RTC
* Saves all data to SDcard
* Gets temperature from DS18B20 temperature sensor


To get this the code working you need to put the right bootloader on the arduino: 
* https://www.arduino.cc/en/Tutorial/ArduinoToBreadboard and http://www.instructables.com/id/Setup-Arduino-Software-for-Atmega328P-with-Interna/?ALLSTEPS are very helpfull here.

Remember, if you have a Arduino that is not configured to use the internal clock you need to connect the resonator while programming it!
