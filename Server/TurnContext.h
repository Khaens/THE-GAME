#pragma once


struct TurnContext {
	int baseRequired = 2;
	int currentRequired = 2;

	size_t HPplayerIndex = -1;
	bool HPOverrideThisTurn = false;
	bool HPAbilityAvailable = false;

	size_t GamblerPlayerIndex = -1;
	int GamblerUses = 0;
	bool GamblerOverrideThisTurn = false;

	bool endgame = false;

	bool HPFlag = false;
};