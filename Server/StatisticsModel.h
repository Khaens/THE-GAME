#pragma once

class StatisticsModel
{
private:
	int id = 0;
	int userId = 0;
	int gamesWon = 0;
	int gamesPlayed = 0;
	float winRate = 0.0f;
	int totalCardsLeftInLosses = 0;
	float performanceScore = 0.0f;

public:
	StatisticsModel() = default;
	StatisticsModel(int userId);
	int GetId() const;
	int GetUserId() const;
	int GetGamesWon() const;
	int GetGamesPlayed() const;
	float GetWinRate() const;
	int GetTotalCardsLeftInLosses() const;
	float GetPerformanceScore() const;

	void SetId(int newId);
	void SetUserId(int newUserId);
	void SetGamesWon(int newGamesWon);
	void SetGamesPlayed(int newGamesPlayed);
	void SetWinRate(float newWinRate);
	void SetTotalCardsLeftInLosses(int v);
	void SetPerformanceScore(float v);

	void UpdatePerformanceScore(int maxHandSize);
};
