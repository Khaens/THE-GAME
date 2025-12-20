#pragma once

struct GameStatistics {
	bool atLeastTwoCardsInEndgame = true;
	int lastPlayedCardValue = -1; //for 67achievement
	int taxEvaderUses = 0;
	bool perfectGame = true; // true until player plays a card with difference > 3
};