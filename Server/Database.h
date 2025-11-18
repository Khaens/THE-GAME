#pragma once
#include <sqlite_orm/sqlite_orm.h>
#include "../THE-GAME/UserModel.h"
#include "../THE-GAME/AchievementsModel.h"

using namespace sqlite_orm;

inline auto initStorage(const std::string& path) {
    return make_storage(path,
        make_table("users",
            make_column("id", &UserModel::id, primary_key().autoincrement()),
            make_column("username", &UserModel::username, unique()),
            make_column("password", &UserModel::password)),
        make_table("achievements", 
            make_column("id", &AchievementsModel::id, primary_key().autoincrement()),
            make_column("user_id", &AchievementsModel::userId), 
            make_column("wins", &AchievementsModel::wins),
            make_column("losses", &AchievementsModel::losses),
            make_column("win_rate", &AchievementsModel::winRate), 
            foreign_key(&AchievementsModel::userId).references(&UserModel::id)) 
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

