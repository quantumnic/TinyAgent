/**
 * TinyAgent - Home Automation Example
 * 
 * An agent that controls home devices via MQTT and GPIO,
 * responding to serial commands as natural language input.
 */

#include <WiFi.h>
#include <TinyAgent.h>
#include "src/tools/MQTTTool.h"

const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASS = "your-password";
const char* LLM_ENDPOINT = "https://api.openai.com/v1/chat/completions";
const char* LLM_API_KEY  = "sk-your-api-key";
const char* LLM_MODEL    = "gpt-4o-mini";

// MQTT Broker
const char* MQTT_BROKER = "192.168.1.100";

// Device pins
const int RELAY_LIGHT   = 26;
const int RELAY_FAN     = 27;
const int PIR_SENSOR    = 14;
const int LIGHT_SENSOR  = 34;

TinyAgent agent;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== TinyAgent Home Automation ===\n");

    // Setup pins
    pinMode(RELAY_LIGHT, OUTPUT);
    pinMode(RELAY_FAN, OUTPUT);
    pinMode(PIR_SENSOR, INPUT);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Connected!");

    // Configure MQTT broker
    MQTTTool::configureBroker(MQTT_BROKER, 1883, "TinyAgent-Home");

    // Configure agent
    agent.getLLMClient().configure(LLM_ENDPOINT, LLM_API_KEY, LLM_MODEL);
    agent.getLLMClient().setMaxTokens(300);

    agent.setSystemPrompt(
        "You are a home automation agent running on an ESP32. You control:\n"
        "- Living room light: GPIO pin 26 (gpio_write, 1=ON, 0=OFF)\n"
        "- Ceiling fan: GPIO pin 27 (gpio_write, 1=ON, 0=OFF)\n"
        "- Motion detector: GPIO pin 14 (gpio_read, 1=motion detected)\n"
        "- Light level: analog pin 34 (analog_read, higher=brighter)\n"
        "- MQTT: publish status to home/livingroom/status\n\n"
        "Be helpful and confirm actions taken."
    );
    agent.setMaxIterations(5);

    // Register tools
    agent.registerBuiltinTools();

    // Custom: room status
    agent.getToolRegistry().registerTool(
        "room_status",
        "Get a summary of all room devices and sensors",
        "{\"type\":\"object\",\"properties\":{}}",
        [](const String& params) -> String {
            int light = digitalRead(RELAY_LIGHT);
            int fan = digitalRead(RELAY_FAN);
            int motion = digitalRead(PIR_SENSOR);
            int brightness = analogRead(LIGHT_SENSOR);
            float brightPct = (brightness / 4095.0) * 100.0;

            return "{\"light\":\"" + String(light ? "ON" : "OFF") +
                   "\",\"fan\":\"" + String(fan ? "ON" : "OFF") +
                   "\",\"motion\":" + String(motion ? "true" : "false") +
                   ",\"brightness_percent\":" + String(brightPct, 1) + "}";
        }
    );

    Serial.println("Home agent ready! Type commands via Serial...");
    Serial.println("Examples:");
    Serial.println("  Turn on the living room light");
    Serial.println("  What's the room status?");
    Serial.println("  It's getting dark, adjust the lights");
    Serial.println();
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        if (command.length() == 0) return;

        Serial.println("\n🗣️ You: " + command);

        AgentResult result = agent.run(command);

        if (result.success) {
            Serial.println("🤖 Agent: " + result.finalAnswer);
        } else {
            Serial.println("❌ Error: " + result.finalAnswer);
        }
        Serial.println();
    }
    delay(100);
}
