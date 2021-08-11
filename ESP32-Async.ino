/****************************************************************************************************************************
  ESP32-Async.ino (For ESP32 boards)
    Stitch together by Bob Pearn (r.j.pearn@gmail.com)

  As I struggle as a beginner, not only with the soldering of tiny components, 
  but also, with the code I wanted to interface with my devices. I could find 
  pieces of this or parts of that, but nothing all inclusive. And there is still 
  a world of IOT I haven't even looked at.
  
  The sketch and files included in this project contain features:
      * Wi-Fi Station and Access Point
      * OTA updates
      * Multi-page Web Server
      * SPIFFS saving and retrieval of system variables
      * NTP time server
   
  Based on  https://github.com/khoih-prog/ESPAsync_WiFiManager
            https://github.com/ayushsharma82/AsyncElegantOTA
            ESPAsyncWebServer
  Licensed under MIT license

  With much assistance from (but not limited to) tutorials and sample code provided by
      https://www.instructables.com/
      https://randomnerdtutorials.com/
      https://techtutorialsx.com/
      https://www.hackster.io/
      https://www.youtube.com/
      https://www.arduino.cc/
      https://www.w3schools.com/

  Special thanks to several members of the ESP8266/ESP32 group 
      https://www.facebook.com/groups/1606743019578078

  Instructions:
    Open this ESP32-Async.ino with Arduino IDE.
    Verify the libraries list in the include are installed on you system.
    Compile and upload to your ESP32.
    Use the Tools and upload the ESP data files. 
    Go to your Wi-Fi connection and select the ESP32-Async access point.
    Enter "password" for the password. I will now connect.
    Open your browser and type http://192.168.4.1. into the address bar.
    The ESP may timeout as one font it uses is on a url and your not connected to the internet.
    Just wait, reload if required, but it will eventually open.
    It is probably best to configure the network settings first, then connect your Wi-Fi to the local network.
    The serial monitor will identify and display the ESPs IP address.
    I hope you enjoy my App and make good ue of it.

    NOTE: the "Task watchdog" may reboot the ESP while both STN and AP modes are running at the same time.
    If you find a solution to the watchdog please let me know so I can fix my code.

 *****************************************************************************************************************************/
#include <Arduino.h>
#include "time.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

// All global variables are kept in the header_var.h file
#include "header_vars.h"

AsyncWebServer webserver(80);

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// handle WiFi events. Nice to have for debug but otherwise can be removed
void OnWiFiEvent(WiFiEvent_t event){
  switch (event) {
     case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("ESP32 Connected to WiFi Network");
      break;
   case SYSTEM_EVENT_AP_START:
      Serial.println("ESP32 soft AP started");
      break;
    case SYSTEM_EVENT_AP_STACONNECTED:
      Serial.println("A station just connected to ESP32 soft AP");
      break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
      Serial.println("A station just Station disconnected from ESP32 soft AP");
      break;
    default: break;
  }
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void GetNTPtime(){
  //this function needs some error checking added. It won't work if there is no internet connection
  configTzTime(NTPzone.c_str(), NTPserver.c_str()); // adjust to your local time zone with variable timezone
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void updateVars(){ //called from setup, loads all variables stored in files. 
  // serial.print for debug and shows passwords in monitor, can remove serial.print
  Serial.println("***********************************************************");
  alarmtime = readFile(SPIFFS, "/inputAlarmtime.txt");
  Serial.print("*** alarmtime: "); Serial.println(alarmtime);
  alarmswitch = readFile(SPIFFS, "/inputAlarmswitch.txt").toInt();
  Serial.print("*** alarmswitch: "); Serial.println(alarmswitch);
  wifi_network_ssid = readFile(SPIFFS, "/inputNetSSID.txt");
  Serial.print("*** wifi_network_ssid: "); Serial.println(wifi_network_ssid);
  wifi_network_password = readFile(SPIFFS, "/inputNetPASS.txt");
  Serial.print("*** wifi_network_password: "); Serial.println(wifi_network_password);
  wifi_network_hostname = readFile(SPIFFS, "/inputNetHOST.txt");
  Serial.print("*** wifi_network_hostname: "); Serial.println(wifi_network_hostname);
  soft_ap_ssid = readFile(SPIFFS, "/inputAPSSID.txt");
  Serial.print("*** soft_ap_ssid: "); Serial.println(soft_ap_ssid);
  soft_ap_password = readFile(SPIFFS, "/inputAPPASS.txt");
  Serial.print("*** soft_ap_password: "); Serial.println(soft_ap_password);
  AP_period  = readFile(SPIFFS, "/inputAPTTL.txt").toInt();
  Serial.print("*** AP_TTL: "); Serial.println(AP_period);
  Serial.println("***********************************************************");
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// function for when webpage not found
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  readFile is called from functions updatevars and processor
String readFile(fs::FS &fs, const char * path){
//  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
//  Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  file.close();
//  Serial.println(fileContent);
  return fileContent;
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// writeFile is called when data is received from webpages
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.println("***********************************************************");
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
  updateVars();
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Replaces webpage placeholder with stored values
String processor(const String& var){
  //Serial.println(var);
  if(var == "inputAlarmtime"){
    return readFile(SPIFFS, "/inputAlarmtime.txt");
  }
 else if(var == "inputAlarmswitch"){
    return readFile(SPIFFS, "/inputAlarmswitch.txt");
  }
  else if(var == "inputNetSSID"){
    return readFile(SPIFFS, "/inputNetSSID.txt");
  }
  else if(var == "inputNetPASS"){
    return readFile(SPIFFS, "/inputNetPASS.txt");
  }
  else if(var == "inputAPSSID"){
    return readFile(SPIFFS, "/inputAPSSID.txt");
  }
  else if(var == "inputAPPASS"){
    return readFile(SPIFFS, "/inputAPPASS.txt");
  }
  else if(var == "inputAPTTL"){
    return readFile(SPIFFS, "/inputAPTTL.txt");
  }
  else if(var == "inputNetHOST"){
    return readFile(SPIFFS, "/inputNetHOST.txt");
  }
  return String();
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// checks status of access point. AP_period is changable via webpage
void checkAP(){
  if(millis() >= APTimer + (AP_period * 60000)){ // multiply AP_period. 60000 == 1 minute
    if(WiFi.status() != WL_CONNECTED){ //if the Wi-Fi has no connection
      Serial.print("Attempting to connect to Local network, SSID: ");
      Serial.println("Reconnecting Local WiFi");
      WiFi.disconnect(); //disconnect prior attempt
      WiFi.begin(wifi_network_ssid.c_str(), wifi_network_password.c_str()); //restart Wi-Fi
      delay(5000); //gives all resources and extra time for wi-fi to connect
      Serial.print("ESP32 IP on the WiFi network: ");
      Serial.println(WiFi.localIP());
      if(APdisconnected == true){ // if the local network is down restart the AP
        if(WiFi.softAP(soft_ap_ssid.c_str(), soft_ap_password.c_str())){
          IPAddress myIP = WiFi.softAPIP();
          Serial.println("Network " + String(soft_ap_ssid) + " restarted");
          Serial.print("AP IP address: ");
          Serial.println(myIP);
          APdisconnected = false;
          Serial.println("Restarting AP Time-to-Live at " + String(AP_period) + " minutes");
          APTimer = millis(); // resets the timer
        }
      }
      else{
          Serial.println("AP Disconnection Diverted!"); // if no network and AP timer has expired, the AP shutdown was bypassed
      }
    }
    else{
      if(APdisconnected == false){ // if the Wi-Fi is connected and APtimer has expired, shutdown the AP
        WiFi.softAPdisconnect (true);
        Serial.println("AP Disconnected");
        APdisconnected = true;
     }
    }
  }
  else{
    if(APdisconnected == false){ //display on monitor how long before APtimer epires
      Serial.print("AP will disconnect in ");
      int min = ((((APTimer + (AP_period * 60000)) - millis())/1000)/60);
      if(min < 10)Serial.print("0");
      Serial.print(min);
      Serial.print(":");
      int sec = ((((APTimer + (AP_period * 60000)) - millis())/1000)%60);
      if(sec < 10)Serial.print("0");
      Serial.println(sec);
    }
  }
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
String getESPdate(){ // send date in string format to webpage request
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return("Failed ESPtime");
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d, %Y", &timeinfo);
  String asString(timeStringBuff);
  Serial.println(timeStringBuff);
  return(timeStringBuff);
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
String getESPtime(){ // send time HH:MM:SS in string format to webpage request
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return("Failed ESPtime");
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &timeinfo);
  String asString(timeStringBuff);
  Serial.println(timeStringBuff);
  return(timeStringBuff);
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void setup() {
Serial.begin(115200);
  // Initialize SPIFFS
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  // Initialize Vars from files
  updateVars();

  //  Start Wi-Fi and AP
  WiFi.onEvent(OnWiFiEvent);
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAP(soft_ap_ssid.c_str(), soft_ap_password.c_str());
  WiFi.setHostname(wifi_network_hostname.c_str());
  WiFi.begin(wifi_network_ssid.c_str(), wifi_network_password.c_str());
    delay(5000); //allow plenty of time for network connections
  //display IPs on monitor
  Serial.print("ESP32 IP as soft AP: ");
  Serial.println(WiFi.softAPIP());
  if(wifi_network_ssid != ""){ 
    Serial.print("ESP32 IP on the WiFi network: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("Error With Local Credentials");
  }

  // send webpages on requests
  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  webserver.on("/time.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/time.html", String(), false, processor);
  });
  webserver.on("/ElegantOTA.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/ElegantOTA.html", String(), false, processor);
  });
  webserver.on("/network.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/network.html", String(), false, processor);
  });
  webserver.on("/softap.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/softap.html", String(), false, processor);
  });
  // send webpage stylesheet and javascript on request
  webserver.on("/site.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/site.css", String(), false, processor);
  });
  webserver.on("/site.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/site.js", String(), false, processor);
  });
  // send webpage time and date on request
  webserver.on("/ESPDATE", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getESPdate().c_str());
});
  webserver.on("/ESPTIME", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getESPtime().c_str());
});
  // send webpage inputs on request and update variables
  webserver.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam(PARAM_ALARMTIME)) {
      inputMessage = request->getParam(PARAM_ALARMTIME)->value();
      writeFile(SPIFFS, "/inputAlarmtime.txt", inputMessage.c_str());
      alarmtime = inputMessage;
    }
    else if (request->hasParam(PARAM_ALARMSWITCH)) {
      inputMessage = request->getParam(PARAM_ALARMSWITCH)->value();
      writeFile(SPIFFS, "/inputAlarmswitch.txt", inputMessage.c_str());
      alarmswitch = inputMessage.toInt();
    }
    else if (request->hasParam(PARAM_NETSSID)) {
      inputMessage = request->getParam(PARAM_NETSSID)->value();
      writeFile(SPIFFS, "/inputNetSSID.txt", inputMessage.c_str());
      wifi_network_ssid = inputMessage;
}
    else if (request->hasParam(PARAM_NETPASS)) {
      inputMessage = request->getParam(PARAM_NETPASS)->value();
      writeFile(SPIFFS, "/inputNetPASS.txt", inputMessage.c_str());
      wifi_network_password = inputMessage;
    }
    else if (request->hasParam(PARAM_NETHOST)) {
      inputMessage = request->getParam(PARAM_NETHOST)->value();
      writeFile(SPIFFS, "/inputNetHOST.txt", inputMessage.c_str());
      wifi_network_hostname = inputMessage;
    }
    else if (request->hasParam(PARAM_APSSID)) {
      inputMessage = request->getParam(PARAM_APSSID)->value();
      writeFile(SPIFFS, "/inputAPSSID.txt", inputMessage.c_str());
      wifi_network_ssid = inputMessage;
    }
    else if (request->hasParam(PARAM_APPASS)) {
      inputMessage = request->getParam(PARAM_APPASS)->value();
      writeFile(SPIFFS, "/inputAPPASS.txt", inputMessage.c_str());
      wifi_network_password = inputMessage;
    }
    else if (request->hasParam(PARAM_APTTL)) {
      inputMessage = request->getParam(PARAM_APTTL)->value();
      writeFile(SPIFFS, "/inputAPTTL.txt", inputMessage.c_str());
      wifi_network_hostname = inputMessage;
    }
    else if (request->hasParam(PARAM_RESTART)) {
    ESP.restart();
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  webserver.onNotFound(notFound);

  AsyncElegantOTA.begin(&webserver);    // Start ElegantOTA
  webserver.begin();  //Start Webserver
  Serial.println("Webserver Ready!");
  Serial.println("***********************************************************");
  APTimer = millis(); //Start AP timer
  GetNTPtime(); // Get time from NTP server
  LOOPtimer = millis();
  LOOP_period = 1000;
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void loop() {
  if(millis() >= LOOPtimer + LOOP_period){ //better than using delay(1000);
    AsyncElegantOTA.loop(); //keeps OTA in waiting mode
    checkAP(); // keeps an eye on network connections
    LOOPtimer = millis(); //resets this timer
  }
}
