#include "Database.h"
#include <functional>
#include <random>
#include <sstream>
#include <iostream>

Database::Database(const std::string& path) : storage(initStorage(path)), dbPath(path)
{
	storage.sync_schema();
}

std::string Database::GenerateSalt() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    std::stringstream ss;
    for (int i = 0; i < 16; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    return ss.str();
}

std::string Database::HashPassword(const std::string& password, const std::string& salt) {
    std::hash<std::string> hasher;
    size_t hash = hasher(password + salt);

    std::stringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

bool Database::VerifyPassword(const std::string& plainPassword, const std::string& hashedPassword, const std::string& salt) {
    return HashPassword(plainPassword, salt) == hashedPassword;
}

bool Database::UpdateUsername(int userId, const std::string& newUsername) {
    try {
        if (UserExists(newUsername)) {
            return false;  
        }

        UserModel user = GetUserById(userId);
        user.m_username = newUsername;
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
        UserModel user = GetUserById(userId);
        if (!VerifyPassword(oldPassword, user.m_password, user.m_salt)) {
            return false; 
        }

        user.m_salt = GenerateSalt();
        user.m_password = HashPassword(newPassword, user.m_salt);
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
        UserModel user = GetUserById(userId);
        user.m_salt = GenerateSalt();
        user.m_password = HashPassword(newPassword, user.m_salt);
        UpdateUser(user);
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Error updating password (admin): " << e.what() << std::endl;
        return false;
    }
}

int Database::InsertUser(const UserModel& user) {
    UserModel hashedUser = user;
    hashedUser.m_salt = GenerateSalt();  
    hashedUser.m_password = HashPassword(user.m_password, hashedUser.m_salt);  
    return storage.insert(hashedUser);
}

bool Database::VerifyLogin(const std::string& username, const std::string& plainPassword) {
    try {
        UserModel user = GetUserByUsername(username);
        return VerifyPassword(plainPassword, user.m_password, user.m_salt);
    }
    catch (std::runtime_error&) {
        return false;  // User nu există
    }
}

UserModel Database::GetUserById(int id) {
    return storage.get<UserModel>(id);
}

UserModel Database::GetUserByUsername(const std::string& username) {
    using namespace sqlite_orm;
    auto users = storage.get_all<UserModel>(
        where(c(&UserModel::m_username) == username)
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
        where(c(&UserModel::m_username) == username)
    );
    return count > 0;
}
