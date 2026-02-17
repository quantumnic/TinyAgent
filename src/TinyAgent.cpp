/**
 * TinyAgent - Main agent implementation
 * 
 * Implements the ReAct loop: Observe → Think → Act
 */

#include "TinyAgent.h"
#include <ArduinoJson.h>

TinyAgent::TinyAgent()
    : _state(AgentState::IDLE) {}

void TinyAgent::setSystemPrompt(const String& prompt) {
    _config.systemPrompt = prompt;
}

void TinyAgent::setMaxIterations(uint8_t max) {
    _config.maxIterations = max;
}

void TinyAgent::setVerbose(bool verbose) {
    _config.verbose = verbose;
}

LLMClient& TinyAgent::getLLMClient() {
    return _llmClient;
}

ToolRegistry& TinyAgent::getToolRegistry() {
    return _toolRegistry;
}

void TinyAgent::registerBuiltinTools() {
    _toolRegistry.registerBuiltins();
}

AgentState TinyAgent::getState() const {
    return _state;
}

String TinyAgent::_formatToolDescriptions() {
    String desc = "Available tools:\n";
    auto& tools = _toolRegistry.getTools();
    for (size_t i = 0; i < tools.size(); i++) {
        desc += "- " + tools[i].name + ": " + tools[i].description + "\n";
        desc += "  Parameters: " + tools[i].parametersSchema + "\n";
    }
    return desc;
}

String TinyAgent::_buildPrompt(const String& userQuery, const String& history) {
    String prompt = _config.systemPrompt + "\n\n";
    prompt += _formatToolDescriptions();
    prompt += "\nTo use a tool, respond with EXACTLY this format:\n";
    prompt += "TOOL_CALL: {\"tool\": \"tool_name\", \"params\": {<parameters>}}\n\n";
    prompt += "When you have the final answer, respond with:\n";
    prompt += "FINAL_ANSWER: <your answer>\n\n";

    if (history.length() > 0) {
        prompt += "Conversation so far:\n" + history + "\n";
    }

    prompt += "User: " + userQuery + "\n";
    prompt += "Assistant:";
    return prompt;
}

bool TinyAgent::_parseToolCall(const String& response, String& toolName, String& toolParams) {
    int idx = response.indexOf("TOOL_CALL:");
    if (idx < 0) return false;

    String jsonPart = response.substring(idx + 10);
    jsonPart.trim();

    // Find the JSON object
    int braceStart = jsonPart.indexOf('{');
    if (braceStart < 0) return false;

    // Find matching closing brace
    int depth = 0;
    int braceEnd = -1;
    for (int i = braceStart; i < (int)jsonPart.length(); i++) {
        if (jsonPart[i] == '{') depth++;
        else if (jsonPart[i] == '}') {
            depth--;
            if (depth == 0) { braceEnd = i; break; }
        }
    }
    if (braceEnd < 0) return false;

    String jsonStr = jsonPart.substring(braceStart, braceEnd + 1);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, jsonStr);
    if (err) {
        if (_config.verbose) {
            Serial.print("[TinyAgent] JSON parse error: ");
            Serial.println(err.c_str());
        }
        return false;
    }

    toolName = doc["tool"].as<String>();
    // Serialize params back to string
    String params;
    serializeJson(doc["params"], params);
    toolParams = params;

    return toolName.length() > 0;
}

AgentResult TinyAgent::run(const String& userQuery) {
    AgentResult result;
    result.success = false;
    result.iterationsUsed = 0;
    result.lastState = AgentState::IDLE;

    if (_config.verbose) {
        Serial.println("╔══════════════════════════════════════╗");
        Serial.println("║         TinyAgent ReAct Loop         ║");
        Serial.println("╚══════════════════════════════════════╝");
        Serial.print("[TinyAgent] Query: ");
        Serial.println(userQuery);
    }

    String history = "";
    _state = AgentState::OBSERVING;

    for (uint8_t i = 0; i < _config.maxIterations; i++) {
        result.iterationsUsed = i + 1;

        if (_config.verbose) {
            Serial.printf("\n── Iteration %d/%d ──\n", i + 1, _config.maxIterations);
        }

        // THINK: Send to LLM
        _state = AgentState::THINKING;
        if (_config.verbose) Serial.println("[Think] Calling LLM...");

        String prompt = _buildPrompt(userQuery, history);
        String llmResponse = _llmClient.complete(prompt);

        if (llmResponse.length() == 0) {
            if (_config.verbose) Serial.println("[Think] ERROR: Empty LLM response");
            _state = AgentState::ERROR;
            result.lastState = AgentState::ERROR;
            result.finalAnswer = "Error: LLM returned empty response";
            return result;
        }

        if (_config.verbose) {
            Serial.print("[Think] LLM response: ");
            Serial.println(llmResponse.substring(0, 200));
        }

        // Check for FINAL_ANSWER
        int finalIdx = llmResponse.indexOf("FINAL_ANSWER:");
        if (finalIdx >= 0) {
            String answer = llmResponse.substring(finalIdx + 13);
            answer.trim();
            _state = AgentState::FINISHED;
            result.success = true;
            result.finalAnswer = answer;
            result.lastState = AgentState::FINISHED;

            if (_config.verbose) {
                Serial.println("\n[TinyAgent] ✓ Final answer reached");
                Serial.print("[TinyAgent] Answer: ");
                Serial.println(answer);
            }
            return result;
        }

        // ACT: Parse and execute tool call
        _state = AgentState::ACTING;
        String toolName, toolParams;
        if (_parseToolCall(llmResponse, toolName, toolParams)) {
            if (_config.verbose) {
                Serial.print("[Act] Tool: ");
                Serial.print(toolName);
                Serial.print(" Params: ");
                Serial.println(toolParams);
            }

            String toolResult = _toolRegistry.executeTool(toolName, toolParams);

            if (_config.verbose) {
                Serial.print("[Act] Result: ");
                Serial.println(toolResult.substring(0, 200));
            }

            // Append to history for next iteration
            history += "Assistant: " + llmResponse + "\n";
            history += "Tool Result (" + toolName + "): " + toolResult + "\n";
        } else {
            // No tool call and no final answer — treat response as final answer
            _state = AgentState::FINISHED;
            result.success = true;
            result.finalAnswer = llmResponse;
            result.lastState = AgentState::FINISHED;
            return result;
        }

        _state = AgentState::OBSERVING;
    }

    // Max iterations reached
    if (_config.verbose) {
        Serial.println("[TinyAgent] Max iterations reached");
    }
    _state = AgentState::ERROR;
    result.lastState = AgentState::ERROR;
    result.finalAnswer = "Max iterations (" + String(_config.maxIterations) + ") reached without final answer. Last history:\n" + history;
    return result;
}
