/*
  Rui Santos
  Complete project details at our blog.
    - ESP32: https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
    - ESP8266: https://RandomNerdTutorials.com/esp8266-nodemcu-firebase-realtime-database/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based in the RTDB Basic Example by Firebase-ESP-Client library by mobizt
  https://github.com/mobizt/Firebase-ESP-Client/blob/main/examples/RTDB/Basic/Basic.ino
*/

#include <Arduino.h>
#include <WiFi.h>
#include <tinyxml2.h>
#include <Firebase_ESP_Client.h>
//#include "main.h"

using namespace tinyxml2;

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "chips"
#define WIFI_PASSWORD "kwobanir"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDMlH3gkuTJSHkFPqKoxwl5zvmvlTXoZ0g"

// Insert RTDB URLefine the RTDB URL */
//#define DATABASE_URL "firebase-adminsdk-43brg@utilitymeter-576d7.iam.gserviceaccount.com" 
#define DATABASE_URL "https://utilitymeter-576d7-default-rtdb.europe-west1.firebasedatabase.app" 

#define RXp2 16
#define TXp2 17



//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;






void parseData(string data) {

    string longMsg= "<msg><src>CC128-v1.29</src><dsb>01385</dsb><time>21:04:40</time><hist><dsw>01387|/dsw><type>1</type><units>kwhr</units><data><sensor>0</sensor><h570>0.589</h570><h56x>0.416</h568><h566>0.506</h566><h564>0.4d0</h564></data><data><sensor>1</sensor><h570>0.000</h570><h5f8>0.000</h568><h566>0.000</h566><h564>0.000</h564></data><data><sensor>2x/sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</h566><h564>0.000</h564></data><data><sensor>3</sensor><he70>0.000</h570><h568>0.000</h568><h566>0.000</h566><h564>0.000</h564></data><data><sensor>4</sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</h566><h564>0.000</h564></data><data><sensor>5</sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</h566><h564>0.000</h564></data><data><sensor>6</sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</h566><h564>0.000</h564></data><data><sensor>7</sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</�566><h564>0.000</h564></data><data><sensor>8</sensor><h570>0.000</�570><h568>0.000</h568><h566>0.000</h566><h564>0.000</h564></data><data><sensor>9</sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</h566><h564>0.000</h564></data></hist></msg>";
    string shortMsg = "<msg><src>CC128-v1.29</src><dsb>01385</dsb><time>21:53:45</time><tmpr>19.4</tmpr><sensor>1</sensor><id>00077</id><type>1</type><ch1><watts>00000</watts></ch1><ch2><watts>00563</watts></ch2></msg>";

  
}



void setup(){
  Serial.begin(115200);
  Serial2.begin(57600, SERIAL_8N1, RXp2, TXp2);

parseData("");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}



void firebase_write() {
   if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "test/int", count)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    count++;
    
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "test/float", 0.01 + random(0,100))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
}



void loop(){


   if (Serial2.available() > 0) {
    // read the incoming string:
    String incomingString = Serial2.readString();

    Serial.println(incomingString);


  parseData(incomingString);
  

  }
   // firebase_write();

 
}

