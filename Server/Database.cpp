#define _CRT_SECURE_NO_WARNINGS
#include "Database.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>

Database::Database(const std::string& path) : storage(initStorage(path)), dbPath(path)
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "backups/pre_sync_" << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S") << ".db";

    BackupDatabase(ss.str());
    std::cout << "[DATABASE] Pre-sync backup created: " << ss.str() << std::endl;

    storage.sync_schema();

    try {
        auto users = storage.get_all<UserModel>();
        for (const auto& user : users) {
            if (!AchievementsExistForUser(user.GetId())) {
                AchievementsModel achievements(user.GetId());
                InsertAchievements(achievements);
                std::cout << "Created achievements for user: "
                    << user.GetUsername() << " (ID: " << user.GetId() << ")" << std::endl;
            }

            if (!StatisticsExistForUser(user.GetId())) {
                StatisticsModel stats(user.GetId());
                storage.insert(stats);
                std::cout << "Created statistics for user: "
                    << user.GetUsername() << " (ID: " << user.GetId() << ")" << std::endl;
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << "Error creating achievements for existing users: " << e.what() << std::endl;
    }
}

static std::string HashPassword(std::string_view password) {
    std::hash<std::string_view> hasher;
    return std::to_string(hasher(password));
}


int Database::InsertUser(const UserModel& user) {
    try {
        int newUserId = -1;
        bool result = storage.transaction([&]() -> bool {
            UserModel hashedUser = user;
            hashedUser.SetPassword(HashPassword(user.GetPassword()));
            storage.insert(hashedUser);

            auto users = storage.get_all<UserModel>(where(c(&UserModel::GetUsername) == user.GetUsername()));
            if (users.empty()) return false;

            newUserId = users[0].GetId();

            AchievementsModel achievements(newUserId);
            InsertAchievements(achievements);

            StatisticsModel statistics(newUserId);
            InsertStatistics(statistics);

            return true;
            });

        if (result) {
            return newUserId;
        }
        else {
            return -1;
        }
    }
    catch (std::exception& e) {
        std::cerr << "Error inserting user (transaction failed): " << e.what() << std::endl;
        return -1;
    }
}

bool Database::VerifyLogin(const std::string& username, const std::string& plainPassword) {
    try {
        auto users = storage.get_all<UserModel>(where(c(&UserModel::GetUsername) == username));

        if (users.empty()) {
            return false;
        }

        std::string computedHash = HashPassword(plainPassword);
        return computedHash == users[0].GetPassword();
    }
    catch (std::exception& e) {
        std::cerr << "Login error: " << e.what() << std::endl;
        return false;
    }
}

UserModel Database::GetUserByUsername(const std::string& username) {
    try {
        auto users = storage.get_all<UserModel>(where(c(&UserModel::GetUsername) == username));

        if (users.empty()) {
            throw std::runtime_error("User not found");
        }

        return users[0];
    }
    catch (std::exception& e) {
        std::cerr << "Error getting user: " << e.what() << std::endl;
        throw;
    }
}

UserModel Database::GetUserById(int userId) {
    try {
        return storage.get<UserModel>(userId);
    }
    catch (std::exception& e) {
        std::cerr << "Error getting user by id: " << e.what() << std::endl;
        throw;
    }
}


void Database::UpdateUser(const UserModel& user) {
    storage.update(user);
}


bool Database::UserExists(const std::string& username) {
    try {
        return storage.count<UserModel>(where(c(&UserModel::GetUsername) == username)) > 0;
    }
    catch (std::exception& e) {
        std::cerr << "Error checking user existence: " << e.what() << std::endl;
        return false;
    }
}

bool Database::UpdateProfileImage(int userId, std::vector<char> imageBuffer) {
    try {
        auto user = storage.get_pointer<UserModel>(userId);
        if (!user) return false;

        user->SetProfileImageMove(std::move(imageBuffer));
        storage.update(*user);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Eroare: " << e.what() << std::endl;
        return false;
    }
}

std::vector<char> Database::GetProfileImage(int userId) {
    try {
        UserModel user = storage.get<UserModel>(userId);
        return user.GetProfileImage();
    }
    catch (std::exception& e) {
        std::cerr << "Error getting profile image: " << e.what() << std::endl;
        return std::vector<char>();
    }
}

bool Database::HasProfileImage(int userId) {
    try {
        UserModel user = storage.get<UserModel>(userId);
        return !user.GetProfileImage().empty();
    }
    catch (std::exception& e) {
        std::cerr << "Error checking profile image: " << e.what() << std::endl;
        return false;
    }
}

void Database::DeleteProfileImage(int userId) {
    try {
        UserModel user = storage.get<UserModel>(userId);
        user.SetProfileImage(std::vector<char>());
        UpdateUser(user);
        std::cout << "Profile image deleted for user ID: " << userId << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Error deleting profile image: " << e.what() << std::endl;
    }
}

int Database::InsertStatistics(const StatisticsModel& stats)
{
    try {
        return storage.insert(stats);
    }
    catch (std::exception& e) {
        std::cerr << "Error inserting statistics: " << e.what() << std::endl;
        return -1;
    }
}

bool Database::StatisticsExistForUser(int userId)
{
    try {
        auto count = storage.count<StatisticsModel>(
            where(c(&StatisticsModel::GetUserId) == userId)
        );
        return count > 0;
    }
    catch (std::exception& e) {
        std::cerr << "Error checking statistics existence: " << e.what() << std::endl;
        return false;
    }
}

StatisticsModel Database::GetStatisticsByUserId(int userId) {
    try {
        auto stats = storage.get_all<StatisticsModel>(
            where(c(&StatisticsModel::GetUserId) == userId)
        );

        if (stats.empty()) {
            StatisticsModel newStats(userId);
            int id = storage.insert(newStats);
            newStats.SetId(id);

            std::cout << "Created statistics for user id: " << userId << std::endl;

            return newStats;
        }

        return stats[0];
    }
    catch (std::exception& e) {
        std::cerr << "Error getting statistics: " << e.what() << std::endl;
        throw;
    }
}

void Database::UpdateStatistics(const StatisticsModel& statistics) {
    storage.update(statistics);
}


int Database::InsertAchievements(const AchievementsModel& achievements) {
    try {
        return storage.insert(achievements);
    }
    catch (std::exception& e) {
        std::cerr << "Error inserting achievements: " << e.what() << std::endl;
        return -1;
    }
}

AchievementsModel Database::GetAchievementsByUserId(int userId) {
    try {
        auto achievements = storage.get_all<AchievementsModel>(
            where(c(&AchievementsModel::GetUserId) == userId)
        );

        if (achievements.empty()) {
            AchievementsModel newAchievements(userId);
            int id = InsertAchievements(newAchievements);
            newAchievements.SetId(id);

            std::cout << "Created achievements for user id: " << userId << std::endl;

            return newAchievements;
        }

        return achievements[0];
    }
    catch (std::exception& e) {
        std::cerr << "Error getting achievements: " << e.what() << std::endl;
        throw;
    }
}

void Database::UpdateAchievements(const AchievementsModel& achievements) {
    storage.update(achievements);
}

bool Database::AchievementsExistForUser(int userId) {
    try {
        auto count = storage.count<AchievementsModel>(
            where(c(&AchievementsModel::GetUserId) == userId)
        );
        return count > 0;
    }
    catch (std::exception& e) {
        std::cerr << "Error checking achievements existence: " << e.what() << std::endl;
        return false;
    }
}

typedef bool (AchievementsModel::* AchievementGetter)() const;

static const std::unordered_map<std::string, AchievementGetter> ACHIEVEMENT_GETTERS = {
    {"allOnRed", &AchievementsModel::GetAllOnRed},
    {"harryPotter", &AchievementsModel::GetHarryPotter},
    {"soothsayer", &AchievementsModel::GetSoothsayer},
    {"taxEvader", &AchievementsModel::GetTaxEvader},
    {"gambler", &AchievementsModel::GetGambler},
    {"peasant", &AchievementsModel::GetPeasant},
    {"seriousPlayer", &AchievementsModel::GetSeriousPlayer},
    {"talentedPlayer", &AchievementsModel::GetTalentedPlayer},
    {"jack", &AchievementsModel::GetJack},
    {"zeroEffort", &AchievementsModel::GetZeroEffort},
    {"vanillaW", &AchievementsModel::GetVanillaW},
    {"highRisk", &AchievementsModel::GetHighRisk},
    {"perfectGame", &AchievementsModel::GetPerfectGame},
    {"sixSeven", &AchievementsModel::GetSixSeven}
};



typedef void (AchievementsModel::* AchievementSetter)(bool);

static const std::unordered_map<std::string, AchievementSetter> ACHIEVEMENT_SETTERS = {
    {"allOnRed", &AchievementsModel::SetAllOnRed},
    {"harryPotter", &AchievementsModel::SetHarryPotter},
    {"soothsayer", &AchievementsModel::SetSoothsayer},
    {"taxEvader", &AchievementsModel::SetTaxEvader},
    {"gambler", &AchievementsModel::SetGambler},
    {"peasant", &AchievementsModel::SetPeasant},
    {"seriousPlayer", &AchievementsModel::SetSeriousPlayer},
    {"talentedPlayer", &AchievementsModel::SetTalentedPlayer},
    {"jack", &AchievementsModel::SetJack},
    {"zeroEffort", &AchievementsModel::SetZeroEffort},
    {"vanillaW", &AchievementsModel::SetVanillaW},
    {"highRisk", &AchievementsModel::SetHighRisk},
    {"perfectGame", &AchievementsModel::SetPerfectGame},
    {"sixSeven", &AchievementsModel::SetSixSeven}
};

std::vector<std::string> Database::UnlockAchievements(int userId, const std::unordered_map<std::string, bool>& achievementConditions) {
    std::vector<std::string> newlyUnlocked;
    try {
        AchievementsModel achievements = GetAchievementsByUserId(userId);
        bool modified = false;

        for (const auto& [key, condition] : achievementConditions) {
            auto setterIt = ACHIEVEMENT_SETTERS.find(key);
            if (setterIt == ACHIEVEMENT_SETTERS.end()) {
                std::cerr << "Unknown achievement key: " << key << std::endl;
                continue;
            }

            if (condition) {
                auto getterIt = ACHIEVEMENT_GETTERS.find(key);
                if (getterIt != ACHIEVEMENT_GETTERS.end()) {
                    auto getter = getterIt->second;
                    if (!(achievements.*getter)()) {
                        auto setter = setterIt->second;
                        (achievements.*setter)(true);
                        newlyUnlocked.push_back(key);
                        modified = true;
                        std::cout << "Unlocked achievement: " << key << std::endl;
                    }
                }
            }
        }

        if (modified) {
            UpdateAchievements(achievements);
            std::cout << "Achievements updated for user " << userId << std::endl;

            if (CheckAndUnlockJack(userId)) {
                newlyUnlocked.push_back("jack");
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << "Error unlocking achievements: " << e.what() << std::endl;
    }
    return newlyUnlocked;
}

bool Database::CheckAndUnlockJack(int userId)
{
    try {
        AchievementsModel achievements = GetAchievementsByUserId(userId);
        if (achievements.GetJack()) {
            return false;
        }

        bool hasAllRoles = achievements.GetHarryPotter() &&
            achievements.GetSoothsayer() &&
            achievements.GetTaxEvader() &&
            achievements.GetGambler() &&
            achievements.GetPeasant();

        if (hasAllRoles) {
            achievements.SetJack(true);
            UpdateAchievements(achievements);
            std::cout << "Congratulations! User " << userId << " unlocked 'Jack of All Trades' !" << std::endl;
            return true;
        }
    }
    catch (std::exception& e) {
        std::cerr << "Error checking Jack achievement: " << e.what() << std::endl;
    }
    return false;
}

PlaytimeModel Database::GetPlaytimeByUserId(int userId) {
    try {
        auto result = storage.get_all<PlaytimeModel>(where(c(&PlaytimeModel::GetUserId) == userId));

        if (result.empty()) {
            PlaytimeModel pt(userId);
            int id = storage.insert(pt);
            pt.SetId(id);
            return pt;
        }
        return result[0];
    }
    catch (std::exception& e) {
        std::cerr << "Error getting playtime: " << e.what() << std::endl;
        throw;
    }
}

void Database::UpdatePlaytime(const PlaytimeModel& pt) {
    try {
        storage.update(pt);
    }
    catch (std::exception& e) {
        std::cerr << "Error updating playtime: " << e.what() << std::endl;
    }
}

bool Database::BackupDatabase(const std::string& backupPath) const
{
    try {
        const std::filesystem::path path(backupPath);
        std::filesystem::create_directories(path.parent_path());

        const std::ifstream source(dbPath, std::ios::binary);
        if (!source) {
            std::cerr << "Error: Could not open source database file" << std::endl;
            return false;
        }

        std::ofstream dest(backupPath, std::ios::binary);
        if (!dest) {
            std::cerr << "Error: Could not create backup file" << std::endl;
            return false;
        }

        dest << source.rdbuf();

        std::cout << "Backup created successfully: " << backupPath << std::endl;
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Backup error: " << e.what() << std::endl;
        return false;
    }
}

bool Database::RestoreFromBackup(const std::string& backupPath)
{
    try {
        std::ifstream source(backupPath, std::ios::binary);
        if (!source) {
            std::cerr << "Error: Could not open backup file" << std::endl;
            return false;
        }

        std::ofstream dest(dbPath, std::ios::binary);
        if (!dest) {
            std::cerr << "Error: Could not overwrite database file" << std::endl;
            return false;
        }

        dest << source.rdbuf();
        dest.close();
        source.close();

        std::cout << "Database restored from backup successfully: " << backupPath << std::endl;
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "Restore error: " << e.what() << std::endl;
        return false;
    }
}
