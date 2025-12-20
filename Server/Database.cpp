#include "Database.h"
#include <iostream>
#include <tuple>

Database::Database(const std::string& path) : storage(initStorage(path)), dbPath(path)
{
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

static std::string HashPassword(const std::string& password) {
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
        int userId = storage.insert(hashedUser);

        AchievementsModel achievements(userId);
        InsertAchievements(achievements);

        StatisticsModel statistics(userId);
        InsertStatistics(statistics);

        return userId;
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
        std::string test = HashPassword(plainPassword);
        std::cout << "Stored hash: " << rows[0] << ", Computed hash: " << test << std::endl;
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
    try {
        AchievementsModel achievements = GetAchievementsByUserId(id);

        storage.remove<AchievementsModel>(achievements.GetId());
        storage.remove<UserModel>(id);

        std::cout << "User " << id << " and associated achievements deleted." << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Error deleting user and associated data: " << e.what() << std::endl;
    }
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

static const std::unordered_map<std::string, std::string> ACHIEVEMENT_DESCRIPTIONS = {
    {"allOnRed", "ALL ON RED: You played as the Gambler and always placed at least 2 cards in every endgame round."},
    {"harryPotter", "HARRY POTTER: Played a game with the Harry Potter ability"},
    {"soothsayer", "SOOTHSAYER: Played a game with the Soothsayer ability"},
    {"taxEvader", "TAX EVADER: Played a game with the Tax Evader ability"},
    {"seriousPlayer", "SERIOUS PLAYER: You've won 5 games. Keep it up!"},
    {"talentedPlayer", "TALENTED PLAYER: You've won 8 or more games in your first ever 10 games. Keep it up!"},
    {"jack", "MASTER OF ALL TRADES: Played at least one game with all abilities. You are a truly versatile player!"},
    {"zeroEffort", "ZERO EFFORT: Won after using the Tax Evader ability at least 5 times. Laziness is the key to success."},
    {"vanillaW", "VANILLA VICTORY: Your team won without using any special abilities. Pure skill!"},
    {"highRisk", "HIGH-RISK, HIGH-REWARD: You played as the Gambler and utilised all your ability uses."},
    {"perfectGame", "PERFECT GAME: Won the game and always played cards with a maximum difference of 3 points between them throughout the entire match."},
    {"sixSeven", "SIX-SEVEN: You placed both 6 and 7 in a single round."}
};

static const std::unordered_map<std::string, AchievementGetter> ACHIEVEMENT_GETTERS = {
    {"allOnRed", &AchievementsModel::GetAllOnRed},
    {"harryPotter", &AchievementsModel::GetHarryPotter},
    {"soothsayer", &AchievementsModel::GetSoothsayer},
    {"taxEvader", &AchievementsModel::GetTaxEvader},
    {"seriousPlayer", &AchievementsModel::GetSeriousPlayer},
    {"talentedPlayer", &AchievementsModel::GetTalentedPlayer},
    {"jack", &AchievementsModel::GetJack},
    {"zeroEffort", &AchievementsModel::GetZeroEffort},
    {"vanillaW", &AchievementsModel::GetVanillaW},
    {"highRisk", &AchievementsModel::GetHighRisk},
    {"perfectGame", &AchievementsModel::GetPerfectGame},
    {"sixSeven", &AchievementsModel::GetSixSeven}
};

std::vector<std::string> Database::GetUnlockedAchievement(int userId)
{
    std::vector<std::string> achievedDescriptions;

    if (!AchievementsExistForUser(userId)) {
        return achievedDescriptions;
    }

    AchievementsModel achievements = GetAchievementsByUserId(userId);

    for (const auto& [key, getter] : ACHIEVEMENT_GETTERS) {
        if ((achievements.*getter)()) {
            achievedDescriptions.push_back(ACHIEVEMENT_DESCRIPTIONS.at(key));
        }
    }

    return achievedDescriptions;
}

typedef void (AchievementsModel::* AchievementSetter)(bool);

static const std::unordered_map<std::string, AchievementSetter> ACHIEVEMENT_SETTERS = {
    {"allOnRed", &AchievementsModel::SetAllOnRed},
    {"harryPotter", &AchievementsModel::SetHarryPotter},
    {"soothsayer", &AchievementsModel::SetSoothsayer},
    {"taxEvader", &AchievementsModel::SetTaxEvader},
    {"seriousPlayer", &AchievementsModel::SetSeriousPlayer},
    {"talentedPlayer", &AchievementsModel::SetTalentedPlayer},
    {"jack", &AchievementsModel::SetJack},
    {"zeroEffort", &AchievementsModel::SetZeroEffort},
    {"vanillaW", &AchievementsModel::SetVanillaW},
    {"highRisk", &AchievementsModel::SetHighRisk},
    {"perfectGame", &AchievementsModel::SetPerfectGame},
    {"sixSeven", &AchievementsModel::SetSixSeven}
};

void Database::UnlockAchievements(int userId, const std::unordered_map<std::string, bool>& achievementConditions) {
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
                auto setter = setterIt->second;
                (achievements.*setter)(true);
                modified = true;
                std::cout << "Unlocked achievement: " << key << std::endl;
            }
        }

        if (modified) {
            UpdateAchievements(achievements);
            std::cout << "Achievements updated for user " << userId << std::endl;
        }
    }
    catch (std::exception& e) {
        std::cerr << "Error unlocking achievements: " << e.what() << std::endl;
    }
}