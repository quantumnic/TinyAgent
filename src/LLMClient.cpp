/**
 * TinyAgent - LLM Client implementation
 */

#include "LLMClient.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

LLMClient::LLMClient()
    : _lastStatusCode(0) {}

void LLMClient::configure(const String& endpoint, const String& apiKey, const String& model) {
    _config.endpoint = endpoint;
    _config.apiKey = apiKey;
    _config.model = model;
}

void LLMClient::setTemperature(float temp) {
    _config.temperature = temp;
}

void LLMClient::setMaxTokens(uint16_t tokens) {
    _config.maxTokens = tokens;
}

void LLMClient::setTimeout(uint32_t ms) {
    _config.timeoutMs = ms;
}

int LLMClient::getLastStatusCode() const {
    return _lastStatusCode;
}

String LLMClient::getLastError() const {
    return _lastError;
}

String LLMClient::_buildRequestBody(const String& prompt) {
    JsonDocument doc;
    doc["model"] = _config.model;
    doc["temperature"] = _config.temperature;
    doc["max_tokens"] = _config.maxTokens;

    JsonArray messages = doc["messages"].to<JsonArray>();

    JsonObject msg = messages.add<JsonObject>();
    msg["role"] = "user";
    msg["content"] = prompt;

    String body;
    serializeJson(doc, body);
    return body;
}

String LLMClient::_parseResponse(const String& responseBody) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, responseBody);

    if (err) {
        _lastError = "JSON parse error: " + String(err.c_str());
        Serial.println("[LLMClient] " + _lastError);
        return "";
    }

    // OpenAI-compatible format
    if (doc["choices"].is<JsonArray>() && doc["choices"].size() > 0) {
        String content = doc["choices"][0]["message"]["content"].as<String>();
        if (content.length() > 0) return content;

        // Fallback: some APIs use "text" field
        content = doc["choices"][0]["text"].as<String>();
        if (content.length() > 0) return content;
    }

    // Ollama format
    if (doc["response"].is<String>()) {
        return doc["response"].as<String>();
    }

    _lastError = "Unexpected response format";
    Serial.println("[LLMClient] " + _lastError);
    return "";
}

String LLMClient::complete(const String& prompt) {
    _lastError = "";
    _lastStatusCode = 0;

    if (_config.endpoint.length() == 0) {
        _lastError = "No endpoint configured";
        Serial.println("[LLMClient] ERROR: " + _lastError);
        return "";
    }

    HTTPClient http;
    WiFiClientSecure secureClient;
    WiFiClient plainClient;

    bool isHttps = _config.endpoint.startsWith("https");

    if (isHttps) {
        secureClient.setInsecure(); // Skip cert verification for simplicity
        http.begin(secureClient, _config.endpoint);
    } else {
        http.begin(plainClient, _config.endpoint);
    }

    http.setTimeout(_config.timeoutMs);
    http.addHeader("Content-Type", "application/json");

    if (_config.apiKey.length() > 0) {
        http.addHeader("Authorization", "Bearer " + _config.apiKey);
    }

    String requestBody = _buildRequestBody(prompt);

    Serial.printf("[LLMClient] POST %s (%d bytes)\n",
                  _config.endpoint.c_str(), requestBody.length());

    _lastStatusCode = http.POST(requestBody);

    if (_lastStatusCode <= 0) {
        _lastError = "HTTP error: " + String(_lastStatusCode) + " " + http.errorToString(_lastStatusCode);
        Serial.println("[LLMClient] " + _lastError);
        http.end();
        return "";
    }

    if (_lastStatusCode != 200) {
        String errBody = http.getString();
        _lastError = "HTTP " + String(_lastStatusCode) + ": " + errBody.substring(0, 200);
        Serial.println("[LLMClient] " + _lastError);
        http.end();
        return "";
    }

    String responseBody = http.getString();
    http.end();

    return _parseResponse(responseBody);
}

String LLMClient::chatComplete(const String& messagesJson) {
    _lastError = "";
    _lastStatusCode = 0;

    JsonDocument doc;
    doc["model"] = _config.model;
    doc["temperature"] = _config.temperature;
    doc["max_tokens"] = _config.maxTokens;

    // Parse provided messages and add to request
    JsonDocument msgDoc;
    DeserializationError err = deserializeJson(msgDoc, messagesJson);
    if (err) {
        _lastError = "Invalid messages JSON";
        return "";
    }
    doc["messages"] = msgDoc.as<JsonArray>();

    String body;
    serializeJson(doc, body);

    HTTPClient http;
    WiFiClientSecure secureClient;
    WiFiClient plainClient;

    bool isHttps = _config.endpoint.startsWith("https");
    if (isHttps) {
        secureClient.setInsecure();
        http.begin(secureClient, _config.endpoint);
    } else {
        http.begin(plainClient, _config.endpoint);
    }

    http.setTimeout(_config.timeoutMs);
    http.addHeader("Content-Type", "application/json");
    if (_config.apiKey.length() > 0) {
        http.addHeader("Authorization", "Bearer " + _config.apiKey);
    }

    _lastStatusCode = http.POST(body);
    if (_lastStatusCode != 200) {
        _lastError = "HTTP " + String(_lastStatusCode);
        http.end();
        return "";
    }

    String responseBody = http.getString();
    http.end();
    return _parseResponse(responseBody);
}
