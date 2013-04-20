#include <MuxShield.h>
#include <Wire.h>
#include <Adafruit_MCP23008.h>
#include <LiquidCrystal.h>

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

MuxShield muxShield;
float TemperatureReadingsInC[3][16];

// Connect via i2c, default address #0 (A0-A2 not jumpered)
LiquidCrystal lcd0(0);
LiquidCrystal lcd1(1);

void setup() {
  // set up the LCD's number of rows and columns: 
  lcd0.begin(20, 4);
  lcd1.begin(20, 4);

  lcd0.setBacklight(HIGH);

  muxShield.setMode(1,ANALOG_IN);
  muxShield.setMode(2,ANALOG_IN);
  muxShield.setMode(3,ANALOG_IN);
}

float thermistorReadingToResistance(int reading) {
  reading = (1023 / reading)  - 1;
  return SERIESRESISTOR / reading;
}

float resistanceToC(int reading) {
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
  for (int y = 0; y < 3; y++) {
    for (int x = 0; x <= 15; x++) {
      int reading = muxShield.analogReadMS(y,x);
      TemperatureReadingsInC[y][x] = resistanceToC(reading);
    }
  }
}

void displayTemperaturesOnLCD(LiquidCrystal lcd) {
  int row = 0;
  int col = 0;
   for (int x = 0; x <= 15; x++) {
     lcd.setCursor(col, row);
     float reading = TemperatureReadingsInC[0][x]; // first row of MUX only
     lcd.print(reading, 1);
     
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

void loop() {

  readTemperaturesFromMux();
  displayTemperaturesOnLCD(lcd0);

  delay(100);
}
