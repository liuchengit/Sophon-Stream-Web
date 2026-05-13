#pragma once

#include <string>
#include <functional>
#include <map>
#include <mutex>

namespace sophon {
namespace comm {

using MessageCallback = std::function<void(const std::string& topic, const std::string& payload)>;

class MqttClient {
public:
    static MqttClient& instance();

    bool connect(const std::string& brokerUrl, int port = 1883);
    bool disconnect();
    bool publish(const std::string& topic, const std::string& payload, int qos = 0);
    bool subscribe(const std::string& topic, MessageCallback callback, int qos = 0);
    bool unsubscribe(const std::string& topic);

    bool isConnected() const { return connected_; }

private:
    MqttClient() = default;

    bool connected_ = false;
    std::string brokerUrl_;
    std::map<std::string, MessageCallback> subscriptions_;
    std::mutex mutex_;
};

class HttpPlatformClient {
public:
    static HttpPlatformClient& instance();

    void setPlatformUrl(const std::string& url);
    std::string getDeviceStatus();
    std::string getDeviceList();
    bool reportEvent(const std::string& eventType, const std::string& data);

private:
    HttpPlatformClient() = default;

    std::string platformUrl_;
};

class Gb28181Client {
public:
    static Gb28181Client& instance();

    bool registerToPlatform(const std::string& platformIp, int port, const std::string& deviceId);
    bool unregister();
    bool startStream(const std::string& channelId, const std::string& targetIp, int targetPort);
    bool stopStream(const std::string& channelId);

    bool isRegistered() const { return registered_; }

private:
    Gb28181Client() = default;

    bool registered_ = false;
    std::string deviceId_;
    std::string platformIp_;
    int platformPort_ = 0;
};

} // namespace comm
} // namespace sophon
