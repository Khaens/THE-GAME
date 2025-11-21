#pragma once
#include <string>

struct UserModel
{
    int m_id;
    std::string m_username;
    std::string m_password;
    std::string m_salt;

    UserModel() = default;
    UserModel(int id, const std::string& user, const std::string& pass);
};



