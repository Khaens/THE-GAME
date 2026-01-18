#pragma once
#include <sqlite_orm/sqlite_orm.h>
#include <string>
#include "UserModel.h"
#include "AchievementsModel.h"
#include "StatisticsModel.h"
#include "PlaytimeModel.h"

using namespace sqlite_orm;

inline auto initStorage(const std::string& path) {
    auto storage = make_storage(path,
        make_table("users",
            make_column("id", &UserModel::GetId, &UserModel::SetId, primary_key().autoincrement()),
            make_column("username", &UserModel::GetUsername, &UserModel::SetUsername, unique()),
            make_column("password", &UserModel::GetPassword, &UserModel::SetPassword),
            make_column("profile_image", &UserModel::GetProfileImage, &UserModel::SetProfileImage)),
        make_table("achievements",
            make_column("id", &AchievementsModel::GetId, &AchievementsModel::SetId, primary_key().autoincrement()),
            make_column("user_id", &AchievementsModel::GetUserId, &AchievementsModel::SetUserId),
            make_column("all_on_red", &AchievementsModel::GetAllOnRed, &AchievementsModel::SetAllOnRed),
            make_column("harry_potter", &AchievementsModel::GetHarryPotter, &AchievementsModel::SetHarryPotter),
            make_column("soothsayer", &AchievementsModel::GetSoothsayer, &AchievementsModel::SetSoothsayer),
            make_column("tax_evader", &AchievementsModel::GetTaxEvader, &AchievementsModel::SetTaxEvader),
            make_column("gambler", &AchievementsModel::GetGambler, &AchievementsModel::SetGambler),
            make_column("peasant", &AchievementsModel::GetPeasant, &AchievementsModel::SetPeasant),
            make_column("serious_player", &AchievementsModel::GetSeriousPlayer, &AchievementsModel::SetSeriousPlayer),
            make_column("talented_player", &AchievementsModel::GetTalentedPlayer, &AchievementsModel::SetTalentedPlayer),
            make_column("jack_of_all_trades", &AchievementsModel::GetJack, &AchievementsModel::SetJack),
            make_column("zero_effort", &AchievementsModel::GetZeroEffort, &AchievementsModel::SetZeroEffort),
            make_column("vanilla_victory", &AchievementsModel::GetVanillaW, &AchievementsModel::SetVanillaW),
            make_column("high_risk_high_reward", &AchievementsModel::GetHighRisk, &AchievementsModel::SetHighRisk),
            make_column("perfect_game", &AchievementsModel::GetPerfectGame, &AchievementsModel::SetPerfectGame),
            make_column("sixSeven", &AchievementsModel::GetSixSeven, &AchievementsModel::SetSixSeven),
            foreign_key(&AchievementsModel::GetUserId).references(&UserModel::GetId).on_delete.cascade()),
        make_table("statistics",
            make_column("id", &StatisticsModel::GetId, &StatisticsModel::SetId, primary_key().autoincrement()),
            make_column("user_id", &StatisticsModel::GetUserId, &StatisticsModel::SetUserId),
            make_column("games_won", &StatisticsModel::GetGamesWon, &StatisticsModel::SetGamesWon),
            make_column("games_played", &StatisticsModel::GetGamesPlayed, &StatisticsModel::SetGamesPlayed),
            make_column("win_rate", &StatisticsModel::GetWinRate, &StatisticsModel::SetWinRate),
            foreign_key(&StatisticsModel::GetUserId).references(&UserModel::GetId).on_delete.cascade()),
        make_table("playtimes",
            make_column("id", &PlaytimeModel::GetId, &PlaytimeModel::SetId, primary_key().autoincrement()),
            make_column("user_id", &PlaytimeModel::GetUserId, &PlaytimeModel::SetUserId),
            make_column("seconds", &PlaytimeModel::GetSeconds, &PlaytimeModel::SetSeconds),
            foreign_key(&PlaytimeModel::GetUserId).references(&UserModel::GetId).on_delete.cascade())
        );
    
    storage.on_open = [](sqlite3* db) {
        sqlite3_exec(db, "PRAGMA foreign_keys = ON", nullptr, nullptr, nullptr);
    };
    
    return storage;
}


using Storage = decltype(initStorage(""));

class Database
{
private:
    Storage storage;
    std::string dbPath;

    void UpdateUser(const UserModel& user);
    int InsertStatistics(const StatisticsModel& stats);
    bool StatisticsExistForUser(int userId);
    int InsertAchievements(const AchievementsModel& achievements);
    void UpdateAchievements(const AchievementsModel& achievements);
    bool AchievementsExistForUser(int userId);
    bool CheckAndUnlockJack(int userId);

public:
    Database(const std::string& path = "users.db");
    int InsertUser(const UserModel& user);
    bool VerifyLogin(const std::string& username, const std::string& plainPassword);
    UserModel GetUserByUsername(const std::string& username);
    UserModel GetUserById(int userId);
    bool UserExists(const std::string& username);
    bool UpdateProfileImage(int userId, std::vector<char> imageBuffer);
    std::vector<char> GetProfileImage(int userId);
    bool HasProfileImage(int userId);
    void DeleteProfileImage(int userId);

    StatisticsModel GetStatisticsByUserId(int userId);
    void UpdateStatistics(const StatisticsModel& statistics);

    AchievementsModel GetAchievementsByUserId(int userId);
    std::vector<std::string> UnlockAchievements(int userId, const std::unordered_map<std::string, bool>& achievementConditions);

    PlaytimeModel GetPlaytimeByUserId(int userId);
    void UpdatePlaytime(const PlaytimeModel& pt);
};

