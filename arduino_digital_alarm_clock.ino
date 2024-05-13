// Include neceassary arduino libraries
#include <LiquidCrystal.h> 
#include <Wire.h>
#include "RTClib.h"




// define pins on the arduino
#define menuButton 5
#define okButton 4
#define upButton   3
#define downButton 2
#define  beeper    6

const int rs = 12;
const int en = 11;
const int d4 = 10;
const int d5 = 9;
const int d6 = 8;
const int d7 = 7;

//define and  initialize global variables

bool choosingUnit = true; // Flag to indicate if choosing time unit (true) or setting time (false)
bool secSelected = true; // Flag to indicate if minutes is selected
bool hourSet = false;
bool alarmSet = false;
bool unitchose = false;
unsigned int alarmHours;
unsigned int alarmMinutes;
bool callAlarmFlag = false;
static  bool settinghours = true;
unsigned long lastChangeTime = 0;
bool displayingDay = true; // Start by displaying the day
unsigned long delayDuration = 3000;
unsigned int debounceDelay = 300;
unsigned long lastDebounceTime = 0;


// declare functions in the code 
void callAlarm();
void beep();
void setAlarm();
void showTime();
void updateLCD();
void showDate();
void showDay();

// set  number of days in the rtc module
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


// create an instance of libraries called in the program 
LiquidCrystal lcd(rs, en, d4, d5,d6, d7 ) ;
RTC_DS1307 rtc;
DateTime now ;


// create a state for each display on the lcd
enum State
{ 
  SHOW_DAY_DATE_TIME,
  SET_ALARM_DURATION,
  SET_HOUR,
  SET_MINUTE
};

State currentState = SHOW_DAY_DATE_TIME;

void setup() {

 Serial.begin(9600);
 Serial.println("3 POWER SOURCE AUTOMATIC CHANGE OVER SYSTEM");
 Serial.println("Processing available SOURCE lines...");
 

 pinMode(menuButton, INPUT_PULLUP);
 pinMode(okButton, INPUT_PULLUP);
// set push buttons  mode
 pinMode(upButton, INPUT_PULLUP);
 pinMode(downButton, INPUT_PULLUP);


// set beeper pin  mode
  pinMode(beeper, OUTPUT);
  

// setup the LCD's number of columns and rows:
lcd.begin(16, 2);
Serial.println("Main loop  running lcdbegin!");

// Create welcome display sequence for user
lcd.setCursor(0,0);
lcd.print("Digital Clock");
delay(2000);
lcd.clear();
lcd.print("A New Day");
delay(2000);
lcd.setCursor(0,1);
lcd.print("IT'S POSSIBLE");
lcd.blink();
delay(2000);
lcd.clear();
lcd.print("YOU CAN DO IT");
lcd.blink();
delay(2000);

Serial.println("Main loop  running lcdafter!");
if (! rtc.begin()) {
    lcd.println("Couldn't find RTC");
    Serial.println("Couldn't find RTC");
    while (1) delay(10);
}

if (!rtc.isrunning()) 
  {
    lcd.print("RTC is NOT running!");
    Serial.println("RTC is NOT running!");
  }
// set current time  
 rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {

   Serial.println("loop  running!");
   now =  rtc.now();
   unsigned long currentTime = millis();

// a switch case to help move between the three display screens
  switch (currentState){
    
  case SHOW_DAY_DATE_TIME:
      Serial.println("loop  switched!");
            
  if (currentTime - lastChangeTime >= delayDuration) {
    // Toggle between displaying day and time
    Serial.println("time  switched!");

    if (displayingDay) {
      showDate(); 
      showDay();
      Serial.println("Showing day");
    } else {
      showDate(); 
      showTime();
      Serial.println("Showing time");
    }
    displayingDay = !displayingDay; // Toggle the display mode
    lastChangeTime = currentTime; // Update the last change time
  }
  
   break;
   
  case SET_ALARM_DURATION:
       setAlarm();
         break;
         
  case SET_HOUR:
    setHours();
    break;

    case SET_MINUTE:
    setMinutes();
    break;

  }

  if (alarmSet) {
    callAlarm();
   
  }
  

   // Check for button presses
  if ((digitalRead(menuButton) == LOW ) && (millis() - lastDebounceTime > debounceDelay)){
    lastDebounceTime = millis();
    beep();
    currentState = SET_ALARM_DURATION;
  }

  Serial.println("loop  ran!");
} 


void showTime(){
// A function to display the current time
    lcd.setCursor(0, 1);
    lcd.print("TIME:");
    lcd.print(" ");
    lcd.print(now.hour());
    lcd.print(':');
    lcd.print(now.minute());
    lcd.print(':');
    lcd.print(now.second());
  if (alarmSet) {
    lcd.print(" A");
    Serial.print("alarm set");
   }
}

void showDate(){
// A function to show the current date
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DATE:");
    lcd.print(" ");
    lcd.print(now.day());
    lcd.print('/');
    lcd.print(now.month());
    lcd.print('/');
    lcd.print(now.year());
    lcd.print("  ");  
   
}

void showDay(){
// A function to display the current day 
    lcd.setCursor(0, 1);
    lcd.print("DAY: ");
    lcd.setCursor(6, 1);
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
   
}

void setAlarm(){
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("Set Alarm");
   lcd.setCursor(0, 1);
   lcd.print(secSelected ? "<hour>  minutes": "hour  <minutes>");
   
   if (choosingUnit)  {
    //alarmHours = 0;
    alarmMinutes = 0;
    alarmSet = false;
    // Display unit prompt

    if ((digitalRead(menuButton) == LOW ) && (millis() - lastDebounceTime > debounceDelay)){
    lastDebounceTime = millis();
    beep();
    currentState = SHOW_DAY_DATE_TIME;
    
  } else if (digitalRead(upButton) == LOW){
   // up button pressed, toggle unit selection
    secSelected = !secSelected;
    beep();
    delay(200);
      
    }else if (digitalRead(downButton) == LOW) {
      secSelected = true;
      beep();
      delay(200);
      
    } else if (digitalRead(okButton) == LOW) {
      choosingUnit = false;
      //unitchose == true;
  // OK button pressed, move to setting alarm
      beep();      
    if (secSelected){
       currentState =  SET_HOUR;  
    } else  if (!secSelected){
        currentState = SET_MINUTE;
    }
    delay(200);
  }
 }
}

void setHours(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Hour:    ");
  lcd.setCursor(6, 1);
  lcd.print("HH");
  updateLCD();

  if (digitalRead(okButton) == LOW){
    beep();
    hourSet = true;
    currentState = SET_MINUTE;
     delay(200);
     
  } else if (digitalRead(upButton) == LOW) {
       beep();
      alarmHours = (alarmHours + 1) % 24;
      updateLCD();
      delay(200); // Button debounce
      
   } else if (digitalRead(downButton) == LOW)
    {
      beep();
      alarmHours = (alarmHours - 1) % 24;
      if (alarmHours < 0) {
        alarmHours = 23;
      }
      updateLCD();
      delay(200); // Button debounce
    }
  }


void setMinutes(){
  
   static bool  settingMinutes = false;
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("Set Minutes:    ");
   lcd.setCursor(6, 1);
   lcd.print("MM");
   updateLCD();

   if (!hourSet){
      alarmHours = 5;  
          
    } else if ((digitalRead(okButton) == LOW)&& (settingMinutes == true)){
       beep();
      // OK button pressed, save time and move to normal state
       currentState = SHOW_DAY_DATE_TIME;
       choosingUnit = true; // Reset choosing unit for next time
       alarmSet = true;
       delay(200); // Button debounce
       
    }else if (digitalRead(upButton) == LOW) {
        
      beep();
      alarmMinutes = (alarmMinutes + 1) % 60;
      updateLCD();
      delay(200); // Button debounce
      
     }else if (digitalRead(downButton) == LOW) {
      beep();
      alarmMinutes = (alarmMinutes - 1) % 60;
      if (alarmMinutes < 0) {
        alarmMinutes = 59;
      }
      updateLCD();
      delay(200); // Button debounce
      
    }else if (digitalRead(menuButton) == LOW) {
       beep();
       settingMinutes = true;
       lcd.setCursor(0, 0);
       lcd.print("Minutes set");
       updateLCD();
       delay(200); // Button debounce

  }
}


void beep(){
  // Generate a short beep
  tone(beeper, 1000, 100); // 1000 Hz tone for 100ms
}

void updateLCD() {
  lcd.setCursor(0, 1);
  lcd.print(String(alarmHours, DEC) + ":" + String(alarmMinutes, DEC));
}

void callAlarm(){
  
  Serial.println(now.minute()); 
  Serial.println(now.hour());
  if ( (alarmHours == (int)now.hour()) && (alarmMinutes == (int)now.minute())){
    beepAlarm();
    alarmSet = false;
  } else if ((alarmHours == (int)now.hour()) && (alarmMinutes == (int)now.minute()) && (digitalRead(okButton) == LOW)){
       beep();
       alarmSet = false;
      // OK button pressed, stop alarm
       currentState = SHOW_DAY_DATE_TIME; 
    }
}


void beepAlarm(){
  // Generate a beep sound for the alarm
  tone(beeper, 1000, 50000); // 1000 Hz tone for 100ms
}



