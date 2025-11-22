#pragma once

#include "TurnContext.h"

//(only in earlygame) minimum required cards = 1. Can only use it 3 times.
//in late game, the number of times he used this ability = the number of rounds he has to place
// 2 times the minimum required cards

class Gambler
{
public:
	void UseAbility(TurnContext& ctx, size_t currentPIndex);
	bool CanUseAbility(TurnContext& ctx) const;
private:
	size_t m_usesLeft = 2;
};

