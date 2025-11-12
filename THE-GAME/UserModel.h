#pragma once
#include <string>

struct UserModel
{
    int id;
    std::string username;
    std::string password;

    UserModel() = default;
    UserModel(int id, const std::string& user, const std::string& pass);
};

