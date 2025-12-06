#pragma once
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

class Game {
public:
	Game(std::vector<UserModel>& users);
	size_t WhoStartsFirst();
	bool IsGameOver(const IPlayer& currentPlayer);
	void StartGame();
	void NextPlayer();

	Card* DrawCard();

	size_t GetDeckSize() const;
	IPlayer& GetCurrentPlayer();
	const std::vector<std::unique_ptr<IPlayer>>& GetPlayers();
	std::vector<Pile*> GetPiles();
	Deck& GetDeck();

	void ShowCtx();
private:
	size_t m_numberOfPlayers;
	size_t m_currentPlayerIndex = 0;
	std::vector<std::unique_ptr<IPlayer>> m_players;
	std::vector<Pile*> m_piles;
	Deck m_wholeDeck;
	TurnContext m_ctx;
};