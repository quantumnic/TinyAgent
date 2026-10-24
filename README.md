<p align="center">
  <img src="docs/logo-placeholder.png" alt="TinyAgent Logo" width="200">
  <br>
  <em>🤖 Run AI Agents on Microcontrollers</em>
</p>

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="License"></a>
  <a href="https://www.arduino.cc/"><img src="https://img.shields.io/badge/platform-ESP32%20%7C%20Arduino-green.svg" alt="Platform"></a>
  <a href="https://registry.platformio.org/libraries/quantumnic/TinyAgent"><img src="https://img.shields.io/badge/version-0.1.0-orange.svg" alt="Version"></a>
  <a href="https://github.com/quantumnic/TinyAgent/issues"><img src="https://img.shields.io/github/issues/quantumnic/TinyAgent.svg" alt="Issues"></a>
</p>

---

# TinyAgent

**TinyAgent** is a lightweight AI agent framework designed to run on ESP32 and Arduino-compatible microcontrollers. It brings the power of LLM-driven autonomous agents to the embedded world — with minimal memory footprint and maximum flexibility.

## Why TinyAgent?

Modern AI agents (LangChain, AutoGPT, etc.) require powerful servers. But what if your IoT device could **think for itself**? TinyAgent bridges the gap:

- 🧠 **ReAct Loop** — Observe → Think → Act cycle running on your ESP32
- 🌐 **LLM Integration** — Call OpenAI, Ollama, or any compatible API over WiFi
- 🔧 **Tool System** — GPIO, sensors, HTTP requests, MQTT — all as agent tools
- 📦 **Tiny Footprint** — Designed for constrained environments (~20KB RAM usage)
- ⚡ **Easy Setup** — Arduino IDE & PlatformIO compatible

## Features

- **Multiple LLM backends**: OpenAI API, Ollama, any OpenAI-compatible endpoint
- **Built-in tools**: GPIO read/write, analog sensors, HTTP requests, MQTT publish
- **Custom tools**: Register your own tools with a simple callback API
- **Streaming support**: Process LLM responses as they arrive
- **Configurable**: System prompts, max iterations, tool selection — all runtime-configurable
- **Memory efficient**: Reuses buffers, streams JSON parsing, minimal allocations

## Architecture

```
┌─────────────────────────────────────────────────┐
│                   TinyAgent                      │
│                                                  │
│  ┌───────────┐    ┌──────────┐    ┌───────────┐ │
│  │  Observe   │───▶│  Think   │───▶│    Act    │ │
│  │ (sensors)  │    │  (LLM)   │    │  (tools)  │ │
│  └───────────┘    └──────────┘    └───────────┘ │
│       ▲                                  │       │
│       └──────────────────────────────────┘       │
│                  ReAct Loop                      │
├─────────────────────────────────────────────────┤
│                 Tool Registry                    │
│  ┌────────┐ ┌────────┐ ┌───────┐ ┌──────────┐  │
│  │  GPIO  │ │ Sensor │ │ HTTP  │ │   MQTT   │  │
│  └────────┘ └────────┘ └───────┘ └──────────┘  │
├─────────────────────────────────────────────────┤
│               LLM Client (WiFi)                  │
│         OpenAI  │  Ollama  │  Custom             │
└─────────────────────────────────────────────────┘
```

## Quick Start

### Installation

**PlatformIO** (recommended):
```ini
; platformio.ini
[env:esp32]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    quantumnic/TinyAgent
    bblanchon/ArduinoJson@^7.0.0
```

**Arduino IDE**:
1. Download this repository as ZIP
2. Sketch → Include Library → Add .ZIP Library
3. Install dependency: ArduinoJson by Benoît Blanchon

### Minimal Example

```cpp
#include <WiFi.h>
#include <TinyAgent.h>

TinyAgent agent;

void setup() {
    Serial.begin(115200);
    WiFi.begin("your-ssid", "your-password");
    while (WiFi.status() != WL_CONNECTED) delay(500);

    // Configure LLM
    agent.getLLMClient().configure("https://api.openai.com/v1/chat/completions",
                                   "your-api-key", "gpt-4o-mini");

    // Set agent behavior
    agent.setSystemPrompt("You are a helpful IoT assistant. Use tools to interact with hardware.");
    agent.setMaxIterations(5);

    // Register built-in tools
    agent.registerBuiltinTools();

    // Run the agent
    agent.run("What is the current temperature reading from the analog sensor on pin 34?");
}

void loop() {
    // Agent runs in setup(), or call agent.run() here for continuous operation
    delay(10000);
}
```

## Built-in Tools

| Tool | Description | Example |
|------|-------------|---------|
| `gpio_read` | Read digital pin state | `{"pin": 13}` |
| `gpio_write` | Write digital pin state | `{"pin": 2, "value": 1}` |
| `analog_read` | Read analog value (ADC) | `{"pin": 34}` |
| `http_get` | Make HTTP GET request | `{"url": "http://api.example.com/data"}` |
| `http_post` | Make HTTP POST request | `{"url": "...", "body": "..."}` |
| `mqtt_publish` | Publish MQTT message | `{"topic": "home/temp", "payload": "23.5"}` |

### Custom Tools

```cpp
agent.getToolRegistry().registerTool(
    "read_dht",
    "Read temperature and humidity from DHT22 sensor",
    "{\"type\":\"object\",\"properties\":{}}",
    [](const String& params) -> String {
        float temp = dht.readTemperature();
        float hum = dht.readHumidity();
        return "{\"temperature\":" + String(temp) + ",\"humidity\":" + String(hum) + "}";
    }
);
```

## Examples

- **[basic_agent](examples/basic_agent/)** — Minimal agent setup with GPIO tools
- **[sensor_monitor](examples/sensor_monitor/)** — Autonomous sensor monitoring and reporting
- **[home_automation](examples/home_automation/)** — MQTT-based home automation agent

## Roadmap

- [x] Core ReAct loop
- [x] OpenAI API integration
- [x] Built-in GPIO and sensor tools
- [x] MQTT tool
- [ ] Ollama native support
- [ ] Conversation memory (SPIFFS/LittleFS persistence)
- [ ] Multi-agent communication
- [ ] BLE tool
- [ ] OTA update tool
- [ ] Web dashboard for monitoring
- [ ] Function calling (structured tool use)
- [ ] ESP-IDF native support (no Arduino)

## Requirements

- ESP32 board (ESP32, ESP32-S2, ESP32-S3, ESP32-C3)
- Arduino framework
- ArduinoJson >= 7.0.0
- WiFi connectivity
- LLM API endpoint (OpenAI, Ollama, or compatible)

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

MIT License — see [LICENSE](LICENSE) for details.

---

<p align="center">
  Made with ⚡ for the embedded AI community
</p>
