#pragma once
#include <iostream>
#include <thread>
#include <vector>
#include <future>
#include <chrono>
#include <random>
#include "Player.h"
#include "Pile.h"
#include "Deck.h"
class Game {
public:
	Game(size_t numberOfPlayers);
	size_t WhoStartsFirst();
	bool IsGameOver(const Player* currentPlayer);
	void OneRound(Player* currentPlayer);
	void StartGame();
	void NextPlayer();
	Player& GetCurrentPlayer();
	void FirstRoundDealing();
	int NumberOfPlayableCardsInHand();
private:
	size_t m_numberOfPlayers;
	size_t m_currentPlayerIndex = 0;
	std::vector<Player> m_players;
	Pile m_ascPile1{ PileType::ASCENDING };
	Pile m_ascPile2{ PileType::ASCENDING };
	Pile m_descPile1{ PileType::DESCENDING };
	Pile m_descPile2{ PileType::DESCENDING };
	Deck m_wholeDeck;
};