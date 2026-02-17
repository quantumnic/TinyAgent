/**
 * TinyAgent - MQTT Tool
 * 
 * Publish messages to MQTT topics. Uses raw WiFiClient + minimal MQTT CONNECT/PUBLISH.
 * For a full MQTT client, consider integrating PubSubClient as an external dependency.
 */

#ifndef TINY_AGENT_MQTT_TOOL_H
#define TINY_AGENT_MQTT_TOOL_H

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "../ToolRegistry.h"

class MQTTTool {
public:
    /// Set the broker before registering tools
    static String brokerHost;
    static uint16_t brokerPort;
    static String clientId;

    static void configureBroker(const String& host, uint16_t port = 1883, const String& id = "TinyAgent") {
        brokerHost = host;
        brokerPort = port;
        clientId = id;
    }

    static void registerAll(ToolRegistry& registry) {
        registry.registerTool(
            "mqtt_publish",
            "Publish a message to an MQTT topic. Configure broker first with MQTTTool::configureBroker()",
            "{\"type\":\"object\",\"properties\":{\"topic\":{\"type\":\"string\",\"description\":\"MQTT topic\"},\"payload\":{\"type\":\"string\",\"description\":\"Message payload\"},\"host\":{\"type\":\"string\",\"description\":\"Broker host (optional, uses configured default)\"},\"port\":{\"type\":\"integer\",\"description\":\"Broker port (optional, default 1883)\"}},\"required\":[\"topic\",\"payload\"]}",
            [](const String& params) -> String {
                JsonDocument doc;
                if (deserializeJson(doc, params)) return "{\"error\":\"Invalid params\"}";

                String topic = doc["topic"].as<String>();
                String payload = doc["payload"].as<String>();
                String host = doc["host"] | brokerHost;
                uint16_t port = doc["port"] | brokerPort;

                if (topic.length() == 0) return "{\"error\":\"Missing topic\"}";
                if (host.length() == 0) return "{\"error\":\"No MQTT broker configured. Call MQTTTool::configureBroker() or pass host parameter.\"}";

                // Minimal MQTT CONNECT + PUBLISH using raw TCP
                WiFiClient tcp;
                if (!tcp.connect(host.c_str(), port)) {
                    return "{\"error\":\"Failed to connect to MQTT broker " + host + ":" + String(port) + "\"}";
                }

                // MQTT CONNECT packet
                String cid = clientId;
                uint8_t connectPacket[256];
                int pos = 0;

                // Fixed header
                connectPacket[pos++] = 0x10; // CONNECT
                // Remaining length placeholder
                int lenPos = pos++;

                // Variable header: Protocol Name
                connectPacket[pos++] = 0x00; connectPacket[pos++] = 0x04;
                connectPacket[pos++] = 'M'; connectPacket[pos++] = 'Q';
                connectPacket[pos++] = 'T'; connectPacket[pos++] = 'T';
                // Protocol Level (4 = MQTT 3.1.1)
                connectPacket[pos++] = 0x04;
                // Connect Flags (Clean Session)
                connectPacket[pos++] = 0x02;
                // Keep Alive (60s)
                connectPacket[pos++] = 0x00; connectPacket[pos++] = 0x3C;

                // Client ID
                connectPacket[pos++] = (cid.length() >> 8) & 0xFF;
                connectPacket[pos++] = cid.length() & 0xFF;
                memcpy(connectPacket + pos, cid.c_str(), cid.length());
                pos += cid.length();

                // Set remaining length
                connectPacket[lenPos] = pos - 2;

                tcp.write(connectPacket, pos);
                tcp.flush();

                // Wait for CONNACK
                unsigned long start = millis();
                while (tcp.available() < 4 && millis() - start < 3000) delay(10);

                if (tcp.available() < 4) {
                    tcp.stop();
                    return "{\"error\":\"MQTT CONNACK timeout\"}";
                }
                uint8_t connack[4];
                tcp.read(connack, 4);
                if (connack[3] != 0x00) {
                    tcp.stop();
                    return "{\"error\":\"MQTT CONNECT rejected, code: " + String(connack[3]) + "\"}";
                }

                // MQTT PUBLISH packet
                uint8_t publishPacket[1024];
                pos = 0;
                publishPacket[pos++] = 0x30; // PUBLISH (QoS 0)

                int topicLen = topic.length();
                int payloadLen = payload.length();
                int remainingLen = 2 + topicLen + payloadLen;

                // Encode remaining length (variable length encoding)
                int rl = remainingLen;
                do {
                    uint8_t encodedByte = rl % 128;
                    rl /= 128;
                    if (rl > 0) encodedByte |= 0x80;
                    publishPacket[pos++] = encodedByte;
                } while (rl > 0);

                // Topic
                publishPacket[pos++] = (topicLen >> 8) & 0xFF;
                publishPacket[pos++] = topicLen & 0xFF;
                memcpy(publishPacket + pos, topic.c_str(), topicLen);
                pos += topicLen;

                // Payload
                memcpy(publishPacket + pos, payload.c_str(), payloadLen);
                pos += payloadLen;

                tcp.write(publishPacket, pos);
                tcp.flush();

                // DISCONNECT
                uint8_t disconnectPacket[] = {0xE0, 0x00};
                tcp.write(disconnectPacket, 2);
                tcp.flush();
                tcp.stop();

                return "{\"success\":true,\"topic\":\"" + topic + "\",\"payload_length\":" + String(payloadLen) + "}";
            }
        );
    }
};

// Static member initialization
String MQTTTool::brokerHost = "";
uint16_t MQTTTool::brokerPort = 1883;
String MQTTTool::clientId = "TinyAgent";

#endif // TINY_AGENT_MQTT_TOOL_H
