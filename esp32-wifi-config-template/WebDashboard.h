/*
 * WebDashboard.h
 *
 * Modular web server dashboard for ESP32 projects.
 * Separates web interface logic from application logic.
 *
 * Usage:
 *   1. Include this header in your main .ino file
 *   2. Create DashboardData struct with your sensor values
 *   3. Create WebDashboard instance
 *   4. Set callback functions for button actions
 *   5. Call begin() in setup(), loop() in loop()
 */

#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ==================== DATA STRUCTURES ====================

// Sensor/input data to display on dashboard
struct SensorData {
    float value1;           // Generic sensor value 1
    float value2;           // Generic sensor value 2
    float value3;           // Generic sensor value 3
    const char* label1;     // Label for value1
    const char* label2;     // Label for value2
    const char* label3;     // Label for value3
    const char* unit1;      // Unit for value1 (e.g., "°C", "V", "%")
    const char* unit2;      // Unit for value2
    const char* unit3;      // Unit for value3
    bool showValue1;        // Show/hide value1
    bool showValue2;        // Show/hide value2
    bool showValue3;        // Show/hide value3
};

// Output/control states
struct OutputStates {
    bool output1;           // State of output 1 (LED, relay, etc.)
    bool output2;           // State of output 2
    const char* label1;     // Label for output1
    const char* label2;     // Label for output2
    bool showOutput1;       // Show/hide output1 controls
    bool showOutput2;       // Show/hide output2 controls
};

// System information
struct SystemInfo {
    const char* projectName;    // Project title shown on dashboard
    const char* version;        // Version string
    const char* mode;           // Current operation mode
    unsigned long uptime;       // Uptime in seconds
};

// ==================== CALLBACK FUNCTIONS ====================

// Callback function types for button actions
typedef void (*OutputCallback)(bool state);
typedef void (*ModeCallback)(const char* mode);
typedef void (*ActionCallback)();

// ==================== WEB DASHBOARD CLASS ====================

class WebDashboard {
public:
    // Constructor
    WebDashboard(const char* ssid, const char* password);

    // Initialize WiFi AP and web server
    void begin();

    // Call this in loop() to handle web requests
    void loop();

    // Update data that will be displayed
    void updateSensorData(SensorData* data);
    void updateOutputStates(OutputStates* states);
    void updateSystemInfo(SystemInfo* info);

    // Set callback functions for user interactions
    void onOutput1Change(OutputCallback callback);
    void onOutput2Change(OutputCallback callback);
    void onModeChange(ModeCallback callback);
    void onReset(ActionCallback callback);
    void onCustomAction(ActionCallback callback);

    // Get WiFi info
    IPAddress getIP();

private:
    // WiFi credentials
    const char* _ssid;
    const char* _password;

    // Web server
    WebServer* _server;

    // Data pointers
    SensorData* _sensorData;
    OutputStates* _outputStates;
    SystemInfo* _systemInfo;

    // Callbacks
    OutputCallback _output1Callback;
    OutputCallback _output2Callback;
    ModeCallback _modeCallback;
    ActionCallback _resetCallback;
    ActionCallback _customCallback;

    // Web request handlers
    void handleRoot();
    void handleStatus();
    void handleOutput1();
    void handleOutput2();
    void handleMode();
    void handleReset();
    void handleCustom();

    // HTML page
    const char* getHTML();
};

#endif // WEB_DASHBOARD_H
