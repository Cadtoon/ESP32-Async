    getESPDate();
    getESPTime();
	ESPtimer = setInterval(function() {getESPTime();}, 1000); // Call the update function every set interval e.g. 1000mS or 1-sec
    ESPdater = setInterval(function() {getESPDate();}, 60000); // Call the update function every set interval e.g. 1000mS or 1-sec

    function submitMessage() {
//      alert("Saved value to ESP SPIFFS");
      setTimeout(function(){ document.location.reload(false); }, 500);   
    }
   function updateALARMInput(val) {
          document.getElementById('ALARMInput').value=val; 
        }
   function updateAPTTLInput(val) {
          document.getElementById('APTTLInput').value=val; 
        }
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
