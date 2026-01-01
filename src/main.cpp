#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>

// --- Configuration ---
const char *AP_SSID = "NodeMCU_Config";
const char *CONFIG_FILE = "/config.json";

// --- Globals ---
ESP8266WebServer server(80);
String ssid = "";
String password = "";

// State Machine
enum State { STATE_AP_MODE, STATE_CONNECTING, STATE_CONNECTED };
State currentState = STATE_AP_MODE;

// LED Control
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

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
void handleStatus(); // New handler
void updateLED();

// --- HTML Content ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>WiFi Config</title>
  <style>
    body { font-family: sans-serif; background: #222; color: #fff; padding: 20px; text-align: center; }
    h1 { margin-bottom: 20px; }
    button { background: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; margin: 5px; }
    button:hover { background: #0056b3; }
    button.reset { background: #dc3545; }
    button.reset:hover { background: #a71d2a; }
    input { padding: 10px; border-radius: 5px; border: none; width: 80%; max-width: 300px; margin: 10px 0; }
    #networks ul { list-style: none; padding: 0; }
    #networks li { background: #333; margin: 5px auto; padding: 10px; width: 80%; max-width: 300px; cursor: pointer; border-radius: 5px; }
    #networks li:hover { background: #444; }
  </style>
</head>
<body>
  <h1>WiFi Configuration</h1>
  <button onclick="scanNetworks()">Scan Networks</button>
  <div id="networks"></div>
  <br>
  <input type="text" id="ssid" placeholder="SSID"><br>
  <input type="password" id="password" placeholder="Password"><br>
  <button onclick="saveConfig()">Save & Connect</button>
  <br><br>
  <button class="reset" onclick="resetConfig()">Reset Settings</button>

  <script>
    function scanNetworks() {
      document.getElementById('networks').innerHTML = "Scanning...";
      fetch('/scan').then(res => res.json()).then(data => {
        let html = "<ul>";
        data.forEach(net => {
          html += `<li onclick="document.getElementById('ssid').value = '${net.ssid}'">${net.ssid} (${net.rssi} dBm)</li>`;
        });
        html += "</ul>";
        document.getElementById('networks').innerHTML = html;
      });
    }

    function saveConfig() {
      const ssid = document.getElementById('ssid').value;
      const pass = document.getElementById('password').value;
      if(!ssid) return alert("SSID required!");
      
      const formData = new FormData();
      formData.append("ssid", ssid);
      formData.append("password", pass);

      fetch('/save', { method: 'POST', body: formData }).then(res => {
        alert("Saved! Rebooting...");
      });
    }

    function resetConfig() {
      if(confirm("Forget WiFi settings?")) {
        fetch('/reset', { method: 'POST' }).then(res => alert("Reset! Rebooting..."));
      }
    }
  </script>
</body>
</html>
)rawliteral";

const char status_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Device Status</title>
  <style>
    body { font-family: sans-serif; background: #222; color: #fff; padding: 20px; text-align: center; }
    h1 { margin-bottom: 20px; }
    .card { background: #333; padding: 20px; border-radius: 10px; display: inline-block; text-align: left; width: 90%; max-width: 400px; }
    .stat { margin: 15px 0; border-bottom: 1px solid #444; padding-bottom: 5px; }
    .stat:last-child { border: none; }
    .label { color: #aaa; font-size: 0.9em; margin-bottom: 2px; }
    .value { font-size: 1.2em; font-weight: bold; color: #00dbde; }
    button { background: #007bff; color: white; padding: 12px 20px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; margin: 10px 5px; width: 45%; }
    button:hover { background: #0056b3; }
    button.reset { background: #dc3545; }
    button.reset:hover { background: #a71d2a; }
    button.restart { background: #28a745; }
    button.restart:hover { background: #218838; }

    /* WiFi Icon CSS */
    .wifi-icon { position: relative; display: inline-block; width: 30px; height: 30px; margin-left: 10px; vertical-align: middle; }
    .bar { position: absolute; bottom: 0; width: 6px; background: #555; border-radius: 2px; transition: background 0.3s; }
    .bar-1 { left: 0; height: 6px; }  /* Dot */
    .bar-2 { left: 8px; height: 14px; }
    .bar-3 { left: 16px; height: 22px; }
    .bar-4 { left: 24px; height: 30px; }
    
    .signal-1 .bar-1 { background: #00dbde; }
    .signal-2 .bar-1, .signal-2 .bar-2 { background: #00dbde; }
    .signal-3 .bar-1, .signal-3 .bar-2, .signal-3 .bar-3 { background: #00dbde; }
    .signal-4 .bar-1, .signal-4 .bar-2, .signal-4 .bar-3, .signal-4 .bar-4 { background: #00dbde; }

    .flex-row { display: flex; align-items: center; }
  </style>
</head>
<body>
  <h1>Status Dashboard</h1>
  <div class="card">
    <div class="stat"><div class="label">Connected Network</div><div class="value">%SSID%</div></div>
    <div class="stat"><div class="label">IP Address</div><div class="value">%IP%</div></div>
    <div class="stat">
      <div class="label">Signal Strength</div>
      <div class="flex-row">
        <span class="value" id="rssi-val">%RSSI%</span> <span class="value"> dBm</span>
        <div id="wifi-icon" class="wifi-icon signal-0">
          <div class="bar bar-1"></div>
          <div class="bar bar-2"></div>
          <div class="bar bar-3"></div>
          <div class="bar bar-4"></div>
        </div>
      </div>
    </div>
    <div class="stat"><div class="label">Device MAC</div><div class="value">%MAC%</div></div>
  </div>
  <br><br>
  <button class="restart" onclick="restartDev()">Restart</button>
  <button class="reset" onclick="resetConfig()">Reset</button>

  <script>
    function updateSignal() {
      fetch('/status').then(res => res.json()).then(data => {
        const rssi = data.rssi;
        document.getElementById('rssi-val').innerText = rssi;
        
        const icon = document.getElementById('wifi-icon');
        icon.className = 'wifi-icon'; // Reset
        
        if (rssi >= -60) icon.classList.add('signal-4');
        else if (rssi >= -70) icon.classList.add('signal-3');
        else if (rssi >= -80) icon.classList.add('signal-2');
        else icon.classList.add('signal-1');
      });
    }

    // Update every 2 seconds
    setInterval(updateSignal, 2000);
    // Initial call
    updateSignal();

    function restartDev() {
      if(confirm("Restart device?")) {
        fetch('/restart', { method: 'POST' }).then(res => alert("Rebooting..."));
      }
    }
    function resetConfig() {
      if(confirm("Forget ALL WiFi settings and reset?")) {
        fetch('/reset', { method: 'POST' }).then(res => alert("Settings cleared! Rebooting into AP Mode..."));
      }
    }
  </script>
</body>
</html>
)rawliteral";

// --- Setup ---
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

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
    long rssi = WiFi.RSSI();
    int pwmVal = map(rssi, -100, -50, 900, 0);
    pwmVal = constrain(pwmVal, 0, 1023);
    analogWrite(LED_BUILTIN, pwmVal);
  }
}

void setupAP() {
  currentState = STATE_AP_MODE;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID);

  Serial.println("Starting AP Mode");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
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
