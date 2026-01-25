/*
 * ESP32 Temperature & Humidity Monitor Example
 *
 * Demonstrates how clean and simple the code is with WebDashboard!
 * Just focus on reading sensors and updating values.
 *
 * Hardware:
 * - ESP32
 * - DHT22 sensor on GPIO 4
 * - LED on GPIO 2
 *
 * Libraries needed:
 * - DHT sensor library (Adafruit)
 */

#include "WebDashboard.h"
#include <DHT.h>

// ==================== CONFIGURATION ====================

const char* WIFI_SSID = "ESP32-Weather";
const char* WIFI_PASSWORD = "weather123";

// DHT Sensor
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// LED for visual feedback
const int LED_PIN = 2;

// Temperature threshold for automatic fan control
const float TEMP_THRESHOLD = 25.0;  // °C

// ==================== GLOBALS ====================

WebDashboard dashboard(WIFI_SSID, WIFI_PASSWORD);

SensorData sensors;
OutputStates outputs;
SystemInfo systemInfo;

float temperature = 0;
float humidity = 0;
bool fanState = false;
String mode = "auto";

// ==================== CALLBACKS ====================

void onFanControl(bool state) {
    if (mode == "manual") {
        fanState = state;
        digitalWrite(LED_PIN, state ? HIGH : LOW);
        Serial.printf("Fan manually set to %s\n", state ? "ON" : "OFF");
    }
}

void onModeChange(const char* newMode) {
    mode = String(newMode);
    Serial.printf("Mode: %s\n", mode.c_str());

    if (mode == "manual") {
        Serial.println("Manual mode: Use dashboard to control fan");
    } else if (mode == "auto") {
        Serial.println("Auto mode: Fan controlled by temperature");
    }
}

void onReset() {
    Serial.println("Resetting...");
}

void onBlinkTest() {
    Serial.println("💡 Blink LED Test - Blinking onboard LED 5 times...");

    // Blink LED 5 times to confirm ESP32 is responding
    bool originalState = fanState;
    for (int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(150);
        digitalWrite(LED_PIN, LOW);
        delay(150);
    }

    // Restore original state
    digitalWrite(LED_PIN, originalState ? HIGH : LOW);

    Serial.println("✓ LED blink test completed!");
}

// ==================== SETUP ====================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n╔══════════════════════════════╗");
    Serial.println("║  Temperature Monitor v1.0   ║");
    Serial.println("╚══════════════════════════════╝\n");

    // Init hardware
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    dht.begin();

    // Configure display
    sensors.label1 = "Temperature";
    sensors.label2 = "Humidity";
    sensors.label3 = "Heat Index";
    sensors.unit1 = "°C";
    sensors.unit2 = "%";
    sensors.unit3 = "°C";
    sensors.showValue1 = true;
    sensors.showValue2 = true;
    sensors.showValue3 = true;

    outputs.label1 = "Fan/Cooler";
    outputs.label2 = "";  // Not used
    outputs.showOutput1 = true;
    outputs.showOutput2 = false;

    systemInfo.projectName = "🌡️ Weather Station";
    systemInfo.version = "v1.0";
    systemInfo.mode = mode.c_str();

    // Start dashboard
    dashboard.begin();
    dashboard.onOutput1Change(onFanControl);
    dashboard.onModeChange(onModeChange);
    dashboard.onReset(onReset);
    dashboard.onCustomAction(onBlinkTest);

    Serial.println("✓ Ready! Open http://192.168.4.1\n");
}

// ==================== LOOP ====================

void loop() {
    dashboard.loop();

    // Read sensors every 2 seconds
    static unsigned long lastRead = 0;
    if (millis() - lastRead > 2000) {
        temperature = dht.readTemperature();
        humidity = dht.readHumidity();

        // Check if reads failed
        if (isnan(temperature) || isnan(humidity)) {
            Serial.println("Failed to read from DHT sensor!");
            temperature = 0;
            humidity = 0;
        } else {
            // Calculate heat index
            float heatIndex = dht.computeHeatIndex(temperature, humidity, false);
            sensors.value3 = heatIndex;
        }

        lastRead = millis();
    }

    // AUTO MODE: Control fan based on temperature
    if (mode == "auto") {
        if (temperature > TEMP_THRESHOLD && !fanState) {
            fanState = true;
            digitalWrite(LED_PIN, HIGH);
            Serial.printf("Auto: Fan ON (temp: %.1f°C)\n", temperature);
        }
        else if (temperature < TEMP_THRESHOLD - 1.0 && fanState) {
            fanState = false;
            digitalWrite(LED_PIN, LOW);
            Serial.printf("Auto: Fan OFF (temp: %.1f°C)\n", temperature);
        }
    }

    // Update dashboard
    sensors.value1 = temperature;
    sensors.value2 = humidity;
    outputs.output1 = fanState;
    systemInfo.uptime = millis() / 1000;
    systemInfo.mode = mode.c_str();

    dashboard.updateSensorData(&sensors);
    dashboard.updateOutputStates(&outputs);
    dashboard.updateSystemInfo(&systemInfo);

    delay(10);
}
