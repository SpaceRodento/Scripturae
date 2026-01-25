/*
 * ESP32 WiFi Config Dashboard - Clean Template
 *
 * This is the MAIN APPLICATION FILE - keep it simple and focused on your logic!
 * All web server code is in WebDashboard.h/cpp
 *
 * How to use:
 * 1. Define your sensor/GPIO pins
 * 2. Read sensors and update data structures
 * 3. Implement callback functions for button actions
 * 4. That's it! WebDashboard handles the rest.
 *
 * Author: Your Name
 * Date: 2024
 */

#include "WebDashboard.h"

// ==================== CONFIGURATION ====================

// WiFi Access Point credentials
const char* WIFI_SSID = "ESP32-Config";
const char* WIFI_PASSWORD = "esp32pass";

// GPIO Pin Definitions - Customize for your project!
const int LED_PIN = 2;           // Built-in LED
const int RELAY_PIN = 4;         // Relay or output
const int SENSOR_PIN = 34;       // Analog sensor

// ==================== GLOBAL VARIABLES ====================

// Web Dashboard instance
WebDashboard dashboard(WIFI_SSID, WIFI_PASSWORD);

// Data structures for dashboard
SensorData sensors;
OutputStates outputs;
SystemInfo systemInfo;

// Application state
String currentMode = "auto";
bool ledState = false;
bool relayState = false;
int sensorValue = 0;

// ==================== CALLBACK FUNCTIONS ====================
// These are called when user interacts with the dashboard

void onLEDChange(bool state) {
    Serial.print("LED button pressed: ");
    Serial.println(state ? "ON" : "OFF");

    ledState = state;
    digitalWrite(LED_PIN, state ? HIGH : LOW);

    // Update output states
    outputs.output1 = ledState;
}

void onRelayChange(bool state) {
    Serial.print("Relay button pressed: ");
    Serial.println(state ? "ON" : "OFF");

    // Only allow manual control in manual mode
    if (currentMode == "manual") {
        relayState = state;
        digitalWrite(RELAY_PIN, state ? HIGH : LOW);
        outputs.output2 = relayState;
    } else {
        Serial.println("Relay control only available in manual mode");
    }
}

void onModeChange(const char* mode) {
    Serial.print("Mode changed to: ");
    Serial.println(mode);

    currentMode = String(mode);
    systemInfo.mode = currentMode.c_str();

    // Implement mode-specific behavior here
    if (currentMode == "sleep") {
        // Enter low power mode
        Serial.println("Entering sleep mode...");
    }
}

void onReset() {
    Serial.println("Reset requested via dashboard");
    // Do any cleanup before reset
    // ESP will restart automatically after this callback
}

void onCustomAction() {
    Serial.println("💡 Blink LED Test - Blinking onboard LED 5 times...");

    // Blink LED 5 times to confirm ESP32 is responding
    bool originalState = ledState;
    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(150);
        digitalWrite(LED_PIN, LOW);
        delay(150);
    }

    // Restore original LED state
    digitalWrite(LED_PIN, originalState ? HIGH : LOW);

    Serial.println("✓ LED blink test completed - ESP32 is working!");
}

// ==================== SETUP ====================

void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n╔════════════════════════════════════╗");
    Serial.println("║   ESP32 Dashboard Template v2.0   ║");
    Serial.println("╚════════════════════════════════════╝\n");

    // Configure GPIO pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(SENSOR_PIN, INPUT);

    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);

    // Initialize data structures
    initializeDataStructures();

    // Start web dashboard
    dashboard.begin();

    // Register callback functions
    dashboard.onOutput1Change(onLEDChange);
    dashboard.onOutput2Change(onRelayChange);
    dashboard.onModeChange(onModeChange);
    dashboard.onReset(onReset);
    dashboard.onCustomAction(onCustomAction);

    Serial.println("✓ Setup complete!");
    Serial.println("✓ Connect to WiFi and open http://192.168.4.1\n");
}

// ==================== LOOP ====================

void loop() {
    // Handle web requests
    dashboard.loop();

    // Read sensors periodically
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead > 500) {
        readSensors();
        lastSensorRead = millis();
    }

    // Update dashboard data
    updateDashboardData();

    // Your application logic here!
    runApplicationLogic();

    // Small delay to prevent watchdog issues
    delay(10);
}

// ==================== APPLICATION FUNCTIONS ====================

void initializeDataStructures() {
    // Configure sensor data
    sensors.label1 = "Sensor";
    sensors.label2 = "Temperature";
    sensors.label3 = "Humidity";
    sensors.unit1 = "units";
    sensors.unit2 = "°C";
    sensors.unit3 = "%";
    sensors.showValue1 = true;
    sensors.showValue2 = false;  // Hide until you add a real sensor
    sensors.showValue3 = false;  // Hide until you add a real sensor
    sensors.value1 = 0;
    sensors.value2 = 0;
    sensors.value3 = 0;

    // Configure output states
    outputs.label1 = "LED";
    outputs.label2 = "Relay";
    outputs.showOutput1 = true;
    outputs.showOutput2 = true;
    outputs.output1 = false;
    outputs.output2 = false;

    // Configure system info
    systemInfo.projectName = "🔧 My ESP32 Project";
    systemInfo.version = "v1.0";
    systemInfo.mode = currentMode.c_str();
    systemInfo.uptime = 0;
}

void readSensors() {
    // Read analog sensor
    sensorValue = analogRead(SENSOR_PIN);

    // Example: Add more sensors here
    // float temperature = dht.readTemperature();
    // float humidity = dht.readHumidity();
}

void updateDashboardData() {
    // Update sensor values
    sensors.value1 = sensorValue;
    // sensors.value2 = temperature;
    // sensors.value3 = humidity;

    // Update output states
    outputs.output1 = ledState;
    outputs.output2 = relayState;

    // Update system info
    systemInfo.uptime = millis() / 1000;
    systemInfo.mode = currentMode.c_str();

    // Push updates to dashboard
    dashboard.updateSensorData(&sensors);
    dashboard.updateOutputStates(&outputs);
    dashboard.updateSystemInfo(&systemInfo);
}

void runApplicationLogic() {
    // AUTOMATIC MODE: Implement your automatic control logic
    if (currentMode == "auto") {
        // Example: Turn relay ON when sensor > threshold
        if (sensorValue > 2000 && !relayState) {
            relayState = true;
            digitalWrite(RELAY_PIN, HIGH);
            Serial.println("Auto: Relay turned ON");
        }
        else if (sensorValue < 1500 && relayState) {
            relayState = false;
            digitalWrite(RELAY_PIN, LOW);
            Serial.println("Auto: Relay turned OFF");
        }
    }

    // MANUAL MODE: User controls everything via dashboard
    // Nothing to do here - buttons handle it

    // SLEEP MODE: Low power mode
    if (currentMode == "sleep") {
        // Implement power saving
        // WiFi.disconnect();
        // esp_sleep_enable_timer_wakeup(10 * 1000000); // 10 sec
        // esp_light_sleep_start();
    }

    // Add your custom logic here!
    // Examples:
    // - PID control
    // - State machines
    // - Timers
    // - Communication with other devices
}

// ==================== HELPER FUNCTIONS ====================

// Add your helper functions here
// Examples:
// - Sensor calibration
// - Data logging
// - WiFi reconnection
// - EEPROM saving/loading
