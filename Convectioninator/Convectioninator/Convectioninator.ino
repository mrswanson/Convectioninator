#include <SD.h>
#include <MuxShield.h>
#include <Wire.h>
#include <Adafruit_MCP23008.h>
#include <LiquidCrystal.h>
#include "RTClib.h"

typedef struct {
  int port;
  int pin;
} MuxMap;

typedef struct {
  int x;
  int y;
  int z;
} RawAccel;

typedef struct {
  long x;
  long y;
  long z;
} ConvertedAccel;

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

int lowG = -1600;
int highG = 1600;
// Take multiple samples to reduce noise
const int sampleSize = 10;

RawAccel rawAccel;
ConvertedAccel convertedAccel;

const int chipSelect = 10;
File dataFile;
RTC_DS1307 RTC; // define the Real Time Clock object
DateTime now;
MuxShield muxShield;
MuxMap thermoMuxLayout[16];
float TemperatureReadingsInC[16];

// Connect via i2c, default address #0 (A0-A2 not jumpered)
LiquidCrystal lcd0(0);
LiquidCrystal lcd1(1);


void setupMuxLayout() {
  thermoMuxLayout[0].port = 1;
  thermoMuxLayout[0].pin = 0;
  thermoMuxLayout[1].port = 1;
  thermoMuxLayout[1].pin = 1;
  thermoMuxLayout[2].port = 1;
  thermoMuxLayout[2].pin = 2;
  thermoMuxLayout[3].port = 1;
  thermoMuxLayout[3].pin = 3;
  thermoMuxLayout[4].port = 1;
  thermoMuxLayout[4].pin = 4;
  
  thermoMuxLayout[5].port = 2;
  thermoMuxLayout[5].pin = 0;
  thermoMuxLayout[6].port = 2;
  thermoMuxLayout[6].pin = 1;
  thermoMuxLayout[7].port = 2;
  thermoMuxLayout[7].pin = 2;
  thermoMuxLayout[8].port = 2;
  thermoMuxLayout[8].pin = 3;
  thermoMuxLayout[9].port = 2;
  thermoMuxLayout[9].pin = 4;
  
  thermoMuxLayout[10].port = 1;
  thermoMuxLayout[10].pin = 11;
  thermoMuxLayout[11].port = 1;
  thermoMuxLayout[11].pin = 12;
  thermoMuxLayout[12].port = 1;
  thermoMuxLayout[12].pin = 13;
  thermoMuxLayout[13].port = 1;
  thermoMuxLayout[13].pin = 14;
  thermoMuxLayout[14].port = 1;
  thermoMuxLayout[14].pin = 15;
  
  thermoMuxLayout[15].port = 2;
  thermoMuxLayout[15].pin = 11;
  thermoMuxLayout[16].port = 2;
  thermoMuxLayout[16].pin = 12;
}

void setup() {
  setupMuxLayout();
  Serial.begin(9600);
  // set up the LCD's number of rows and columns: 
  lcd0.begin(20, 4);
  lcd1.begin(20, 4);

  lcd0.setBacklight(HIGH);
  lcd1.setBacklight(HIGH);
  
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

  muxShield.setMode(1,ANALOG_IN);
  muxShield.setMode(2,ANALOG_IN);
  muxShield.setMode(3,ANALOG_IN);
  
  Serial.print("Initializing SD card...");
  pinMode(SS, OUTPUT);
  // see if the card is present and can be initialized:
  if (!SD.begin(10,11,12,13)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1) ;
  }
  Serial.println("card initialized.");

  if (!RTC.begin()) {
    Serial.println("RTC failed");
  }
  

  // Open up the file we're going to log to!
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (! dataFile) {
    Serial.println("error opening datalog.txt");
    // Wait forever since we cant write data
    while (1) ;
  }
  
    dataFile.println("Hello world.");

//  RTC.adjust(DateTime(__DATE__, __TIME__));
    RTC.adjust(DateTime("Dec 26 2009","12:34:56"));
}

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
      Serial.print("port: ");
      Serial.print(thermo.port);
      Serial.print(" pin: ");
      Serial.print(thermo.pin);
      Serial.println();
      float reading = muxShield.analogReadMS(thermo.port,thermo.pin);
      Serial.println(reading);
      TemperatureReadingsInC[x] = resistanceToC(reading);
  }
}

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

//
// Read "sampleSize" samples and report the average
//
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

void loop() {
  readAccel();
  readTemperaturesFromMux();
  
  displayTemperaturesOnLCD(lcd0);

  displayOtherStuffOnLCD(lcd1);

  now = RTC.now();
  // log time
  dataFile.println(now.unixtime()); // seconds since 2000
  dataFile.flush();
  
  Serial.println(now.unixtime());
  delay(100);
}

