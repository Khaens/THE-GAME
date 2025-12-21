#pragma once

struct GameStatistics {
	bool atLeastTwoCardsInEndgame = true;
	int lastPlayedCardValue = -1;
	int taxEvaderUses = 0;
	bool perfectGame = true;

	bool wonGame = false;
	bool placed6And7InSameRound = false;
	int gamblerAbilityUses = 0;
	bool usedAllGamblerAbilities = false;
	bool playedWithAllAbilities = false;

	bool usedHarryPotter = false;
	bool usedSoothsayer = false;
	bool usedTaxEvader = false;
	bool usedGambler = false;
	bool usedPeasant = false;
};