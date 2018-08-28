// This is code for ESP8266 with DHT22 senzor on port D4 (IO2) using blynk cloud solution for data representation with OLED wemos D1 mini shield.
// Version 1.0
// by Miro Smola, smolam@gmail.com
/*

#include <DHT.h>
#include <Wire.h>
#include <SPI.h>
#include <ESP8266HTTPClient.h>


#define DHTPIN 2                                            // port where DTH22 is connected (D4 is IO2 on wemos D1 mini)
#define DHTTYPE DHT22                                       // I use DHT22, change to other DHT senzor if you use other one
DHT dht(DHTPIN, DHTTYPE);                                   // enabled DHT sensor




int outTemp;
int lx;
SimpleTimer timer;                                          // enable simple timer
SimpleTimer timer15min;                                          // enable simple timer



void setup()
{
  Serial.begin(9600);                                       // Enabled serial debugging output


  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  
  Blynk.begin(auth, ssid, pass);                            // Connects to wifi and Blynk services
  
 



  timer.setInterval(10000L, sendTemperature);               // every 10 seconds run function sendTemperature
  timer15min.setInterval(900000L, updateOutTemp);  

  updateOutTemp() ;
}

void updateOutTemp()                                      // function send temperature
{
  String json  = getJson();
  json.remove(0, json.indexOf("\"temp\"") + 7);
  json.remove(json.indexOf(","));
  outTemp = json.toInt();
  Serial.println(json);
}



void loop()
{
  Blynk.run();                                              // run blynk
  timer.run();                                              // run timer
  timer15min.run();                                              // run timer

}





*/