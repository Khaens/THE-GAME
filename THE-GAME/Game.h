#pragma once
#include <iostream>
#include <thread>
#include <vector>
#include <future>
#include <chrono>
#include <random>
#include "Player.h"

class Game {
public:
	Game(size_t numberOfPlayers);
	size_t WhoStartsFirst();

private:
	size_t m_numberOfPlayers;
	size_t m_currentPlayerIndex = 0;
	std::vector<Player> m_players;

	void NextPlayer();
	Player& GetCurrentPlayer();
};