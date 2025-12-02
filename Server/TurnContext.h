#pragma once

//stores info about ability usages

struct TurnContext {
	int baseRequired = 2;
	int currentRequired = 2;

	size_t HPplayerIndex = -1;

	size_t GamblerPlayerIndex = -1;

	bool endgame = false;

};