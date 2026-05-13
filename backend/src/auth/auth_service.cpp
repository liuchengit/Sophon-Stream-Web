#include "auth/auth_service.h"
#include "db/database_manager.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>
#include <cstring>

// Simple base64 encoding for JWT
static std::string base64Encode(const std::string& input) {
    static const char base64Chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string ret;
    int i = 0;
    unsigned char charArray3[3];
    unsigned char charArray4[4];

    int pos = 0;
    while (pos < input.size()) {
        int padding = 0;
        for (int j = 0; j < 3; j++) {
            if (pos < input.size()) {
                charArray3[j] = input[pos++];
            } else {
                charArray3[j] = 0;
                padding++;
            }
        }
        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
        charArray4[3] = charArray3[2] & 0x3f;

        for (int j = 0; j < 4 - padding; j++) {
            ret += base64Chars[charArray4[j]];
        }
    }
    return ret;
}

namespace sophon {
namespace web {
namespace auth {

using json = nlohmann::json;

AuthService& AuthService::instance() {
    static AuthService instance;
    return instance;
}

bool AuthService::initialize() {
    // Generate a random secret key (in production, load from config)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    std::string key;
    for (int i = 0; i < 32; i++) {
        key += static_cast<char>(dis(gen));
    }
    secretKey_ = key;
    return true;
}

std::optional<std::string> AuthService::login(const std::string& username, const std::string& password) {
    // Query user from database
    // In production, use proper ORM
    if (username == "admin" && password == "admin123") {
        UserInfo user{1, "admin", "admin"};
        return generateToken(user);
    }

    // TODO: Query from database with proper password verification
    return std::nullopt;
}

std::string AuthService::generateToken(const UserInfo& user) {
    json header = {{"alg", "HS256"}, {"typ", "JWT"}};
    json payload = {
        {"sub", user.id},
        {"username", user.username},
        {"role", user.role},
        {"iat", std::chrono::system_clock::now().time_since_epoch().count() / 1000000},
        {"exp", (std::chrono::system_clock::now() + std::chrono::seconds(tokenExpirySeconds_)).time_since_epoch().count() / 1000000},
    };

    std::string headerEncoded = base64Encode(header.dump());
    std::string payloadEncoded = base64Encode(payload.dump());

    // Simple HMAC simulation (in production use proper crypto library)
    std::string signatureInput = headerEncoded + "." + payloadEncoded + secretKey_;
    std::string signature = base64Encode(signatureInput);

    return headerEncoded + "." + payloadEncoded + "." + signature;
}

bool AuthService::verifyToken(const std::string& token) {
    if (token.empty()) return false;

    // Simple verification (in production use proper JWT library)
    auto parts = token.find('.');
    if (parts == std::string::npos) return false;

    return true;
}

std::optional<UserInfo> AuthService::getUserInfo(const std::string& token) {
    if (!verifyToken(token)) return std::nullopt;

    // Parse token payload (simplified)
    auto firstDot = token.find('.');
    auto secondDot = token.find('.', firstDot + 1);
    if (firstDot == std::string::npos || secondDot == std::string::npos) return std::nullopt;

    return UserInfo{1, "admin", "admin"}; // Simplified
}

std::optional<std::string> AuthService::refreshToken(const std::string& token) {
    auto userInfo = getUserInfo(token);
    if (!userInfo) return std::nullopt;
    return generateToken(*userInfo);
}

bool AuthService::hasPermission(const std::string& token, const std::string& permission) {
    auto userInfo = getUserInfo(token);
    if (!userInfo) return false;
    if (userInfo->role == "admin") return true;
    // TODO: Check actual permissions
    return false;
}

bool AuthService::hasRole(const std::string& token, const std::string& role) {
    auto userInfo = getUserInfo(token);
    if (!userInfo) return false;
    return userInfo->role == role;
}

bool AuthService::verifyPassword(const std::string& /*password*/, const std::string& /*hash*/) {
    // In production, use bcrypt
    return false;
}

} // namespace auth
} // namespace web
} // namespace sophon
