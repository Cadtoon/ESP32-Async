/* Javascript is designed to function along side html. A language of its own. The functions here have been provided by others
I have modified them slightly for the varables used in the sketch. You can do a lot more with it then used here,
like send and get data from a database. */

// these functions get data from the ESP when the webpage is first loaded.
	getESPDate();
    getESPTime();
	getWIFIIP();
	
/* the setInterval is a timer function. both these timers are set to one minute, counted just like millis in the sketch
if I didn't call these functions above, it would take a minute before the page would request the data */
	ESPtimer = setInterval(function() {getESPTime();}, 60000); // Call the update function every set interval e.g. 1000mS or 1-sec
    ESPdater = setInterval(function() {getESPDate();}, 60000); // Call the update function every set interval e.g. 1000mS or 1-sec

/* this function is activated every time an uppdate button is clicked.
if you remove the comment slashes before the alert function, a popup will open. Give it a try!*/
	function submitMessage() {
//      alert("Saved value to ESP SPIFFS");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
/* the following three functions are used with the sliders or switches. when you change or more one of them, 
the value in the grey area is updated before sending it the the ESP */
	function updateAlarmOnOff(val) {
          document.getElementById('AlarmOnOff').value=val; 
        }
   	function updateAPTTL(val) {
          document.getElementById('APTTL').value=val; 
        }
   	function updateWIFItimer(val) {
          document.getElementById('WIFItimer').value=val; 
        }
/* the following three functions are the function we call at the top of this page.
Two of them are also call by the Interval timers. This is how javascript calls the ESP to send it data.
If you sk me what each line of code is doing? Ya, I don't have the answer. */
   	function getESPTime() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("ESPTIME").innerHTML = this.responseText;}
          };  
        xhttp.open("GET", "/ESPTIME", true);
        xhttp.send(); 
      }
	function getESPDate() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("ESPDATE").innerHTML = this.responseText;}
          };  
        xhttp.open("GET", "/ESPDATE", true);
        xhttp.send(); 
      }
   	function getWIFIIP() {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("WIFIIP").innerHTML = this.responseText;}
          };  
        xhttp.open("GET", "/WIFIIP", true);
        xhttp.send(); 
      }
/* I just want to add that javascript does not have to be kept in a file of its own. Javascript can be embedded 
in the html page. Also, I have all the javascript in this one file included in all the pages. Each html page can have 
its own unique script file, or in a number of different script files. What you do need to know is that if you
want to store a javascript file on the ESP, you have to include it in the webserver setup to give the browser permission to
download the file. */
