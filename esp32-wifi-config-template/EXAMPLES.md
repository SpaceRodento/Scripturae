# ESP32 WiFi Dashboard - Practical Examples

Collection of ready-to-use examples for common use cases.

## Table of Contents

1. [Temperature & Humidity Monitor (DHT22)](#1-temperature--humidity-monitor-dht22)
2. [Smart Light Controller (PWM LED)](#2-smart-light-controller-pwm-led)
3. [Garage Door Opener](#3-garage-door-opener)
4. [Plant Watering System](#4-plant-watering-system)
5. [RGB LED Strip Controller](#5-rgb-led-strip-controller)
6. [Distance Sensor Monitor (HC-SR04)](#6-distance-sensor-monitor-hc-sr04)

---

## 1. Temperature & Humidity Monitor (DHT22)

Monitor room temperature and humidity wirelessly.

### Hardware

- ESP32
- DHT22 sensor
- 10kΩ pull-up resistor

### Wiring

```
DHT22 VCC  → ESP32 3.3V
DHT22 GND  → ESP32 GND
DHT22 DATA → ESP32 GPIO 4 (with 10k pull-up to 3.3V)
```

### Code Changes

**Install Library:** DHT sensor library (Adafruit)

**Add to sketch:**

```cpp
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    // ... existing setup
    dht.begin();
}

void handleStatus() {
    StaticJsonDocument<256> doc;

    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();

    doc["temperature"] = isnan(temp) ? 0 : temp;
    doc["humidity"] = isnan(humidity) ? 0 : humidity;
    doc["uptime"] = millis() / 1000;
    doc["led"] = ledState;
    doc["mode"] = projectMode;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}
```

**Update HTML (in INDEX_HTML):**

```html
<!-- Replace sensor value box with: -->
<div class="value-box">
    <div class="value-label">Temperature</div>
    <div>
        <span class="value-number" id="temperature">0</span>
        <span class="value-unit">°C</span>
    </div>
</div>
<div class="value-box">
    <div class="value-label">Humidity</div>
    <div>
        <span class="value-number" id="humidity">0</span>
        <span class="value-unit">%</span>
    </div>
</div>
```

**Update JavaScript:**

```javascript
function refreshData() {
    fetch('/api/status')
        .then(response => response.json())
        .then(data => {
            document.getElementById('temperature').textContent = data.temperature.toFixed(1);
            document.getElementById('humidity').textContent = data.humidity.toFixed(1);
            document.getElementById('uptime').textContent = data.uptime;
            // ... rest of code
        });
}
```

---

## 2. Smart Light Controller (PWM LED)

Control LED brightness with a slider.

### Hardware

- ESP32
- LED
- 220Ω resistor

### Wiring

```
LED Anode (+) → 220Ω resistor → ESP32 GPIO 4
LED Cathode (-) → ESP32 GND
```

### Code Changes

```cpp
const int LED_PWM_PIN = 4;
const int PWM_CHANNEL = 0;
const int PWM_FREQ = 5000;      // 5 kHz
const int PWM_RESOLUTION = 8;   // 8-bit (0-255)
int ledBrightness = 0;

void setup() {
    // ... existing setup

    // Setup PWM
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(LED_PWM_PIN, PWM_CHANNEL);
    ledcWrite(PWM_CHANNEL, 0);
}

// Add new API endpoint
void handleBrightness() {
    if (server.hasArg("value")) {
        ledBrightness = server.arg("value").toInt();
        ledBrightness = constrain(ledBrightness, 0, 255);
        ledcWrite(PWM_CHANNEL, ledBrightness);

        StaticJsonDocument<64> doc;
        doc["success"] = true;
        doc["brightness"] = ledBrightness;

        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    }
}

void setup() {
    // ... existing setup
    server.on("/api/brightness", HTTP_GET, handleBrightness);
}
```

**Add to HTML:**

```html
<div class="section">
    <h2>💡 LED Brightness</h2>
    <input type="range" min="0" max="255" value="0" id="brightnessSlider"
           oninput="setBrightness(this.value)" style="width: 100%;">
    <p style="margin-top: 10px;">
        Current: <strong id="currentBrightness">0</strong>/255
    </p>
</div>
```

**Add to JavaScript:**

```javascript
function setBrightness(value) {
    fetch('/api/brightness?value=' + value)
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                document.getElementById('currentBrightness').textContent = value;
            }
        });
}
```

---

## 3. Garage Door Opener

Open/close garage door with button and limit switches.

### Hardware

- ESP32
- Relay module
- 2x limit switches (door open/closed detection)
- Garage door opener motor

### Safety Warning

⚠️ **Always include safety features:**
- Limit switches to detect fully open/closed
- Auto-stop after timeout
- Emergency stop button
- Light/sound warning when door moves

### Wiring

```
Relay IN   → ESP32 GPIO 4
Relay VCC  → ESP32 5V (or external 5V)
Relay GND  → ESP32 GND
Relay COM  → Garage door opener button terminal 1
Relay NO   → Garage door opener button terminal 2

Limit Switch 1 (Open)  → ESP32 GPIO 35 & GND
Limit Switch 2 (Closed) → ESP32 GPIO 34 & GND
```

### Code Changes

```cpp
const int RELAY_PIN = 4;
const int LIMIT_OPEN = 35;
const int LIMIT_CLOSED = 34;
const int RELAY_PULSE_TIME = 500;  // 500ms pulse

bool doorMoving = false;
String doorState = "unknown";

void setup() {
    // ... existing setup
    pinMode(LIMIT_OPEN, INPUT_PULLUP);
    pinMode(LIMIT_CLOSED, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
}

void loop() {
    server.handleClient();

    // Update door state based on limit switches
    if (digitalRead(LIMIT_OPEN) == LOW) {
        doorState = "open";
        doorMoving = false;
    } else if (digitalRead(LIMIT_CLOSED) == LOW) {
        doorState = "closed";
        doorMoving = false;
    }
}

void handleDoorControl() {
    // Pulse relay to trigger door (simulates button press)
    digitalWrite(RELAY_PIN, HIGH);
    delay(RELAY_PULSE_TIME);
    digitalWrite(RELAY_PIN, LOW);

    doorMoving = true;

    StaticJsonDocument<128> doc;
    doc["success"] = true;
    doc["message"] = "Door triggered";

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void handleStatus() {
    StaticJsonDocument<256> doc;
    doc["doorState"] = doorState;
    doc["doorMoving"] = doorMoving;
    doc["uptime"] = millis() / 1000;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void setup() {
    // ... existing setup
    server.on("/api/door", HTTP_GET, handleDoorControl);
}
```

---

## 4. Plant Watering System

Automatic plant watering based on soil moisture.

### Hardware

- ESP32
- Soil moisture sensor
- Water pump or solenoid valve
- Relay module
- Power supply for pump

### Wiring

```
Moisture Sensor VCC → ESP32 3.3V
Moisture Sensor GND → ESP32 GND
Moisture Sensor AO  → ESP32 GPIO 34

Relay IN  → ESP32 GPIO 4
Pump+     → Relay NO
Pump-     → Power supply -
Power +   → Relay COM
```

### Code Changes

```cpp
const int MOISTURE_PIN = 34;
const int PUMP_PIN = 4;

int moistureThreshold = 2000;  // Adjust based on your sensor
int wateringDuration = 5000;   // 5 seconds
unsigned long lastWatering = 0;
unsigned long wateringCooldown = 3600000;  // 1 hour between waterings

int moistureLevel = 0;
bool pumpActive = false;

void setup() {
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(PUMP_PIN, LOW);
    // ... existing setup
}

void loop() {
    server.handleClient();

    moistureLevel = analogRead(MOISTURE_PIN);

    // Auto watering logic
    if (projectMode == "auto") {
        unsigned long now = millis();

        // Start watering if soil is dry and cooldown period passed
        if (moistureLevel < moistureThreshold &&
            (now - lastWatering) > wateringCooldown &&
            !pumpActive) {

            startWatering();
        }

        // Stop watering after duration
        if (pumpActive && (now - lastWatering) > wateringDuration) {
            stopWatering();
        }
    }
}

void startWatering() {
    digitalWrite(PUMP_PIN, HIGH);
    pumpActive = true;
    lastWatering = millis();
    Serial.println("💧 Started watering");
}

void stopWatering() {
    digitalWrite(PUMP_PIN, LOW);
    pumpActive = false;
    Serial.println("💧 Stopped watering");
}

void handleManualWater() {
    if (projectMode == "manual") {
        startWatering();
        delay(wateringDuration);
        stopWatering();

        server.send(200, "application/json", "{\"success\":true}");
    } else {
        server.send(400, "application/json",
                   "{\"error\":\"Switch to manual mode first\"}");
    }
}

void setup() {
    // ... existing setup
    server.on("/api/water", HTTP_GET, handleManualWater);
}
```

---

## 5. RGB LED Strip Controller

Control color and brightness of RGB LED strip.

### Hardware

- ESP32
- RGB LED strip (common anode or cathode)
- 3x MOSFETs or transistors
- Power supply for LEDs

### Code Changes

```cpp
const int LED_R = 25;
const int LED_G = 26;
const int LED_B = 27;

const int PWM_R = 0;
const int PWM_G = 1;
const int PWM_B = 2;

int colorR = 0, colorG = 0, colorB = 0;

void setup() {
    ledcSetup(PWM_R, 5000, 8);
    ledcSetup(PWM_G, 5000, 8);
    ledcSetup(PWM_B, 5000, 8);

    ledcAttachPin(LED_R, PWM_R);
    ledcAttachPin(LED_G, PWM_G);
    ledcAttachPin(LED_B, PWM_B);

    // ... existing setup
}

void handleRGB() {
    if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
        colorR = constrain(server.arg("r").toInt(), 0, 255);
        colorG = constrain(server.arg("g").toInt(), 0, 255);
        colorB = constrain(server.arg("b").toInt(), 0, 255);

        ledcWrite(PWM_R, colorR);
        ledcWrite(PWM_G, colorG);
        ledcWrite(PWM_B, colorB);

        StaticJsonDocument<128> doc;
        doc["success"] = true;
        doc["r"] = colorR;
        doc["g"] = colorG;
        doc["b"] = colorB;

        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    } else {
        server.send(400, "application/json", "{\"error\":\"Missing RGB values\"}");
    }
}

void setup() {
    // ... existing setup
    server.on("/api/rgb", HTTP_GET, handleRGB);
}
```

**Add to HTML:**

```html
<div class="section">
    <h2>🎨 RGB Color</h2>
    <div style="display: grid; gap: 10px;">
        <div>
            <label>Red: <span id="rValue">0</span></label>
            <input type="range" min="0" max="255" value="0" id="rSlider"
                   oninput="updateRGB()" style="width: 100%;">
        </div>
        <div>
            <label>Green: <span id="gValue">0</span></label>
            <input type="range" min="0" max="255" value="0" id="gSlider"
                   oninput="updateRGB()" style="width: 100%;">
        </div>
        <div>
            <label>Blue: <span id="bValue">0</span></label>
            <input type="range" min="0" max="255" value="0" id="bSlider"
                   oninput="updateRGB()" style="width: 100%;">
        </div>
        <div id="colorPreview" style="height: 50px; border-radius: 8px; background: rgb(0,0,0);"></div>
    </div>
</div>
```

**Add to JavaScript:**

```javascript
function updateRGB() {
    let r = document.getElementById('rSlider').value;
    let g = document.getElementById('gSlider').value;
    let b = document.getElementById('bSlider').value;

    document.getElementById('rValue').textContent = r;
    document.getElementById('gValue').textContent = g;
    document.getElementById('bValue').textContent = b;
    document.getElementById('colorPreview').style.background = `rgb(${r},${g},${b})`;

    fetch(`/api/rgb?r=${r}&g=${g}&b=${b}`)
        .then(response => response.json())
        .then(data => console.log('RGB updated:', data));
}
```

---

## 6. Distance Sensor Monitor (HC-SR04)

Monitor distance with ultrasonic sensor.

### Hardware

- ESP32
- HC-SR04 ultrasonic sensor

### Wiring

```
HC-SR04 VCC  → ESP32 5V
HC-SR04 GND  → ESP32 GND
HC-SR04 TRIG → ESP32 GPIO 5
HC-SR04 ECHO → ESP32 GPIO 18
```

### Code Changes

```cpp
const int TRIG_PIN = 5;
const int ECHO_PIN = 18;

float distance_cm = 0;

void setup() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    // ... existing setup
}

float getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000);  // 30ms timeout
    float distance = duration * 0.034 / 2;  // Speed of sound = 340m/s

    return (distance > 0 && distance < 400) ? distance : -1;
}

void loop() {
    server.handleClient();

    static unsigned long lastRead = 0;
    if (millis() - lastRead > 500) {
        distance_cm = getDistance();
        lastRead = millis();
    }
}

void handleStatus() {
    StaticJsonDocument<256> doc;
    doc["distance"] = distance_cm;
    doc["uptime"] = millis() / 1000;
    // ... rest of your status

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}
```

---

## More Ideas

- **Security System** - Motion sensor + camera trigger + notifications
- **Weather Station** - Multiple sensors (temp, humidity, pressure, light)
- **Aquarium Controller** - Light timer, heater control, feeding reminder
- **Thermostat** - Temperature-based relay control
- **Door/Window Monitor** - Reed switches for home security
- **Pet Feeder** - Servo-controlled food dispenser
- **Air Quality Monitor** - MQ sensors for gas/smoke detection

## Tips

1. **Always test with Serial Monitor first** before relying on web interface
2. **Add timeouts** to prevent relays staying on forever
3. **Use pull-up/pull-down resistors** on inputs to prevent floating pins
4. **Debounce buttons** in code (check state for 50ms)
5. **Add error handling** for sensor read failures
6. **Log events** to help debugging
7. **Test edge cases** (sensor unplugged, WiFi disconnected, etc.)

Happy building! 🔧
