#include <SPI.h>   // not used here, but needed to prevent a RTClib compile error
#include <SD.h>

const int LED = 3;
const int LED_N = 7;
const int LED_P = 6;
const int BUFFER_SIZE = 5;
const int chipSelect = 8;
const int INTERVAL = 5000;
unsigned long lastMillis = 0;

void initSDcard() {
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    initSDcard();
    return;
  }
  Serial.println("card initialized.");
}
void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  //analogReference(EXTERNAL);
  Serial.begin(9600);
  initSDcard();
  Serial.println("Setup done");

}

void loop() {
  if (millis() > lastMillis + INTERVAL) {
    unsigned long valHigh[BUFFER_SIZE];
    unsigned long valLow[BUFFER_SIZE];
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      //Measure
      valLow[i] = measureLight(LED_N, LED_P);
      digitalWrite(LED, HIGH);
      delay(20);
      valHigh[i] = measureLight(LED_N, LED_P);
      digitalWrite(LED, LOW);
    }
    //Sum up
    unsigned long sumLow = 0;
    unsigned long sumHigh = 0;
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
      sumLow += valLow[i];
      sumHigh += valHigh[i];
    }
    char text[30];
    unsigned long timeMs = millis() / 1000;
    sprintf(text, "%lu,%lu,%lu,%lu", timeMs , sumHigh , sumLow, sumLow - sumHigh);
    File dataFile = SD.open("datalog0.txt", FILE_WRITE);

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

unsigned long measureLight(int ledN, int ledP) {
  unsigned long j;

  // Apply reverse voltage, charge up the pin and led capacitance
  pinMode(ledN, OUTPUT);
  pinMode(ledP, OUTPUT);
  digitalWrite(ledN, HIGH);
  digitalWrite(ledP, LOW);

  // Isolate the pin 2 end of the diode
  pinMode(ledN, INPUT);
  digitalWrite(ledN, LOW); // turn off internal pull-up resistor
  unsigned long microsStart = micros();
  // Count how long it takes the diode to bleed back down to a logic zero
  for ( j = 0; j < 60000; j++) {
    if ( digitalRead(ledN) == 0) break;
  }
  unsigned long microsEnd = micros();
  //Serial.print(microsEnd - microsStart);
  //Serial.print("->");
  //Serial.println(j);
  return microsEnd - microsStart;
}

