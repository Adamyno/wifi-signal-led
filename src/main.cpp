#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <SPI.h>
#include <TFT_eSPI.h>

#include "display_utils.h"
#include "web_pages.h"

// --- Configuration ---
const char *AP_SSID = "NodeMCU_Config";
const char *CONFIG_FILE = "/config.json";

// --- Globals ---
ESP8266WebServer server(80);
TFT_eSPI tft = TFT_eSPI();
String ssid = "";
String password = "";

State currentState = STATE_AP_MODE;

// LED Control
#undef LED_BUILTIN
#define LED_BUILTIN 16

unsigned long lastBlinkTime = 0;
bool ledState = false;

// --- Function Prototypes ---
void loadConfig();
void saveConfig(const String &s, const String &p);
void deleteConfig();
void setupAP();
void setupServerRoutes();
void handleRoot();
void handleScan();
void handleSave();
void handleReset();
void handleRestart();
void handleStatus();
void updateLED();

// --- Setup ---
void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Initial background for status bar
  tft.fillRect(0, 0, 240, 24, TFT_DARKGREY);
  tft.drawFastHLine(0, 24, 240, TFT_WHITE);
  drawStatusBar();

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
  }

  loadConfig();
  setupServerRoutes();

  if (ssid != "") {
    currentState = STATE_CONNECTING;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to: ");
    Serial.println(ssid);
  } else {
    setupAP();
  }

  server.begin();
  Serial.println("HTTP server started");
}

// --- Loop ---
void loop() {
  updateLED();

  if (currentState == STATE_CONNECTING) {
    if (WiFi.status() == WL_CONNECTED) {
      currentState = STATE_CONNECTED;
      Serial.println("\nConnected!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());

      tft.fillRect(0, 25, 240, 295, TFT_BLACK); // Clear only below status bar
      drawStatusBar();

    } else {
      static unsigned long startAttemptInfo = millis();
      if (millis() - startAttemptInfo > 20000) {
        Serial.println("Connection timeout. Switching to AP.");
        setupAP();
      }
    }
  } else if (currentState == STATE_CONNECTED) {
    if (WiFi.status() != WL_CONNECTED) {
      currentState = STATE_CONNECTING;
    }
  }

  server.handleClient();

  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 500) {
    lastStatusUpdate = millis();
    drawStatusBar();
  }
}

// --- Implementation ---

void updateLED() {
  unsigned long currentMillis = millis();

  if (currentState == STATE_AP_MODE) {
    if (currentMillis - lastBlinkTime >= 1000) {
      lastBlinkTime = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
    }
  } else if (currentState == STATE_CONNECTING) {
    if (currentMillis - lastBlinkTime >= 200) {
      lastBlinkTime = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
    }
  } else if (currentState == STATE_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH); // Off (active low)
  }
}

void setupAP() {
  currentState = STATE_AP_MODE;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID);

  Serial.println("Starting AP Mode");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  tft.fillRect(0, 25, 240, 295, TFT_BLUE); // Clear only below status bar
  drawStatusBar();
}

void setupServerRoutes() {
  server.on("/", handleRoot);
  server.on("/scan", handleScan);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/reset", HTTP_POST, handleReset);
  server.on("/restart", HTTP_POST, handleRestart);
  server.on("/status", handleStatus);
}

void loadConfig() {
  if (LittleFS.exists(CONFIG_FILE)) {
    File file = LittleFS.open(CONFIG_FILE, "r");
    DynamicJsonDocument doc(512);
    deserializeJson(doc, file);
    ssid = doc["ssid"].as<String>();
    password = doc["password"].as<String>();
    file.close();
    Serial.println("Config loaded.");
  }
}

void saveConfig(const String &s, const String &p) {
  DynamicJsonDocument doc(512);
  doc["ssid"] = s;
  doc["password"] = p;

  File file = LittleFS.open(CONFIG_FILE, "w");
  serializeJson(doc, file);
  file.close();
}

void deleteConfig() { LittleFS.remove(CONFIG_FILE); }

void handleRoot() {
  if (currentState == STATE_CONNECTED) {
    String page = FPSTR(status_html);
    page.replace("%SSID%", WiFi.SSID());
    page.replace("%IP%", WiFi.localIP().toString());
    page.replace("%RSSI%", String(WiFi.RSSI()));
    page.replace("%MAC%", WiFi.macAddress());
    server.send(200, "text/html", page);
  } else {
    server.send(200, "text/html", index_html);
  }
}

void handleScan() {
  int n = WiFi.scanNetworks();
  DynamicJsonDocument doc(2048);
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < n; ++i) {
    JsonObject obj = arr.createNestedObject();
    obj["ssid"] = WiFi.SSID(i);
    obj["rssi"] = WiFi.RSSI(i);
  }

  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleSave() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    saveConfig(server.arg("ssid"), server.arg("password"));
    server.send(200, "text/plain", "Saved");
    delay(1000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Missing args");
  }
}

void handleReset() {
  deleteConfig();
  server.send(200, "text/plain", "Reset");
  delay(1000);
  ESP.restart();
}

void handleRestart() {
  server.send(200, "text/plain", "Restarting...");
  delay(1000);
  ESP.restart();
}

void handleStatus() {
  DynamicJsonDocument doc(64);
  doc["rssi"] = WiFi.RSSI();
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}
