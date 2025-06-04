#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// WLAN Zugangsdaten
const char* ssid = "Galaxy S20 FE";
const char* password = "cemil123";

// DHT Sensor Setup
#define DHTPIN 3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// WS2812 LED Setup
#define LED_PIN 8
Adafruit_NeoPixel pixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
bool ledEnabled = true;

// Relais Setup
#define RELAY_PIN 2
bool relayState = false;

// EEPROM Setup (für LED-Farbe)
#define EEPROM_SIZE 3
uint8_t ledColor[3] = {0, 0, 255}; // Standardfarbe: Blau

// Webserver auf Port 80
WebServer server(80);

// NTP Client für Zeit
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); // UTC+1

// Messwerte speichern
struct Messung {
  float temp;
  float hum;
  String time;
};
const int maxMessungen = 4;
Messung messungen[maxMessungen];
int messIndex = 0;
int messCount = 0;

// Farben in EEPROM speichern und laden
void saveColorToEEPROM() {
  for (int i = 0; i < 3; i++) EEPROM.write(i, ledColor[i]);
  EEPROM.commit();
}

void loadColorFromEEPROM() {
  for (int i = 0; i < 3; i++) ledColor[i] = EEPROM.read(i);
}

// LED Farbe setzen oder ausmachen
void setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
  if (!ledEnabled) {
    pixel.clear();
    pixel.show();
    return;
  }
  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.show();
}

void setLEDStatus(const char* status) {
  if (!ledEnabled) {
    pixel.clear();
    pixel.show();
    return;
  }
  if (strcmp(status, "noWiFi") == 0) setLEDColor(255, 0, 0);
  else if (strcmp(status, "measuring") == 0) setLEDColor(0, 0, 255);
  else if (strcmp(status, "tempHigh") == 0) setLEDColor(255, 69, 0);
  else setLEDColor(ledColor[0], ledColor[1], ledColor[2]);
}

// Webserver: HTML-Seite ausliefern
void handleRoot() {
  String html = R"=====(<!DOCTYPE html>
  ... (HTML und JavaScript hier, unverändert) ...
  )=====";
  server.send(200, "text/html", html);
}

// Messwerte als JSON zurückgeben
void handleData() {
  String json = "[";
  for (int i = 0; i < messCount; i++) {
    int index = (messIndex - 1 - i + maxMessungen) % maxMessungen;
    Messung m = messungen[index];
    json += "{\"temp\":" + String(m.temp, 1) +
            ",\"hum\":" + String(m.hum, 1) +
            ",\"time\":\"" + m.time + "\"}";
    if (i < messCount - 1) json += ",";
  }
  json += "]";
  server.send(200, "application/json", json);
}

// LED-Farbe setzen über URL-Parameter
void handleSetColor() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    ledColor[0] = server.arg("r").toInt();
    ledColor[1] = server.arg("g").toInt();
    ledColor[2] = server.arg("b").toInt();
    setLEDColor(ledColor[0], ledColor[1], ledColor[2]);
    saveColorToEEPROM();
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Fehlende Parameter");
  }
}

// LED-Farbe als JSON zurückgeben
void handleGetColor() {
  String json = "{";
  json += "\"r\":" + String(ledColor[0]) + ",";
  json += "\"g\":" + String(ledColor[1]) + ",";
  json += "\"b\":" + String(ledColor[2]) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  loadColorFromEEPROM();
  pixel.begin();
  pixel.setBrightness(50);
  dht.begin();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // WLAN verbinden
  WiFi.begin(ssid, password);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries++ < 20) {
    delay(500);
    setLEDStatus("noWiFi");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WLAN-Verbindung fehlgeschlagen, Neustart...");
    ESP.restart();
  }

  Serial.print("Verbunden mit WLAN. IP-Adresse: ");
  Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.update();

  // Webserver Routen
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/setcolor", handleSetColor);
  server.on("/getcolor", handleGetColor);
  server.on("/status", []() {
    server.send(200, "text/plain", relayState ? "ZU HEIß: Lüfter EIN" : "OPTIMALE TEMPERATUR: Lüfter AUS");
  });
  server.on("/toggleled", []() {
    ledEnabled = !ledEnabled;
    if (!ledEnabled) pixel.clear(); else setLEDColor(ledColor[0], ledColor[1], ledColor[2]);
    pixel.show();
    server.send(200, "text/plain", ledEnabled ? "on" : "off");
  });
  server.on("/ledstatus", []() {
    server.send(200, "text/plain", ledEnabled ? "on" : "off");
  });

  server.begin();
}

void loop() {
  server.handleClient();
  static unsigned long lastMeasurement = 0;
  if (millis() - lastMeasurement > 4000) {
    lastMeasurement = millis();
    setLEDStatus("measuring");

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    timeClient.update();

    if (!isnan(t) && !isnan(h)) {
      messungen[messIndex] = {t, h, timeClient.getFormattedTime()};
      messIndex = (messIndex + 1) % maxMessungen;
      if (messCount < maxMessungen) messCount++;

      // Relais steuern bei hoher Temperatur
      if (t > 30.0) {
        digitalWrite(RELAY_PIN, HIGH);
        relayState = true;
        setLEDStatus("tempHigh");
      } else {
        digitalWrite(RELAY_PIN, LOW);
        relayState = false;
        setLEDColor(ledColor[0], ledColor[1], ledColor[2]);
      }
    }
  }
}
