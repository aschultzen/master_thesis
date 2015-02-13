#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

#define I2C_ADDR    0x27 // <<----- Add your address here.  Find it from I2C Scanner
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7
int i = 0;

LiquidCrystal_I2C	lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

void setup()
{
  lcd.begin (16,2);
// Switch on the backlight
lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
lcd.setBacklight(HIGH);
lcd.home (); // go home
lcd.print("Counting: ");
}

void display_text(String incoming, int row)
{
  int x = 0;
  int y = 0;
  
  if(row == 2){
   y = 1;
  } 
  lcd.setCursor (x,y);          //Adjust the cursor for the desired pos.
  lcd.print("                ");    //Clearing
  lcd.setCursor (x,y);          //Re-Adjusting
  lcd.print(incoming);          //Printing
}

void loop()
{
  lcd.setCursor (0,1);        // go to start of 2nd line
  lcd.print(i++,DEC);
}

void smiley()
{
  byte smiley[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
  };
  
  lcd.createChar(0,smiley);
  lcd.begin(16, 2);  
  lcd.write(byte(0));
}

