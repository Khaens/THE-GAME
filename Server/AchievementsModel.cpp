#include "AchievementsModel.h"

AchievementsModel::AchievementsModel(int userId) : userId(userId)
{
}

int AchievementsModel::GetId() const { return id; }
int AchievementsModel::GetUserId() const { return userId; }
bool AchievementsModel::GetAllOnRed() const { return allOnRed; }
bool AchievementsModel::GetHarryPotter() const { return harryPotter; }
bool AchievementsModel::GetSoothsayer() const { return soothsayer; }
bool AchievementsModel::GetTaxEvader() const { return taxEvader; }
bool AchievementsModel::GetSeriousPlayer() const { return seriousPlayer; }
bool AchievementsModel::GetJack() const { return jack; }
bool AchievementsModel::GetZeroEffort() const { return zeroEffort; }
bool AchievementsModel::GetVanillaW() const { return vanillaW; }
bool AchievementsModel::GetHighRisk() const { return highRisk; }
bool AchievementsModel::GetPerfectGame() const { return perfectGame; }
bool AchievementsModel::GetSixSeven() const { return sixSeven; }

void AchievementsModel::SetId(int newId) { id = newId; }
void AchievementsModel::SetUserId(int newUserId) { userId = newUserId; }
void AchievementsModel::SetAllOnRed(bool value) { allOnRed = value; }
void AchievementsModel::SetHarryPotter(bool value) { harryPotter = value; }
void AchievementsModel::SetSoothsayer(bool value) { soothsayer = value; }
void AchievementsModel::SetTaxEvader(bool value) { taxEvader = value; }
void AchievementsModel::SetSeriousPlayer(bool value) { seriousPlayer = value; }
void AchievementsModel::SetJack(bool value) { jack = value; }
void AchievementsModel::SetZeroEffort(bool value) { zeroEffort = value; }
void AchievementsModel::SetVanillaW(bool value) { vanillaW = value; }
void AchievementsModel::SetHighRisk(bool value) { highRisk = value; }
void AchievementsModel::SetPerfectGame(bool value) { perfectGame = value; }
void AchievementsModel::SetSixSeven(bool value) { sixSeven = value; }
