/**
 * TinyAgent - Lightweight AI Agent Framework for ESP32/Arduino
 * 
 * Main agent class implementing the ReAct (Observe → Think → Act) loop.
 * 
 * MIT License - Copyright (c) 2026 Nicola Spieser
 */

#ifndef TINY_AGENT_H
#define TINY_AGENT_H

#include <Arduino.h>
#include "LLMClient.h"
#include "ToolRegistry.h"

enum class AgentState {
    IDLE,
    OBSERVING,
    THINKING,
    ACTING,
    FINISHED,
    ERROR
};

struct AgentConfig {
    String systemPrompt = "You are a helpful IoT assistant running on an ESP32 microcontroller. "
                          "You have access to tools to interact with hardware and the network. "
                          "Use tools when needed to accomplish tasks. "
                          "Always respond with either a tool call or a final answer.";
    uint8_t maxIterations = 5;
    uint32_t thinkTimeoutMs = 30000;
    bool verbose = true;
};

struct AgentResult {
    bool success;
    String finalAnswer;
    uint8_t iterationsUsed;
    AgentState lastState;
};

class TinyAgent {
public:
    TinyAgent();

    /// Configure the agent
    void setSystemPrompt(const String& prompt);
    void setMaxIterations(uint8_t max);
    void setVerbose(bool verbose);

    /// Access sub-components
    LLMClient& getLLMClient();
    ToolRegistry& getToolRegistry();

    /// Register all built-in tools (GPIO, Sensor, HTTP, MQTT)
    void registerBuiltinTools();

    /// Run the agent with a user query (blocking ReAct loop)
    AgentResult run(const String& userQuery);

    /// Get current state
    AgentState getState() const;

private:
    AgentConfig _config;
    AgentState _state;
    LLMClient _llmClient;
    ToolRegistry _toolRegistry;

    /// Internal ReAct step methods
    String _buildPrompt(const String& userQuery, const String& history);
    bool _parseToolCall(const String& response, String& toolName, String& toolParams);
    String _formatToolDescriptions();
};

#endif // TINY_AGENT_H
