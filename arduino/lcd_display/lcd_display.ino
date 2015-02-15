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

LiquidCrystal_I2C	lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

void setup()
{
  lcd.begin (16,2);
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home (); // go home
  lcd.print("   * READY *   ");
  Serial.begin(9600);
  Serial.println("IM_UP");
}
String content = "";
char character;
int curs = 0;
int myDelay = 1000;
void loop()
{
  while(Serial.available()) {
    character = Serial.read();
    content.concat(character);

    if (content.endsWith("END")) {
      Serial.println(content);
      display_text(content.substring(0,(content.length()-3)),curs);
      content="";
    }
    if (content.endsWith("CLEAR")) {
      Serial.println("Buffer cleared..");
      content="";
    } 

    if (content.equals("SWITCH LINE")) {
      curs++;
      curs = curs % 2; 
      Serial.println(curs);
      content="";
    }
   
   if (content.equals("SPEED UP")) {
      myDelay = myDelay - 100;
      Serial.println(myDelay);
      content="";
    } 

  }
}

String getWordFromSerial()
{
  byte mybyte = 0;
  char buffer[128];
  int i = 0;
  if(Serial.available()>0){
    buffer[i] = (char)Serial.read();
    i++;

    if(&buffer[i-1]=="\\"){
      char temp = Serial.read();
      i++;
      buffer[i] = temp;

      if(temp == 'n'){
        String str(buffer);
        //return str;
        Serial.println(str);
      }
    }
  }
}

void display_text(String incoming, int row)
{
  if((incoming.length()<16))
  {
    int x = 0;
    lcd.setCursor (x,row);              //Adjust the cursor for the desired pos.
    lcd.print("                ");    //Clearing
    lcd.setCursor (x,row);              //Re-Adjusting
    lcd.print(incoming);              //Printing
  }
  else
  {
     int i;
     int x = incoming.length() - 16;
     for (i = 0; i < x+2; i++){
     String sub = incoming.substring(i,i+16);
     delay(myDelay);
     lcd.setCursor(0,row);            
     lcd.print(sub);
     Serial.println(sub);
     }
   }
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


