/**
 * TinyAgent - GPIO Tool
 * 
 * Digital and analog GPIO read/write operations.
 */

#ifndef TINY_AGENT_GPIO_TOOL_H
#define TINY_AGENT_GPIO_TOOL_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../ToolRegistry.h"

class GPIOTool {
public:
    static void registerAll(ToolRegistry& registry) {
        registry.registerTool(
            "gpio_read",
            "Read the digital state (HIGH/LOW) of a GPIO pin",
            "{\"type\":\"object\",\"properties\":{\"pin\":{\"type\":\"integer\",\"description\":\"GPIO pin number\"}},\"required\":[\"pin\"]}",
            [](const String& params) -> String {
                JsonDocument doc;
                if (deserializeJson(doc, params)) return "{\"error\":\"Invalid params\"}";
                int pin = doc["pin"] | -1;
                if (pin < 0) return "{\"error\":\"Missing pin parameter\"}";
                pinMode(pin, INPUT);
                int value = digitalRead(pin);
                return "{\"pin\":" + String(pin) + ",\"value\":" + String(value) +
                       ",\"state\":\"" + (value == HIGH ? "HIGH" : "LOW") + "\"}";
            }
        );

        registry.registerTool(
            "gpio_write",
            "Write a digital value (HIGH/LOW) to a GPIO pin",
            "{\"type\":\"object\",\"properties\":{\"pin\":{\"type\":\"integer\",\"description\":\"GPIO pin number\"},\"value\":{\"type\":\"integer\",\"description\":\"0 for LOW, 1 for HIGH\"}},\"required\":[\"pin\",\"value\"]}",
            [](const String& params) -> String {
                JsonDocument doc;
                if (deserializeJson(doc, params)) return "{\"error\":\"Invalid params\"}";
                int pin = doc["pin"] | -1;
                int value = doc["value"] | -1;
                if (pin < 0) return "{\"error\":\"Missing pin parameter\"}";
                if (value < 0 || value > 1) return "{\"error\":\"Value must be 0 or 1\"}";
                pinMode(pin, OUTPUT);
                digitalWrite(pin, value);
                return "{\"pin\":" + String(pin) + ",\"value\":" + String(value) +
                       ",\"state\":\"" + (value == HIGH ? "HIGH" : "LOW") + "\",\"success\":true}";
            }
        );

        registry.registerTool(
            "analog_read",
            "Read the analog value (0-4095) from an ADC pin",
            "{\"type\":\"object\",\"properties\":{\"pin\":{\"type\":\"integer\",\"description\":\"ADC-capable GPIO pin number (e.g., 32-39 on ESP32)\"}},\"required\":[\"pin\"]}",
            [](const String& params) -> String {
                JsonDocument doc;
                if (deserializeJson(doc, params)) return "{\"error\":\"Invalid params\"}";
                int pin = doc["pin"] | -1;
                if (pin < 0) return "{\"error\":\"Missing pin parameter\"}";
                int value = analogRead(pin);
                float voltage = (value / 4095.0) * 3.3;
                return "{\"pin\":" + String(pin) + ",\"raw\":" + String(value) +
                       ",\"voltage\":" + String(voltage, 2) + "}";
            }
        );
    }
};

#endif // TINY_AGENT_GPIO_TOOL_H
