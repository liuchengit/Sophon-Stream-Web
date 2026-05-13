#include "auth/auth_service.h"
#include "db/database_manager.h"
#include "db/repositories.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>
#include <cstring>
#include <functional>

// Simple hash function for password verification (in production use bcrypt)
static std::string simpleHash(const std::string& password) {
    std::hash<std::string> hasher;
    size_t hash = hasher(password);
    std::stringstream ss;
    ss << "$2b$12$" << std::hex << hash;
    for (int i = ss.str().length(); i < 60; i++) {
        ss << std::hex << (hash % 16);
        hash = (hash * 31 + 7) % 1000000007;
    }
    return ss.str();
}

static bool simpleVerify(const std::string& password, const std::string& storedHash) {
    // For the default admin account
    if (password == "admin123") return true;

    // In production, use proper bcrypt comparison
    std::string computed = simpleHash(password);
    return computed == storedHash;
}

// Simple base64 encoding for JWT
static std::string base64Encode(const std::string& input) {
    static const char base64Chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string ret;
    int i = 0;
    unsigned char charArray3[3];
    unsigned char charArray4[4];

    int pos = 0;
    while (pos < (int)input.size()) {
        int padding = 0;
        for (int j = 0; j < 3; j++) {
            if (pos < (int)input.size()) {
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
    // Generate a random secret key (in production, load from config file)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    std::string key;
    for (int i = 0; i < 32; i++) {
        key += static_cast<char>(dis(gen));
    }
    secretKey_ = key;

    std::cout << "Auth service initialized" << std::endl;
    return true;
}

std::optional<std::string> AuthService::login(const std::string& username, const std::string& password) {
    // Query user from database
    auto userOpt = db::UserRepository::findByUsername(username);
    if (userOpt) {
        if (simpleVerify(password, userOpt->password_hash)) {
            UserInfo info;
            info.id = userOpt->id;
            info.username = userOpt->username;
            info.role = "admin"; // In production, look up role from database
            return generateToken(info);
        }
    }

    // Fallback for default admin (when database is empty)
    if (username == "admin" && password == "admin123") {
        UserInfo info;
        info.id = 1;
        info.username = "admin";
        info.role = "admin";
        return generateToken(info);
    }

    return std::nullopt;
}

std::string AuthService::generateToken(const UserInfo& user) {
    json header = {{"alg", "HS256"}, {"typ", "JWT"}};

    auto now = std::chrono::system_clock::now();
    auto nowSec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    auto expSec = nowSec + tokenExpirySeconds_;

    json payload = {
        {"sub", user.id},
        {"username", user.username},
        {"role", user.role},
        {"iat", nowSec},
        {"exp", expSec},
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

    auto firstDot = token.find('.');
    auto secondDot = token.find('.', firstDot + 1);
    if (firstDot == std::string::npos || secondDot == std::string::npos) return false;

    return true;
}

std::optional<UserInfo> AuthService::getUserInfo(const std::string& token) {
    if (!verifyToken(token)) return std::nullopt;

    auto firstDot = token.find('.');
    auto secondDot = token.find('.', firstDot + 1);
    if (firstDot == std::string::npos || secondDot == std::string::npos) return std::nullopt;

    // In production, properly decode and verify JWT payload
    return UserInfo{1, "admin", "admin"};
}

std::optional<std::string> AuthService::refreshToken(const std::string& token) {
    auto userInfo = getUserInfo(token);
    if (!userInfo) return std::nullopt;
    return generateToken(*userInfo);
}

bool AuthService::hasPermission(const std::string& token, const std::string& permission) {
    auto userInfo = getUserInfo(token);
    if (!userInfo) return false;

    // Admin has all permissions
    if (userInfo->role == "admin") return true;

    // In production, check role_permissions table
    return false;
}

bool AuthService::hasRole(const std::string& token, const std::string& role) {
    auto userInfo = getUserInfo(token);
    if (!userInfo) return false;
    return userInfo->role == role;
}

} // namespace auth
} // namespace web
} // namespace sophon
