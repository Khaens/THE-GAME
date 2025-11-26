#include "Database.h"
#include <iostream>

Database::Database(const std::string& path) : storage(initStorage(path)), dbPath(path)
{
    storage.sync_schema();
}

bool Database::UpdateUsername(int userId, const std::string& newUsername) {
    try {
        if (UserExists(newUsername)) {
            return false;
        }

        UserModel user = storage.get<UserModel>(userId);
        user.SetUsername(newUsername);
        UpdateUser(user);
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Error updating username: " << e.what() << std::endl;
        return false;
    }
}

bool Database::UpdatePassword(int userId, const std::string& oldPassword, const std::string& newPassword) {
    try {
        UserModel user = storage.get<UserModel>(userId);
        if (user.GetPassword() != oldPassword) { 
            return false;
        }

        user.SetPassword(newPassword); 
        UpdateUser(user);
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Error updating password: " << e.what() << std::endl;
        return false;
    }
}

bool Database::UpdatePasswordRecovery(int userId, const std::string& newPassword) {
    try {
        UserModel user = storage.get<UserModel>(userId);
        user.SetPassword(newPassword); 
        UpdateUser(user);
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Error updating password (recovery): " << e.what() << std::endl;
        return false;
    }
}

int Database::InsertUser(const UserModel& user) {
    return storage.insert(user);
}

bool Database::VerifyLogin(const std::string& username, const std::string& plainPassword) {
    try {
        UserModel user = GetUserByUsername(username);
        return (user.GetPassword() == plainPassword); 
    }
    catch (std::runtime_error&) {
        return false;  
    }
}

UserModel Database::GetUserByUsername(const std::string& username) {
    using namespace sqlite_orm;
    auto users = storage.get_all<UserModel>(
        where(c(&UserModel::GetUsername) == username)
    );
    if (users.empty()) {
        throw std::runtime_error("User not found");
    }
    return users[0];
}

std::vector<UserModel> Database::GetAllUsers() {
    return storage.get_all<UserModel>();
}

void Database::UpdateUser(const UserModel& user) {
    storage.update(user);
}

void Database::DeleteUser(int id) {
    storage.remove<UserModel>(id);
}

bool Database::UserExists(const std::string& username) {
    using namespace sqlite_orm;
    auto count = storage.count<UserModel>(
        where(c(&UserModel::GetUsername) == username)
    );
    return count > 0;
}