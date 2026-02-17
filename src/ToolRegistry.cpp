/**
 * TinyAgent - Tool Registry implementation
 */

#include "ToolRegistry.h"
#include "tools/GPIOTool.h"
#include "tools/SensorTool.h"
#include "tools/HTTPTool.h"
#include "tools/MQTTTool.h"

ToolRegistry::ToolRegistry() {}

void ToolRegistry::registerTool(const String& name, const String& description,
                                 const String& parametersSchema, ToolCallback callback) {
    Tool tool;
    tool.name = name;
    tool.description = description;
    tool.parametersSchema = parametersSchema;
    tool.callback = callback;
    _tools.push_back(tool);

    Serial.print("[ToolRegistry] Registered: ");
    Serial.println(name);
}

void ToolRegistry::registerBuiltins() {
    GPIOTool::registerAll(*this);
    SensorTool::registerAll(*this);
    HTTPTool::registerAll(*this);
    MQTTTool::registerAll(*this);
}

String ToolRegistry::executeTool(const String& name, const String& params) {
    for (auto& tool : _tools) {
        if (tool.name == name) {
            if (tool.callback) {
                return tool.callback(params);
            }
            return "{\"error\": \"Tool has no callback\"}";
        }
    }
    return "{\"error\": \"Unknown tool: " + name + "\"}";
}

bool ToolRegistry::hasTool(const String& name) const {
    for (const auto& tool : _tools) {
        if (tool.name == name) return true;
    }
    return false;
}

const std::vector<Tool>& ToolRegistry::getTools() const {
    return _tools;
}

size_t ToolRegistry::count() const {
    return _tools.size();
}
