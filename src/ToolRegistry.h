/**
 * TinyAgent - Tool Registry
 * 
 * Manages registration and execution of agent tools.
 */

#ifndef TINY_AGENT_TOOL_REGISTRY_H
#define TINY_AGENT_TOOL_REGISTRY_H

#include <Arduino.h>
#include <functional>
#include <vector>

using ToolCallback = std::function<String(const String& params)>;

struct Tool {
    String name;
    String description;
    String parametersSchema;  // JSON Schema string
    ToolCallback callback;
};

class ToolRegistry {
public:
    ToolRegistry();

    /// Register a custom tool
    void registerTool(const String& name, const String& description,
                      const String& parametersSchema, ToolCallback callback);

    /// Register all built-in tools (GPIO, Sensor, HTTP, MQTT)
    void registerBuiltins();

    /// Execute a tool by name
    String executeTool(const String& name, const String& params);

    /// Check if a tool exists
    bool hasTool(const String& name) const;

    /// Get all registered tools
    const std::vector<Tool>& getTools() const;

    /// Get tool count
    size_t count() const;

private:
    std::vector<Tool> _tools;
};

#endif // TINY_AGENT_TOOL_REGISTRY_H
