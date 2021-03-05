#include <Wire.h>
#include <Arduino.h>
#include <OneButton.h>
#include <Arduino_JSON.h>
#include <Firebase_Arduino_WiFiNINA_HTTPClient.h>
#include <Firebase_Arduino_WiFiNINA.h>
#include <LiquidCrystal.h>
#include "functions.h"
#include "secrets.h"

OneButton btn = OneButton(7, true, true); // initialize button
LiquidCrystal lcd(0, 1, 2, 3, 4, 5);      // initialize lcd screen
FirebaseData firebaseData;                // initialize firebase

int status = WL_IDLE_STATUS;
String USER = "Kaavya"; // which user in database it should find
String category = "videos"; // songs or videos

void setup() {
  Serial.begin(2400);
  lcd.begin(16, 2);
  Wire.setClock(10000);
  printString("starting...");

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // connect with wifi until connected

    delay(10000);
  }

  printWifiStatus();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, WIFI_SSID, WIFI_PASSWORD); // connect with firebase
  Firebase.reconnectWiFi(true);

  btn.attachClick(reload); // attach interrupt for button, reload is called everytime it is pressed

  Serial.println("Ready");
  printString("Ready :)");
}

static void reload() {
  Serial.println("getting from firebase");

  String creator = "artist";
  String thing = "track";

  if (category == "videos") {
    creator = "channel";
    thing = "title";
  };
  
  QueryFilter query; // search latest entry on database
  query.clearQuery();
  query.orderBy("$key");
  query.limitToLast(1);

  // retrieve from firebase
  if (Firebase.getJSON(firebaseData, USER + "/" + category, query)) {
    if (firebaseData.dataType() == "json") {

      // justs gets the child, aka reformatting retrieved data
      JSONVar res = JSON.parse(firebaseData.jsonData());
      JSONVar key = res.keys()[0];
      JSONVar json = res[key];
      Serial.println(firebaseData.jsonData());
      Serial.println(json);

      // if data is not correct format, dont go on and show error
      if (JSON.typeof(json) == "undefined" || JSON.typeof(json) != "object") {
        Serial.println("invalid result");
        printString("invalid result");
        return;
      }

      // printing data
      lcd.clear();
      lcd.setCursor(0, 0);

      //either prints the track or title or "----"
      if (json.hasOwnProperty(thing)) {
        lcd.print((String) JSON.stringify(json[thing]));
      } else {
        lcd.print("----");
      }

      lcd.setCursor(0, 1);
      // either prints channel, artist, or "----"
      if (json.hasOwnProperty(creator)) {
        lcd.print((String) JSON.stringify(json[creator]));
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

// print anything on lcd screen
void printString(String str) {
  lcd.clear();
  lcd.print(str);
}

void loop() {
  btn.tick();

  // if wifi disconnects, reconnect
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(WIFI_SSID);
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(10000);
  }
}
