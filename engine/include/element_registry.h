#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <mutex>
#include <nlohmann/json.hpp>

namespace sophon {
namespace stream {

using json = nlohmann::json;

struct RoiRect {
    int x, y, width, height;
};

struct ElementConfig {
    std::string type;
    std::string name;
    json params;
    std::vector<RoiRect> rois;
};

class ElementRegistry {
public:
    static ElementRegistry& instance();

    bool registerElement(const std::string& type, const std::string& name);
    bool unregisterElement(const std::string& type, const std::string& name);

    ElementConfig getConfig(const std::string& type, const std::string& name) const;
    void setConfig(const std::string& type, const std::string& name, const json& params);

    void addRoi(const std::string& elementType, const std::string& elementName, const RoiRect& roi);
    void clearRois(const std::string& elementType, const std::string& elementName);
    std::vector<RoiRect> getRois(const std::string& elementType, const std::string& elementName) const;

    json serializeConfig() const;
    bool loadConfig(const json& config);

private:
    ElementRegistry() = default;

    std::map<std::string, ElementConfig> elements_;
    mutable std::mutex mutex_;
};

} // namespace stream
} // namespace sophon
