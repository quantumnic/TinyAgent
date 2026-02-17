/**
 * TinyAgent - LLM Client
 * 
 * HTTP client for communicating with LLM APIs (OpenAI, Ollama, compatible).
 */

#ifndef TINY_AGENT_LLM_CLIENT_H
#define TINY_AGENT_LLM_CLIENT_H

#include <Arduino.h>

enum class LLMProvider {
    OPENAI_COMPATIBLE,  // Works for OpenAI, Ollama, LM Studio, etc.
};

struct LLMConfig {
    String endpoint = "https://api.openai.com/v1/chat/completions";
    String apiKey = "";
    String model = "gpt-4o-mini";
    float temperature = 0.7;
    uint16_t maxTokens = 512;
    uint32_t timeoutMs = 30000;
    LLMProvider provider = LLMProvider::OPENAI_COMPATIBLE;
};

class LLMClient {
public:
    LLMClient();

    /// Configure the LLM endpoint
    void configure(const String& endpoint, const String& apiKey, const String& model);
    void setTemperature(float temp);
    void setMaxTokens(uint16_t tokens);
    void setTimeout(uint32_t ms);

    /// Send a completion request and return the response text
    String complete(const String& prompt);

    /// Send a chat completion with messages array
    String chatComplete(const String& messagesJson);

    /// Get last HTTP status code
    int getLastStatusCode() const;

    /// Get last error message
    String getLastError() const;

private:
    LLMConfig _config;
    int _lastStatusCode;
    String _lastError;

    String _buildRequestBody(const String& prompt);
    String _parseResponse(const String& responseBody);
};

#endif // TINY_AGENT_LLM_CLIENT_H
