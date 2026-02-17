/**
 * TinyAgent - HTTP Tool
 * 
 * Make HTTP GET/POST requests from the agent.
 */

#ifndef TINY_AGENT_HTTP_TOOL_H
#define TINY_AGENT_HTTP_TOOL_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "../ToolRegistry.h"

class HTTPTool {
public:
    static void registerAll(ToolRegistry& registry) {
        registry.registerTool(
            "http_get",
            "Make an HTTP GET request to a URL and return the response",
            "{\"type\":\"object\",\"properties\":{\"url\":{\"type\":\"string\",\"description\":\"URL to request\"},\"max_length\":{\"type\":\"integer\",\"description\":\"Max response length (default 1024)\"}},\"required\":[\"url\"]}",
            [](const String& params) -> String {
                JsonDocument doc;
                if (deserializeJson(doc, params)) return "{\"error\":\"Invalid params\"}";

                String url = doc["url"].as<String>();
                int maxLen = doc["max_length"] | 1024;

                if (url.length() == 0) return "{\"error\":\"Missing url parameter\"}";

                HTTPClient http;
                WiFiClientSecure secureClient;
                WiFiClient plainClient;

                bool isHttps = url.startsWith("https");
                if (isHttps) {
                    secureClient.setInsecure();
                    http.begin(secureClient, url);
                } else {
                    http.begin(plainClient, url);
                }

                http.setTimeout(10000);
                int code = http.GET();

                String result;
                if (code > 0) {
                    String body = http.getString();
                    if ((int)body.length() > maxLen) body = body.substring(0, maxLen) + "...(truncated)";
                    result = "{\"status\":" + String(code) + ",\"body\":" + 
                             "\"" + body + "\"}";
                    // Proper JSON escaping
                    JsonDocument resDoc;
                    resDoc["status"] = code;
                    resDoc["body"] = body;
                    result = "";
                    serializeJson(resDoc, result);
                } else {
                    result = "{\"error\":\"HTTP error " + String(code) + ": " + http.errorToString(code) + "\"}";
                }

                http.end();
                return result;
            }
        );

        registry.registerTool(
            "http_post",
            "Make an HTTP POST request with a body",
            "{\"type\":\"object\",\"properties\":{\"url\":{\"type\":\"string\",\"description\":\"URL to request\"},\"body\":{\"type\":\"string\",\"description\":\"Request body\"},\"content_type\":{\"type\":\"string\",\"description\":\"Content-Type header (default application/json)\"}},\"required\":[\"url\",\"body\"]}",
            [](const String& params) -> String {
                JsonDocument doc;
                if (deserializeJson(doc, params)) return "{\"error\":\"Invalid params\"}";

                String url = doc["url"].as<String>();
                String body = doc["body"].as<String>();
                String contentType = doc["content_type"] | "application/json";

                if (url.length() == 0) return "{\"error\":\"Missing url parameter\"}";

                HTTPClient http;
                WiFiClientSecure secureClient;
                WiFiClient plainClient;

                bool isHttps = url.startsWith("https");
                if (isHttps) {
                    secureClient.setInsecure();
                    http.begin(secureClient, url);
                } else {
                    http.begin(plainClient, url);
                }

                http.setTimeout(10000);
                http.addHeader("Content-Type", contentType);
                int code = http.POST(body);

                JsonDocument resDoc;
                if (code > 0) {
                    String respBody = http.getString();
                    if (respBody.length() > 1024) respBody = respBody.substring(0, 1024) + "...(truncated)";
                    resDoc["status"] = code;
                    resDoc["body"] = respBody;
                } else {
                    resDoc["error"] = "HTTP error " + String(code) + ": " + http.errorToString(code);
                }

                http.end();
                String result;
                serializeJson(resDoc, result);
                return result;
            }
        );
    }
};

#endif // TINY_AGENT_HTTP_TOOL_H
