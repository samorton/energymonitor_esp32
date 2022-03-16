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
#include "main.h"
#include <Regexp.h>
using namespace tinyxml2;

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "chips"
#define WIFI_PASSWORD "kwobanir"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDMlH3gkuTJSHkFPqKoxwl5zvmvlTXoZ0g"

// Insert RTDB URLefine the RTDB URL */
//#define DATABASE_URL "firebase-adminsdk-43brg@utilitymeter-576d7.iam.gserviceaccount.com"
#define DATABASE_URL "https://utilitymeter-576d7-default-rtdb.europe-west1.firebasedatabase.app"

#define USER_EMAIL "sam.orton@gmail.com"
#define USER_PASSWORD "jamdonut"

#define RXp2 16
#define TXp2 17

// Define Firebase Data object
FirebaseData fbdo;

// firebase auth
FirebaseAuth auth;
FirebaseConfig config;

// Variable to save USER UID
String uid;

bool signupOK = false;

String energyPath = "/energy";
String timePath = "/timestamp";

// Database main path (to be updated in setup with the user UID)
String databasePath;

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;


const char *ntpServer = "pool.ntp.org";

void initWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void initFirebase()
{

  // Firebase: Assign the api key (required)
  config.api_key = API_KEY;

  // Firebase: Assign the RTDB URL
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // Sign up as anon
  /*
    if (Firebase.signUp(&config, &auth, "", ""))
    {
      Serial.println("Firebase Connected and Authenticated OK");
      signupOK = true;
    }

    else
    {
      Serial.printf("%s\n", config.signer.signupError.message.c_str());
    }
  */

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "")
  {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // Update database path
  databasePath = "/UsersData/" + uid + "/readings";
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(57600, SERIAL_8N1, RXp2, TXp2);

  initWifi();

  // Set current time
  configTime(0, 0, ntpServer);

  initFirebase();
}

// Function that gets current epoch time
unsigned long getTime()
{
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    // Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}

void loop()
{

  if (Serial2.available() > 0)
  {
    // read the incoming string:
    String incomingString = Serial2.readString();

    Serial.println(incomingString);

    handleMessage(incomingString);
  }
}

void handleMessage(String data)
{
  // 1. decide what kind of message it is.

  // 2 parse it.

  // 3. whack it in firebase

  if (data.length() < 250)
  {
    handleRegularMessage(data);
  }
  else
  {
    handleHistoryMessage(data);
  }
}

/*
  <msg>
    <src>CC128-v1.29</src>
    <dsb>01385</dsb>
    <time>21:53:45</time>
    <tmpr>19.4</tmpr>
    <sensor>1</sensor>
    <id>00077</id>
    <type>1</type>
    <ch1>
      <watts>00000</watts>
    </ch1>
    <ch2>
      <watts>00563</watts>
    </ch2>
  </msg>
  */

void handleRegularMessageAsXml(String data)
{

  XMLDocument xmlDocument;

  char *newData = StringToChar(data);

  Serial.println(newData);

  if (xmlDocument.Parse(newData) != XML_SUCCESS)
  {
    Serial.println("Error parsing");
    return;
  };

  XMLNode *msg = xmlDocument.FirstChild();

  UsageMsg usage;

  usage.src = msg->FirstChildElement("src")->FirstChild()->ToText()->Value();
  usage.dsb = atoi(msg->FirstChildElement("dsb")->FirstChild()->ToText()->Value());
  usage.time = msg->FirstChildElement("time")->FirstChild()->ToText()->Value();
  usage.tmpr = atof(msg->FirstChildElement("tmpr")->FirstChild()->ToText()->Value());
  usage.sensor_id = msg->FirstChildElement("id")->FirstChild()->ToText()->Value();
  usage.sensor_tp = atoi(msg->FirstChildElement("type")->FirstChild()->ToText()->Value());
  usage.ch1_watts = atoi(msg->FirstChildElement("ch1")->FirstChildElement("watts")->FirstChild()->ToText()->Value());
  usage.ch2_watts = atoi(msg->FirstChildElement("ch2")->FirstChildElement("watts")->FirstChild()->ToText()->Value());

  debug_msg(usage);
  firebase_write(usage);
}

void handleRegularMessage(String data)
{

  int n = 0;
  UsageMsg usage;
  usage.src = xml_parse(data, "src", n);
  usage.dsb = xml_parse(data, "dsb", n).toInt();
  usage.time = xml_parse(data, "time", n);
  usage.tmpr = xml_parse(data, "tmpr", n).toFloat();
  usage.sensor_id = xml_parse(data, "id", n);
  usage.sensor_tp = xml_parse(data, "type", n).toInt();
  usage.ch1_watts = xml_parse(data, "watts", n).toInt();
  usage.ch2_watts = xml_parse(data, "watts", n).toInt();

  //cal cost

/*

For example: let’s predict how much it costs to power a light bulb every hour. 
A 100 watt light bulb uses 100 watts of power. To convert the power in watts to kilowatt-hours, 
multiply 100 watts by 1 hour, then divide by 1,000 to find the energy usage in kWh.

Energy = (100 × 1) ÷ 1,000
Energy = 100 ÷ 1,000
Energy = 0.1 kWh

If electricity costs $0.12 per kWh, then a 100 watt light bulb will cost 1.2 cents per hour that it’s on.

*/

  debug_msg(usage);
  firebase_write(usage);
}

void debug_msg(UsageMsg msg)
{
#ifdef VERBOSE
  Serial.printf("src: %s, dsb: %d, time: %s, tmpr: %f, sensor_id: %s, sensor_tp: %d, ch1_watts: %d, ch2_watts: %d\r\n",
                msg.src, msg.dsb, msg.time, msg.tmpr, msg.sensor_id, msg.sensor_tp, msg.ch1_watts, msg.ch2_watts);
#endif
}

void firebase_write(UsageMsg msg)
{
  if (Firebase.ready())
  {

    
    timestamp = getTime();
    Serial.print("time: ");
    Serial.println(timestamp);

    parentPath = databasePath + "/" + String(timestamp);

    FirebaseJson json;

    json.set("/timestamp", String(timestamp));
    json.set("/dsb", msg.dsb);
    json.set("/time", msg.time);
    json.set("/tmpr", String(msg.tmpr));
    json.set("/sensor_id", msg.sensor_id);
    json.set("/sensor_tp", msg.sensor_tp);
    json.set("/ch1_watts", String(msg.ch1_watts));
    json.set("/ch2_watts", String(msg.ch2_watts));

    Serial.printf("Set json... %s\n", 
      Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    
  }
}

void firebase_write_old(UsageMsg msg)
{
  if (Firebase.ready())
  {
    evaluate(Firebase.RTDB.setString(&fbdo, "Usage/src", msg.src));
    evaluate(Firebase.RTDB.setInt(&fbdo, "Usage/dsb", msg.dsb));
    evaluate(Firebase.RTDB.setString(&fbdo, "Usage/time", msg.time));
    evaluate(Firebase.RTDB.setFloat(&fbdo, "Usage/tmpr", msg.tmpr));
    evaluate(Firebase.RTDB.setString(&fbdo, "Usage/sensor_id", msg.sensor_id));
    evaluate(Firebase.RTDB.setInt(&fbdo, "Usage/sensor_tp", msg.sensor_tp));
    evaluate(Firebase.RTDB.setInt(&fbdo, "Usage/ch1_watts", msg.ch1_watts));
    evaluate(Firebase.RTDB.setInt(&fbdo, "Usage/ch2_watts", msg.ch2_watts));
  }
}

void evaluate(bool action)
{
  if (action)
  {
    Serial.println("PASSED:  PATH: " + fbdo.dataPath() + ", TYPE: " + fbdo.dataType());
  }
  else
  {
    Serial.println("FAILED - REASON: " + fbdo.errorReason());
  }
}

// handle the longer messages

/*
  <msg><src>CC128-v1.29</src><dsb>01385</dsb><time>21:04:40</time><hist><dsw>01387|/dsw><type>1</type><units>kwhr</units><data><sensor>0</sensor><h570>0.589</h570><h56x>0.416</h568><h566>0.506</h566>
  <h564>0.4d0</h564></data><data><sensor>1</sensor><h570>0.000</h570><h5f8>0.000</h568><h566>0.000</h566><h564>0.000</h564></data><data><sensor>2x/sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</h566>
  <h564>0.000</h564></data><data><sensor>3</sensor><he70>0.000</h570><h568>0.000</h568><h566>0.000</h566><h564>0.000</h564></data><data><sensor>4</sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</h566>
  <h564>0.000</h564></data><data><sensor>5</sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</h566><h564>0.000</h564></data><data><sensor>6</sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</h566>
  <h564>0.000</h564></data><data><sensor>7</sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</�566><h564>0.000</h564></data><data><sensor>8</sensor><h570>0.000</�570><h568>0.000</h568><h566>0.000</h566>
  <h564>0.000</h564></data><data><sensor>9</sensor><h570>0.000</h570><h568>0.000</h568><h566>0.000</h566><h564>0.000</h564></data></hist></msg>
  */
void handleHistoryMessage(String data)
{
}

char *StringToChar(String str)
{

  int len = str.length() + 1;

  // Prepare the character array (the buffer)
  char *arr = new char(len);

  // Copy it over
  str.toCharArray(arr, len);

  arr[len] = 0;

  return arr;
}

void parseMessageDataAsRegex(String data)
{
  // REgex possibility, too fragile maybe
  //  https://registry.platformio.org/libraries/nickgammon/Regexp/examples/Match/Match.pde
}

String xml_parse(String inStr, String needParam, int &startIndex)
{

  int istart = inStr.indexOf("<" + needParam + ">", startIndex);
  if (istart > 0)
  {
    int n = needParam.length();
    int iend = inStr.indexOf("</" + needParam + ">", istart);
    if (iend > -1)
      startIndex = iend;
    String result = inStr.substring(istart + n + 2, iend);

#ifdef VERBOSE
    Serial.println("needParam: " + needParam + ", startIndex:" + startIndex + ", result: " + result);
#endif

    return result;
  }
  return "";
}
