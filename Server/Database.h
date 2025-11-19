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
            make_column("password", &UserModel::m_password)),
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
    int insertUser(const UserModel& user);
    UserModel getUserById(int id);
    UserModel getUserByUsername(const std::string& username);
    std::vector<UserModel> getAllUsers();
    void updateUser(const UserModel& user);
    void deleteUser(int id);
    bool userExists(const std::string& username);
};

