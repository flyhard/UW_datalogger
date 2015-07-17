// Date, Time and Alarm functions using a DS3231 RTC connected via I2C and Wire lib

#include <Wire.h>
#include <SPI.h>   // not used here, but needed to prevent a RTClib compile error
#include <SD.h>
#include <avr/sleep.h>
#include <RTClib.h>


RTC_DS3231 RTC;
const int INTERRUPT_PIN = 2; //interupt is on pin D2
const int INTERRUPT_NO = 0; // interupt is internally called 0
const int chipSelect = 10;
volatile int state = LOW;
boolean logComplete = true;

void setup () {
  pinMode(INTERRUPT_PIN, INPUT);
  //pull up the interrupt pin
  digitalWrite(INTERRUPT_PIN, HIGH);

  Serial.begin(9600);
  Wire.begin();
  RTC.begin();

  // Uncomment next line if you want to set the RTC to the compile time of the sketch at boot of the arduino
  RTC.adjust(DateTime(__DATE__, __TIME__));
  setNewAlarm(2);
  Serial.println("setup done");
}

void writeToSD(String dataString) {
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

  File dataFile = SD.open("datalogger.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }

}

void setNewAlarm(int minutes) {
  DateTime now = RTC.now();
  int hour = now.hour();
  int minute = now.minute() + minutes;
  if (minute > 59 | minute < 0) {
    hour = hour + 1;
    minute = 0;
  }
  RTC.setAlarm1Simple(hour, minute);
  RTC.turnOnAlarm(1);
  RTC.turnOffAlarm(2);
  if (RTC.checkAlarmEnabled(1) ) {
    Serial.print("Alarms Enabled for ");
    Serial.print(hour);
    Serial.print(":");
    Serial.println(minute);
  }
  delay(3000);
}

void loop () {
  DateTime now = RTC.now();
  if (RTC.checkIfAlarm(1)) {
    Serial.println("Alarm Triggered");
  }

  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
  while (!logComplete)
    delay(1000); // Wait for datalogging to complete
  Serial.println("Going to Sleep");
  delay(600);
  sleepNow();
  Serial.println("AWAKE");
  setNewAlarm(1);

}

void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(INTERRUPT_NO, logData, FALLING);
  sleep_mode();
  //HERE AFTER WAKING UP
  sleep_disable();
  detachInterrupt(INTERRUPT_NO);
}

void logData() {
  logComplete = false;
  //do something quick, flip a flag, and handle in loop();
  Serial.println("======================================");
  Serial.println("Log data");
  DateTime now = RTC.now();
  String dataString = now.year() + "-";
  dataString += now.month();
  dataString +=  "-";
  dataString +=  now.day() ;
  dataString +=  "T" ;
  dataString +=  now.hour() ;
  dataString +=  ":" ;
  dataString +=  now.minute() ;
  dataString +=  ":" ;
  dataString +=  now.second();
  writeToSD(dataString);
  Serial.println("======================================");
  logComplete = true;
}
