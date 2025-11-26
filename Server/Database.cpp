#include "Database.h"
#include <iostream>
#include <tuple>

Database::Database(const std::string& path) : storage(initStorage(path)), dbPath(path)
{
    storage.sync_schema();
}

std::string HashPassword(const std::string& password) {
    std::hash<std::string> hasher;
    return std::to_string(hasher(password));
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
        if (HashPassword(oldPassword) != user.GetPassword()) {
            return false;
        }

        user.SetPassword(HashPassword(newPassword));
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
        user.SetPassword(HashPassword(newPassword));
        UpdateUser(user);
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Error updating password (recovery): " << e.what() << std::endl;
        return false;
    }
}

int Database::InsertUser(const UserModel& user) {
    try {
        UserModel hashedUser = user;
        hashedUser.SetPassword(HashPassword(user.GetPassword()));
        return storage.insert(hashedUser);
    }
    catch (std::exception& e) {
        std::cerr << "Error inserting user: " << e.what() << std::endl;
        return -1;
    }
}

bool Database::VerifyLogin(const std::string& username, const std::string& plainPassword) {
    try {
        auto selectStatement = storage.prepare(
            select(&UserModel::GetPassword,
                where(c(&UserModel::GetUsername) == username))
        );

        auto rows = storage.execute(selectStatement);

        if (rows.empty()) {
            return false;
        }
        return HashPassword(plainPassword) == rows[0];
    }
    catch (std::exception& e) {
        std::cerr << "Login error: " << e.what() << std::endl;
        return false;
    }
}

UserModel Database::GetUserByUsername(const std::string& username) {
    try {
        auto selectStatement = storage.prepare(
            select(columns(&UserModel::GetId,
                &UserModel::GetUsername,
                &UserModel::GetPassword),
                where(c(&UserModel::GetUsername) == username))
        );

        auto rows = storage.execute(selectStatement);

        if (rows.empty()) {
            throw std::runtime_error("User not found");
        }

        auto& row = rows[0];
        UserModel user;
        user.SetId(std::get<0>(row));
        user.SetUsername(std::get<1>(row));
        user.SetPassword(std::get<2>(row));

        return user;
    }
    catch (std::exception& e) {
        std::cerr << "Error getting user: " << e.what() << std::endl;
        throw;
    }
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
    try {
        auto selectStatement = storage.prepare(
            select(count<UserModel>(),
                where(c(&UserModel::GetUsername) == username))
        );

        auto result = storage.execute(selectStatement);
        return !result.empty() && result[0] > 0;
    }
    catch (std::exception& e) {
        std::cerr << "Error checking user existence: " << e.what() << std::endl;
        return false;
    }
}