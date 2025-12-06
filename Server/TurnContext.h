//stores info about ability usages
#pragma once

struct TurnContext {
	int baseRequired = 2;
	int currentRequired = 2;

	size_t HPplayerIndex = -1;
	size_t GamblerPlayerIndex = -1;
	size_t TaxEvPlayerIndex = -1;
	size_t SoothPlayerIndex = -1;

	bool endgame = false;
};