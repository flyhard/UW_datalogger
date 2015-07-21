// Date, Time and Alarm functions using a DS3231 RTC connected via I2C and Wire lib

#include <Wire.h>
#include <SPI.h>   // not used here, but needed to prevent a RTClib compile error
#include <SD.h>
#include <avr/sleep.h>
#include <RTClib.h>
#include <OneWire.h>
#include <DallasTemperature.h>


const int INTERRUPT_PIN = 2; //interupt is on pin D2
const int INTERRUPT_NO = 0; // interupt is internally called 0
const int ONEWIRE_PIN = 8;
const int chipSelect = 10;
const int SERIAL_TOGGLE = 3;
const int SERIAL_TOGGLE_INT = 1;
const int SERIAL_STATE_LED = 4;
const int PWR_CTRL = 5;
volatile boolean serialOn = true;
volatile unsigned long button_time = 0;
volatile unsigned long last_button_time = 0;
volatile boolean serialTogglePressed = false;
volatile boolean wakeupPressed = false;
RTC_DS3231 RTC;
DeviceAddress TempSensorAddr = { 0x28, 0x8B, 0x42, 0x5C, 0x06, 0x00, 0x00, 0x7D};
OneWire ds(ONEWIRE_PIN);  // Connect your 1-wire device(tempSensor) to pin 8
DallasTemperature sensors(&ds);

void initSerial() {
  pinMode(SERIAL_TOGGLE, INPUT);
  pinMode(SERIAL_STATE_LED, OUTPUT);
  digitalWrite(SERIAL_STATE_LED, HIGH);
  digitalWrite(SERIAL_TOGGLE, HIGH);
  attachInterrupt(SERIAL_TOGGLE_INT, toggleSerial, LOW);
  Serial.begin(9600);
  serialOn = true;
}

void toggleSerial() {
  serialTogglePressed = true;
}

void setSerialMode() {
  if (digitalRead(SERIAL_TOGGLE) == LOW) {
    serialOn = !serialOn;
    if (serialOn) {
      Serial.begin(9600);
      Serial.println("Serial on");
    } else {
      Serial.println("Serial off");
      Serial.end();
    }
    blinkLED(3);
    delay(1000);
  }
}

void blinkLED(const int count) {
  for (int i = 0; i < count; i++) {
    if (serialOn) {
      digitalWrite(SERIAL_STATE_LED, LOW);
      delay(200);
      digitalWrite(SERIAL_STATE_LED, HIGH);
      delay(200);
    } else {
      digitalWrite(SERIAL_STATE_LED, HIGH);
      delay(200);
      digitalWrite(SERIAL_STATE_LED, LOW);
      delay(200);
    }
  }
  delay(1000);
}

void print(const char text[]) {
  if (serialOn)
    Serial.print(text);
}

void println(const char text[]) {
  if (serialOn)
    Serial.println(text);
}

void setup () {
  pinMode(PWR_CTRL, OUTPUT);
  digitalWrite(PWR_CTRL, HIGH);
  delay(500);
  initSerial();
  pinMode(INTERRUPT_PIN, INPUT);
  //pull up the interrupt pin
  digitalWrite(INTERRUPT_PIN, HIGH);


  discoverOneWireDevices();

  Wire.begin();
  RTC.begin();

  // Uncomment next line if you want to set the RTC to the compile time of the sketch at boot of the arduino
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  setNewAlarm(2);

  print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  println("card initialized.");
  println("setup done");
  sensors.begin();
  print("Parasite power is: ");
  if (sensors.isParasitePowerMode())
    println("ON");
  else
    println("OFF");
  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(TempSensorAddr, 9);
  print("Device 0 Resolution: ");
  if (serialOn)Serial.println(sensors.getResolution(TempSensorAddr), DEC);
  if (serialOn)Serial.println();
  print("Waiting for conversions: ");
  if (sensors.getWaitForConversion())
    println("TRUE");
  else
    println("FALSE");

}

void discoverOneWireDevices(void) {
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];

  println("Looking for 1-Wire devices...\n\r");
  while (ds.search(addr)) {
    print("\n\rFound \'1-Wire\' device with address:\n\r");
    for ( i = 0; i < 8; i++) {
      print("0x");
      if (addr[i] < 16) {
        if (serialOn)Serial.print('0');
      }
      if (serialOn)Serial.print(addr[i], HEX);
      if (i < 7) {
        print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
      println("CRC is not valid!\n");
      return;
    }
  }
  println("\n\r\n\rThat's it.\r\n");
  ds.reset_search();
  return;
}

void writeToSD(const String dataString) {
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    blinkLED(1);
  }
  // if the file isn't open, pop up an error:
  else {
    println("error opening datalog.txt");
    blinkLED(5);
  }
  if (serialOn)Serial.println(dataString);
}

void setNewAlarm(const int minutes) {
  DateTime now = RTC.now();
  now += 60; //+ 60 seconds
  int hour = now.hour();
  int minute = now.minute();
  RTC.setAlarm1Simple(hour, minute);
  //  RTC.setA1Time(now.day(),now.hour(),now.minute(),now.second(),)
  RTC.turnOnAlarm(1);
  RTC.turnOffAlarm(2);
  if (RTC.checkAlarmEnabled(1) ) {
    char text[25];
    sprintf(text, "Alarms Enabled for %02d:%02d", hour, minute);
    println(text);
  }
}

void loop () {
  DateTime now = RTC.now();
  if (RTC.checkIfAlarm(1)) {
    println("Alarm Triggered");
  }

  println("Going to Sleep");
  delay(600);
  sleepNow();
  println("AWAKE");
  if (serialTogglePressed) {
    setSerialMode();
    serialTogglePressed = false;
  }
  if (wakeupPressed) {
    setNewAlarm(1);

    //do something quick, flip a flag, and handle in loop();
    println("======================================");
    println("Log data");
    now = RTC.now();
    ds.reset();
    if (sensors.isConnected(TempSensorAddr))
      println("Sensor ready");
    else
      println("Sensor NOT ready");
    sensors.requestTemperatures();
    float temp = sensors.getTempC(TempSensorAddr);
    char str_temp[6];
    /* 4 is mininum width, 2 is precision; float value is copied onto str_temp*/
    dtostrf(temp, 4, 2, str_temp);
    char text[30];
    sprintf(text, "%4d-%02d-%02dT%02d:%02d:%02d, %sC", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second(), str_temp);
    writeToSD(text);
    println("======================================");
    wakeupPressed = false;
  }
}

void sleepNow() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  attachInterrupt(INTERRUPT_NO, logData, FALLING);
  digitalWrite(PWR_CTRL, LOW);
  byte adcsra_save = ADCSRA;
  ADCSRA = 0;  // disable ADC
  sleep_mode();
  //HERE AFTER WAKING UP
  sleep_disable();
  ADCSRA = adcsra_save;
  digitalWrite(PWR_CTRL, HIGH);
  detachInterrupt(INTERRUPT_NO);
}

void logData() {
  // Intentionally left blank
  wakeupPressed = true;
}
