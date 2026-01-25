/*
 * WebDashboard.cpp
 *
 * Implementation of modular web dashboard for ESP32.
 */

#include "WebDashboard.h"

// ==================== HTML PAGE ====================
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Dashboard</title>
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
        .btn-warning { background: #ffc107; color: #333; }
        .btn-warning:hover { background: #e0a800; transform: translateY(-2px); }
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
        select, input[type="text"] {
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
            <h1 id="projectName">🔧 ESP32 Dashboard</h1>
            <p class="subtitle" id="version">v1.0</p>
        </header>

        <div class="content">
            <div class="wifi-info">
                <strong>📡 Connected</strong> | <strong>IP:</strong> <span id="ipAddress">192.168.4.1</span>
            </div>

            <!-- Sensor Values -->
            <div class="section" id="sensorSection">
                <h2>📊 Sensor Data</h2>
                <div class="value-display" id="sensorValues">
                    <!-- Populated dynamically -->
                </div>
            </div>

            <!-- Output Controls -->
            <div class="section" id="output1Section" style="display:none;">
                <h2>💡 <span id="output1Label">Output 1</span></h2>
                <p style="margin-bottom: 15px;">
                    Status: <span class="status" id="output1Status">OFF</span>
                </p>
                <div class="controls">
                    <button class="btn-success" onclick="setOutput1(true)">Turn ON</button>
                    <button class="btn-danger" onclick="setOutput1(false)">Turn OFF</button>
                </div>
            </div>

            <div class="section" id="output2Section" style="display:none;">
                <h2>⚡ <span id="output2Label">Output 2</span></h2>
                <p style="margin-bottom: 15px;">
                    Status: <span class="status" id="output2Status">OFF</span>
                </p>
                <div class="controls">
                    <button class="btn-success" onclick="setOutput2(true)">Turn ON</button>
                    <button class="btn-danger" onclick="setOutput2(false)">Turn OFF</button>
                </div>
            </div>

            <!-- Mode Selection -->
            <div class="section">
                <h2>⚙️ Operation Mode</h2>
                <select id="modeSelect" onchange="changeMode()">
                    <option value="auto">Automatic</option>
                    <option value="manual">Manual</option>
                    <option value="sleep">Sleep</option>
                </select>
                <p style="margin-top: 10px; font-size: 14px; color: #666;">
                    Current: <strong id="currentMode">auto</strong>
                </p>
            </div>

            <!-- System Actions -->
            <div class="section">
                <h2>🔄 System</h2>
                <div class="controls">
                    <button class="btn-primary" onclick="refreshData()">Refresh Data</button>
                    <button class="btn-warning" onclick="blinkLED()">💡 Blink LED</button>
                    <button class="btn-secondary" onclick="resetSystem()">Reset ESP32</button>
                </div>
            </div>
        </div>

        <footer>
            <span id="footerText">ESP32 Dashboard</span> | Uptime: <span id="uptime">0</span>s
        </footer>
    </div>

    <script>
        // Auto-refresh every 2 seconds
        setInterval(refreshData, 2000);
        refreshData();

        function refreshData() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    // Update sensor values
                    updateSensorDisplay(data.sensors);

                    // Update outputs
                    if (data.outputs) {
                        updateOutput('output1', data.outputs.output1, data.outputs.label1, data.outputs.show1);
                        updateOutput('output2', data.outputs.output2, data.outputs.label2, data.outputs.show2);
                    }

                    // Update system info
                    if (data.system) {
                        document.getElementById('projectName').textContent = data.system.name || 'ESP32 Dashboard';
                        document.getElementById('version').textContent = data.system.version || 'v1.0';
                        document.getElementById('uptime').textContent = data.system.uptime || 0;
                        document.getElementById('currentMode').textContent = data.system.mode || 'auto';
                        document.getElementById('modeSelect').value = data.system.mode || 'auto';
                    }
                })
                .catch(error => console.error('Error:', error));
        }

        function updateSensorDisplay(sensors) {
            if (!sensors) return;

            let html = '';
            if (sensors.show1) {
                html += `
                    <div class="value-box">
                        <div class="value-label">${sensors.label1}</div>
                        <div>
                            <span class="value-number">${sensors.value1.toFixed(1)}</span>
                            <span class="value-unit">${sensors.unit1}</span>
                        </div>
                    </div>
                `;
            }
            if (sensors.show2) {
                html += `
                    <div class="value-box">
                        <div class="value-label">${sensors.label2}</div>
                        <div>
                            <span class="value-number">${sensors.value2.toFixed(1)}</span>
                            <span class="value-unit">${sensors.unit2}</span>
                        </div>
                    </div>
                `;
            }
            if (sensors.show3) {
                html += `
                    <div class="value-box">
                        <div class="value-label">${sensors.label3}</div>
                        <div>
                            <span class="value-number">${sensors.value3.toFixed(1)}</span>
                            <span class="value-unit">${sensors.unit3}</span>
                        </div>
                    </div>
                `;
            }

            document.getElementById('sensorValues').innerHTML = html;
        }

        function updateOutput(id, state, label, show) {
            const section = document.getElementById(id + 'Section');
            const status = document.getElementById(id + 'Status');
            const labelEl = document.getElementById(id + 'Label');

            if (show) {
                section.style.display = 'block';
                labelEl.textContent = label || id;
                status.textContent = state ? 'ON' : 'OFF';
                status.className = state ? 'status status-on' : 'status status-off';
            } else {
                section.style.display = 'none';
            }
        }

        function setOutput1(state) {
            fetch('/api/output1?state=' + (state ? '1' : '0'))
                .then(response => response.json())
                .then(data => { if (data.success) refreshData(); });
        }

        function setOutput2(state) {
            fetch('/api/output2?state=' + (state ? '1' : '0'))
                .then(response => response.json())
                .then(data => { if (data.success) refreshData(); });
        }

        function changeMode() {
            const mode = document.getElementById('modeSelect').value;
            fetch('/api/mode?mode=' + mode)
                .then(response => response.json())
                .then(data => { if (data.success) refreshData(); });
        }

        function blinkLED() {
            fetch('/api/custom')
                .then(response => response.json())
                .then(data => {
                    // Don't show alert - LED blink is visible enough
                    console.log('LED blink:', data.message);
                    refreshData();
                });
        }

        function resetSystem() {
            if (confirm('Reset the system?')) {
                fetch('/api/reset')
                    .then(response => response.json())
                    .then(data => {
                        alert('System resetting...');
                        setTimeout(() => location.reload(), 3000);
                    });
            }
        }
    </script>
</body>
</html>
)rawliteral";

// ==================== CONSTRUCTOR ====================

WebDashboard::WebDashboard(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;
    _server = nullptr;

    // Initialize data pointers to nullptr
    _sensorData = nullptr;
    _outputStates = nullptr;
    _systemInfo = nullptr;

    // Initialize callbacks to nullptr
    _output1Callback = nullptr;
    _output2Callback = nullptr;
    _modeCallback = nullptr;
    _resetCallback = nullptr;
    _customCallback = nullptr;
}

// ==================== PUBLIC METHODS ====================

void WebDashboard::begin() {
    Serial.println("\n=== Starting WiFi Access Point ===");

    // Start WiFi AP
    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);

    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(_ssid, _password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP SSID: ");
    Serial.println(_ssid);
    Serial.print("AP IP: ");
    Serial.println(IP);
    Serial.println("Open browser to: http://192.168.4.1");

    // Create web server
    _server = new WebServer(80);

    // Setup routes
    _server->on("/", [this]() { this->handleRoot(); });
    _server->on("/api/status", [this]() { this->handleStatus(); });
    _server->on("/api/output1", [this]() { this->handleOutput1(); });
    _server->on("/api/output2", [this]() { this->handleOutput2(); });
    _server->on("/api/mode", [this]() { this->handleMode(); });
    _server->on("/api/reset", [this]() { this->handleReset(); });
    _server->on("/api/custom", [this]() { this->handleCustom(); });

    // Start server
    _server->begin();
    Serial.println("Web server started!\n");
}

void WebDashboard::loop() {
    if (_server) {
        _server->handleClient();
    }
}

void WebDashboard::updateSensorData(SensorData* data) {
    _sensorData = data;
}

void WebDashboard::updateOutputStates(OutputStates* states) {
    _outputStates = states;
}

void WebDashboard::updateSystemInfo(SystemInfo* info) {
    _systemInfo = info;
}

void WebDashboard::onOutput1Change(OutputCallback callback) {
    _output1Callback = callback;
}

void WebDashboard::onOutput2Change(OutputCallback callback) {
    _output2Callback = callback;
}

void WebDashboard::onModeChange(ModeCallback callback) {
    _modeCallback = callback;
}

void WebDashboard::onReset(ActionCallback callback) {
    _resetCallback = callback;
}

void WebDashboard::onCustomAction(ActionCallback callback) {
    _customCallback = callback;
}

IPAddress WebDashboard::getIP() {
    return WiFi.softAPIP();
}

// ==================== PRIVATE HANDLERS ====================

void WebDashboard::handleRoot() {
    _server->send_P(200, "text/html", HTML_PAGE);
}

void WebDashboard::handleStatus() {
    StaticJsonDocument<512> doc;

    // Add sensor data
    if (_sensorData) {
        JsonObject sensors = doc.createNestedObject("sensors");
        sensors["value1"] = _sensorData->value1;
        sensors["value2"] = _sensorData->value2;
        sensors["value3"] = _sensorData->value3;
        sensors["label1"] = _sensorData->label1;
        sensors["label2"] = _sensorData->label2;
        sensors["label3"] = _sensorData->label3;
        sensors["unit1"] = _sensorData->unit1;
        sensors["unit2"] = _sensorData->unit2;
        sensors["unit3"] = _sensorData->unit3;
        sensors["show1"] = _sensorData->showValue1;
        sensors["show2"] = _sensorData->showValue2;
        sensors["show3"] = _sensorData->showValue3;
    }

    // Add output states
    if (_outputStates) {
        JsonObject outputs = doc.createNestedObject("outputs");
        outputs["output1"] = _outputStates->output1;
        outputs["output2"] = _outputStates->output2;
        outputs["label1"] = _outputStates->label1;
        outputs["label2"] = _outputStates->label2;
        outputs["show1"] = _outputStates->showOutput1;
        outputs["show2"] = _outputStates->showOutput2;
    }

    // Add system info
    if (_systemInfo) {
        JsonObject system = doc.createNestedObject("system");
        system["name"] = _systemInfo->projectName;
        system["version"] = _systemInfo->version;
        system["mode"] = _systemInfo->mode;
        system["uptime"] = _systemInfo->uptime;
    }

    String response;
    serializeJson(doc, response);
    _server->send(200, "application/json", response);
}

void WebDashboard::handleOutput1() {
    if (_server->hasArg("state") && _output1Callback) {
        bool state = _server->arg("state") == "1";
        _output1Callback(state);

        StaticJsonDocument<64> doc;
        doc["success"] = true;
        String response;
        serializeJson(doc, response);
        _server->send(200, "application/json", response);
    } else {
        _server->send(400, "application/json", "{\"error\":\"Invalid request\"}");
    }
}

void WebDashboard::handleOutput2() {
    if (_server->hasArg("state") && _output2Callback) {
        bool state = _server->arg("state") == "1";
        _output2Callback(state);

        StaticJsonDocument<64> doc;
        doc["success"] = true;
        String response;
        serializeJson(doc, response);
        _server->send(200, "application/json", response);
    } else {
        _server->send(400, "application/json", "{\"error\":\"Invalid request\"}");
    }
}

void WebDashboard::handleMode() {
    if (_server->hasArg("mode") && _modeCallback) {
        String mode = _server->arg("mode");
        _modeCallback(mode.c_str());

        StaticJsonDocument<64> doc;
        doc["success"] = true;
        String response;
        serializeJson(doc, response);
        _server->send(200, "application/json", response);
    } else {
        _server->send(400, "application/json", "{\"error\":\"Invalid request\"}");
    }
}

void WebDashboard::handleReset() {
    if (_resetCallback) {
        _resetCallback();
    }

    StaticJsonDocument<64> doc;
    doc["success"] = true;
    String response;
    serializeJson(doc, response);
    _server->send(200, "application/json", response);

    delay(1000);
    ESP.restart();
}

void WebDashboard::handleCustom() {
    if (_customCallback) {
        _customCallback();
    }

    StaticJsonDocument<128> doc;
    doc["success"] = true;
    doc["message"] = "Custom action completed";
    String response;
    serializeJson(doc, response);
    _server->send(200, "application/json", response);
}
