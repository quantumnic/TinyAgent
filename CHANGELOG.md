# Changelog

All notable changes to TinyAgent will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2026-02-17

### Added
- Core `TinyAgent` class with ReAct loop (Observe → Think → Act)
- `LLMClient` with OpenAI-compatible API support (OpenAI, Ollama, LM Studio)
- `ToolRegistry` for registering and executing agent tools
- Built-in tools:
  - `GPIOTool` — digital read/write, analog read
  - `SensorTool` — internal temperature, hall sensor, touch pins, free heap
  - `HTTPTool` — HTTP GET/POST requests
  - `MQTTTool` — MQTT publish with raw TCP (no external dependency)
- Example sketches: basic_agent, sensor_monitor, home_automation
- PlatformIO (`library.json`) and Arduino IDE (`library.properties`) support
- MIT License
