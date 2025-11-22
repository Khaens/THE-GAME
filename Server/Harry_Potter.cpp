#include "Harry_Potter.h"
#include "GameServer.h"

void Harry_Potter::UseAbility(TurnContext& ctx, size_t currentPIndex) // special round rules for said ability
{
	ctx.HPplayerIndex = currentPIndex;
	ctx.HPOverrideThisTurn = true;
	ctx.HPAbilityAvailable = false;
	ctx.HPFlag = true;
	m_canUse = false;
}

bool Harry_Potter::CanUseAbility(TurnContext& ctx) const
{
	if (ctx.endgame && m_canUse) return true;
	return false;
}


