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

void setup() 
{

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



