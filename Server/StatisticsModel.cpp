#include "StatisticsModel.h"

StatisticsModel::StatisticsModel(int userId) : userId(userId)
{
}

int StatisticsModel::GetId() const { return id; }
int StatisticsModel::GetUserId() const { return userId; }
int StatisticsModel::GetGamesWon() const { return gamesWon; }
int StatisticsModel::GetGamesPlayed() const { return gamesPlayed; }
float StatisticsModel::GetWinRate() const { return winRate; }

void StatisticsModel::SetId(int newId) { id = newId; }
void StatisticsModel::SetUserId(int newUserId) { userId = newUserId; }
void StatisticsModel::SetGamesWon(int newGamesWon) { gamesWon = newGamesWon; }
void StatisticsModel::SetGamesPlayed(int newGamesPlayed) { gamesPlayed = newGamesPlayed; }
void StatisticsModel::SetWinRate(float newWinRate) { winRate = newWinRate; }