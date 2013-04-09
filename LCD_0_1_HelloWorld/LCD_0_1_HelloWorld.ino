/*
 Demonstration sketch for Adafruit i2c/SPI LCD backpack
 using MCP23008 I2C expander
 ( http://www.ladyada.net/products/i2cspilcdbackpack/index.html )

 This sketch prints "Hello World!" to the LCD
 and shows the time.
 
  The circuit:
 * 5V to Arduino 5V pin
 * GND to Arduino GND pin
 * CLK to Analog #5
 * DAT to Analog #4
*/

// include the library code:
#include "Wire.h"
#include "LiquidCrystal.h"

// Connect via i2c, default address #0 (A0-A2 not jumpered)
LiquidCrystal lcd0(0);
LiquidCrystal lcd1(1);

void setup() {
  // set up the LCD's number of rows and columns: 
  lcd0.begin(20, 4);
  lcd1.begin(20, 4);
  // Print a message to the LCD.
  lcd0.print("I'm LCD 0.");
  lcd1.print("I'm LCD 1.");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd0.setCursor(10, 2);
  // print the number of seconds since reset:
  lcd0.print(millis()/1000);

  lcd0.setBacklight(HIGH);
  delay(1000);
  lcd0.setBacklight(LOW);
  
  lcd1.setCursor(10, 2);
  lcd1.print(millis()/1000);
  lcd1.setBacklight(HIGH);
   delay(1000);
  lcd1.setBacklight(LOW);

}
