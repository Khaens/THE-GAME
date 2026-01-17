#include "Gambler.h"
#include <stdexcept>


AbilityType Gambler::GetAbilityType() const
{
	return m_type;
}

void Gambler::UseAbility(TurnContext& ctx, size_t currentPIndex) //special round rules for said ability
{
	if (m_usesLeft > 0 && !ctx.endgame) {
		m_usesLeft--;
		ctx.GamblerPlayerIndex = currentPIndex;
		ctx.currentRequired = ctx.baseRequired - 1;
		m_uses++;
		m_active = true;
		// Track how many endgame penalty turns: each use = 1 penalty turn in endgame
		ctx.GamblerEndgamePenaltyTurns++;
	}
}

bool Gambler::CanUseAbility(TurnContext& ctx) const
{
	return (!ctx.endgame && m_usesLeft > 0);
}

void Gambler::SetGActive(bool state)
{
	m_active = state;
}

const bool Gambler::GActive()
{
	return m_active;
}

const size_t Gambler::GetGamblerUses()
{
	// Return uses remaining, no side effects
	return m_usesLeft;
}

const bool Gambler::HPActive()
{
	return false;
}

const bool Gambler::GetHPFlag() const
{
	return false;
}

void Gambler::SetHPFlag(bool state)
{
	// No-op
}

void Gambler::SetHPActive(bool state)
{
	// No-op
}

const bool Gambler::IsTaxActive()
{
	return false;
}

void Gambler::SetTaxActive(bool state)
{
	// No-op
}

const bool Gambler::IsSoothActive()
{
	return false;
}

void Gambler::SetSoothState(bool state)
{
	// No-op
}

const size_t Gambler::GetSoothsayerUses()
{
	return 0; // Gambler doesn't have soothsayer uses
}

bool Gambler::GetSameTurn()
{
	return false;
}

void Gambler::SetSameTurn(bool sameTurn)
{
	
}

