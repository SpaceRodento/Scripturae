/*
 * ESP32 Phototransistor/Photodiode Light Sensor
 * Layout 5 - Valon havaitseminen
 *
 * Kytkenta:
 *   3V3 -> 10kOhm vastus -> GPIO 32 -> Fototransistorin kollektori
 *                                   -> Fototransistorin emitteri -> GND
 *
 * Toiminta:
 *   - Valossa fototransistori johtaa -> jannite laskee -> ADC-arvo pienempi
 *   - Pimeassa fototransistori ei johda -> jannite nousee -> ADC-arvo suurempi
 *   - Varjossa vastus noin 10MOhm
 *
 * Huom: Samankaltainen kuin LDR (Light Dependent Resistor) -toteutus
 *
 * Author: ESP32 Project
 * Date: 2025
 */

#include "WebDashboard.h"

// ==================== CONFIGURATION ====================

// WiFi Access Point credentials
const char* WIFI_SSID = "ESP32-LightSensor";
const char* WIFI_PASSWORD = "esp32light";

// GPIO Pin Definitions
const int LED_PIN = 2;                 // Built-in LED
const int PHOTOTRANSISTOR_PIN = 32;    // Fototransistori analoginen sisaantulo

// Light sensor calibration
// Saada naita arvoja omalle fototransistorillesi
const int LIGHT_MIN_ADC = 0;           // ADC-arvo kirkkaassa valossa
const int LIGHT_MAX_ADC = 4095;        // ADC-arvo pimeassa
const int LIGHT_THRESHOLD_DARK = 3000; // Raja pimeaksi (ADC)
const int LIGHT_THRESHOLD_DIM = 2000;  // Raja himmeaksi
const int LIGHT_THRESHOLD_BRIGHT = 500;// Raja kirkkaaksi

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
int lightRawValue = 0;        // Raaka ADC-arvo (0-4095)
int lightPercent = 0;         // Valon maara prosentteina (0-100%)
String lightStatus = "?";     // Valon tila tekstina

// Averaging for stable readings
const int NUM_SAMPLES = 10;
int lightSamples[NUM_SAMPLES];
int sampleIndex = 0;

// ==================== CALLBACK FUNCTIONS ====================

void onLEDChange(bool state) {
    Serial.print("LED: ");
    Serial.println(state ? "PAALLA" : "POIS");

    ledState = state;
    digitalWrite(LED_PIN, state ? HIGH : LOW);
    outputs.output1 = ledState;
}

void onAutoLightChange(bool state) {
    Serial.print("Automaattinen valo: ");
    Serial.println(state ? "PAALLA" : "POIS");

    if (state) {
        currentMode = "auto";
    } else {
        currentMode = "manual";
    }
    systemInfo.mode = currentMode.c_str();
}

void onModeChange(const char* mode) {
    Serial.print("Tila vaihdettu: ");
    Serial.println(mode);

    currentMode = String(mode);
    systemInfo.mode = currentMode.c_str();
}

void onReset() {
    Serial.println("Kaynnistetaan uudelleen...");
}

void onCustomAction() {
    // Kalibrointitoiminto - tulosta nykyiset arvot
    Serial.println("\n========== KALIBROINTI ==========");
    Serial.print("Raaka ADC-arvo: ");
    Serial.println(lightRawValue);
    Serial.print("Valoprosentti: ");
    Serial.print(lightPercent);
    Serial.println("%");
    Serial.print("Tila: ");
    Serial.println(lightStatus);
    Serial.println("=================================\n");

    // Vilkuta LED kerran kuitattaksesi toiminnon
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
}

// ==================== SETUP ====================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n========================================");
    Serial.println("  Fototransistori Valosensori");
    Serial.println("  Layout 5 - Valon havaitseminen");
    Serial.println("========================================\n");

    // Configure GPIO pins
    pinMode(LED_PIN, OUTPUT);
    pinMode(PHOTOTRANSISTOR_PIN, INPUT);

    digitalWrite(LED_PIN, LOW);

    // Initialize sample array
    for (int i = 0; i < NUM_SAMPLES; i++) {
        lightSamples[i] = 0;
    }

    // Initialize data structures
    initializeDataStructures();

    // Start web dashboard
    dashboard.begin();

    // Register callback functions
    dashboard.onOutput1Change(onLEDChange);
    dashboard.onOutput2Change(onAutoLightChange);
    dashboard.onModeChange(onModeChange);
    dashboard.onReset(onReset);
    dashboard.onCustomAction(onCustomAction);

    Serial.println("Kaynnistys valmis!");
    Serial.println("Yhdista WiFi-verkkoon ja avaa http://192.168.4.1");
    Serial.print("WiFi: ");
    Serial.println(WIFI_SSID);
    Serial.println();
}

// ==================== LOOP ====================

void loop() {
    // Handle web requests
    dashboard.loop();

    // Read light sensor periodically
    static unsigned long lastSensorRead = 0;
    if (millis() - lastSensorRead > 100) {  // 10Hz lukutaajuus
        readLightSensor();
        lastSensorRead = millis();
    }

    // Update dashboard
    static unsigned long lastDashboardUpdate = 0;
    if (millis() - lastDashboardUpdate > 250) {  // 4Hz paivitys
        updateDashboardData();
        lastDashboardUpdate = millis();
    }

    // Run automatic light control
    runLightControl();

    // Print debug info periodically
    static unsigned long lastDebugPrint = 0;
    if (millis() - lastDebugPrint > 2000) {
        printDebugInfo();
        lastDebugPrint = millis();
    }

    delay(10);
}

// ==================== LIGHT SENSOR FUNCTIONS ====================

void readLightSensor() {
    // Read raw ADC value from phototransistor
    int rawValue = analogRead(PHOTOTRANSISTOR_PIN);

    // Add to samples array for averaging
    lightSamples[sampleIndex] = rawValue;
    sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;

    // Calculate average
    long sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        sum += lightSamples[i];
    }
    lightRawValue = sum / NUM_SAMPLES;

    // Convert to percentage (inverted because dark = high ADC)
    // Kirkas valo = pieni ADC = korkea prosentti
    // Pimea = suuri ADC = matala prosentti
    lightPercent = map(lightRawValue, LIGHT_MAX_ADC, LIGHT_MIN_ADC, 0, 100);
    lightPercent = constrain(lightPercent, 0, 100);

    // Determine light status
    if (lightRawValue >= LIGHT_THRESHOLD_DARK) {
        lightStatus = "PIMEA";
    } else if (lightRawValue >= LIGHT_THRESHOLD_DIM) {
        lightStatus = "HIMMEA";
    } else if (lightRawValue >= LIGHT_THRESHOLD_BRIGHT) {
        lightStatus = "VALOISA";
    } else {
        lightStatus = "KIRKAS";
    }
}

void runLightControl() {
    // Automatic mode: Turn LED on when dark
    if (currentMode == "auto") {
        bool shouldLedBeOn = (lightRawValue >= LIGHT_THRESHOLD_DIM);

        if (shouldLedBeOn && !ledState) {
            ledState = true;
            digitalWrite(LED_PIN, HIGH);
            outputs.output1 = true;
            Serial.println("Auto: LED PAALLE (pimeaa)");
        } else if (!shouldLedBeOn && ledState) {
            ledState = false;
            digitalWrite(LED_PIN, LOW);
            outputs.output1 = false;
            Serial.println("Auto: LED POIS (valoisaa)");
        }
    }
}

// ==================== DASHBOARD FUNCTIONS ====================

void initializeDataStructures() {
    // Configure sensor data display
    sensors.label1 = "Valo";
    sensors.label2 = "ADC";
    sensors.label3 = "Tila";
    sensors.unit1 = "%";
    sensors.unit2 = "raw";
    sensors.unit3 = "";
    sensors.showValue1 = true;   // Valoprosentti
    sensors.showValue2 = true;   // Raaka ADC-arvo
    sensors.showValue3 = false;  // Tila (ei numerona)
    sensors.value1 = 0;
    sensors.value2 = 0;
    sensors.value3 = 0;

    // Configure output controls
    outputs.label1 = "LED";
    outputs.label2 = "Auto";
    outputs.showOutput1 = true;   // LED on/off
    outputs.showOutput2 = true;   // Auto mode toggle
    outputs.output1 = false;
    outputs.output2 = true;       // Auto mode on by default

    // Configure system info
    systemInfo.projectName = "Valosensori (Layout 5)";
    systemInfo.version = "v1.0";
    systemInfo.mode = currentMode.c_str();
    systemInfo.uptime = 0;
}

void updateDashboardData() {
    // Update sensor values
    sensors.value1 = lightPercent;
    sensors.value2 = lightRawValue;
    sensors.value3 = 0;  // Light status shown as text in serial

    // Update output states
    outputs.output1 = ledState;
    outputs.output2 = (currentMode == "auto");

    // Update system info
    systemInfo.uptime = millis() / 1000;
    systemInfo.mode = currentMode.c_str();

    // Push updates to dashboard
    dashboard.updateSensorData(&sensors);
    dashboard.updateOutputStates(&outputs);
    dashboard.updateSystemInfo(&systemInfo);
}

void printDebugInfo() {
    Serial.print("Valo: ");
    Serial.print(lightPercent);
    Serial.print("% | ADC: ");
    Serial.print(lightRawValue);
    Serial.print(" | ");
    Serial.print(lightStatus);
    Serial.print(" | LED: ");
    Serial.print(ledState ? "ON" : "OFF");
    Serial.print(" | Mode: ");
    Serial.println(currentMode);
}
