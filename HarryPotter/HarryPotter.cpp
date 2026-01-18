#include "HarryPotter.h"
#include <stdexcept>

AbilityType HarryPotter::GetAbilityType() const
{
	return m_type;
}

void HarryPotter::UseAbility(TurnContext& ctx, size_t currentPIndex)
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
}

const bool HarryPotter::IsTaxActive()
{
	return false;
}

void HarryPotter::SetTaxActive(bool state)
{
}

const bool HarryPotter::IsSoothActive()
{
	return false;
}

void HarryPotter::SetSoothState(bool state)
{
}

const size_t HarryPotter::GetSoothsayerUses()
{
	return 0;
}

const size_t HarryPotter::GetTaxEvaderUses()
{
	return 0;
}

bool HarryPotter::GetSameTurn()
{
	return m_sameTurn;
}

void HarryPotter::SetSameTurn(bool sameTurn)
{
	m_sameTurn = sameTurn;
}
