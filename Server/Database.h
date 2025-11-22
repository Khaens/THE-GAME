#pragma once
#include <sqlite_orm/sqlite_orm.h>
#include "UserModel.h"
#include "AchievementsModel.h"

using namespace sqlite_orm;

inline auto initStorage(const std::string& path) {
    return make_storage(path,
        make_table("users",
            make_column("id", &UserModel::m_id, primary_key().autoincrement()),
            make_column("username", &UserModel::m_username, unique()),
            make_column("password", &UserModel::m_password),
            make_column("salt", &UserModel::m_salt)),
        make_table("achievements", 
            make_column("id", &AchievementsModel::id, primary_key().autoincrement()),
            make_column("user_id", &AchievementsModel::userId), 
            make_column("wins", &AchievementsModel::wins),
            make_column("losses", &AchievementsModel::losses),
            make_column("win_rate", &AchievementsModel::winRate), 
            foreign_key(&AchievementsModel::userId).references(&UserModel::m_id)) 
    );
}

using Storage = decltype(initStorage(""));

class Database
{
private:
    Storage storage;
    std::string dbPath;

public:
    Database(const std::string& path = "users.db");
    std::string GenerateSalt();
    std::string HashPassword(const std::string& password, const std::string& salt);
    bool VerifyPassword(const std::string& plainPassword, const std::string& hashedPassword, const std::string& salt);
    bool UpdateUsername(int userId, const std::string& newUsername);
    bool UpdatePassword(int userId, const std::string& oldPassword, const std::string& newPassword);
    bool UpdatePasswordRecovery(int userId, const std::string& newPassword);
    int InsertUser(const UserModel& user);
    bool VerifyLogin(const std::string& username, const std::string& plainPassword);
    UserModel GetUserById(int id);
    UserModel GetUserByUsername(const std::string& username);
    std::vector<UserModel> GetAllUsers();
    void UpdateUser(const UserModel& user);
    void DeleteUser(int id);
    bool UserExists(const std::string& username);
};

