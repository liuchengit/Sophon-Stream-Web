#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>

namespace sophon {
namespace plugins {

struct PluginInfo {
    std::string name;
    std::string version;
    std::string path;
    std::string signature;
    bool loaded;
};

class PluginLoader {
public:
    static PluginLoader& instance();

    bool loadPlugin(const std::string& pluginPath);
    bool unloadPlugin(const std::string& pluginName);
    bool validateSignature(const std::string& pluginPath);

    const std::map<std::string, PluginInfo>& getLoadedPlugins() const { return plugins_; }

private:
    PluginLoader() = default;

    std::map<std::string, PluginInfo> plugins_;
    std::map<std::string, void*> handles_;
};

class PluginRegistry {
public:
    static PluginRegistry& instance();

    void registerPlugin(const PluginInfo& info);
    void unregisterPlugin(const std::string& name);
    PluginInfo getPlugin(const std::string& name) const;
    std::vector<PluginInfo> listPlugins() const;
    bool checkCompatibility(const std::string& name, const std::string& requiredVersion) const;

private:
    PluginRegistry() = default;

    std::map<std::string, PluginInfo> registry_;
};

} // namespace plugins
} // namespace sophon
