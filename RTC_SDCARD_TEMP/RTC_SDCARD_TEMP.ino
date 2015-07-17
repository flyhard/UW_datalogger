// Date, Time and Alarm functions using a DS3231 RTC connected via I2C and Wire lib

#include <Wire.h>
#include <SPI.h>   // not used here, but needed to prevent a RTClib compile error
#include <SD.h>
#include <avr/sleep.h>
#include <RTClib.h>
#include <OneWire.h>


RTC_DS3231 RTC;
const int INTERRUPT_PIN = 2; //interupt is on pin D2
const int INTERRUPT_NO = 0; // interupt is internally called 0
const int chipSelect = 10;
volatile int state = LOW;

const byte TempSensorAddr[] = { 0x28, 0x8B, 0x42, 0x5C, 0x06, 0x00, 0x00, 0x7D};
OneWire  ds(8);  // Connect your 1-wire device(tempSensor) to pin 8

void setup () {
  pinMode(INTERRUPT_PIN, INPUT);
  //pull up the interrupt pin
  digitalWrite(INTERRUPT_PIN, HIGH);

  Serial.begin(9600);

  discoverOneWireDevices();

  Wire.begin();
  RTC.begin();

  // Uncomment next line if you want to set the RTC to the compile time of the sketch at boot of the arduino
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  setNewAlarm(2);
  Serial.println("setup done");
}

void discoverOneWireDevices(void) {
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];

  Serial.print("Looking for 1-Wire devices...\n\r");
  while (ds.search(addr)) {
    Serial.print("\n\rFound \'1-Wire\' device with address:\n\r");
    for ( i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.print("CRC is not valid!\n");
      return;
    }
  }
  Serial.print("\n\r\n\rThat's it.\r\n");
  ds.reset_search();
  return;
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

  File dataFile = SD.open("datalog.txt", FILE_WRITE);

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
  Serial.println("Going to Sleep");
  delay(600);
  sleepNow();
  Serial.println("AWAKE");
  setNewAlarm(1);

  //do something quick, flip a flag, and handle in loop();
  Serial.println("======================================");
  Serial.println("Log data");
  now = RTC.now();
  String dataString = ""+now.year();
  dataString += "-";
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
  // Intentionally left blank
}
