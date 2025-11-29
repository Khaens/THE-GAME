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

class Game {
public:
	Game(std::vector<UserModel>& users);
	size_t WhoStartsFirst();
	bool IsGameOver(const IPlayer& currentPlayer);
	void OneRound(IPlayer& currentPlayer);
	void StartGame();
	void NextPlayer();
	IPlayer& GetCurrentPlayer();
	void FirstRoundDealing();
	int NumberOfPlayableCardsInHand();
	bool CanPlaceCard(const Card* card, Pile& pile);
	Pile* GetPile(const std::string& pileChoice);
	Card* DrawCard();
	size_t GetDeckSize() const;


	void ShowCtx();
private:
	size_t m_numberOfPlayers;
	size_t m_currentPlayerIndex = 0;
	std::vector<std::unique_ptr<IPlayer>> m_players;
	Pile m_ascPile1{ PileType::ASCENDING };
	Pile m_ascPile2{ PileType::ASCENDING };
	Pile m_descPile1{ PileType::DESCENDING };
	Pile m_descPile2{ PileType::DESCENDING };
	Deck m_wholeDeck;
	TurnContext m_ctx;
};