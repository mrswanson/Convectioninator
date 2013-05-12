// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
 
#include <Wire.h>
#include "RTClib.h"
#include "SD.h"
#include "LiquidCrystal.h"
 
RTC_DS1307 RTC;

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;
 
// the logging file
File logfile;

#define redLEDpin 3
#define greenLEDpin 4


// Connect via i2c, default address #0 (A0-A2 not jumpered)
LiquidCrystal lcd(0);

//const int xInput = A0;
//const int yInput = A1;
//const int zInput = A2;
const int xInput = A8;
const int yInput = A9;
const int zInput = A10;
const int buttonPin = 2;

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

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);
  
  while(1);
}
 
 
 
void setup () {
    Serial.begin(9600);
    Wire.begin();
    RTC.begin();
 
 // if (! RTC.isrunning())
 {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
    
    // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(10,11,12,13)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);
  
  lcd.begin(20,4);
  }
  
   Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  
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
 
void loop () {
    DateTime now = RTC.now();
    
    // log time
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  
  logfile.flush();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
    Serial.print(" since 1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
    
    // calculate a date which is 7 days and 30 seconds into the future
    DateTime future (now.unixtime() + 7 * 86400L + 30);
    
    Serial.print(" now + 7d + 30s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();
    
    Serial.println();
    
     int xRaw = ReadAxis(xInput);
  int yRaw = ReadAxis(yInput);
  int zRaw = ReadAxis(zInput);
  

       
    // +/-  16Gs is range of sensor, if we use 16000 then we're ranging across the potential G values.
    long xScaled = map(xRaw, xRawMin, xRawMax, lowG, highG);
    long yScaled = map(yRaw, yRawMin, yRawMax, lowG, highG);
    long zScaled = map(zRaw, zRawMin, zRawMax, lowG, highG);
  
    // re-scale to fractional Gs
    float units = 100;
    float xAccel = xScaled / units;
    float yAccel = yScaled / units;
    float zAccel = zScaled / units;
  
    Serial.print(xRaw);
    Serial.print(", ");
    Serial.print(yRaw);
    Serial.print(", ");
    Serial.print(zRaw);
  
    Serial.print(" :: ");
    Serial.print(xAccel);
    Serial.print("G, ");
    Serial.print(yAccel);
    Serial.print("G, ");
    Serial.print(zAccel);
    Serial.println("G");
    
    lcd.setBacklight(HIGH);
    lcd.setCursor(0, 0);
  // print the time
  
  lcd.print(now.hour(), DEC);
  lcd.print(":");
  lcd.print(now.minute(), DEC);
  lcd.print(":");
  lcd.print(now.second(), DEC);
  
 
lcd.setCursor(0, 2);
  lcd.print(xRaw);
  lcd.print(", ");
  lcd.print(yRaw);
  lcd.print(", ");
  lcd.print(zRaw);

  lcd.setCursor(0, 3);
  lcd.print(xAccel);
  lcd.print("G ");
  lcd.print(yAccel);
  lcd.print("G ");
  lcd.print(zAccel);
  
  Serial.println();
  Serial.print("LCD");
  Serial.println();
  
    delay(1000);
}

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

