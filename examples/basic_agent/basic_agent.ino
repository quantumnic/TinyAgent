/**
 * TinyAgent - Basic Agent Example
 * 
 * Minimal setup: connects to WiFi, configures an LLM endpoint,
 * registers GPIO tools, and runs a simple query.
 */

#include <WiFi.h>
#include <TinyAgent.h>

// WiFi credentials
const char* WIFI_SSID = "your-ssid";
const char* WIFI_PASS = "your-password";

// LLM API configuration
const char* LLM_ENDPOINT = "https://api.openai.com/v1/chat/completions";
const char* LLM_API_KEY  = "sk-your-api-key";
const char* LLM_MODEL    = "gpt-4o-mini";

TinyAgent agent;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== TinyAgent Basic Example ===\n");

    // Connect to WiFi
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());

    // Configure LLM client
    agent.getLLMClient().configure(LLM_ENDPOINT, LLM_API_KEY, LLM_MODEL);
    agent.getLLMClient().setMaxTokens(256);

    // Configure agent
    agent.setSystemPrompt(
        "You are an IoT assistant on an ESP32. You can read and write GPIO pins. "
        "The built-in LED is on pin 2. Use tools to interact with hardware."
    );
    agent.setMaxIterations(3);

    // Register built-in tools
    agent.registerBuiltinTools();

    Serial.println("Agent ready! Tools registered: " + String(agent.getToolRegistry().count()));

    // Run the agent with a query
    AgentResult result = agent.run("Turn on the built-in LED (pin 2) and confirm it's on by reading it back.");

    // Print result
    Serial.println("\n=== Result ===");
    Serial.println("Success: " + String(result.success ? "YES" : "NO"));
    Serial.println("Iterations: " + String(result.iterationsUsed));
    Serial.println("Answer: " + result.finalAnswer);
}

void loop() {
    // Nothing to do here
    delay(10000);
}
