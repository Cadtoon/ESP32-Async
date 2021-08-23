# ESP32-Async
Wi-Fi Station and Access Point, OTA, Multi-page Web Server, SPIFFS saving and retrieval of ESP variables and NTP configuration. Variables are editable and can be changed on the fly.

Do you want to setup a Wi-Fi Station (STN)?
Do you want to setup a Wi-Fi Access Point (AP)?
How about running both at the same time? 
Do you want to setup a Web Server?
Do you want it to serve up multiple web pages?
Do you want those pages to be able to make changes to your App settings?
Do you want to have those changes saved to a file and loaded when the App is restarted?
Do you want to perform Over-the-Air (OTA) updates?
And how about an NTP connection?

That’s what I wanted and that’s what I did. It wasn’t easy. I could find this tutorial to do that, and that example to do this, but nothing that had it all together. You will find an active web clock on all the pages including the date. You can set an alarm time and switch it on or off, although there are no functions written for them. There is a link to the ElegantOTA page for uploading bin files. You can change the NTP Server and Time Zone to any location. You can enter your network credentials and give both the AP and STN names. The AP has a Time-to-Live setting. When booted it will stay active for a set number of minutes. 
