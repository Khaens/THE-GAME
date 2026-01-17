#include "HarryPotter.h"
#include <stdexcept>

AbilityType HarryPotter::GetAbilityType() const
{
	return m_type;
}

void HarryPotter::UseAbility(TurnContext& ctx, size_t currentPIndex) // special round rules for said ability
{
	ctx.HPplayerIndex = currentPIndex;
	m_active = true;
	m_HPFlag = true;
	m_canUse = false;
}

bool HarryPotter::CanUseAbility(TurnContext& ctx) const
{
	if (ctx.endgame && m_canUse) return true;
	return false;
}

const bool HarryPotter::HPActive()
{
	return m_active;
}

const bool HarryPotter::GetHPFlag() const
{
	return m_HPFlag;
}

void HarryPotter::SetHPFlag(bool state)
{
	m_HPFlag = state;
}

void HarryPotter::SetHPActive(bool state)
{
	m_active = state;
}

const size_t HarryPotter::GetGamblerUses()
{
	return 0;
}

const bool HarryPotter::GActive()
{
	return false;
}

void HarryPotter::SetGActive(bool state)
{
	// No-op
}

const bool HarryPotter::IsTaxActive()
{
	return false;
}

void HarryPotter::SetTaxActive(bool state)
{
	// No-op
}

const bool HarryPotter::IsSoothActive()
{
	return false;
}

void HarryPotter::SetSoothState(bool state)
{
	// No-op
}

const size_t HarryPotter::GetSoothsayerUses()
{
	return 0; // HarryPotter doesn't have soothsayer uses
}

bool HarryPotter::GetSameTurn()
{
	return m_sameTurn;
}

void HarryPotter::SetSameTurn(bool sameTurn)
{
	m_sameTurn = sameTurn;
}
