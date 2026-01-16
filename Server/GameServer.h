#pragma once

constexpr size_t PILES_AMOUNT = 4;
constexpr int CARD_SET = 100;
constexpr size_t MAX_PLAYERS = 5;
constexpr size_t MIN_PLAYERS = 2;

enum class Info {
	PILE_NOT_FOUND,
	CARD_NOT_PLAYABLE,
	NOT_CURRENT_PLAYER_TURN,
	ABILITY_NOT_AVAILABLE,
	NOT_ENOUGH_PLAYED_CARDS,
	TAX_ABILITY_USED,
	PEASANT_ABILITY_USED,
	GAME_WON,
	GAME_LOST,
	TURN_ENDED,
	ABILITY_USED,
	CARD_PLACED
};

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
import PileServer;
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
	std::array<std::unique_ptr<Pile>, PILES_AMOUNT> m_piles;
	Deck m_wholeDeck;
	TurnContext m_ctx;
	Database& m_database;
	std::unordered_map<int, GameStatistics> m_gameStats;
	mutable std::mutex m_stateMutex;

public:
	Game(std::vector<UserModel>& users, Database& db);
	~Game();

    // Rule of 5: Move Semantics
    Game(Game&& other) noexcept;
    Game& operator=(Game&& other) noexcept;

    // Rule of 5: Delete Copy
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

	size_t WhoStartsFirst();
	bool IsGameOver(IPlayer& currentPlayer);
	void StartGame();
	void NextPlayer();

	Info PlaceCard(size_t playerIndex, const Card& card, int chosenPile);
	Info UseAbility(size_t playerIndex);
	Info EndTurn(size_t playerIndex);

	void UnlockAchievements();
	void CheckAchievements(IPlayer& currentPlayer);
	void UpdateGameStats(bool won);

	std::unique_ptr<Card> DrawCard();

	size_t GetDeckSize() const;
	IPlayer& GetCurrentPlayer();
	TurnContext& GetCtx();
	const std::vector<std::unique_ptr<IPlayer>>& GetPlayers();
	std::array<Pile*, PILES_AMOUNT> GetPiles();
	Deck& GetDeck();
	std::mutex& GetStateMutex() { return m_stateMutex; }
};