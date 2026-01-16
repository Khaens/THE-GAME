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
	throw std::runtime_error("HP called Gambler function");
	return -1;
}

const bool HarryPotter::GActive()
{
	throw std::runtime_error("HP called Gambler function");
	return false;
}

void HarryPotter::SetGActive(bool state)
{
	throw std::runtime_error("HP called Gambler function");
}

const bool HarryPotter::IsTaxActive()
{
	throw std::runtime_error("HP called TaxEvader function");
}

void HarryPotter::SetTaxActive(bool state)
{
	throw std::runtime_error("HP called TaxEvader function");
}

const bool HarryPotter::IsSoothActive()
{
	throw std::runtime_error("HP called Soothsayer function");
}

void HarryPotter::SetSoothState(bool state)
{
	throw std::runtime_error("HP called Soothsayer function");
}


