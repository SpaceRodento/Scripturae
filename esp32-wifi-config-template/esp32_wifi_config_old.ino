/*
 * ESP32 WiFi Config Dashboard Template
 *
 * Creates a WiFi Access Point with a web-based configuration interface.
 * Perfect for projects that need wireless configuration without USB/serial.
 *
 * Features:
 * - WiFi AP with configurable SSID/password
 * - Web dashboard accessible via browser
 * - Read sensor values
 * - Control outputs (LEDs, relays, etc.)
 * - JSON API for easy integration
 * - Responsive design (works on mobile & desktop)
 *
 * Hardware: ESP32 (any variant)
 *
 * Libraries needed:
 * - WiFi (built-in)
 * - WebServer (built-in)
 * - ArduinoJson (install from Library Manager)
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ==================== CONFIGURATION ====================
// Customize these for your project

// WiFi Access Point settings
const char* AP_SSID = "ESP32-Config";       // AP name
const char* AP_PASSWORD = "esp32pass";      // AP password (min 8 chars, or empty for open)
const IPAddress AP_IP(192, 168, 4, 1);      // AP IP address
const IPAddress AP_GATEWAY(192, 168, 4, 1);
const IPAddress AP_SUBNET(255, 255, 255, 0);

// Web server
WebServer server(80);

// Example: GPIO pins for your project
const int LED_PIN = 2;           // Built-in LED
const int RELAY_PIN = 4;         // Example relay/output
const int SENSOR_PIN = 34;       // Example analog sensor

// Example: Project state variables
bool ledState = false;
bool relayState = false;
int sensorValue = 0;
String projectMode = "auto";     // Example: auto/manual mode

// ==================== WEB PAGE HTML ====================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Config Dashboard</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
            color: #333;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background: white;
            border-radius: 16px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }
        header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 30px;
            text-align: center;
        }
        h1 { font-size: 28px; margin-bottom: 5px; }
        .subtitle { opacity: 0.9; font-size: 14px; }
        .content { padding: 30px; }
        .section {
            background: #f8f9fa;
            border-radius: 12px;
            padding: 20px;
            margin-bottom: 20px;
        }
        .section h2 {
            font-size: 18px;
            margin-bottom: 15px;
            color: #667eea;
            display: flex;
            align-items: center;
        }
        .section h2::before {
            content: "●";
            margin-right: 10px;
            font-size: 12px;
        }
        .value-display {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-bottom: 15px;
        }
        .value-box {
            background: white;
            padding: 15px;
            border-radius: 8px;
            border-left: 4px solid #667eea;
        }
        .value-label {
            font-size: 12px;
            color: #666;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            margin-bottom: 5px;
        }
        .value-number {
            font-size: 32px;
            font-weight: bold;
            color: #333;
        }
        .value-unit {
            font-size: 16px;
            color: #999;
            margin-left: 5px;
        }
        .controls {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
        }
        button {
            flex: 1;
            min-width: 150px;
            padding: 12px 24px;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .btn-primary {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }
        .btn-primary:hover { transform: translateY(-2px); box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4); }
        .btn-success { background: #28a745; color: white; }
        .btn-success:hover { background: #218838; transform: translateY(-2px); }
        .btn-danger { background: #dc3545; color: white; }
        .btn-danger:hover { background: #c82333; transform: translateY(-2px); }
        .btn-secondary { background: #6c757d; color: white; }
        .btn-secondary:hover { background: #5a6268; transform: translateY(-2px); }
        .status {
            display: inline-block;
            padding: 4px 12px;
            border-radius: 20px;
            font-size: 12px;
            font-weight: 600;
            text-transform: uppercase;
        }
        .status-on { background: #d4edda; color: #155724; }
        .status-off { background: #f8d7da; color: #721c24; }
        select, input[type="text"], input[type="number"] {
            width: 100%;
            padding: 10px;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            font-size: 14px;
            margin-bottom: 10px;
        }
        select:focus, input:focus {
            outline: none;
            border-color: #667eea;
        }
        footer {
            background: #f8f9fa;
            padding: 20px;
            text-align: center;
            font-size: 12px;
            color: #666;
            border-top: 1px solid #e0e0e0;
        }
        .wifi-info {
            background: #e7f3ff;
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 20px;
            border-left: 4px solid #2196F3;
        }
        .wifi-info strong { color: #1976D2; }
        @media (max-width: 600px) {
            .value-display { grid-template-columns: 1fr; }
            button { min-width: 100%; }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>🔧 ESP32 Control Dashboard</h1>
            <p class="subtitle">Wireless Configuration & Monitoring</p>
        </header>

        <div class="content">
            <div class="wifi-info">
                <strong>📡 Connected to:</strong> ESP32-Config | <strong>IP:</strong> 192.168.4.1
            </div>

            <!-- Sensor Readings -->
            <div class="section">
                <h2>📊 Sensor Readings</h2>
                <div class="value-display">
                    <div class="value-box">
                        <div class="value-label">Analog Sensor</div>
                        <div>
                            <span class="value-number" id="sensorValue">0</span>
                            <span class="value-unit">units</span>
                        </div>
                    </div>
                    <div class="value-box">
                        <div class="value-label">Uptime</div>
                        <div>
                            <span class="value-number" id="uptime">0</span>
                            <span class="value-unit">sec</span>
                        </div>
                    </div>
                </div>
            </div>

            <!-- Output Controls -->
            <div class="section">
                <h2>💡 Output Control</h2>
                <p style="margin-bottom: 15px;">
                    LED Status: <span class="status" id="ledStatus">OFF</span>
                </p>
                <div class="controls">
                    <button class="btn-success" onclick="toggleLED(true)">Turn LED ON</button>
                    <button class="btn-danger" onclick="toggleLED(false)">Turn LED OFF</button>
                </div>
            </div>

            <!-- Relay Control -->
            <div class="section">
                <h2>⚡ Relay Control</h2>
                <p style="margin-bottom: 15px;">
                    Relay Status: <span class="status" id="relayStatus">OFF</span>
                </p>
                <div class="controls">
                    <button class="btn-success" onclick="toggleRelay(true)">Turn Relay ON</button>
                    <button class="btn-danger" onclick="toggleRelay(false)">Turn Relay OFF</button>
                </div>
            </div>

            <!-- Mode Selection -->
            <div class="section">
                <h2>⚙️ Operation Mode</h2>
                <select id="modeSelect" onchange="changeMode()">
                    <option value="auto">Automatic Mode</option>
                    <option value="manual">Manual Mode</option>
                    <option value="sleep">Sleep Mode</option>
                </select>
                <p style="margin-top: 10px; font-size: 14px; color: #666;">
                    Current mode: <strong id="currentMode">auto</strong>
                </p>
            </div>

            <!-- System Actions -->
            <div class="section">
                <h2>🔄 System Actions</h2>
                <div class="controls">
                    <button class="btn-primary" onclick="refreshData()">Refresh Data</button>
                    <button class="btn-secondary" onclick="resetSystem()">Reset System</button>
                </div>
            </div>
        </div>

        <footer>
            ESP32 WiFi Config Template | Auto-refresh: <span id="autoRefresh">ON</span>
        </footer>
    </div>

    <script>
        // Auto-refresh data every 2 seconds
        setInterval(refreshData, 2000);

        // Initial data load
        refreshData();

        // Refresh all sensor data
        function refreshData() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('sensorValue').textContent = data.sensor;
                    document.getElementById('uptime').textContent = data.uptime;

                    updateStatus('ledStatus', data.led);
                    updateStatus('relayStatus', data.relay);

                    document.getElementById('currentMode').textContent = data.mode;
                    document.getElementById('modeSelect').value = data.mode;
                })
                .catch(error => console.error('Error:', error));
        }

        // Update status badge
        function updateStatus(elementId, state) {
            const element = document.getElementById(elementId);
            if (state) {
                element.textContent = 'ON';
                element.className = 'status status-on';
            } else {
                element.textContent = 'OFF';
                element.className = 'status status-off';
            }
        }

        // Toggle LED
        function toggleLED(state) {
            fetch('/api/led?state=' + (state ? '1' : '0'))
                .then(response => response.json())
                .then(data => {
                    if (data.success) refreshData();
                });
        }

        // Toggle Relay
        function toggleRelay(state) {
            fetch('/api/relay?state=' + (state ? '1' : '0'))
                .then(response => response.json())
                .then(data => {
                    if (data.success) refreshData();
                });
        }

        // Change operation mode
        function changeMode() {
            const mode = document.getElementById('modeSelect').value;
            fetch('/api/mode?mode=' + mode)
                .then(response => response.json())
                .then(data => {
                    if (data.success) refreshData();
                });
        }

        // Reset system
        function resetSystem() {
            if (confirm('Are you sure you want to reset the system?')) {
                fetch('/api/reset')
                    .then(response => response.json())
                    .then(data => {
                        if (data.success) {
                            alert('System will reset in 3 seconds...');
                            setTimeout(() => location.reload(), 3000);
                        }
                    });
            }
        }
    </script>
</body>
</html>
)rawliteral";

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n=== ESP32 WiFi Config Dashboard ===");

  // Configure GPIO pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);

  // Start WiFi Access Point
  Serial.println("Starting WiFi Access Point...");
  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.println("Connect to the WiFi and open http://192.168.4.1");

  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/led", HTTP_GET, handleLED);
  server.on("/api/relay", HTTP_GET, handleRelay);
  server.on("/api/mode", HTTP_GET, handleMode);
  server.on("/api/reset", HTTP_GET, handleReset);

  // Start server
  server.begin();
  Serial.println("Web server started!");
  Serial.println("===================================\n");
}

// ==================== LOOP ====================
void loop() {
  server.handleClient();

  // Read sensor value periodically
  static unsigned long lastRead = 0;
  if (millis() - lastRead > 500) {
    sensorValue = analogRead(SENSOR_PIN);
    lastRead = millis();
  }

  // Your custom logic here
  // Example: Auto mode could control relay based on sensor
  if (projectMode == "auto") {
    // Auto logic example
    if (sensorValue > 2000) {
      digitalWrite(RELAY_PIN, HIGH);
      relayState = true;
    } else {
      digitalWrite(RELAY_PIN, LOW);
      relayState = false;
    }
  }
}

// ==================== WEB HANDLERS ====================

// Serve main page
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

// Get system status (JSON)
void handleStatus() {
  StaticJsonDocument<256> doc;

  doc["sensor"] = sensorValue;
  doc["uptime"] = millis() / 1000;
  doc["led"] = ledState;
  doc["relay"] = relayState;
  doc["mode"] = projectMode;

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// Control LED
void handleLED() {
  if (server.hasArg("state")) {
    bool state = server.arg("state") == "1";
    ledState = state;
    digitalWrite(LED_PIN, state ? HIGH : LOW);

    StaticJsonDocument<64> doc;
    doc["success"] = true;
    doc["led"] = ledState;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  } else {
    server.send(400, "application/json", "{\"error\":\"Missing state parameter\"}");
  }
}

// Control Relay
void handleRelay() {
  if (server.hasArg("state")) {
    bool state = server.arg("state") == "1";

    // Only allow manual control in manual mode
    if (projectMode == "manual") {
      relayState = state;
      digitalWrite(RELAY_PIN, state ? HIGH : LOW);
    }

    StaticJsonDocument<64> doc;
    doc["success"] = true;
    doc["relay"] = relayState;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  } else {
    server.send(400, "application/json", "{\"error\":\"Missing state parameter\"}");
  }
}

// Change operation mode
void handleMode() {
  if (server.hasArg("mode")) {
    projectMode = server.arg("mode");

    StaticJsonDocument<64> doc;
    doc["success"] = true;
    doc["mode"] = projectMode;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  } else {
    server.send(400, "application/json", "{\"error\":\"Missing mode parameter\"}");
  }
}

// Reset system
void handleReset() {
  StaticJsonDocument<64> doc;
  doc["success"] = true;
  doc["message"] = "Resetting in 3 seconds...";

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);

  delay(3000);
  ESP.restart();
}
