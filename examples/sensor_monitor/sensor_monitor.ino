/**
 * TinyAgent - Sensor Monitor Example
 * 
 * Periodically asks the agent to check sensors and report findings.
 * Demonstrates continuous agent operation with custom tools.
 */

#include <WiFi.h>
#include <TinyAgent.h>

const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASS = "your-password";
const char* LLM_ENDPOINT = "https://api.openai.com/v1/chat/completions";
const char* LLM_API_KEY  = "sk-your-api-key";
const char* LLM_MODEL    = "gpt-4o-mini";

TinyAgent agent;

// Simulated sensor data (replace with real sensor reads)
float lastTemperature = 0;
float lastHumidity = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== TinyAgent Sensor Monitor ===\n");

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Connected!");

    agent.getLLMClient().configure(LLM_ENDPOINT, LLM_API_KEY, LLM_MODEL);

    agent.setSystemPrompt(
        "You are a sensor monitoring agent on an ESP32. "
        "Your job is to read sensors, analyze the data, and provide a brief status report. "
        "If temperature is above 30°C or humidity above 80%, flag it as a warning."
    );
    agent.setMaxIterations(3);

    // Register built-in tools
    agent.registerBuiltinTools();

    // Register a custom temperature/humidity tool (simulated)
    agent.getToolRegistry().registerTool(
        "read_environment",
        "Read temperature (°C) and humidity (%) from the environment sensor",
        "{\"type\":\"object\",\"properties\":{}}",
        [](const String& params) -> String {
            // In real use, read from DHT22, BME280, etc.
            float temp = 22.5 + random(-50, 50) / 10.0;
            float hum = 55.0 + random(-100, 100) / 10.0;
            lastTemperature = temp;
            lastHumidity = hum;
            return "{\"temperature_c\":" + String(temp, 1) +
                   ",\"humidity_percent\":" + String(hum, 1) +
                   ",\"sensor\":\"simulated\"}";
        }
    );

    // Register uptime tool
    agent.getToolRegistry().registerTool(
        "get_uptime",
        "Get the device uptime in seconds",
        "{\"type\":\"object\",\"properties\":{}}",
        [](const String& params) -> String {
            return "{\"uptime_seconds\":" + String(millis() / 1000) +
                   ",\"uptime_minutes\":" + String(millis() / 60000.0, 1) + "}";
        }
    );

    Serial.println("Monitor ready! Tools: " + String(agent.getToolRegistry().count()));
}

void loop() {
    Serial.println("\n--- Sensor Check ---");

    AgentResult result = agent.run(
        "Check the environment sensor and internal temperature. Give a brief status report."
    );

    if (result.success) {
        Serial.println("📊 Report: " + result.finalAnswer);
    } else {
        Serial.println("❌ Agent failed: " + result.finalAnswer);
    }

    // Wait 60 seconds before next check
    Serial.println("Next check in 60 seconds...");
    delay(60000);
}
