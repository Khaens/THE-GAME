#pragma once
#include <string>

class UserModel
{
private:
    int m_id = -1;
    std::string m_username;
    std::string m_password;

public:
    UserModel() = default;
    UserModel(int id, const std::string& user, const std::string& pass);
    UserModel(const UserModel& user);
    UserModel(UserModel&& user) noexcept;  // Move constructor
    UserModel& operator=(const UserModel& user);  // Copy assignment
    UserModel& operator=(UserModel&& user) noexcept;  // Move assignment
    const int& GetId() const;
    const std::string& GetUsername() const;
    const std::string& GetPassword() const;
    void SetId(int id);
    void SetUsername(const std::string& username);
    void SetPassword(const std::string& password);
};



