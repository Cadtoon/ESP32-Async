int LOOPtimer;
int LOOP_period;
int AP_period;
String alarmtime;
bool Alarm_onoff;
bool APdisconnected = false;
unsigned long APTimer;

String wifi_network_hostname = "";
String wifi_network_ssid = "";
String wifi_network_password = "";
String soft_ap_ssid = "ESPAsync";
String soft_ap_password = "password";
    //MST7MDT,M3.2.0,M11.1.0
String NTPserver = "pool.ntp.org";
String NTPzone = "MST7MDT,M3.2.0,M11.1.0";

const char* PARAM_ALARMTIME = "inputAlarmtime";
const char* PARAM_ALARMONOFF = "inputAlarm";
const char* PARAM_NETSSID = "inputNetSSID";
const char* PARAM_NETPASS = "inputNetPASS";
const char* PARAM_NETHOST = "inputNetHOST";
const char* PARAM_APSSID = "inputAPSSID";
const char* PARAM_APPASS = "inputAPPASS";
const char* PARAM_APTTL = "inputAPTTL";
const char* PARAM_NTPSERVER = "inputNTPSERVER";
const char* PARAM_NTPZONE = "inputNTPZONE";
const char* PARAM_RESTART = "restartESP";
