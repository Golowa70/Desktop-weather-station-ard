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
#include <EncButton.h>
//#include "GyverButton.h"
//#include <microDS3231.h>
#include <Wire.h>
#include <ErriezDS1307.h>
#include <microDS18B20.h>




#define TFT_CS 10
#define TFT_DC  8
#define TFT_RST 9
#define TFT_LED 6
//#define BUTTON  7
#define ENC_A       4
#define ENC_B       5
#define ENC_BUTTON  7 
#define SENSOR_PIN  A0

EncButton<EB_TICK, ENC_A, ENC_B, ENC_BUTTON> enc(INPUT_PULLUP);        // энкодер с кнопкой <A, B, KEY>
Arduino_ILI9163C lcd = Arduino_ILI9163C(TFT_DC, TFT_RST, TFT_CS);
ErriezDS1307 rtc;
MicroDS18B20 sensor(SENSOR_PIN);

uint8_t cnt;
bool flag_change;
uint8_t hour;
uint8_t min;
uint8_t sec;
uint8_t old_sec;
uint8_t temp;

void Wakeup();



void setup() {

  //pinMode(BUTTON, INPUT_PULLUP);
  pinMode(TFT_LED, OUTPUT);
  analogWrite(TFT_LED, 127);
  Serial.begin(9600);
  lcd.init();
  lcd.setRotation(2);
  lcd.fillScreen(BLUE);
  lcd.setTextColor(WHITE,BLUE);
  lcd.setFont(&FreeMonoBold12pt7b );
  //lcd.setFont(&FreeSansBold18pt7b);

  //lcd.setFont(&FreeSerifBold24pt7b);
  
  enc.counter = 100;      // изменение счётчика
 
  attachInterrupt(digitalPinToInterrupt(ENC_A), Wakeup,CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), Wakeup,CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_BUTTON), Wakeup,CHANGE);

  Wire.begin();
  Wire.setClock(100000);
  rtc.begin();
  rtc.setSquareWave(SquareWaveDisable);
  rtc.setTime(12, 0, 0);
  rtc.getTime(&hour, &min, &sec);
  //lcd.sleepDisplay(1);
  //analogWrite(TFT_LED, 0);

  sensor.setResolution(9);

  
}

void loop() {

  enc.tick();
  lcd.setCursor(10, 60);
  lcd.setTextSize(3);
  lcd.println("27");
  lcd.setTextSize(1);
  lcd.setCursor(95, 30);
  lcd.println("o");

  //lcd.setTextSize(1);
  //lcd.setCursor(5, 100);
  //lcd.println(sec);
  rtc.getTime(&hour, &min, &sec);
  if(sec != old_sec){

    rtc.getTime(&hour, &min, &sec);
    lcd.fillRect(20,70,40,40,BLUE);
    lcd.setCursor(20, 100);
    lcd.println(sec);
    old_sec = sec;

    if(flag_change)sensor.requestTemp(); 
    else temp = (uint8_t)sensor.getTemp();


    flag_change = 1 - flag_change;

    lcd.fillRect(60,70,50,50,BLUE);
    lcd.setCursor(60, 100);
    lcd.println(temp);

    Serial.println(temp);
  }

  if(enc.isTurn()){
    
    cnt++;
    lcd.sleepDisplay(0);
    analogWrite(TFT_LED, 127);
    

  }
      

  delay(100);
  
  
}

//**********************************************
void Wakeup(void) {

  enc.tick();

}