#include "AchievementsModel.h"

AchievementsModel::AchievementsModel(int userId) : userId(userId) {}

int AchievementsModel::GetId() const {
    return id;
}

int AchievementsModel::GetUserId() const {
    return userId;
}

bool AchievementsModel::GetAllOnRed() const {
    return allOnRed;
}

bool AchievementsModel::GetHarryPotter() const {
    return harryPotter;
}

bool AchievementsModel::GetSeriousPlayer() const {
    return seriousPlayer;
}

void AchievementsModel::SetId(int newId) {
    id = newId;
}

void AchievementsModel::SetUserId(int newUserId) {
    userId = newUserId;
}

void AchievementsModel::SetAllOnRed(bool value) {
    allOnRed = value;
}

void AchievementsModel::SetHarryPotter(bool value) {
    harryPotter = value;
}

void AchievementsModel::SetSeriousPlayer(bool value) {
    seriousPlayer = value;
}