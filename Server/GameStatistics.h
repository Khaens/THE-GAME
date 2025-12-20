#pragma once

struct GameStatistics {
	bool atLeastTwoCardsInEndgame = true; //GameServer
	int lastPlayedCardValue = -1; //for 67achievement -> round
	int taxEvaderUses = 0; //round
	bool perfectGame = true; // true until player plays a card with difference > 3 // round
};