#pragma once

#include "TurnContext.h"

class Gambler
{
public:
	void UseAbility(TurnContext& ctx, size_t currentPIndex);
	bool CanUseAbility(TurnContext& ctx) const;
private:
	size_t m_usesLeft = 2;
};

