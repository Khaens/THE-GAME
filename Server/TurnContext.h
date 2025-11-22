#pragma once

//stores info about ability usages

struct TurnContext {
	int baseRequired = 2;
	int currentRequired = 2;

	size_t HPplayerIndex = -1;
	bool HPOverrideThisTurn = false;
	bool HPFlag = false;

	size_t GamblerPlayerIndex = -1;
	int GamblerUses = 0;
	bool GamblerOverrideThisTurn = false;

	bool endgame = false;

};