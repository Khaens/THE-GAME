#pragma once

#ifdef ABILITYCORE_EXPORTS
#define ABILITY_API __declspec(dllexport)
#else
#define ABILITY_API __declspec(dllimport)
#endif

struct ABILITY_API TurnContext {
	int baseRequired = 2;
	int currentRequired = 2;

	size_t HPplayerIndex = -1;
	size_t GamblerPlayerIndex = -1;
	size_t TaxEvPlayerIndex = -1;
	size_t SoothPlayerIndex = -1;
	size_t PeasantPlayerIndex = -1;

	size_t placedCardsThisTurn = 0;

	bool PeasantAbilityUse = false;

	bool endgame = false;
	
	size_t GamblerEndgamePenaltyTurns = 0;
};