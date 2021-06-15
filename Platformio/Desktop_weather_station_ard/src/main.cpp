#include <Arduino.h>
#include "defines.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Arduino_ILI9163C_Fast.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include "GyverButton.h"
#include <microDS18B20.h>
#include "GyverTimer.h"
#include <avr/wdt.h>
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <stdio.h>
#include "LowPower.h"

Arduino_ILI9163C lcd = Arduino_ILI9163C(TFT_DC, TFT_RST, TFT_CS);
MicroDS18B20 sensor(DS18B20_SENSOR_PIN);
GTimer TimerActiveTime(MS);
GTimer TimerTempUpdate(MS);

volatile bool flag_sleep;
volatile bool flag_button_pressed;
uint8_t temp;
volatile uint16_t cnt_time_send;

void fnPreSleep(void);
void fnPostSleep(void);
bool fnPxSensorScan(void);
void fnDrawTemp(uint8_t);
void fnGoToSleep(uint8_t sleep_time);
void fnButton_ISR(void);
void fnCc1101SendData(uint8_t data);

void setup()
{
  #if(DEBUG)
    Serial.begin(115200);
  #endif

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TFT_LED, OUTPUT);
  pinMode(DS18B20_POWER_OUT_PIN, OUTPUT);

  digitalWrite(DS18B20_POWER_OUT_PIN, HIGH);
  analogWrite(TFT_LED, TFT_DEFAULT_BRIGHTNESS);

  lcd.init();
  lcd.setRotation(4);
  lcd.fillScreen(BLUE);
  lcd.setTextColor(WHITE, BLUE);
  lcd.setFont(&FreeMonoBold12pt7b);

  sensor.setResolution(9);

  TimerActiveTime.setMode(MANUAL);
  TimerActiveTime.setInterval(ACTIVE_TIME);
  TimerTempUpdate.setInterval(DS18B20_UPDATE_PERIOD);

  ELECHOUSE_cc1101.Init();           
  ELECHOUSE_cc1101.setGDO(6, 0);     
  ELECHOUSE_cc1101.setCCMode(1);     
  ELECHOUSE_cc1101.setModulation(0); 
  ELECHOUSE_cc1101.setMHZ(435.0);    
  ELECHOUSE_cc1101.setSyncMode(2);   
  ELECHOUSE_cc1101.setPA(12);    
  ELECHOUSE_cc1101.setCrc(1);
  ELECHOUSE_cc1101.goSleep();

 #if(DEBUG)
    Serial.println("setup");
  #endif
}

//--------------------------------------------------------------------------

void loop()
{
  #if(DEBUG)
    delay(300);
  #endif

  while (!flag_sleep)
  {
    digitalWrite(DS18B20_POWER_OUT_PIN, HIGH);
    delay(5);
    sensor.requestTemp();
    delay(100);
    temp = (uint8_t)sensor.getTemp();
    digitalWrite(DS18B20_POWER_OUT_PIN, LOW);
    #if(DEBUG)
    Serial.println("getTemp");
    #endif
    fnDrawTemp(temp);

    if (TimerActiveTime.isReady())
    {
      fnPreSleep();
      flag_sleep = true;
      #if(DEBUG)
      Serial.println("TimerActiveTime.isReady");
      #endif
    }
  }

  while (flag_sleep)
  {
    #if(DEBUG)
      Serial.println("fnGoToSleep");
    #endif

    fnGoToSleep(WDTO_1S);
    cnt_time_send++;


    if (cnt_time_send >= CC1101_SEND_PERIOD)
    {
      cnt_time_send = 0;
      digitalWrite(DS18B20_POWER_OUT_PIN, HIGH);
      delay(5);
      sensor.requestTemp();
      delay(100);
      temp = (uint8_t)sensor.getTemp();
      digitalWrite(DS18B20_POWER_OUT_PIN, LOW);
      #if(DEBUG)
      Serial.println("fnCc1101SendData");
      #endif
      fnCc1101SendData(temp);
    }
    

    if(flag_button_pressed)
    {
      flag_button_pressed = false;
      flag_sleep = false;
      TimerActiveTime.setInterval(ACTIVE_TIME);
      TimerActiveTime.start();
      #if(DEBUG)
      Serial.println("fnPostSleep");
      #endif
      fnPostSleep();
    }
  }

}

//---------------------------------------------------------------------------------------


//*****************************************
void fnGoToSleep(uint8_t sleep_time)
{ 
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), fnButton_ISR, LOW);
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); 

  //- SLEEP -

  wdt_disable();
  detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
}

//******************************************
void fnPreSleep(void)
{
  lcd.sleepDisplay(1);
  analogWrite(TFT_LED, 0);
}

//*****************************************
void fnPostSleep(void)
{
  lcd.sleepDisplay(0);
  analogWrite(TFT_LED, TFT_DEFAULT_BRIGHTNESS);
}


//******************************************
void fnButton_ISR(void)
{
  detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
  flag_button_pressed = true;
  #if(DEBUG)
  Serial.println("fnButton_ISR");
  #endif

}

//****************************************
void fnDrawTemp(uint8_t l_temp)
{

  static uint8_t old_temp = 85;

  if (l_temp == 85)
  {
    lcd.fillRect(10, 10, 90, 60, BLUE);
    lcd.setCursor(45, 60);
    lcd.setTextSize(3);
    lcd.println('!');
    old_temp = l_temp;
  }
  else
  {
    if (l_temp != old_temp)
    {
      lcd.fillRect(10, 10, 90, 60, BLUE);
      lcd.setCursor(15, 60);
      lcd.setTextSize(3);
      lcd.println(temp);
      lcd.setTextSize(1);
      lcd.setCursor(100, 30);
      lcd.println("o");
      lcd.setTextSize(1);
      lcd.setCursor(30, 100);
      lcd.println("Temp");
      old_temp = l_temp;
    }
  }
}

//*****************************************
void fnCc1101SendData(uint8_t data)
{
  char msg[10] = {
      0,
  };
  sprintf(msg, "Temp %d", data);
  ELECHOUSE_cc1101.SendData(msg, 10);
  ELECHOUSE_cc1101.goSleep();
}