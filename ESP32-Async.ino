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
      * Reconnects Wi-Fi connection when lost
   
  Based on but not specific to:
            https://github.com/khoih-prog/ESPAsync_WiFiManager
            https://github.com/ayushsharma82/AsyncElegantOTA
            https://github.com/me-no-dev/ESPAsyncWebServer
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
    Enter "password" for the password. It will now connect.
    Open your browser and type http://192.168.4.1/ into the address bar.
    The ESP may timeout as one font it uses is on a url and your not connected to the internet.
    Just wait, reload if required, but it will eventually open.
    Configure the network settings first, restart the ESP32.
    Reconnect to the ESP32-Async access point and refresh the browser page. The local IP will be listed just above the Menu bar.
    Disconnect from the ESP32-Async access point, reconnect to your local network.
    Type the local IP address given on the webpage into the address bar. It may require the http://
    I hope you enjoy my App and make good ue of it.


 *****************************************************************************************************************************/
// These are the libraries needed in this sketch
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

// All global variables are kept in the header_var.h file. You should see its tab above
#include "header_vars.h"

/* This assigns to the AsyncWebServer my server name "webserver" and tells it to listen 
on port 80. All web browsers use port 80 by default. */
AsyncWebServer webserver(80);

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// handle WiFi events. Nice to have for debug but otherwise can be removed
void OnWiFiEvent(WiFiEvent_t event){
  switch (event) {
     case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("ESP32 Connected to Local WiFi Network");
      break;
   case SYSTEM_EVENT_AP_START:
      Serial.println("ESP32 soft AP started or stopped");
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
// go get the NTP time from a server
void GetNTPtime(){
  if(WiFi.status() == WL_CONNECTED){//if the Wi-Fi has a connection
    configTzTime(NTPzone.c_str(), NTPserver.c_str()); // adjust to your local time zone with variable timezone on the webpage
    Serial.println("NTP time retrieved");
  }
  else{
    Serial.println("NTP Server not available"); // if no WI-Fi send message to the serial monitor
  }
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void updateVars(){ //called from setup and when a value has changed. loads all variables stored in files. 
  // serial.print for debug and shows passwords in monitor
  Serial.println("***********************************************************");
  alarmtime = readFile(SPIFFS, "/Alarmtime.txt");
  Serial.print("*** alarmtime: "); Serial.println(alarmtime);
  Alarm_onoff = readFile(SPIFFS, "/AlarmOnOff.txt").toInt();
  Serial.print("*** AlarmOnOff: "); Serial.println(Alarm_onoff);
  NTPserver = readFile(SPIFFS, "/NTPSERVER.txt");
  Serial.print("*** NTPSERVER: "); Serial.println(NTPserver);
  NTPzone = readFile(SPIFFS, "/NTPZONE.txt");
  Serial.print("*** NTPZONE: "); Serial.println(NTPzone);
  wifi_network_ssid = readFile(SPIFFS, "/NetSSID.txt");
  Serial.print("*** wifi_network_ssid: "); Serial.println(wifi_network_ssid);
  wifi_network_password = readFile(SPIFFS, "/NetPASS.txt");
  Serial.print("*** wifi_network_password: "); Serial.println(wifi_network_password);
  wifi_network_hostname = readFile(SPIFFS, "/NetHOST.txt");
  Serial.print("*** wifi_network_hostname: "); Serial.println(wifi_network_hostname);
  soft_ap_ssid = readFile(SPIFFS, "/APSSID.txt");
  Serial.print("*** soft_ap_ssid: "); Serial.println(soft_ap_ssid);
  soft_ap_password = readFile(SPIFFS, "/APPASS.txt");
  Serial.print("*** soft_ap_password: "); Serial.println(soft_ap_password);
  AP_period  = readFile(SPIFFS, "/APTTL.txt").toInt();
  Serial.print("*** AP_TTL: "); Serial.println(AP_period);
  WIFI_period  = readFile(SPIFFS, "/WIFItimer.txt").toInt();
  Serial.print("*** WIFI_period: "); Serial.println(WIFI_period);
  Serial.println("***********************************************************");
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// function for when a webpage request is not found
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  readFile is called from functions updatevars and processor. It reads the file and returns the value found
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
// writeFile is called when new data is received from webpages
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
// Replaces webpage place holder with stored values
String processor(const String& var){
  //Serial.println(var);
  if(var == "Alarmtime"){
    return readFile(SPIFFS, "/Alarmtime.txt");
  }
  else if(var == "AlarmOnOff"){
    return readFile(SPIFFS, "/AlarmOnOff.txt");
  }
  else if(var == "NTPSERVER"){
    return readFile(SPIFFS, "/NTPSERVER.txt");
  }
  else if(var == "NTPZONE"){
    return readFile(SPIFFS, "/NTPZONE.txt");
  }
  else if(var == "NetSSID"){
    return readFile(SPIFFS, "/NetSSID.txt");
  }
  else if(var == "NetPASS"){
    return readFile(SPIFFS, "/NetPASS.txt");
  }
  else if(var == "APSSID"){
    return readFile(SPIFFS, "/APSSID.txt");
  }
  else if(var == "APPASS"){
    return readFile(SPIFFS, "/APPASS.txt");
  }
  else if(var == "APTTL"){
    return readFile(SPIFFS, "/APTTL.txt");
  }
  else if(var == "NetHOST"){
    return readFile(SPIFFS, "/NetHOST.txt");
  }
  else if(var == "WIFItimer"){
    return readFile(SPIFFS, "/WIFItimer.txt");
  }
  return String();
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Checks status of access point. AP_period is changable via webpage.
My initial code would check the network connection and attempt to reconnect to it. 
These repeated attempts caused the "watchdog" to restart the ESP.
KISS = keep it simple stupid

OK, so now I added it again, but it is different. Before I was trying to get the 
network to restart with new credentials provided. I am guessing what was happening
is when the Wi-Fi was first started with no or bad credentials, the TCPIP sunk its
teeth into the settings, and even though I used wifi.disconnect() it would not forget. So,
when I used wifi.begin() with new credentials it choked and the watchdog shut the
ESP down.
A restart is now required to use new credentials. If the WI-Fi connection is lost,
the wifi.reconnect() is called, allowing the TCPIP to keep its grip on the credentials.
This helps to prevent the router from dropping the connection from lack of use.
*/
void checkWIFI(){
  Serial.println("Checking Wi-Fi");
  if(WIFIconnected){ //did it connect on startup?
    if(millis() >= (WIFItimer + (WIFI_period * 60000))){ // is it time to check it again?
      if(WiFi.status() != WL_CONNECTED){ //if the Wi-Fi actually lost its connection
        WiFi.disconnect(); // release the previous connection
        delay(3000); // give it time to shut down
        WiFi.reconnect(); // reconnect with previous credentials
        delay(3000); // give it time to reconnect
        Serial.println("Wi-Fi reconnected");
      }
      WIFItimer = millis(); //Restart WIFI timer for the next check
    }
    if(!APdisconnected){ //is the soft_AP still connected? - still inside the WIFIconnected if
      if(millis() >= APTimer + (AP_period * 60000)){ // has the APtimer expired?
        WiFi.softAPdisconnect (true); //disconnect the AP
        Serial.println("AP Disconnected");
        APdisconnected = true; //change the AP status
        /* 
        In the loop(); there is a timer set to 1000 that is used to execute 
        this function. Once the AP has disconnected there is no reason to be 
        executing it every second. We still want to check the network 
        connection and the minimum setting for that is every minute. So, lets 
        change the LOOP_period to match that minimum.
        */
        LOOP_period = 60000; //Loop every minute
      }
      else { //display on monitor how long before APtimer epires
        Serial.print("AP will disconnect in ");
        int min = ((((APTimer + (AP_period * 60000)) - millis())/1000)/60); //converts the millis count to minutes
        if(min < 10){
          Serial.print("0"); //if less than 10 display as 0x
        }
        Serial.print(min);
        Serial.print(":"); //add a colon 0x:
        int sec = ((((APTimer + (AP_period * 60000)) - millis())/1000)%60); // after converting to minutes, what is the remainder
        if(sec < 10){ 
          Serial.print("0"); //if less than 10 display as 0x:0
        }
        Serial.println(sec); // display what is left as 0x:xx
      }
    }
  }
  else{ //if there is no network connection
    Serial.println("AP Lives Forever!"); // No reason to use time-to-live timer
    kill_LOOP = true; //Kill the checkWiFI loop, no reason to check the network, we know a ESP restart is required.
  }
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
String getESPtimedate(String value){ // send date or time in string format for webpage place holders
  if(WiFi.status() == WL_CONNECTED){//if the Wi-Fi has a connection than we should have NTP time
    char timeStringBuff[50];
    struct tm timeinfo; //setup for the time varables
    if(!getLocalTime(&timeinfo)){ //get the time on the ESP
    return("Failed DateTime Request"); //just in case the NTP failed
    }
    if(value == "getdate"){ //does the webpage want the date?
      Serial.println("Date Requested");
      strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d, %Y", &timeinfo);
    }
    else if(value == "gettime"){ // does the webpage want the time?
      Serial.println("Time Requested");
      strftime(timeStringBuff, sizeof(timeStringBuff), "%H %M", &timeinfo);
    }
    String asString(timeStringBuff); //convert char to a string
    Serial.println(timeStringBuff);
    return(timeStringBuff); // send result back to place holder
  }
  else{ // if no Wi-Fi
    if(value == "getdate"){
      return("No Wi-Fi Connection"); //send something back
    }
    else if(value == "gettime"){
      return("00 00"); //send something back
     }
  }
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void setupWIFI(){
 //  Start Wi-Fi and AP
  WiFi.onEvent(OnWiFiEvent); //display on monitor what WiFi is doing
  WiFi.mode(WIFI_MODE_APSTA); // tells the WiFi to setup as both an AP and STA 
  WiFi.softAP(soft_ap_ssid.c_str(), soft_ap_password.c_str()); // Start the AP with default settings
  delay(2000); //allow plenty of time for AP to start

  Serial.print("Open http://"); //display reminder for http:// text
  Serial.println(WiFi.softAPIP());  //display IP on monitor
  // check if default network settings have been changed and also they are not blanks
  if(wifi_network_ssid != "YOURNETWORKSSID" || wifi_network_password != "YOURNETWORKPASSWORD" || wifi_network_ssid != "" || wifi_network_password != ""){ 
    WiFi.setHostname(wifi_network_hostname.c_str()); // set the hostname for the ESP
    WiFi.begin(wifi_network_ssid.c_str(), wifi_network_password.c_str()); //Start the local WiFI connection
    delay(5000); //allow plenty of time for network connections
    if(WiFi.status() == WL_CONNECTED){ // check if the Wi-Fi has a connection
      Serial.print("Open http://"); //display reminder for http:// text
      Serial.println(WiFi.localIP());  //display IP on monitor
      WIFIconnected = true; //The ESP is connected!
    }
    else {
      Serial.println("Error With Local Credentials"); //something went wrong
      Serial.println("WiFI could not connect");
    }
  }
  else {
    Serial.println("No Local Credentials Provided"); //default credentials or blanks found
  }
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void setupwebserver(){
  // handle time requests
  webserver.on("/ESPTIME", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getESPtimedate("gettime")); //looking for time? get it from getESPtimedate function
  });
  webserver.on("/ESPDATE", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", getESPtimedate("getdate")); //looking for date? get it from getESPtimedate function
  });
// handle IP requests
  webserver.on("/WIFIIP", HTTP_GET, [](AsyncWebServerRequest *request){
    String ipAsString = WiFi.localIP().toString().c_str(); //convert the IP address to a string. I am told this wouldn't work in C++. Arduino does it!
    request->send(200, "text/plain", ipAsString); //send back converted string
  });
// handle webpage requests, this handles the use of multiple webpages
  webserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor); //get the requested file from SPIFFS storage and use the processor function to fill in the place holders
  });
  webserver.on("/time.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/time.html", String(), false, processor);  //get the requested file from SPIFFS storage and use the processor function to fill in the place holders
  });
  webserver.on("/ElegantOTA.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/ElegantOTA.html", String(), false, processor);  //get the requested file from SPIFFS storage and use the processor function to fill in the place holders
  });
  webserver.on("/network.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/network.html", String(), false, processor);  //get the requested file from SPIFFS storage and use the processor function to fill in the place holders
  });
  webserver.on("/softap.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/softap.html", String(), false, processor);  //get the requested file from SPIFFS storage and use the processor function to fill in the place holders
  });
  // send webpage stylesheet and javascript on request
  webserver.on("/site.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/site.css", String(), false, processor); //get the requested file from SPIFFS storage and use the processor function to fill in the place holders
  });
  webserver.on("/site.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/site.js", String(), false, processor);  //get the requested file from SPIFFS storage and use the processor function to fill in the place holders
  });
   webserver.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) { //if the webpage is sending data
    String inputMessage;
// handle changes made to the Alarm settings
    if (request->hasParam(PARAM_ALARMTIME)) {
      inputMessage = request->getParam(PARAM_ALARMTIME)->value();
      writeFile(SPIFFS, "/Alarmtime.txt", inputMessage.c_str());
      alarmtime = inputMessage;
    }
    else if (request->hasParam(PARAM_ALARMONOFF)) {
      inputMessage = request->getParam(PARAM_ALARMONOFF)->value();
      writeFile(SPIFFS, "/AlarmOnOff.txt", inputMessage.c_str());
      Alarm_onoff = inputMessage.toInt();
    }
// handle changes made to the NTP settings
    else if (request->hasParam(PARAM_NTPSERVER)) {
      inputMessage = request->getParam(PARAM_NTPSERVER)->value();
      writeFile(SPIFFS, "/NTPSERVER.txt", inputMessage.c_str());
      NTPserver = inputMessage;
      GetNTPtime(); //when a change is made update the ESP time
    }
    else if (request->hasParam(PARAM_NTPZONE)) {
      inputMessage = request->getParam(PARAM_NTPZONE)->value();
      writeFile(SPIFFS, "/NTPZONE.txt", inputMessage.c_str());
      NTPzone = inputMessage;
      GetNTPtime(); //when a change is made update the ESP time
    }
// handle changes made to the Network settings
    else if (request->hasParam(PARAM_NETSSID)) {
      inputMessage = request->getParam(PARAM_NETSSID)->value();
      writeFile(SPIFFS, "/NetSSID.txt", inputMessage.c_str());
      wifi_network_ssid = inputMessage;
}
    else if (request->hasParam(PARAM_NETPASS)) {
      inputMessage = request->getParam(PARAM_NETPASS)->value();
      writeFile(SPIFFS, "/NetPASS.txt", inputMessage.c_str());
      wifi_network_password = inputMessage;
    }
    else if (request->hasParam(PARAM_NETHOST)) {
      inputMessage = request->getParam(PARAM_NETHOST)->value();
      writeFile(SPIFFS, "/NetHOST.txt", inputMessage.c_str());
      wifi_network_hostname = inputMessage;
    }
// handle changes made to the AP settings
    else if (request->hasParam(PARAM_APSSID)) {
      inputMessage = request->getParam(PARAM_APSSID)->value();
      writeFile(SPIFFS, "/APSSID.txt", inputMessage.c_str());
      wifi_network_ssid = inputMessage;
    }
    else if (request->hasParam(PARAM_APPASS)) {
      inputMessage = request->getParam(PARAM_APPASS)->value();
      writeFile(SPIFFS, "/APPASS.txt", inputMessage.c_str());
      wifi_network_password = inputMessage;
    }
    else if (request->hasParam(PARAM_APTTL)) {
      inputMessage = request->getParam(PARAM_APTTL)->value();
      writeFile(SPIFFS, "/APTTL.txt", inputMessage.c_str());
      wifi_network_hostname = inputMessage;
    }
// handle changes made to the WIFI timer
      else if (request->hasParam(PARAM_WIFITIMER)) {
      inputMessage = request->getParam(PARAM_WIFITIMER)->value();
      writeFile(SPIFFS, "/WIFItimer.txt", inputMessage.c_str());
      WIFI_period = inputMessage.toInt();
    }
// when the restart button is pressed
      else if (request->hasParam(PARAM_RESTART)) {
    ESP.restart();
    }
// when no PARAM is found
      else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
// when webpage request is wrong
    webserver.onNotFound(notFound);

  AsyncElegantOTA.begin(&webserver);    // Start ElegantOTA
  webserver.begin();  //Start the webserver we declaired earlier on port 80
  Serial.println("Webserver Ready!");
  Serial.println("***********************************************************");
  
}
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void setup() {
Serial.begin(115200);
  // Initialize the SPIFFS file space
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }

  updateVars();  // Update all the varables from the SPIFFS files
  setupWIFI();  //Get the Wi-Fi started in AP STA or both modes
  setupwebserver();  // start all the webserver functionalities

  APTimer = millis(); //Start AP timer
  GetNTPtime(); // Get the time from the NTP server
  LOOPtimer = millis(); //Start Loop timer
  LOOP_period = 1000; //Loop every second
  WIFItimer = millis(); //Start WIFI timer
  WIFI_period = 2;  //Loop every 2 minutes
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
void loop() {
  if(!kill_LOOP){
    if(millis() >= LOOPtimer + LOOP_period){ //better than using delay(1000);
      checkWIFI(); // keeps an eye on network connections
      LOOPtimer = millis(); //resets this timer
    }
  }
}
