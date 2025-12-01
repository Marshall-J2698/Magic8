#include <Arduino.h>
#include <TFT_eSPI.h>
#include <Adafruit_ADXL345_U.h>
#include "Free_Fonts.h"
#include "CACERT.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <Preferences.h>

// #define DEBUG
#define DEF_BLUE 0x0027

#define MAXPROJ 20
#define PROJLEN 40
#define WIFICONNECTTIMEOUT 15000

void projectAnimation(char *projectName);
void storeHTTPSdata();

const long MINDELAY = 5000;
const long HOLDFOR = 4000;
const float SHAKETHRESHOLD = 22;
float Ax,Ay,Az,mag;

String targ = "<li><a href=";

TFT_eSPI tft = TFT_eSPI();
WiFiMulti WiFiMulti;
Preferences preferences;

int xpos, ypos;

char projBuffer[MAXPROJ][PROJLEN];
char keyStr[4];

enum ScreenStates{
  IDLE,
  SHAKEN
};

enum ScreenStates curScreen = IDLE;


Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
unsigned long last_fire = 0;

#define HOR_RES 240
#define VER_RES 240

void setup() {
  xpos = 120;
  ypos = 105;
  accel.begin();
  accel.setRange(ADXL345_RANGE_16_G);
  Serial.begin();
  tft.begin();
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  preferences.begin("projStorage", false);

  tft.drawString("Connecting...",xpos,ypos,GFXFF);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("SSID", "PASS");

  // wait for WiFi connection
  Serial.print("Waiting for WiFi to connect...");
  
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.print(".");
    
    if(millis() > WIFICONNECTTIMEOUT){
      Serial.println("WIFI connection failed");
      tft.fillScreen(TFT_BLACK);
      tft.drawString("Connection Failed.",xpos,ypos,GFXFF);
      delay(1000);
      tft.fillScreen(TFT_BLACK);
      break;
    };
  }
  delay(500);

  if(WiFiMulti.run() == WL_CONNECTED){
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Connected! Fetching...",xpos,ypos,GFXFF);
    storeHTTPSdata();
    tft.fillScreen(TFT_BLACK);
  }
  


  
}

void loop() {
  delay(10);
  sensors_event_t event;
  accel.getEvent(&event);
  Ax = event.acceleration.x;
  Ay = event.acceleration.y;
  Az = event.acceleration.z;
  mag = sqrt(pow(Ax,2) + pow(Ay,2)+ pow(Az,2));
  if (millis()-last_fire > MINDELAY && mag > SHAKETHRESHOLD){
    int ind = random(0,preferences.getInt("numProj"));
    char tempBuf[PROJLEN];
    itoa(ind,keyStr,10);
    preferences.getString(keyStr,tempBuf,PROJLEN);
    projectAnimation(tempBuf);
  }

  #ifdef DEBUG
    Serial.print(Ax);
    Serial.print(',');
    Serial.print(Ay);
    Serial.print(',');
    Serial.print(Az);
    Serial.print(',');
    Serial.print("mag: ");
    Serial.print(mag);
    Serial.println();
  #endif

  if (millis()-last_fire > HOLDFOR && curScreen == SHAKEN){
    curScreen = IDLE;
    for(int i = 0; i < 256; i+=2){
      tft.fillTriangle(120,16,30,172,210,172,tft.alphaBlend(i,TFT_BLACK,DEF_BLUE));
    }
    tft.fillScreen(TFT_BLACK);
  }
}

void projectAnimation(char *projectName){
  delay(1000);
  tft.setFreeFont(FSB24);
  
  
  
  char *token = strtok(projectName,"_");
  for(int i = 0; i < 256; i+=2){
    tft.fillTriangle(120,16,30,172,210,172,tft.alphaBlend(i,DEF_BLUE,TFT_BLACK));
    if (i > 120){
      tft.setTextColor(tft.alphaBlend(i,TFT_WHITE,TFT_BLACK));
      tft.drawString(projectName,xpos,ypos,GFXFF);
      }
    }
      int offset = 0;
      tft.setTextColor(TFT_WHITE);
      while (token != NULL){
        tft.drawString(token,xpos,ypos+offset,GFXFF); 
        offset += 8;
        token = strtok(NULL, "_");
  }

  curScreen = SHAKEN;
  last_fire = millis();
}

void storeHTTPSdata() {
  int projInd = 0;
  
  WiFiClientSecure* client = new WiFiClientSecure;
  if (client) {
    client->setCACert(rootCACertificate);
    client->setTimeout(30000);
    {
      HTTPClient https;
      Serial.print("[HTTPS] begin...\n");
      if (https.begin(*client, "https://makerspace.cc/Category:SimpleProjects")) {  // HTTPS
        Serial.print("[HTTPS] GET...\n");
        int httpCode = https.GET();
        if (httpCode > 0) {
          Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = https.getString();
            int iter = 0;
            while (1) {
              iter = payload.indexOf(targ, iter);
              if (iter == -1) break;
              iter += targ.length()+2;
              int i = 0;
              while (payload[iter+i]!= '"') {
                if (payload[iter+i] == '(') break;
                projBuffer[projInd][i] = payload[iter+i];
                #ifdef DEBUG
                  Serial.print(payload[iter+i]);
                #endif
                i++;
              }
              projBuffer[projInd][i+1] = '\0';
              iter++;
              projInd++;
              Serial.println();
            }
              preferences.putInt("numProj",projInd);
              for(int j = 0; j < projInd; j++){
                itoa(j,keyStr,10);
                // Serial.printf("Trying to store: %s. ", projBuffer[j]);
                preferences.putString(keyStr,projBuffer[j]);
              }
              Serial.println("Stored");
            }
          } else {
            Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
          }

          https.end();
        } else {
          tft.fillScreen(TFT_BLACK);
          tft.drawString("Unable to make HTTPS req",xpos,ypos,GFXFF);
          delay(1000);
          tft.fillScreen(TFT_BLACK);
          #ifdef DEBUG
            Serial.printf("[HTTPS] Unable to connect\n");
          #endif

        }

      }
      delete client;
    }
    else {
      #ifdef DEBUG
        Serial.println("Unable to create client");
      #endif 
    }
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Successful req!",xpos,ypos,GFXFF);
    delay(1000);
    tft.fillScreen(TFT_BLACK);
    #ifdef DEBUG
      Serial.print("Current values: ");
      char tempBuf[PROJLEN];
      for(int i=0 ; i < preferences.getInt("numProj"); i++){
        itoa(i,keyStr,10);
        preferences.getString(keyStr,tempBuf,PROJLEN);
        Serial.printf("%s",tempBuf);
        Serial.print(" / ");
      }
    #endif
  }



