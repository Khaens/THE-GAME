#pragma once

constexpr size_t PILES_AMOUNT = 4;
#include <unordered_map>
#include <iostream>
#include <thread>
#include <vector>
#include <future>
#include <chrono>
#include <random>
#include <memory>
#include "IPlayer.h"
#include "PlayerServer.h"
#include "Pile.h"
#include "DeckServer.h"
#include "TurnContext.h"
#include "Round.h"
#include "GameStatistics.h"
#include "Database.h"

class Game {
private:
	size_t m_numberOfPlayers;
	size_t m_currentPlayerIndex = 0;
	size_t m_pileIndex = 0;
	std::vector<std::unique_ptr<IPlayer>> m_players;
	std::array<Pile*, PILES_AMOUNT> m_piles;
	Deck m_wholeDeck;
	TurnContext m_ctx;
	Database& m_database;

public:
	std::unordered_map<int, GameStatistics> m_gameStats;
	Game(std::vector<UserModel>& users, Database& db);
	size_t WhoStartsFirst();
	bool IsGameOver(IPlayer& currentPlayer);
	void StartGame();
	void NextPlayer();

	void CheckAndUnlockAchievements();

	Card* DrawCard();

	size_t GetDeckSize() const;
	IPlayer& GetCurrentPlayer();
	const std::vector<std::unique_ptr<IPlayer>>& GetPlayers();
	std::array<Pile*, PILES_AMOUNT> GetPiles();
	Deck& GetDeck();

	void ShowCtx();

};