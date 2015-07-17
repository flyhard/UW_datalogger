// Date, Time and Alarm functions using a DS3231 RTC connected via I2C and Wire lib

#include <Wire.h>
#include <SPI.h>   // not used here, but needed to prevent a RTClib compile error
#include <avr/sleep.h>
#include <RTClib.h>


RTC_DS3231 RTC;
int INTERRUPT_PIN = 2;
int LED = 13;
volatile int state = LOW;

void setup () {
  pinMode(INTERRUPT_PIN, INPUT);
  pinMode(LED, OUTPUT);
  //pull up the interrupt pin
  digitalWrite(INTERRUPT_PIN, HIGH);
  digitalWrite(LED, LOW);

  Serial.begin(9600);
  Wire.begin();
  RTC.begin();

  // Uncomment next line if you want to set the RTC to the compile time of the sketch at boot of the arduino
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  setNewAlarm(2);
  Serial.println("setup done");
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
}

void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(0, logData, FALLING);
  sleep_mode();
  //HERE AFTER WAKING UP
  sleep_disable();
  detachInterrupt(0);
}

void logData() {
  //do something quick, flip a flag, and handle in loop();
  Serial.println("======================================");
  Serial.println("Log data");
  Serial.println("======================================");
}
