#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

const char* ssid = "IOT";
const char* password = "20tgmiot18";

#define DHTPIN 3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define LED_PIN 8
Adafruit_NeoPixel pixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
bool ledEnabled = true;

#define RELAY_PIN 2
bool relayState = false;

#define EEPROM_SIZE 3
uint8_t ledColor[3] = {0, 0, 255};

WebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); 

struct Messung {
  float temp;
  float hum;
  String time;
};
const int maxMessungen = 4;
Messung messungen[maxMessungen];
int messIndex = 0;
int messCount = 0;

void saveColorToEEPROM() {
  for (int i = 0; i < 3; i++) EEPROM.write(i, ledColor[i]);
  EEPROM.commit();
}
void loadColorFromEEPROM() {
  for (int i = 0; i < 3; i++) ledColor[i] = EEPROM.read(i);
}

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

void handleRoot() {
  String html = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Wetterstation</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: linear-gradient(to right, #8EC5FC, #E0C3FC);
      margin: 0;
      padding: 0;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
    }
    .card {
      background: #fff;
      padding: 30px;
      border-radius: 15px;
      box-shadow: 0 8px 20px rgba(0,0,0,0.2);
      max-width: 500px;
      width: 90%;
    }
    h2 {
      text-align: center;
    }
    #fanStatus {
      font-weight: bold;
      text-align: center;
      font-size: 1.2em;
      margin-bottom: 10px;
    }
    .indicator {
      display: inline-block;
      width: 12px;
      height: 12px;
      border-radius: 50%;
      margin-right: 10px;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 15px;
    }
    th, td {
      padding: 10px;
      border-bottom: 1px solid #ccc;
      text-align: center;
    }
    .colorpicker, .ledcontrol {
      margin-top: 20px;
      text-align: center;
    }
    input[type="color"] {
      width: 60px;
      height: 40px;
      border: none;
      cursor: pointer;
    }
    button {
      padding: 10px 16px;
      font-size: 16px;
      background: #007bff;
      color: white;
      border: none;
      border-radius: 10px;
      cursor: pointer;
    }
    button:hover {
      background: #0056b3;
    }
  </style>
</head>
<body>
  <div class="card">
    <h2>ESP32 Wetterstation</h2>
    <p id="fanStatus"><span id="fanIndicator" class="indicator" style="background: #aaa;"></span>Status wird geladen...</p>
    <table>
      <thead><tr><th>Zeit</th><th>Temp (°C)</th><th>Feuchte (%)</th></tr></thead>
      <tbody id="dataTable"><tr><td colspan="3">Lade...</td></tr></tbody>
    </table>
    <div class="colorpicker">
      <label>LED-Farbe:</label>
      <input type="color" id="ledColor">
    </div>
    <div class="ledcontrol">
      <button id="toggleLedBtn">LED ausschalten</button>
    </div>
  </div>
  <script>
    function updateData() {
      fetch("/data").then(res => res.json()).then(data => {
        let html = "";
        data.reverse().forEach(entry => {
          html += <tr><td>${entry.time}</td><td>${entry.temp.toFixed(1)}</td><td>${entry.hum.toFixed(1)}</td></tr>;
        });
        document.getElementById("dataTable").innerHTML = html;
      });
    }

    function updateStatus() {
      fetch("/status").then(res => res.text()).then(status => {
        const indicator = document.getElementById("fanIndicator");
        indicator.style.background = status.includes("EIN") ? "#f00" : "#0f0";
        document.getElementById("fanStatus").innerHTML = <span class="indicator" style="background:${indicator.style.background};"></span>${status};
      });
    }

    function updateLedButton() {
      fetch("/ledstatus").then(res => res.text()).then(state => {
        document.getElementById("toggleLedBtn").textContent = state === "on" ? "LED ausschalten" : "LED einschalten";
      });
    }

    document.getElementById("toggleLedBtn").addEventListener("click", () => {
      fetch("/toggleled").then(() => updateLedButton());
    });

    document.getElementById("ledColor").addEventListener("input", function () {
      const hex = this.value;
      const bigint = parseInt(hex.slice(1), 16);
      const r = (bigint >> 16) & 255;
      const g = (bigint >> 8) & 255;
      const b = bigint & 255;
      fetch(/setcolor?r=${r}&g=${g}&b=${b});
    });

    function setInitialColor() {
      fetch("/getcolor").then(res => res.json()).then(data => {
        const hex = "#" + data.r.toString(16).padStart(2, '0') +
                           data.g.toString(16).padStart(2, '0') +
                           data.b.toString(16).padStart(2, '0');
        document.getElementById("ledColor").value = hex;
      });
    }

    setInterval(() => {
      updateData();
      updateStatus();
    }, 4000);

    window.onload = () => {
      updateData();
      updateStatus();
      updateLedButton();
      setInitialColor();
    };
  </script>
</body>
</html>)=====";

  server.send(200, "text/html", html);
}

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

  WiFi.begin(ssid, password);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries++ < 20) {
    delay(500);
    setLEDStatus("noWiFi");
  }
  if (WiFi.status() != WL_CONNECTED) ESP.restart();

  timeClient.begin();
  timeClient.update();

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

      if (t > 24.0) {
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
