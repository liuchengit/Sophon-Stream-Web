#include "plugin_system.h"
#include <dlfcn.h>
#include <iostream>
#include <fstream>

namespace sophon {
namespace plugins {

PluginLoader& PluginLoader::instance() {
    static PluginLoader instance;
    return instance;
}

bool PluginLoader::loadPlugin(const std::string& pluginPath) {
    if (!validateSignature(pluginPath)) {
        std::cerr << "Plugin signature validation failed: " << pluginPath << std::endl;
        return false;
    }

    void* handle = dlopen(pluginPath.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "Failed to load plugin: " << dlerror() << std::endl;
        return false;
    }

    PluginInfo info;
    info.path = pluginPath;
    info.loaded = true;

    // Extract plugin name from path
    auto pos = pluginPath.find_last_of('/');
    info.name = (pos == std::string::npos) ? pluginPath : pluginPath.substr(pos + 1);

    plugins_[info.name] = info;
    handles_[info.name] = handle;

    PluginRegistry::instance().registerPlugin(info);

    std::cout << "Plugin loaded: " << info.name << std::endl;
    return true;
}

bool PluginLoader::unloadPlugin(const std::string& pluginName) {
    auto it = handles_.find(pluginName);
    if (it == handles_.end()) {
        std::cerr << "Plugin not found: " << pluginName << std::endl;
        return false;
    }

    if (dlclose(it->second) != 0) {
        std::cerr << "Failed to unload plugin: " << dlerror() << std::endl;
        return false;
    }

    handles_.erase(it);
    plugins_[pluginName].loaded = false;
    PluginRegistry::instance().unregisterPlugin(pluginName);

    std::cout << "Plugin unloaded: " << pluginName << std::endl;
    return true;
}

bool PluginLoader::validateSignature(const std::string& pluginPath) {
    // In production, verify digital signature
    // For now, just check file exists
    std::ifstream file(pluginPath);
    return file.good();
}

PluginRegistry& PluginRegistry::instance() {
    static PluginRegistry instance;
    return instance;
}

void PluginRegistry::registerPlugin(const PluginInfo& info) {
    registry_[info.name] = info;
}

void PluginRegistry::unregisterPlugin(const std::string& name) {
    registry_.erase(name);
}

PluginInfo PluginRegistry::getPlugin(const std::string& name) const {
    auto it = registry_.find(name);
    if (it != registry_.end()) {
        return it->second;
    }
    return PluginInfo{};
}

std::vector<PluginInfo> PluginRegistry::listPlugins() const {
    std::vector<PluginInfo> result;
    for (const auto& pair : registry_) {
        result.push_back(pair.second);
    }
    return result;
}

bool PluginRegistry::checkCompatibility(const std::string& name, const std::string& requiredVersion) const {
    auto it = registry_.find(name);
    if (it == registry_.end()) return false;
    return it->second.version == requiredVersion;
}

} // namespace plugins
} // namespace sophon
