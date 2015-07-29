#include <SPI.h>   // not used here, but needed to prevent a RTClib compile error
#include <SD.h>

const int LED = 3;
const int SENSOR = 4;
const int BUFFER_SIZE = 5;
const int chipSelect = 10;
const int INTERVAL = 10000;
unsigned long lastMillis = 0;

void initSDcard() {
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}
void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  analogReference(EXTERNAL);
  Serial.begin(9600);
  initSDcard();
  Serial.println("Setup done");

}

void loop() {
  if (millis() > lastMillis + INTERVAL) {
    int valHigh[BUFFER_SIZE];
    int valLow[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      //Measure
      valLow[i] = RCtime(SENSOR);
      digitalWrite(LED, HIGH);
      delay(20);
      valHigh[i] = RCtime(SENSOR);
      digitalWrite(LED, LOW);
    }
    //Sum up
    int sumLow = 0;
    int sumHigh = 0;
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      sumLow += valLow[i];
      sumHigh += valHigh[i];
    }
    char text[30];
    int timeMs = millis() / 1000;
    sprintf(text, "%d,%d,%d,%d", timeMs , sumHigh , sumLow, sumLow - sumHigh);
    File dataFile = SD.open("datalog1.txt", FILE_WRITE);

    if (dataFile) {
      dataFile.println(text);
      dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening datalog.txt");
    }
    Serial.println(text);
    lastMillis += INTERVAL;
  }
}

// Uses a digital pin to measure a resistor (like an FSR or photocell!)
// We do this by having the resistor feed current into a capacitor and
// counting how long it takes to get to Vcc/2 (for most arduinos, thats 2.5V)
int RCtime(int RCpin) {
  int reading = 0;  // start with 0

  // set the pin to an output and pull to LOW (ground)
  pinMode(RCpin, OUTPUT);
  digitalWrite(RCpin, LOW);
  // Now set the pin to an input and...
  pinMode(RCpin, INPUT);
  while (digitalRead(RCpin) == LOW) { // count how long it takes to rise up to HIGH
    reading++;      // increment to keep track of time

    if (reading == 30000) {
      // if we got this far, the resistance is so high
      // its likely that nothing is connected!
      break;           // leave the loop
    }
  }
  // OK either we maxed out at 30000 or hopefully got a reading, return the count

  return reading;
}
