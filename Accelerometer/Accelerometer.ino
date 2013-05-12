#include <Wire.h>

#include <Adafruit_MCP23008.h>
#include <LiquidCrystal.h>

//LCD identifier (based on the way the LCD pins that set the LCD id are bridged)
LiquidCrystal lcd1(1);

//analog inputs the accelerometer ports are plugged into
const int xInput = A13;
const int yInput = A14;
const int zInput = A15;

// Take multiple samples to reduce noise, in this case since we're gathering data just pull it all
const int sampleSize = 1;

// Raw Ranges:
// initialize to mid-range and allow calibration to
// find the minimum and maximum for each axis
int xRawMin = 512;
int xRawMax = 512;
 
int yRawMin = 512;
int yRawMax = 512;
 
int zRawMin = 512;
int zRawMax = 512; 

void setup() {
  // set up the LCD's number of columns and rows.  Our lcd is 20 x 4: 
  lcd1.begin(20, 4);
  
  // Print a message to the LCD just to see it's working.
  lcd1.setCursor(0, 0);
  lcd1.print("I'm LCD 1.");
  lcd1.setBacklight(HIGH);
  
  //Read all the axis for calibration...
  int xRaw = ReadAxis(xInput);
  int yRaw = ReadAxis(yInput);
  int zRaw = ReadAxis(zInput);
  //...calibrate.  Requires that the accel is wired to the aref on the arduino board
  AutoCalibrate(xRaw, yRaw, zRaw);

}

void loop() {
  char xLabel[4] = "X: ";
  int xRaw = ReadAxis(xInput);
  long xScaled = map(xRaw, xRawMin, xRawMax, -1000, 1000);
  print(1, 0, xLabel);
  print(1, 4, xScaled);

  char yLabel[4] = "Y: ";
  int yRaw = ReadAxis(yInput);
  long yScaled = map(yRaw, yRawMin, yRawMax, -1000, 1000);
  print(2, 0, yLabel);
  print(2, 4, yScaled);


  char zLabel[4] = "Z: ";
  int zRaw = ReadAxis(zInput);
  long zScaled = map(zRaw, zRawMin, zRawMax, -1000, 1000);
  print(3, 0, zLabel);
  print(3, 4, zScaled);

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

//
// Find the extreme raw readings from each axis
//
void AutoCalibrate(int xRaw, int yRaw, int zRaw)
{
  Serial.println("Calibrate");
  if (xRaw < xRawMin)
  {
    xRawMin = xRaw;
  }
  if (xRaw > xRawMax)
  {
    xRawMax = xRaw;
  }
  
  if (yRaw < yRawMin)
  {
    yRawMin = yRaw;
  }
  if (yRaw > yRawMax)
  {
    yRawMax = yRaw;
  }
 
  if (zRaw < zRawMin)
  {
    zRawMin = zRaw;
  }
  if (zRaw > zRawMax)
  {
    zRawMax = zRaw;
  }
}

void print(int row, int column, char* message) {
  lcd1.setCursor(column, row);
  lcd1.print(message);
}

void print(int row, int column, long message) {
  lcd1.setCursor(column, row);
  lcd1.print(message);
}


