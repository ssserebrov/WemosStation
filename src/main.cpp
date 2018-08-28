#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_SSD1306.h>
#include <SimpleTimer.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#include <Utilities.h>

// OLED initialization
#define OLED_RESET D4
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
Adafruit_SSD1306 display(OLED_RESET);

// DHT initialization
#define DHTPIN 2          // port where DTH22 is connected (D4 is IO2 on wemos D1 mini)
#define DHTTYPE DHT22     // I use DHT22, change to other DHT senzor if you use other one
DHT dht(DHTPIN, DHTTYPE); // enabled DHT sensor

// Blynk initialization
char auth[] = "7bb7485f6eba41a0a36de66a90ed8ea1";
char ssid[] = "RAKAMAKAFO";
char pass[] = "22312231";
// PIN MAPPING
// V1   Temperature
// V3   Humidity

// Timers
SimpleTimer sendTemperatureTimer;
SimpleTimer updateOutTempTimer;

int outTemp;
String outConditions;
String customValue1;
String customValue2;
String customValue3;
StaticJsonBuffer<2048> jsonBuffer;

String getJson()
{
    char url[] = "http://api.openweathermap.org/data/2.5/weather?q=Tyumen&appid=e027651ebb115e41945437e09b3d0087&units=metric";
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            // Serial.println(payload);
            http.end();
            return payload;
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    return "";
}

void updateOutTemp()
{
    String json = getJson();
    JsonObject &root = jsonBuffer.parseObject(json);

    // Test if parsing succeeds.
    if (!root.success())
    {
        Serial.println("parseObject() failed");
        return;
    }
    
    outTemp = root["main"]["temp"];
    outConditions = root["weather"][0]["main"].asString();
    Serial.println(json);
}

void renderPage1(float intTemp, float outTemp, String customValue1)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(3);
    display.println(Utilities::floatToString(intTemp, 0));
    String outTempSign = outTemp < 0 ? "" : "+";

    display.println(outTempSign + outTemp);
    display.setTextSize(2);

    display.println(customValue1);
    display.display();
}

void sendTemperature()
{
    Serial.println("sendTemperature");

    float h = dht.readHumidity();    // reads humidity from senzor and save to h
    float t = dht.readTemperature(); // reads temperature from senzor and save to t
    if (isnan(h) or isnan(t))
    { // checks if readings from sensors were obtained
        Serial.println("sendTemperature error");
    }
    else
    {
        Blynk.virtualWrite(1, t); // send to Blynk virtual pin 1 temperature value
        Blynk.virtualWrite(3, h); // send to Blynk virtual pin 3 humidity value
        renderPage1(t, outTemp, outConditions);
    }
}

BLYNK_WRITE(V2)
{
    customValue1 = param.asString();
    Blynk.virtualWrite(5, 100);
}

void setup()
{
    Serial.begin(9600);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.display();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    Blynk.begin(auth, ssid, pass);

    sendTemperatureTimer.setInterval(10000L, sendTemperature);
    updateOutTempTimer.setInterval(900000L, updateOutTemp);

    sendTemperature();
    updateOutTemp();
}

void loop()
{
    Blynk.run();                
    sendTemperatureTimer.run(); 
    updateOutTempTimer.run();
}