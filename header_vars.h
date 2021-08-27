// Used for timing loops
int WIFI_period;
int LOOP_period;
int AP_period;

/* used for flow control, it is like an on/off switch.
If on, do this, if off, do that. It holds a value of
0 or 1. To make it easier to the mind, it recognizes a 
0 equal to false and a 1 equal to true. You can use either
or both. In an if() statement, you can write 
if(Alarm_onoff == false) or, if(Alarm_onoff == 0) 
which is exactly the same as writting if(!Alarm_onoff), or, 
if(Alarm_onoff == true) vs if(Alarm_onoff == 1) vs if(Alarm_onoff)
 */
bool Alarm_onoff;
bool APdisconnected = false;
bool WIFIconnected = false;
bool kill_LOOP = false;

/* used for timing loops, set to equal millis(). When the ESP is started
millis() starts counting from 0. There is 1000 millis in a second and
60000 millis in a minute. On the Network Setting HTML page there is a
Wi-Fi Timer slider. It is used to set how often the App checks to see
if the Wi-Fi connection is still connected. It starts by assigning it
WIFItimer = millis(). Millis could be any value from when the ESP was
powered up, say 2346765. By default the WIFI_period value is 2. To check
if the timer is up, if(millis() == WIFItimer + (WIFI_period * 60000)).
Because of the brackets, it is calculated first. 2 * 60000 = 120000.
Next it is added to the value of WIFItimer, 2346765 + 120000 = 2466765.
If the current value of millis(), remember it is still counting, is
equal to 2466765, the if statment is true, time is up. Now because millis
counts so fast and the ESP might be busy doing other things, it might not
notice in time when it is exactly equal. Better to use if(millis() >= WIFItimer... 
They say you should avoid using delay() in your code. A delay stops most
functions from continuing. A timer just checks and moves on.
*/
unsigned long WIFItimer;
unsigned long LOOPtimer;
unsigned long APTimer;

//vars used in the sketch
String alarmtime;
String wifi_network_hostname = "";
String wifi_network_ssid = "";
String wifi_network_password = "";
String soft_ap_ssid = "ESPAsync";
String soft_ap_password = "password";
String NTPserver = "pool.ntp.org";
String NTPzone = "MST7MDT,M3.2.0,M11.1.0"; //mountain time

/* 
A parameter (PARAM) is a special kind of variable in computer 
programming language that is used to pass information between 
functions or procedures. In this case between the html pages 
(javascript) and the sketch. Also uses its value "Alarmtime" as 
the file name to store the value it passed in the "inputMessage".

The webserver is listening for posts sent to it. Watching for a PARAM  
called "ALARMTIME", it will have a varable with the name "Alarmtime". 
The value of "Alarmtime" is held in the "inputMessage". That value is 
written into a file named "Alarmtime.txt". The value is also assigned 
to the String varable "alarmtime" that is used in the sketch code. 
Note the differences by uppercase letters.

The javascript is also asking for the value of %Alarmtime%. The Processor
function reads the file "Alarmtime.txt" and returns the value back to
the webpage. It is called a place holder. Anywhere that there is a place
holder %Alarmtime% will display the value passed to it.
*/

const char* PARAM_ALARMTIME = "Alarmtime";
const char* PARAM_ALARMONOFF = "AlarmOnOff";
const char* PARAM_NETSSID = "NetSSID";
const char* PARAM_NETPASS = "NetPASS";
const char* PARAM_NETHOST = "NetHOST";
const char* PARAM_APSSID = "APSSID";
const char* PARAM_APPASS = "APPASS";
const char* PARAM_APTTL = "APTTL";
const char* PARAM_NTPSERVER = "NTPSERVER";
const char* PARAM_NTPZONE = "NTPZONE";
const char* PARAM_WIFITIMER = "WIFItimer";
const char* PARAM_RESTART = "restartESP";
