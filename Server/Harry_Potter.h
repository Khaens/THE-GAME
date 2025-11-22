#pragma once

#include "TurnContext.h"

class Harry_Potter
{
public:
	void UseAbility(TurnContext& ctx,size_t CurrentPIndex);
	bool CanUseAbility(TurnContext& ctx) const;

private:
	bool m_canUse = true;
};

