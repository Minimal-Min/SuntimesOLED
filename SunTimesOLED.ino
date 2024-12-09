#include <Wire.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <time.h>

// Define screen parameters
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 48

U8G2_SSD1306_64X48_ER_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 10, /* data=*/ 8, /* reset=*/ U8X8_PIN_NONE);

//------- Replace the following details! ------
// Replace these values with your own WiFi and OpenWeatherMap API details
const char ssid[] = "YOUR_WIFI_SSID";           // Your network SSID (name)
const char password[] = "YOUR_WIFI_PASSWORD";   // Your network password
const char apiKey[] = "YOUR_API_KEY";           // Your OpenWeatherMap API key
const char cityName[] = "YOUR_CITY,COUNTRY_CODE"; // City name for the weather data (format: "City,CountryCode", e.g., "Wuerzburg,de")
//------- ----------------------------- ------

unsigned long timeBetweenRequests = 60 * 1000;  // 60 seconds in milliseconds
WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  
  // Initialize the OLED display
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x8_tr); // Smaller font
  u8g2.drawStr(5, 15, "Start..."); // Adjusted position for smaller display
  u8g2.sendBuffer();
  delay(2000);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.print("\nConnecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Required for secure client connection
  client.setInsecure();

  // Synchronize time
  configTime(3600, 0, "pool.ntp.org"); // NTP server for time synchronization (3600 seconds = 1 hour offset for CET)
}

void displayTime(const char* label, time_t time, uint8_t y) {
  char timeStr[6];
  strftime(timeStr, sizeof(timeStr), "%H:%M", localtime(&time));
  u8g2.setCursor(0, y);
  u8g2.print(label);
  u8g2.setCursor(0, y + 10);
  u8g2.print(timeStr);
}

void loop() {
  HTTPClient http;
  String weatherURL = "https://api.openweathermap.org/data/2.5/weather?q=" + String(cityName) + "&appid=" + String(apiKey) + "&units=metric";
  Serial.println("Weather URL: " + weatherURL);
  http.begin(client, weatherURL);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("Current weather JSON payload:");
    Serial.println(payload);

    // Parse JSON data
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    // Get sunrise and sunset times
    time_t sunrise = doc["sys"]["sunrise"];
    time_t sunset = doc["sys"]["sunset"];

    Serial.print("Sunrise: ");
    Serial.println(doc["sys"]["sunrise"].as<String>());
    Serial.print("Sunset: ");
    Serial.println(doc["sys"]["sunset"].as<String>());

    // Update OLED display
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x8_tr); // Smaller font

    displayTime("Sunrise:", sunrise, 10);
    displayTime("Sunset:", sunset, 37);

    u8g2.sendBuffer();
  } else {
    Serial.println("Error during request");
  }
  
  http.end();
  delay(timeBetweenRequests);
}
