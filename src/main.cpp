#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Adafruit_SSD1306.h>
#include <SimpleTimer.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_BME280.h>

#include <Utilities.h>

struct Sensor
{
    float temperature;
    int humidity;
    int pressure;
    String conditions;
};

// OLED initialization
#define OLED_RESET D4
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
Adafruit_SSD1306 display(OLED_RESET);

// OLED 2: U8g2

// DHT initialization
#define DHTPIN 2          // port where DTH22 is connected (D4 is IO2 on wemos D1 mini)
#define DHTTYPE DHT22     // I use DHT22, change to other DHT senzor if you use other one
DHT dht(DHTPIN, DHTTYPE); // enabled DHT sensor

// BME initialization
Adafruit_BME280 bme; // I2C

// Blynk initialization
char auth[] = "7bb7485f6eba41a0a36de66a90ed8ea1";
char ssid[] = "RAKAMAKAFO";
char pass[] = "22312231";
// PIN MAPPING
// V1   DHT Temperature
// V3   DHT Humidity

// Timers
SimpleTimer sendTemperatureTimer;
SimpleTimer updateOutTempTimer;

Sensor openWeatherSensor;
Sensor dhtSensor;
Sensor bmeSensor;

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

void updateOpenWeatherSensor()
{
    String json = getJson();
    JsonObject &root = jsonBuffer.parseObject(json);

    // Test if parsing succeeds.
    if (!root.success())
    {
        Serial.println("parseObject() failed");
        return;
    }

    openWeatherSensor.temperature = root["main"]["temp"];
    openWeatherSensor.humidity = root["main"]["humidity"];
    openWeatherSensor.conditions = root["weather"][0]["main"].asString();
    Serial.println(json);
}

void renderStartPage() 
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.println("Starting");
    display.display();
}

void renderPage1(float temp1, int hum1, float temp2, int hum2, String customValue1, String customValue2)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(3);

    String temp1Sign = temp1 < 0 ? "" : "+";
    String spacer1 = " ";
    spacer1 += abs(temp1) < 10 ? " " : "";
    spacer1 += abs(hum1) < 10 ? " " : "";
    display.println(temp1Sign + Utilities::floatToString(temp1, 0) + spacer1 + hum1 + "%");

    String temp2Sign = temp2 < 0 ? "" : "+";
    String spacer2 = " ";
    spacer2 += abs(temp2) < 10 ? " " : "";
    spacer2 += abs(hum2) < 10 ? " " : "";
    display.println(temp2Sign + Utilities::floatToString(temp2, 0) + spacer2 + hum2 + "%");

    display.setTextSize(2);
    display.println(customValue1);
    //display.println(customValue1 + "|" + customValue2);
    display.display();
}

void renderPage2(float temp1, int hum1, float temp2, int hum2, float temp3, int hum3,
    String customValue1, String customValue2)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);

    String temp1Sign = temp1 < 0 ? "" : "+";
    String spacer1 = " ";
    spacer1 += abs(temp1) < 10 ? " " : "";
    spacer1 += abs(hum1) < 10 ? " " : "";
    display.println(temp1Sign + Utilities::floatToString(temp1, 0) + spacer1 + hum1 + "%");

    String temp2Sign = temp2 < 0 ? "" : "+";
    String spacer2 = " ";
    spacer2 += abs(temp2) < 10 ? " " : "";
    spacer2 += abs(hum2) < 10 ? " " : "";
    display.println(temp2Sign + Utilities::floatToString(temp2, 0) + spacer2 + hum2 + "%");

    String temp3Sign = temp3 < 0 ? "" : "+";
    String spacer3 = " ";
    spacer3 += abs(temp3) < 10 ? " " : "";
    spacer3 += abs(hum3) < 10 ? " " : "";
    display.println(temp3Sign + Utilities::floatToString(temp3, 0) + spacer3 + hum3 + "%");

    display.println(customValue1);
    //display.println(customValue1 + "|" + customValue2);
    display.display();
}

void updateDhtSensor()
{
    Serial.println("sendTemperature");

    dhtSensor.humidity = dht.readHumidity();
    dhtSensor.temperature = dht.readTemperature();
    if (isnan(dhtSensor.humidity) || isnan(dhtSensor.temperature))
    {
        Serial.println("sendTemperature error");
    }
    else
    {
        Blynk.virtualWrite(1, dhtSensor.temperature); // send to Blynk virtual pin 1 temperature value
        Blynk.virtualWrite(3, dhtSensor.humidity);    // send to Blynk virtual pin 3 humidity value
    }
}

void updateBmeSensor()
{
    bmeSensor.temperature = bme.readTemperature();
    bmeSensor.humidity = bme.readHumidity();
    bmeSensor.pressure = bme.readPressure() / 100.0F * 0.750062;
}

BLYNK_WRITE(V11)
{
    customValue2 = param.asString();
    Serial.println("BLYNK_WRITE(V11): " + customValue2);

    // Blynk.virtualWrite(5, 100);
}

void updateHardwareSensors()
{
    updateDhtSensor();
    updateBmeSensor();
    renderPage2(dhtSensor.temperature, dhtSensor.humidity,
                bmeSensor.temperature, bmeSensor.humidity,
                openWeatherSensor.temperature, openWeatherSensor.humidity,
                openWeatherSensor.conditions, customValue2);
}

void updateRemoteSensors()
{
    updateOpenWeatherSensor();
}

void setup()
{
    Serial.begin(9600);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.display();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    renderStartPage();

    Blynk.begin(auth, ssid, pass);

    if (!bme.begin()) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
    }

    sendTemperatureTimer.setInterval(10000L, updateHardwareSensors);
    updateOutTempTimer.setInterval(900000L, updateRemoteSensors);

    updateOpenWeatherSensor();
    updateDhtSensor();
}

void loop()
{
    Blynk.run();
    sendTemperatureTimer.run();
    updateOutTempTimer.run();
}