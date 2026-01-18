#include "Peasant.h"

AbilityType Peasant::GetAbilityType() const
{
	return m_type;
}

void Peasant::UseAbility(TurnContext& ctx, size_t currentPIndex)
{
	ctx.PeasantAbilityUse = true;
	ctx.PeasantPlayerIndex = currentPIndex;
}

bool Peasant::CanUseAbility(TurnContext& ctx) const
{
	return true;
}

const bool Peasant::HPActive()
{
	return false;
}

const bool Peasant::GetHPFlag() const
{
	return false;
}

void Peasant::SetHPFlag(bool state)
{
}

void Peasant::SetHPActive(bool state)
{
}

const size_t Peasant::GetGamblerUses()
{
	return 0;
}

const bool Peasant::GActive()
{
	return false;
}

void Peasant::SetGActive(bool state)
{
}

const bool Peasant::IsTaxActive()
{
	return false;
}

void Peasant::SetTaxActive(bool state)
{
}

const bool Peasant::IsSoothActive()
{
	return false;
}

void Peasant::SetSoothState(bool state)
{
}

const size_t Peasant::GetSoothsayerUses()
{
	return 0;
}

const size_t Peasant::GetTaxEvaderUses()
{
	return 0;
}

bool Peasant::GetSameTurn()
{
	return false;
}

void Peasant::SetSameTurn(bool sameTurn)
{
}
