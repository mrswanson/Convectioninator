#include <SD.h>
#include <MuxShield.h>
#include <Wire.h>
#include <Adafruit_MCP23008.h>
#include <LiquidCrystal.h>
#include "RTClib.h"

#define DEBUG 1

#define LOOP_DELAY_MS 100

typedef struct {
  int port;
  int pin;
} 
MuxMap;

typedef struct {
  int x;
  int y;
  int z;
} 
RawAccel;

typedef struct {
  long x;
  long y;
  long z;
} 
ConvertedAccel;

// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000

const int xInput = A13;
const int yInput = A14;
const int zInput = A15;

// Raw Ranges:
// initialize to mid-range and allow calibration to
// find the minimum and maximum for each axis

const int _oneGUnits = 19;
const int _16GUnits = _oneGUnits * 16;

int xRawMin = 512 - _16GUnits;
int xRawMax = 512 + _16GUnits;

int yRawMin = 512 - _16GUnits;
int yRawMax = 512 + _16GUnits;

int zRawMin = 512 - _16GUnits;
int zRawMax = 512 + _16GUnits;

int lowG = -1600;                    // low value of acceleration reading
int highG = 1600;                    // high value of acceleration reading
const int sampleSize = 10;           // number of acceleration samples to read at a time
RawAccel rawAccel;                   // current acceleration raw values (voltage)
ConvertedAccel convertedAccel;       // current acceleration converted to Gs
const int chipSelect = 10;           // for the SD Card shield
File dataFile;                       // file we write data to
RTC_DS1307 RTC;                      // define the Real Time Clock object
DateTime now;                        // current time
MuxShield muxShield;                 // reference to mux for temp reading
MuxMap thermoMuxLayout[16];          // array of MuxMap structs (to map thermos to mux port/pin)
float TemperatureReadingsInC[16];    // array of temperatures read from mux in C

LiquidCrystal lcd0(0);               // setup LCD screen 0
LiquidCrystal lcd1(1);               // setup LCD screen 1

void mapThermoMuxPortAndPin(int thermoIndex, int muxPort, int muxPin) {
  thermoMuxLayout[thermoIndex].port = muxPort;
  thermoMuxLayout[thermoIndex].pin = muxPin;
}

void setupMuxLayout() {
  mapThermoMuxPortAndPin(0, 1, 0);
  mapThermoMuxPortAndPin(1, 1, 1);
  mapThermoMuxPortAndPin(2, 1, 2);
  mapThermoMuxPortAndPin(3, 1, 3);
  mapThermoMuxPortAndPin(4, 1, 4);

  mapThermoMuxPortAndPin(5, 2, 0);
  mapThermoMuxPortAndPin(6, 2, 1);
  mapThermoMuxPortAndPin(7, 2, 2);
  mapThermoMuxPortAndPin(8, 2, 3);
  mapThermoMuxPortAndPin(9, 2, 4);

  mapThermoMuxPortAndPin(10, 1, 11);
  mapThermoMuxPortAndPin(11, 1, 12);
  mapThermoMuxPortAndPin(12, 1, 13);
  mapThermoMuxPortAndPin(13, 1, 14);
  mapThermoMuxPortAndPin(14, 1, 15);

  mapThermoMuxPortAndPin(15, 2, 11);
  mapThermoMuxPortAndPin(16, 2, 12);
}

void setupMux() {
  muxShield.setMode(1,ANALOG_IN);
  muxShield.setMode(2,ANALOG_IN);
  muxShield.setMode(3,ANALOG_IN);
}

void setupAccelerometer() {
  analogReference(EXTERNAL);
  int xRaw = ReadAxis(xInput);
  int yRaw = ReadAxis(yInput);
  int zRaw = ReadAxis(zInput);
  delay(3000);
  xRaw = ReadAxis(xInput);
  yRaw = ReadAxis(yInput);
  zRaw = ReadAxis(zInput);

  Serial.print("Raw values at start: ");

  Serial.print(xRaw);
  Serial.print(", ");
  Serial.print(yRaw);
  Serial.print(", ");
  Serial.print(zRaw);
  Serial.println(); 
}

void setupLCDScreens() {
  // set up the LCD's number of rows and columns: 
  lcd0.begin(20, 4);
  lcd1.begin(20, 4);

  lcd0.setBacklight(HIGH);
  lcd1.setBacklight(HIGH);
}

void setupSDCard() {
  Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);
  // see if the card is present and can be initialized:
  if (!SD.begin(10,11,12,13)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1) ;
  }
#ifdef DEBUG
  Serial.println("card initialized.");
#endif
}

void setupClock() {
  if (!RTC.begin()) {
    Serial.println("RTC failed");
  }
  RTC.adjust(DateTime(__DATE__, __TIME__));
}

void openDataFile() {
  // Open up the file we're going to log to!
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (! dataFile) {
    Serial.println("error opening datalog.txt");
    // Wait forever since we cant write data
    while (1) ;
  }
}

void setup() {
  Serial.begin(9600);

  setupLCDScreens();
  setupSDCard();
  setupMuxLayout();
  setupMux();  
  setupAccelerometer();

  openDataFile();

  dataFile.println("Beginning of new data file.");
}

/////////////////////////////////////////////////////////////
// Temperature reading
/////////////////////////////////////////////////////////////
float thermistorReadingToResistance(float reading) {
  if (reading) {
    reading = 1023 / reading  - 1;
    return (SERIESRESISTOR / reading);
  }
  return reading;
}

float resistanceToC(float reading) {
  // convert the value to resistance
  reading = thermistorReadingToResistance(reading);

  float steinhart;
  steinhart = reading / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;  // convert to C
  //  steinhart = (steinhart * 9/5) + 32;
  return steinhart;
}

void readTemperaturesFromMux() {
  for (int x = 0; x < 16; x++) {
    MuxMap thermo = thermoMuxLayout[x];
    float reading = muxShield.analogReadMS(thermo.port,thermo.pin);
    TemperatureReadingsInC[x] = resistanceToC(reading);

#ifdef DEBUG
    Serial.print("port: ");
    Serial.print(thermo.port);
    Serial.print(" pin: ");
    Serial.print(thermo.pin);
    Serial.println();
    Serial.println(reading);
#endif
  }
}


/////////////////////////////////////////////////////////////
// Accelerometer reading
/////////////////////////////////////////////////////////////
int ReadAxis(int axisPin)
{
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < sampleSize; i++)
  {
    reading += analogRead(axisPin);
  }
  return reading/sampleSize;
}

float convertRawAccel(int accel, int min, int max) {
  long scaled = map(accel, min, max, lowG, highG);
  return scaled / 100;
}

void readAccel() {
  int xRaw = ReadAxis(xInput);
  int yRaw = ReadAxis(yInput);
  int zRaw = ReadAxis(zInput);

  rawAccel.x = xRaw;
  rawAccel.y = yRaw;
  rawAccel.x = zRaw;

  convertedAccel.x = convertRawAccel(xRaw, xRawMin, xRawMax);
  convertedAccel.y = convertRawAccel(yRaw, yRawMin, yRawMax);
  convertedAccel.z = convertRawAccel(zRaw, zRawMin, zRawMax);
}

/////////////////////////////////////////////////////////////
// Read time from Real Time Clock (RTC)
/////////////////////////////////////////////////////////////
void readTime() {
  now = RTC.now();
}

/////////////////////////////////////////////////////////////
// Display values on LCD Screens
/////////////////////////////////////////////////////////////
void displayTemperaturesOnLCD(LiquidCrystal lcd) {
  int row = 0;
  int col = 0;
  for (int x = 0; x <= 15; x++) {
    lcd.setCursor(col, row);
    float reading = TemperatureReadingsInC[x]; // first row of MUX only
    if (reading < 60 && reading > 0) {
      lcd.print(reading, 1);
    }
    else {
      lcd.print(00.0, 1);
    }

    if (col >= 15) {
      // wrap around to next row
      col = 0;
      row++;
    }
    else {
      // move over 5 spaces for next temp reading
      col += 5;
    }
  }
}

void displayOtherStuffOnLCD(LiquidCrystal lcd) {
  lcd.setCursor(0,0);
  lcd.print("Hello LCD 1 foo");
}

void loop() {
  readTime();
  readAccel();
  readTemperaturesFromMux();

  displayTemperaturesOnLCD(lcd0);
  displayOtherStuffOnLCD(lcd1);

  delay(LOOP_DELAY_MS);
}

