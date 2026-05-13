#include "element_registry.h"
#include <iostream>
#include <algorithm>

namespace sophon {
namespace stream {

ElementRegistry& ElementRegistry::instance() {
    static ElementRegistry instance;
    return instance;
}

bool ElementRegistry::registerElement(const std::string& type, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string key = type + ":" + name;

    if (elements_.find(key) != elements_.end()) {
        std::cerr << "Element already registered: " << key << std::endl;
        return false;
    }

    ElementConfig config;
    config.type = type;
    config.name = name;
    elements_[key] = config;

    std::cout << "Registered element: " << key << std::endl;
    return true;
}

bool ElementRegistry::unregisterElement(const std::string& type, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string key = type + ":" + name;

    auto it = elements_.find(key);
    if (it == elements_.end()) return false;

    elements_.erase(it);
    std::cout << "Unregistered element: " << key << std::endl;
    return true;
}

ElementConfig ElementRegistry::getConfig(const std::string& type, const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string key = type + ":" + name;

    auto it = elements_.find(key);
    if (it != elements_.end()) {
        return it->second;
    }
    return ElementConfig{};
}

void ElementRegistry::setConfig(const std::string& type, const std::string& name, const json& params) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string key = type + ":" + name;

    auto it = elements_.find(key);
    if (it != elements_.end()) {
        it->second.params = params;
    }
}

void ElementRegistry::addRoi(const std::string& elementType, const std::string& elementName, const RoiRect& roi) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string key = elementType + ":" + elementName;

    auto it = elements_.find(key);
    if (it != elements_.end()) {
        it->second.rois.push_back(roi);
    }
}

void ElementRegistry::clearRois(const std::string& elementType, const std::string& elementName) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string key = elementType + ":" + elementName;

    auto it = elements_.find(key);
    if (it != elements_.end()) {
        it->second.rois.clear();
    }
}

std::vector<RoiRect> ElementRegistry::getRois(const std::string& elementType, const std::string& elementName) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string key = elementType + ":" + elementName;

    auto it = elements_.find(key);
    if (it != elements_.end()) {
        return it->second.rois;
    }
    return {};
}

json ElementRegistry::serializeConfig() const {
    std::lock_guard<std::mutex> lock(mutex_);
    json result = json::array();

    for (const auto& pair : elements_) {
        json elem;
        elem["type"] = pair.second.type;
        elem["name"] = pair.second.name;
        elem["params"] = pair.second.params;

        json rois = json::array();
        for (const auto& roi : pair.second.rois) {
            rois.push_back({{"x", roi.x}, {"y", roi.y}, {"width", roi.width}, {"height", roi.height}});
        }
        elem["rois"] = rois;

        result.push_back(elem);
    }

    return result;
}

bool ElementRegistry::loadConfig(const json& config) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!config.is_array()) return false;

    for (const auto& elemJson : config) {
        std::string type = elemJson.value("type", "");
        std::string name = elemJson.value("name", "");
        if (type.empty() || name.empty()) continue;

        std::string key = type + ":" + name;
        ElementConfig cfg;
        cfg.type = type;
        cfg.name = name;
        cfg.params = elemJson.value("params", json::object());

        if (elemJson.contains("rois")) {
            for (const auto& roiJson : elemJson["rois"]) {
                RoiRect roi;
                roi.x = roiJson.value("x", 0);
                roi.y = roiJson.value("y", 0);
                roi.width = roiJson.value("width", 0);
                roi.height = roiJson.value("height", 0);
                cfg.rois.push_back(roi);
            }
        }

        elements_[key] = cfg;
    }

    return true;
}

} // namespace stream
} // namespace sophon
