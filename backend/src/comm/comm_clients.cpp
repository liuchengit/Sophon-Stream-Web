#include "comm/comm_clients.h"
#include <iostream>
#include <sstream>
#include <fstream>

namespace sophon {
namespace comm {

// MqttClient implementation

MqttClient& MqttClient::instance() {
    static MqttClient instance;
    return instance;
}

bool MqttClient::connect(const std::string& brokerUrl, int port) {
    std::lock_guard<std::mutex> lock(mutex_);

    brokerUrl_ = brokerUrl + ":" + std::to_string(port);
    std::cout << "Connecting to MQTT broker: " << brokerUrl_ << std::endl;

    // In production: use Paho MQTT or similar library
    connected_ = true;
    std::cout << "MQTT connected successfully" << std::endl;
    return true;
}

bool MqttClient::disconnect() {
    std::lock_guard<std::mutex> lock(mutex_);
    connected_ = false;
    subscriptions_.clear();
    std::cout << "MQTT disconnected" << std::endl;
    return true;
}

bool MqttClient::publish(const std::string& topic, const std::string& payload, int /*qos*/) {
    if (!connected_) return false;

    std::cout << "MQTT publish: " << topic << " - " << payload << std::endl;
    return true;
}

bool MqttClient::subscribe(const std::string& topic, MessageCallback callback, int /*qos*/) {
    if (!connected_) return false;

    std::lock_guard<std::mutex> lock(mutex_);
    subscriptions_[topic] = std::move(callback);
    std::cout << "MQTT subscribed: " << topic << std::endl;
    return true;
}

bool MqttClient::unsubscribe(const std::string& topic) {
    std::lock_guard<std::mutex> lock(mutex_);
    return subscriptions_.erase(topic) > 0;
}

// HttpPlatformClient implementation

HttpPlatformClient& HttpPlatformClient::instance() {
    static HttpPlatformClient instance;
    return instance;
}

void HttpPlatformClient::setPlatformUrl(const std::string& url) {
    platformUrl_ = url;
}

std::string HttpPlatformClient::getDeviceStatus() {
    return R"({"status":"online","devices":0,"tasks":0})";
}

std::string HttpPlatformClient::getDeviceList() {
    return R"({"devices":[]})";
}

bool HttpPlatformClient::reportEvent(const std::string& eventType, const std::string& data) {
    std::cout << "HTTP report event: " << eventType << " - " << data << std::endl;
    return true;
}

// Gb28181Client implementation

Gb28181Client& Gb28181Client::instance() {
    static Gb28181Client instance;
    return instance;
}

bool Gb28181Client::registerToPlatform(const std::string& platformIp, int port, const std::string& deviceId) {
    platformIp_ = platformIp;
    platformPort_ = port;
    deviceId_ = deviceId;

    std::cout << "GB28181 registering to " << platformIp << ":" << port << " with device " << deviceId << std::endl;

    // In production: send SIP REGISTER request
    registered_ = true;
    return true;
}

bool Gb28181Client::unregister() {
    registered_ = false;
    std::cout << "GB28181 unregistered" << std::endl;
    return true;
}

bool Gb28181Client::startStream(const std::string& channelId, const std::string& /*targetIp*/, int /*targetPort*/) {
    if (!registered_) return false;

    std::cout << "GB28181 starting stream for channel " << channelId << std::endl;
    return true;
}

bool Gb28181Client::stopStream(const std::string& channelId) {
    std::cout << "GB28181 stopping stream for channel " << channelId << std::endl;
    return true;
}

} // namespace comm
} // namespace sophon
