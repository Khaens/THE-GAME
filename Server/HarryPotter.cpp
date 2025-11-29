#include "HarryPotter.h"
#include "GameServer.h"

void HarryPotter::UseAbility(TurnContext& ctx, size_t currentPIndex) // special round rules for said ability
{
	ctx.HPplayerIndex = currentPIndex;
	ctx.HPOverrideThisTurn = true;
	ctx.HPFlag = true;
	m_canUse = false;
}

bool HarryPotter::CanUseAbility(TurnContext& ctx) const
{
	if (ctx.endgame && m_canUse) return true;
	return false;
}


