#include "Gambler.h"
#include <iostream>

AbilityType Gambler::GetAbilityType() const
{
	return m_type;
}

void Gambler::UseAbility(TurnContext& ctx, size_t CurrentPIndex)
{
	if (!CanUseAbility(ctx)) return;

	m_usesLeft--;
	m_uses++;
	m_active = true;
	ctx.currentRequired = 1;
}

bool Gambler::CanUseAbility(TurnContext& ctx) const
{
	if (!ctx.endgame) {
		return m_usesLeft > 0;
	}
	return false;
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
}

void Gambler::SetHPActive(bool state)
{
}

const bool Gambler::IsTaxActive()
{
	return false;
}

void Gambler::SetTaxActive(bool state)
{
}

const bool Gambler::IsSoothActive()
{
	return false;
}

void Gambler::SetSoothState(bool state)
{
}

const size_t Gambler::GetSoothsayerUses()
{
	return 0;
}

const size_t Gambler::GetTaxEvaderUses()
{
	return 0;
}

bool Gambler::GetSameTurn()
{
	return false;
}

void Gambler::SetSameTurn(bool sameTurn)
{
}
