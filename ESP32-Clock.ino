/*

  Japanese.ino
  
  Use the (Arduino compatible) u8g2 function "print"  to draw a text.

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  

*/

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

#include <HTTPClient.h>

const char* ssid = "TP-Link_E02D";
const char* password = "45647956";

int GMTOffset = 7200;  // Replace with your GMT Offset in seconds
int daylightOffset = 0;  // Replace with your daylight savings offset in seconds

/*
  U8g2lib Example Overview:
    Frame Buffer Examples: clearBuffer/sendBuffer. Fast, but may not work with all Arduino boards because of RAM consumption
    Page Buffer Examples: firstPage/nextPage. Less RAM usage, should work with all Arduino boards.
    U8x8 Text Only Example: No RAM usage, direct communication with display controller. No graphics, 8x8 Text only.
    
  This is a page buffer example.    
*/

U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // All Boards without Reset of the Display

#include <millisDelay.h>

millisDelay wordDelay;

String randomJapaneseWord = "";
String randomEnglishWord = "";

void getRandomWord()
{
  if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin("https://cdn.russellaleksejevs.nl/clock/random.php");
      int httpCode = http.GET();
      if (httpCode > 0) {
        while (httpCode != HTTP_CODE_OK) {
          delay(100);
          Serial.println("Connecting to the website...");
        }
        String payload = http.getString();
        randomJapaneseWord = payload.substring(0, payload.indexOf("|"));
        randomEnglishWord = payload.substring(payload.indexOf("|") + 1);
      }
      else {
        Serial.println("Error durring http request.");
      }
      http.end();
    }
  else {
    Serial.println("WiFi Disconnected.");
  }
}

unsigned long delayTime = 1000 * 60 * 60;

void setup(void) {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println("Connected to Wi-Fi!");

  getRandomWord();

  configTime(GMTOffset, daylightOffset, "pool.ntp.org","time.nist.gov");
  
  u8g2.begin();
  u8g2.enableUTF8Print();		// enable UTF8 support for the Arduino print() function

  wordDelay.start(delayTime);
}

void loop(void) {
  if (wordDelay.justFinished()) {
    Serial.println("Getting new word.");

    getRandomWord();

    wordDelay.start(delayTime);
  }

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_profont22_tf);  // all the glyphs of "こんにちは世界" are already included in japanese1: Lerning Level 1-6
    u8g2.setFontDirection(0);

    time_t rawtime = time(nullptr);
    struct tm* timeinfo = localtime(&rawtime);
  
    u8g2.setCursor(0, 15);
    String time = "";
    if (timeinfo->tm_hour < 10) {
      time += 0;
    }
    time += timeinfo->tm_hour;
    time += ":";
    if (timeinfo->tm_min < 10) {
      time += 0;
    }
    time += timeinfo->tm_min;
    u8g2.print(time);

    u8g2.setFont(u8g2_font_b12_t_japanese1);  // all the glyphs of "こんにちは世界" are already included in japanese1: Lerning Level 1-6
    u8g2.setFontDirection(0);
    
    u8g2.setCursor(0, 30);
    u8g2.print(randomEnglishWord);
    u8g2.setCursor(0, 45);
    u8g2.print(randomJapaneseWord);
  
  } while ( u8g2.nextPage() );
  delay(1000);
}