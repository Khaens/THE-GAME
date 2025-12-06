#pragma once
#include <sqlite_orm/sqlite_orm.h>
#include <string>
#include "UserModel.h"
#include "AchievementsModel.h"

using namespace sqlite_orm;

inline auto initStorage(const std::string& path) {
    return make_storage(path,
        make_table("users",
            make_column("id", &UserModel::GetId, &UserModel::SetId, primary_key().autoincrement()),
            make_column("username", &UserModel::GetUsername, &UserModel::SetUsername, unique()),
            make_column("password", &UserModel::GetPassword, &UserModel::SetPassword)),
        make_table("achievements",
            make_column("id", &AchievementsModel::GetId, &AchievementsModel::SetId, primary_key().autoincrement()),
            make_column("user_id", &AchievementsModel::GetUserId, &AchievementsModel::SetUserId),
            make_column("all_on_red", &AchievementsModel::GetAllOnRed, &AchievementsModel::SetAllOnRed),
            make_column("harry_potter", &AchievementsModel::GetHarryPotter, &AchievementsModel::SetHarryPotter),
            make_column("serious_player", &AchievementsModel::GetSeriousPlayer, &AchievementsModel::SetSeriousPlayer),
            foreign_key(&AchievementsModel::GetUserId).references(&UserModel::GetId))
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
    bool UpdateUsername(int userId, const std::string& newUsername);
    bool UpdatePassword(int userId, const std::string& oldPassword, const std::string& newPassword);
    bool UpdatePasswordRecovery(int userId, const std::string& newPassword);
    int InsertUser(const UserModel& user);
    bool VerifyLogin(const std::string& username, const std::string& plainPassword);
    UserModel GetUserByUsername(const std::string& username);
    std::vector<UserModel> GetAllUsers();
    void UpdateUser(const UserModel& user);
    void DeleteUser(int id);
    bool UserExists(const std::string& username);

    int InsertAchievements(const AchievementsModel& achievements);
    AchievementsModel GetAchievementsByUserId(int userId);
    void UpdateAchievements(const AchievementsModel& achievements);
    void DeleteAchievements(int id);
    bool AchievementsExistForUser(int userId);
};

