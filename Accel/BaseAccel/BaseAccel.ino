//const int xInput = A0;
//const int yInput = A1;
//const int zInput = A2;
const int xInput = A3;
const int yInput = A4;
const int zInput = A5;
const int buttonPin = 2;

// Raw Ranges:
// initialize to mid-range and allow calibration to
// find the minimum and maximum for each axis

const int _oneGUnits = 19;
const int _16GUnits = _oneGUnits * 16;

int defaultCenter = 512;

int xOrigin = defaultCenter;
int yOrigin = defaultCenter;
int zOrigin = defaultCenter;

int xRawMin = defaultCenter - _16GUnits;
int xRawMax = defaultCenter + _16GUnits;

int yRawMin = defaultCenter - _16GUnits;
int yRawMax = defaultCenter + _16GUnits;

int zRawMin = defaultCenter - _16GUnits;
int zRawMax = defaultCenter + _16GUnits;

int lowG = -1600;
int highG = 1600;


// Take multiple samples to reduce noise
const int sampleSize = 10;

void setup() 
{

  analogReference(EXTERNAL);
  Serial.begin(9600);
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
  

  
  xOrigin = xRaw;
  yOrigin = yRaw;
  zOrigin = zRaw - _oneGUnits; //starts at 1G so origin is start minus 1G
  
  Serial.print("Origin at start: ");
  Serial.print(xOrigin);
  Serial.print(", ");
  Serial.print(yOrigin);
  Serial.print(", ");
  Serial.print(zOrigin);
  Serial.println(); 
  
  center();
  
}

void loop() 
{
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
  
  delay(500);
//  }
}

void center() {
  xRawMin = xOrigin - _16GUnits;
  xRawMax = xOrigin + _16GUnits;
  
  yRawMin = yOrigin - _16GUnits;
  yRawMax = yOrigin + _16GUnits;
  
  zRawMin = zOrigin - _16GUnits;
  zRawMax = zOrigin + _16GUnits;
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



