#pragma once

struct GameStatistics {
	bool atLeastTwoCardsInEndgame = true;
	int lastPlayedCardValue = -1;
	int taxEvaderUses = 0;
	bool perfectGame = true;

	bool usedAbility = false;
	bool usedAnyAbility = false;
	bool wonGame = false;
	bool placed6And7InSameRound = false;
	int gamblerAbilityUses = 0;
	bool usedAllGamblerAbilities = false;
	int maxCardDifference = 0;
	bool playedWithAllAbilities = false;

	bool placed6ThisRound = false;
	bool placed7ThisRound = false;

	bool usedHarryPotter = false;
	bool usedSoothsayer = false;
	bool usedTaxEvader = false;
	bool usedGambler = false;
};