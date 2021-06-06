#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Arduino_ILI9163C_Fast.h>
//#include "RREFont.h"
//#include "rre_5x8.h"
//#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
//#include <Fonts/FreeSerifBold24pt7b.h>
//#include "GyverButton.h"
//#include <microDS3231.h>
#include <Wire.h>
#include <ErriezDS1307.h>
#include <microDS18B20.h>
#include "GyverEncoder.h"
//#include <GyverPower.h>
#include "LowPower.h"



#define TFT_CS 10
#define TFT_DC  8
#define TFT_RST 9
#define TFT_LED 6

#define ENC_A       2
#define ENC_B       3
#define ENC_BUTTON  7 
#define SENSOR_PIN  A0
#define DS18B20_POWER_OUT 4
#define DS1307_POWER_OUT  5

Arduino_ILI9163C lcd = Arduino_ILI9163C(TFT_DC, TFT_RST, TFT_CS);
ErriezDS1307 rtc;
MicroDS18B20 sensor(SENSOR_PIN);

Encoder enc1(ENC_A, ENC_B, ENC_BUTTON);

volatile uint8_t cnt;
bool flag_change;
bool flag_sleep;
uint8_t hour;
uint8_t min;
uint8_t sec;
uint8_t old_sec;
uint8_t temp;
uint8_t old_temp;

void Wakeup();




void setup() {

  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_BUTTON, INPUT_PULLUP);
  pinMode(TFT_LED, OUTPUT);
  pinMode(DS18B20_POWER_OUT,OUTPUT);
  pinMode(DS1307_POWER_OUT,OUTPUT);
  digitalWrite(DS18B20_POWER_OUT,HIGH);
  digitalWrite(DS1307_POWER_OUT,HIGH);
  analogWrite(TFT_LED, 127);
  Serial.begin(9600);
  lcd.init();
  lcd.setRotation(2);
  lcd.fillScreen(BLUE);
  lcd.setTextColor(WHITE,BLUE);
  lcd.setFont(&FreeMonoBold12pt7b );
  //lcd.setFont(&FreeSansBold18pt7b);
  //lcd.setFont(&FreeSerifBold24pt7b);
  
  attachInterrupt(digitalPinToInterrupt(ENC_A), Wakeup,CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), Wakeup,CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_BUTTON), Wakeup,CHANGE);
 
  Wire.begin();
  Wire.setClock(100000);
  rtc.begin();
  rtc.setSquareWave(SquareWaveDisable);
  //rtc.setTime(12, 0, 0);
  rtc.getTime(&hour, &min, &sec);
  //lcd.sleepDisplay(1);
  //analogWrite(TFT_LED, 0);

  sensor.setResolution(9);

  //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
 // LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
 
  
}

void loop() {

  enc1.tick();

  if(temp != old_temp){
    lcd.fillRect(10,10,90,60,BLUE);
    lcd.setCursor(10, 60);
    lcd.setTextSize(3);
    lcd.println(temp);
    lcd.setTextSize(1);
    lcd.setCursor(95, 30);
    lcd.println("o");
    old_temp = temp;
    
  }

  
  rtc.getTime(&hour, &min, &sec);
  if(sec != old_sec){

    rtc.getTime(&hour, &min, &sec);
    lcd.fillRect(20,86,28,15,BLUE);
    lcd.setCursor(20, 100);
    //lcd.print("   ");
    lcd.print(sec);
    old_sec = sec;

    if(flag_change)sensor.requestTemp(); 
    else temp = (uint8_t)sensor.getTemp();


    flag_change = 1 - flag_change;

    lcd.fillRect(60,70,50,50,BLUE);
    lcd.setCursor(60, 100);
    lcd.println(temp);

    Serial.println(temp);
  }

  if(enc1.isRight()){
    cnt++;   
  }

  if(enc1.isLeft()){
    cnt--;   
  }

  if(enc1.isPress()){
    analogWrite(TFT_LED, 0);
    lcd.sleepDisplay(1);
    digitalWrite(DS18B20_POWER_OUT,LOW);
    digitalWrite(DS1307_POWER_OUT,LOW);
    flag_sleep = true;

    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  }
      
  lcd.fillRect(60,110,30,20,BLUE);
  lcd.setCursor(60, 128);
  lcd.print(cnt);


  delay(1);
  
  
}

//**********************************************
void Wakeup(void) {
  
  enc1.tick();
 
  if(flag_sleep){
    cli();
    digitalWrite(DS18B20_POWER_OUT,HIGH);
    digitalWrite(DS1307_POWER_OUT,HIGH);
    delay(500);
   // Wire.begin();
   // Wire.setClock(100000);
   // rtc.begin();
    //rtc.getTime(&hour, &min, &sec);
   // rtc.setSquareWave(SquareWaveDisable);
    lcd.sleepDisplay(0);
    analogWrite(TFT_LED, 127);
    flag_sleep = false;
    sei();

  }
  

}

