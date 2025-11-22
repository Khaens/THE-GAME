#pragma once

#include "TurnContext.h"
//(only in endgame) can place any card he wants, but if no other player plays a 
// +/- 10 card until his round then it's game over

class Harry_Potter 
{
public:
	void UseAbility(TurnContext& ctx,size_t CurrentPIndex);
	bool CanUseAbility(TurnContext& ctx) const;

private:
	bool m_canUse = true;
};

