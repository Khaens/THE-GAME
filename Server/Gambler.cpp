#include "Gambler.h"


void Gambler::UseAbility(TurnContext& ctx, size_t currentPIndex) //special round rules for said ability
{
	if(m_usesLeft > 0 && !ctx.endgame) {
		m_usesLeft--;
		ctx.GamblerPlayerIndex = currentPIndex;
		ctx.currentRequired = ctx.baseRequired - 1;
		ctx.GamblerUses++;
		ctx.GamblerOverrideThisTurn = true;
	}
}

bool Gambler::CanUseAbility(TurnContext& ctx) const
{
	return (!ctx.endgame && m_usesLeft > 0);
}