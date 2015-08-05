#include <JeeLib.h>
#include <PortsLCD.h>

#include <Servo.h> 


Servo servo;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);// select the pins used on the LCD panel
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
int page        = 0;
float irrigate    = 512;
long lastAction = 0;
long lastIrrigate = 0;
boolean sleepy = false;

#define numberPages 2
#define maxMsIrrigate  3000
#define irrigateMargin 180000
#define loopPeriod     100
#define sleepyTime     5000
#define sleepTime 30000
#define minMoisture 500

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define backLight 10


#define tempSensor     A1
#define lightSensor    A2
#define flameSensor    A3
#define moistureSensor A4
#define motionSensor   A5
#define potSensor      A5
#define portServo      3


double read_temp() {
    return (457*analogRead(tempSensor))/1024;
}

int read_light() {
  return analogRead(lightSensor);
}

int read_flame() {
  return analogRead(flameSensor);
}

int read_moisture() {
  return analogRead(moistureSensor);
}

int read_motion() {
  return digitalRead(motionSensor);
}

int read_pot() {
  return 1024-analogRead(motionSensor);
}


void read_LCD_buttons(){               // read the buttons
    adc_key_in = analogRead(0);       // read the value from the sensor 
    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    // We make this the 1st option for speed reasons since it will be the most likely result

    if (adc_key_in > 1000)  lcd_key = btnNONE; 

    // For V1.1 us this threshold
    if (adc_key_in < 850)  lcd_key = btnSELECT;
    if (adc_key_in < 650)  lcd_key = btnLEFT;  
    if (adc_key_in < 450)  lcd_key = btnDOWN; 
    if (adc_key_in < 250)  lcd_key = btnUP; 
    if (adc_key_in < 50)   lcd_key = btnRIGHT;  

    

    switch (lcd_key){               // depending on which button was pushed, we perform an action

       case btnRIGHT:{
            page = (page+1)%numberPages;
            activate();
            break;
       }
       case btnLEFT:{
             page--;
             if(page < 0)
              page = numberPages-1;
             activate();
             break;
       }    
       case btnUP:{
        activate();
             //lcd.print("UP    ");  //  push button "UP" and show the word on the screen
             break;
       }
       case btnDOWN:{
        activate();
             //lcd.print("DOWN  ");  //  push button "DOWN" and show the word on the screen
             break;
       }
       case btnSELECT:{
        activate();
             //lcd.print("SELECT");  //  push button "SELECT" and show the word on the screen
             break;
       }
       case btnNONE:{
             //lcd.print("NONE  ");  //  No action  will show "None" on the screen
             break;
       }
   }
   
}

void printPage() {

  char line2[16];

 
  lcd.setCursor(13,0);
  lcd.print(page+1);
  lcd.print('/');
  lcd.print(numberPages);

  switch(page) {
    case 0: {
      lcd.setCursor(0,0);
      lcd.print("T:");
      lcd.print(read_temp());
      lcd.print((char)223);
      lcd.print('C');

      sprintf(line2, "L:%4i    M:%4i", read_light(), read_moisture());
      
      lcd.setCursor(0,1);
      lcd.print(line2);
      break;
    }
   /* case 1: {
      lcd.setCursor(0,0);
      lcd.print("T:");
      lcd.print(read_temp(), 2);
      lcd.print((char)223);
      lcd.print('C');

      
      sprintf(line2, "M:%4i    P:%4i", read_moisture(), read_pot());
      lcd.setCursor(0,1);
      lcd.print(line2);      
      break;
    }*/
    case 1: {
      lcd.setCursor(0,0);
      lcd.print("T:");
      lcd.print(read_temp(), 2);
      lcd.print((char)223);
      lcd.print('C');

      int filled = 16*read_pot()/1024;

      for(int i = 0; i < filled; i++) {
        line2[i] = 0xFF;
      }

      for(int i = filled; i < 16; i++) {
        line2[i] = ' ';
      }
      
      lcd.setCursor(0,1);
      lcd.print(line2);      
      break;
    }
  }

  
}

void activate() {
  lastAction = millis();
  lcd.display();
  digitalWrite(backLight, HIGH);
  sleepy = false;
}

void deactivate() {
  lcd.noDisplay();
  digitalWrite(backLight, LOW);
  sleepy = true;
}


void setup(){

    lastAction = millis();
   
   pinMode(8, OUTPUT);
   pinMode(9, OUTPUT);
   pinMode(4, OUTPUT);
   pinMode(5, OUTPUT);
   pinMode(6, OUTPUT);
   pinMode(7, OUTPUT);
   pinMode(backLight, OUTPUT);
   lcd.begin(16, 2);               // start the library

   
   servo.attach(portServo);
   servo.write(0);
}
 
void loop(){
read_LCD_buttons();   // read the buttons

   int tIrrig = read_pot();

   if(abs(irrigate - tIrrig) > 50 || read_moisture() < minMoisture) {
      activate();
   }
   irrigate = tIrrig;
  if(sleepy) {
    Sleepy::loseSomeTime(5000);
    return;
  }
  
           // move to the begining of the second line
   
   
   printPage();
   
   if(read_moisture() < minMoisture && (lastIrrigate + irrigateMargin) < millis()) {
    int msIrrigate = maxMsIrrigate - maxMsIrrigate*(1023-irrigate)/1023;
    servo.write(100);
    delay(msIrrigate);
    servo.write(0);
    lastIrrigate = millis();
   } else {
    Sleepy::loseSomeTime(loopPeriod);
   }

   if(millis()-lastAction > sleepTime) {
    deactivate();
   }
}

