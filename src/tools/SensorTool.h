/**
 * TinyAgent - Sensor Tool
 * 
 * Built-in sensor reading utilities (internal temp, hall sensor, touch pins).
 */

#ifndef TINY_AGENT_SENSOR_TOOL_H
#define TINY_AGENT_SENSOR_TOOL_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../ToolRegistry.h"

class SensorTool {
public:
    static void registerAll(ToolRegistry& registry) {
        registry.registerTool(
            "internal_temp",
            "Read the ESP32 internal temperature sensor (approximate, in Fahrenheit)",
            "{\"type\":\"object\",\"properties\":{}}",
            [](const String& params) -> String {
                #ifdef ESP32
                float tempF = temperatureRead();
                float tempC = (tempF - 32.0) * 5.0 / 9.0;
                return "{\"temperature_f\":" + String(tempF, 1) +
                       ",\"temperature_c\":" + String(tempC, 1) + "}";
                #else
                return "{\"error\":\"Internal temp sensor only available on ESP32\"}";
                #endif
            }
        );

        registry.registerTool(
            "hall_sensor",
            "Read the ESP32 built-in hall effect sensor (detects magnetic fields)",
            "{\"type\":\"object\",\"properties\":{}}",
            [](const String& params) -> String {
                #ifdef ESP32
                int value = hallRead();
                return "{\"hall_value\":" + String(value) + "}";
                #else
                return "{\"error\":\"Hall sensor only available on ESP32\"}";
                #endif
            }
        );

        registry.registerTool(
            "touch_read",
            "Read a capacitive touch pin value (lower = touched)",
            "{\"type\":\"object\",\"properties\":{\"pin\":{\"type\":\"integer\",\"description\":\"Touch-capable GPIO pin (0,2,4,12-15,27,32,33 on ESP32)\"}},\"required\":[\"pin\"]}",
            [](const String& params) -> String {
                #ifdef ESP32
                JsonDocument doc;
                if (deserializeJson(doc, params)) return "{\"error\":\"Invalid params\"}";
                int pin = doc["pin"] | -1;
                if (pin < 0) return "{\"error\":\"Missing pin parameter\"}";
                int value = touchRead(pin);
                bool touched = value < 40;  // Threshold
                return "{\"pin\":" + String(pin) + ",\"value\":" + String(value) +
                       ",\"touched\":" + (touched ? "true" : "false") + "}";
                #else
                return "{\"error\":\"Touch pins only available on ESP32\"}";
                #endif
            }
        );

        registry.registerTool(
            "free_heap",
            "Get the current free heap memory in bytes",
            "{\"type\":\"object\",\"properties\":{}}",
            [](const String& params) -> String {
                uint32_t freeHeap = ESP.getFreeHeap();
                uint32_t totalHeap = ESP.getHeapSize();
                return "{\"free_heap\":" + String(freeHeap) +
                       ",\"total_heap\":" + String(totalHeap) +
                       ",\"used_percent\":" + String(100.0 - (freeHeap * 100.0 / totalHeap), 1) + "}";
            }
        );
    }
};

#endif // TINY_AGENT_SENSOR_TOOL_H
