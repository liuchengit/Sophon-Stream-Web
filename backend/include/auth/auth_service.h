#pragma once

#include <string>
#include <optional>

namespace sophon {
namespace web {
namespace auth {

struct UserInfo {
    int id;
    std::string username;
    std::string role;
};

class AuthService {
public:
    static AuthService& instance();

    bool initialize();

    std::optional<std::string> login(const std::string& username, const std::string& password);
    bool verifyToken(const std::string& token);
    std::optional<UserInfo> getUserInfo(const std::string& token);
    std::optional<std::string> refreshToken(const std::string& token);

    bool hasPermission(const std::string& token, const std::string& permission);
    bool hasRole(const std::string& token, const std::string& role);

private:
    AuthService() = default;

    std::string generateToken(const UserInfo& user);
    bool verifyPassword(const std::string& password, const std::string& hash);

    std::string secretKey_;
    int tokenExpirySeconds_ = 7200; // 2 hours
};

} // namespace auth
} // namespace web
} // namespace sophon
