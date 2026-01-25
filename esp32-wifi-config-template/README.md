# ESP32 WiFi Config Dashboard Template

A ready-to-use, **modular** template for ESP32 projects that need wireless configuration and monitoring without USB/serial connection. Perfect for embedded projects, IoT devices, and prototypes.

## ⭐ NEW: Clean Modular Architecture!

**Your code stays clean and focused on your application logic!**

All web server code is now in separate files (`WebDashboard.h/cpp`). Your main program just:
1. Creates data structures (sensors, outputs, system info)
2. Updates values from sensors
3. Registers callback functions for buttons
4. Runs your application logic

**No more giant HTML strings or web server boilerplate in your main code!**

## Features

✅ **Modular Architecture** - Web code separate from application logic
✅ **Clean Main Program** - Focus on YOUR code, not web server details
✅ **WiFi Access Point** - ESP32 creates its own WiFi network
✅ **Web-based Dashboard** - Clean, responsive UI works on mobile & desktop
✅ **No USB/Serial Required** - Configure and monitor wirelessly
✅ **JSON API** - Easy integration with other systems
✅ **Real-time Updates** - Auto-refreshing sensor readings
✅ **Output Control** - Control LEDs, relays, servos via web
✅ **Multiple Modes** - Auto/Manual/Sleep operation modes
✅ **Mobile Friendly** - Responsive design for phones and tablets
✅ **Easy to Customize** - Well-documented code, easy to adapt

## Project Structure

```
esp32-wifi-config-template/
├── WebDashboard.h              # Web server class header
├── WebDashboard.cpp            # Web server implementation (HTML, API, handlers)
├── esp32_wifi_config_template.ino  # YOUR MAIN CODE - clean and simple!
├── example_temperature_monitor.ino # Complete working example
├── platformio.ini              # PlatformIO config
├── config.h.example            # Configuration template
├── README.md                   # This file
└── EXAMPLES.md                 # More examples
```

**Key Concept:**
- **WebDashboard files** = All web/HTML/server code (you rarely need to touch these!)
- **Your .ino file** = Your application logic ONLY (sensors, outputs, business logic)

## Quick Start

### Hardware Required

- ESP32 board (any variant: ESP32, ESP32-S2, ESP32-C3)
- USB cable for initial upload
- Optional: LEDs, relays, sensors for testing

### Software Required

**Option A: Arduino IDE**
- Arduino IDE 1.8.x or 2.x
- ESP32 board support installed

**Option B: PlatformIO (Recommended)**
- Visual Studio Code
- PlatformIO extension

## Installation

### Arduino IDE Method

1. **Install ESP32 Board Support**
   - Open Arduino IDE
   - Go to `File` → `Preferences`
   - Add to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to `Tools` → `Board` → `Boards Manager`
   - Search for "esp32" and install

2. **Install ArduinoJson Library**
   - Go to `Sketch` → `Include Library` → `Manage Libraries`
   - Search for "ArduinoJson"
   - Install version 6.x

3. **Upload the Code**
   - Open `esp32_wifi_config_template.ino` (or use `example_temperature_monitor.ino` to start)
   - **Important**: All 3 files must be in the same folder:
     - `esp32_wifi_config_template.ino`
     - `WebDashboard.h`
     - `WebDashboard.cpp`
   - Select your board: `Tools` → `Board` → `ESP32 Dev Module`
   - Select COM port: `Tools` → `Port` → (your ESP32 port)
   - Click Upload

### PlatformIO Method

1. **Open Project**
   - Open VS Code
   - `File` → `Open Folder` → Select this directory
   - PlatformIO will auto-detect `platformio.ini`

2. **Build & Upload**
   - Click the PlatformIO icon in sidebar
   - Click "Upload" (→) button
   - Or use shortcut: `Ctrl+Alt+U` (Windows/Linux) or `Cmd+Alt+U` (Mac)

## Usage

### 1. Power Up ESP32

After uploading, the ESP32 will:
- Start a WiFi Access Point named `ESP32-Config`
- Password: `esp32pass`
- IP address: `192.168.4.1`

Serial Monitor will show:
```
=== ESP32 WiFi Config Dashboard ===
Starting WiFi Access Point...
AP SSID: ESP32-Config
AP IP address: 192.168.4.1
Connect to the WiFi and open http://192.168.4.1
Web server started!
===================================
```

### 2. Connect to WiFi

**On Computer/Laptop:**
- Open WiFi settings
- Connect to network: `ESP32-Config`
- Password: `esp32pass`

**On Mobile Phone:**
- Settings → WiFi
- Select: `ESP32-Config`
- Enter password: `esp32pass`

### 3. Open Dashboard

- Open web browser
- Navigate to: `http://192.168.4.1`
- Dashboard loads automatically

### 4. Use the Interface

The dashboard includes:

- **📊 Sensor Readings** - Real-time analog sensor values, uptime
- **💡 LED Control** - Turn built-in LED ON/OFF
- **⚡ Relay Control** - Control relay/output (only in manual mode)
- **⚙️ Mode Selection** - Switch between Auto/Manual/Sleep modes
- **🔄 System Actions** - Refresh data, **Blink LED** (test connectivity), reset ESP32

**Tip:** Click the "💡 Blink LED" button to verify the ESP32 is responding. The onboard LED will blink 5 times.

## How to Use the Modular Structure

The new architecture makes customization **much simpler**! You only work in your main .ino file.

### Step 1: Configure Data Structures

In your main .ino file's `initializeDataStructures()` function:

```cpp
void initializeDataStructures() {
    // Configure what sensors to show
    sensors.label1 = "Temperature";  // Label shown on dashboard
    sensors.unit1 = "°C";            // Unit shown
    sensors.showValue1 = true;       // Show this value
    sensors.value1 = 0;              // Will be updated in loop()

    sensors.label2 = "Humidity";
    sensors.unit2 = "%";
    sensors.showValue2 = true;
    sensors.value2 = 0;

    // Configure outputs (buttons on dashboard)
    outputs.label1 = "Fan";
    outputs.showOutput1 = true;      // Show Fan control buttons
    outputs.output1 = false;         // Initial state

    // System info
    systemInfo.projectName = "🌡️ My Weather Station";
    systemInfo.version = "v1.0";
}
```

### Step 2: Read Sensors

In your `readSensors()` function:

```cpp
void readSensors() {
    // Just read your sensors - that's it!
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();
    sensorValue = analogRead(SENSOR_PIN);
}
```

### Step 3: Update Dashboard

In `updateDashboardData()`:

```cpp
void updateDashboardData() {
    // Update values
    sensors.value1 = temperature;
    sensors.value2 = humidity;
    outputs.output1 = fanState;
    systemInfo.uptime = millis() / 1000;

    // Push to dashboard - WebDashboard handles the rest!
    dashboard.updateSensorData(&sensors);
    dashboard.updateOutputStates(&outputs);
    dashboard.updateSystemInfo(&systemInfo);
}
```

### Step 4: Handle Button Presses

Register callback functions that are called when user clicks buttons:

```cpp
void onFanControl(bool state) {
    // This runs when user clicks Fan ON/OFF button
    fanState = state;
    digitalWrite(FAN_PIN, state ? HIGH : LOW);
    Serial.println(state ? "Fan ON" : "Fan OFF");
}

void setup() {
    // ... other setup code ...

    // Register the callback
    dashboard.onOutput1Change(onFanControl);
}
```

### Complete Example

See `example_temperature_monitor.ino` for a complete working example!

**That's it!** No HTML, no web server code, no JSON parsing. Just:
1. Configure what to show
2. Update values
3. Handle button clicks

### Want to Add More Sensors?

Just change the configuration:

```cpp
sensors.label3 = "Pressure";
sensors.unit3 = "hPa";
sensors.showValue3 = true;  // Now shows 3 sensors!
```

### Want to Add More Buttons?

```cpp
outputs.label2 = "Pump";
outputs.showOutput2 = true;  // Now shows 2 output controls!

// Register callback
dashboard.onOutput2Change(onPumpControl);
```

### Advanced: Modify Web Interface

If you need to change colors, layout, or add new features to the web interface:
- Edit `WebDashboard.cpp` (HTML is at the top)
- Most projects won't need to touch this file!

## Old Monolithic Version

The previous version with everything in one file is saved as `esp32_wifi_config_old.ino` for reference.

## API Reference

The ESP32 provides a JSON REST API:

### GET /api/status

Get current system status

**Response:**
```json
{
  "sensor": 1234,
  "uptime": 3600,
  "led": true,
  "relay": false,
  "mode": "auto"
}
```

### GET /api/led?state=\[0|1\]

Control LED state

**Parameters:**
- `state`: `0` = OFF, `1` = ON

**Response:**
```json
{
  "success": true,
  "led": true
}
```

### GET /api/relay?state=\[0|1\]

Control relay state (only works in manual mode)

**Parameters:**
- `state`: `0` = OFF, `1` = ON

**Response:**
```json
{
  "success": true,
  "relay": true
}
```

### GET /api/mode?mode=\[auto|manual|sleep\]

Change operation mode

**Parameters:**
- `mode`: `auto`, `manual`, or `sleep`

**Response:**
```json
{
  "success": true,
  "mode": "manual"
}
```

### GET /api/reset

Reset ESP32 (reboots after 3 seconds)

**Response:**
```json
{
  "success": true,
  "message": "Resetting in 3 seconds..."
}
```

## Operation Modes

### Auto Mode
- ESP32 automatically controls outputs based on sensor readings
- Example: Relay turns ON when sensor > 2000

### Manual Mode
- User has full control via web interface
- All automation disabled

### Sleep Mode
- Low power mode (to be implemented in your project)
- Reduces power consumption

## Pin Configuration

**Default Pins:**

| Function | GPIO | Notes |
|----------|------|-------|
| Built-in LED | 2 | Most ESP32 boards |
| Relay Output | 4 | Configurable |
| Analog Sensor | 34 | ADC1 channel |

**Safe ADC Pins (ESP32):**
- ADC1: GPIO 32, 33, 34, 35, 36, 39
- Avoid ADC2 pins when using WiFi

**PWM Capable:**
- Any GPIO pin (use `ledcSetup()` and `ledcAttachPin()`)

## Troubleshooting

### Cannot Connect to WiFi

**Problem:** ESP32-Config network not visible

**Solutions:**
- Check serial monitor - is AP started?
- Try different WiFi channel (change in code)
- Some devices don't show 2.4GHz networks if 5GHz is available
- Restart ESP32

### Cannot Access Web Page

**Problem:** Connected to WiFi but http://192.168.4.1 doesn't load

**Solutions:**
- Verify you're connected to ESP32-Config network
- Try `http://192.168.4.1` (not https)
- Clear browser cache
- Try different browser
- Disable mobile data on phone (use WiFi only)
- Check firewall settings on computer

### Serial Monitor Shows Gibberish

**Problem:** Unreadable characters in serial monitor

**Solutions:**
- Set baud rate to 115200
- Arduino IDE: `Tools` → `Serial Monitor` → Select "115200 baud"
- PlatformIO: Already set in `platformio.ini`

### Upload Failed

**Problem:** Cannot upload code to ESP32

**Solutions:**
- Hold BOOT button while clicking upload
- Check USB cable (use data cable, not charge-only)
- Install CH340/CP2102 drivers if needed
- Select correct COM port
- Try different USB port

### Page Doesn't Auto-Refresh

**Problem:** Sensor values don't update

**Solutions:**
- Check JavaScript console for errors (F12 in browser)
- Verify `/api/status` endpoint works: visit `http://192.168.4.1/api/status`
- Check serial monitor for errors

### High Power Consumption

**Problem:** ESP32 gets hot or drains battery fast

**Solutions:**
- Reduce WiFi transmit power:
  ```cpp
  WiFi.setTxPower(WIFI_POWER_11dBm);
  ```
- Implement sleep mode properly
- Reduce refresh rate
- Turn off WiFi when not needed

## Security Considerations

⚠️ **Important:** This template uses an open/simple WiFi network for ease of use.

**For Production/Public Use:**

1. **Use WPA2 Password** - Already enabled by default
2. **Change Default Password** - Never use "esp32pass" in production
3. **Add Authentication** - Implement HTTP Basic Auth or token-based auth
4. **HTTPS** - Use ESP32's SSL support for encrypted communication
5. **Input Validation** - Always validate user inputs server-side
6. **Rate Limiting** - Prevent API abuse

**Example: HTTP Basic Auth**

```cpp
if (!server.authenticate(AUTH_USERNAME, AUTH_PASSWORD)) {
    return server.requestAuthentication();
}
```

## Example Projects

### Temperature Monitor

```cpp
#include <DHT.h>

DHT dht(SENSOR_PIN, DHT22);

void setup() {
    dht.begin();
    // ... existing setup
}

// In handleStatus():
doc["temperature"] = dht.readTemperature();
doc["humidity"] = dht.readHumidity();
```

### Motor Controller

```cpp
const int MOTOR_PIN = 4;
const int PWM_CHANNEL = 0;

void setup() {
    ledcSetup(PWM_CHANNEL, 5000, 8);  // 5kHz, 8-bit
    ledcAttachPin(MOTOR_PIN, PWM_CHANNEL);
    // ... existing setup
}

// Add API endpoint:
server.on("/api/motor", HTTP_GET, []() {
    int speed = server.arg("speed").toInt();  // 0-255
    ledcWrite(PWM_CHANNEL, speed);
    server.send(200, "application/json", "{\"success\":true}");
});
```

### RGB LED Control

```cpp
const int LED_R = 25, LED_G = 26, LED_B = 27;

server.on("/api/rgb", HTTP_GET, []() {
    int r = server.arg("r").toInt();
    int g = server.arg("g").toInt();
    int b = server.arg("b").toInt();

    analogWrite(LED_R, r);
    analogWrite(LED_G, g);
    analogWrite(LED_B, b);

    server.send(200, "application/json", "{\"success\":true}");
});
```

## Advanced Features

### Save Settings to Flash

Use Preferences library to save settings:

```cpp
#include <Preferences.h>

Preferences preferences;

void saveMode() {
    preferences.begin("config", false);
    preferences.putString("mode", projectMode);
    preferences.end();
}

void loadMode() {
    preferences.begin("config", true);
    projectMode = preferences.getString("mode", "auto");
    preferences.end();
}
```

### Add mDNS for Easy Access

Access via `http://esp32.local` instead of IP:

```cpp
#include <ESPmDNS.h>

void setup() {
    // ... existing setup
    if (MDNS.begin("esp32")) {
        Serial.println("mDNS responder started: http://esp32.local");
    }
}
```

### WebSocket for Real-Time Updates

More efficient than polling for live data:

```cpp
#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);

// Send updates when data changes instead of polling
```

## Contributing

Feel free to:
- Report bugs
- Suggest features
- Submit improvements
- Share your projects built with this template

## License

Free to use and modify for personal and commercial projects.

## Credits

Built with:
- ESP32 Arduino Core
- ArduinoJson library
- Modern web technologies (HTML5, CSS3, JavaScript)

## Support

If you encounter issues:
1. Check troubleshooting section above
2. Enable debug output in Serial Monitor
3. Test API endpoints directly in browser
4. Check browser console (F12) for errors

Happy coding! 🚀
