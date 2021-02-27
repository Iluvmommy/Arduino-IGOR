#include <Wire.h>
#include <Arduino.h>
#include <OneButton.h>
#include <Arduino_JSON.h>
#include <Firebase_Arduino_WiFiNINA_HTTPClient.h>
#include <Firebase_Arduino_WiFiNINA.h>
#include <LiquidCrystal.h>
#include "functions.h"
#include "secrets.h"

OneButton btn = OneButton(7, true, true);
LiquidCrystal lcd(0, 1, 2, 3, 4, 5);
FirebaseData firebaseData;

int status = WL_IDLE_STATUS;
String USER = "Kaavya";

void setup() {
  Serial.begin(2400);
  lcd.begin(16, 2);
  Wire.setClock(10000);
  printString("starting...");

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    delay(10000);
  }

  printWifiStatus();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD);
  Firebase.reconnectWiFi(true);

  btn.attachClick(reload);
  
  Serial.println("Ready");
  printString("Ready :)");
}

static void reload() {
  Serial.println("getting from firebase");
  QueryFilter query;
  query.clearQuery();
  query.orderBy("$key");
  query.limitToLast(1);

  if (Firebase.getJSON(firebaseData, USER+"/songs", query)) {
    if (firebaseData.dataType() == "json") {
      Serial.println(firebaseData.jsonData());
      
      JSONVar res = JSON.parse(firebaseData.jsonData());
      JSONVar key = res.keys()[0];
      JSONVar json = res[key];
            
      if (JSON.typeof(json) == "undefined" || JSON.typeof(json) != "object") {
        Serial.println("invalid result");
        printString("invalid result");
        return;
      }

      lcd.clear();
      lcd.setCursor(0, 0);
      if (json.hasOwnProperty("track")) {
        lcd.print((String) JSON.stringify(json["track"]));
      } else {
        lcd.print("----");
      }

      lcd.setCursor(0, 1);
      if (json.hasOwnProperty("artist")) {
        lcd.print((String) JSON.stringify(json["artist"]));
      } else {
        lcd.print("----");
      }
    }
  } else {
    Serial.println("----------Can't get data--------");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("--------------------------------");
    Serial.println();
    printString("Error :(");
  }
}

void printString(String str) {
  lcd.clear();
  lcd.print(str);
}

void loop() {
  btn.tick();
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(10000);
  }
}
