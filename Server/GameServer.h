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
#include "Gambler.h"
class Game {
public:
	Game(size_t numberOfPlayers);
	size_t WhoStartsFirst();
	bool IsGameOver(const IPlayer& currentPlayer);
	void OneRound(IPlayer& currentPlayer);
	void StartGame();
	void NextPlayer();
	IPlayer& GetCurrentPlayer();
	void FirstRoundDealing();
	int NumberOfPlayableCardsInHand();
private:
	size_t m_numberOfPlayers;
	size_t m_currentPlayerIndex = 0;
	std::vector<std::unique_ptr<IPlayer>> m_players;
	Pile m_ascPile1{ PileType::ASCENDING };
	Pile m_ascPile2{ PileType::ASCENDING };
	Pile m_descPile1{ PileType::DESCENDING };
	Pile m_descPile2{ PileType::DESCENDING };
	Deck m_wholeDeck;
};