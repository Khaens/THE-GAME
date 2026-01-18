#include "StatisticsModel.h"
#include <algorithm>
#include <cmath>

StatisticsModel::StatisticsModel(int userId) : userId(userId)
{
}

int StatisticsModel::GetId() const { return id; }
int StatisticsModel::GetUserId() const { return userId; }
int StatisticsModel::GetGamesWon() const { return gamesWon; }
int StatisticsModel::GetGamesPlayed() const { return gamesPlayed; }
float StatisticsModel::GetWinRate() const { return winRate; }
int StatisticsModel::GetTotalCardsLeftInLosses() const { return totalCardsLeftInLosses; }
float StatisticsModel::GetPerformanceScore() const { return performanceScore; }

void StatisticsModel::SetId(int newId) { id = newId; }
void StatisticsModel::SetUserId(int newUserId) { userId = newUserId; }
void StatisticsModel::SetGamesWon(int newGamesWon) { gamesWon = newGamesWon; }
void StatisticsModel::SetGamesPlayed(int newGamesPlayed) { gamesPlayed = newGamesPlayed; }
void StatisticsModel::SetWinRate(float newWinRate) { winRate = newWinRate; }
void StatisticsModel::SetTotalCardsLeftInLosses(int v) { totalCardsLeftInLosses = v; }
void StatisticsModel::SetPerformanceScore(float v) { performanceScore = v; }

void StatisticsModel::UpdatePerformanceScore(int maxHandSize) {
    if (gamesPlayed == 0) {
        performanceScore = 0.0f;
        return;
    }

    float winComponent = std::min(5.0f, (winRate / 25.0f) * 5.0f);
    // 40% win - 60% efficiency 
    // 25% win rate - great for any player (tough game)
    int losses = gamesPlayed - gamesWon;
    float efficiency = 1.0f;

    if (losses > 0) {
        float maxPossibleCards = static_cast<float>(losses) * maxHandSize;
        efficiency = 1.0f - (static_cast<float>(totalCardsLeftInLosses) / maxPossibleCards);
    }

    float effComponent = efficiency * 5.0f;
    float rawScore = (winComponent * 0.4f) + (effComponent * 0.6f);

    performanceScore = std::round(rawScore * 100.0f) / 100.0f;
}