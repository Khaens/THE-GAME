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
	throw std::runtime_error("Peasant called HP function");
}

const bool Peasant::GetHPFlag() const
{
	throw std::runtime_error("Peasant called HP function");
}

void Peasant::SetHPFlag(bool state)
{
	throw std::runtime_error("Peasant called HP function");
}

void Peasant::SetHPActive(bool state)
{
	throw std::runtime_error("Peasant called HP function");
}

const size_t Peasant::GetGamblerUses()
{
	throw std::runtime_error("Peasant called Gambler function");
}

const bool Peasant::GActive()
{
	throw std::runtime_error("Peasant called Gambler function");
}

void Peasant::SetGActive(bool state)
{
	throw std::runtime_error("Peasant called Gambler function");
}

const bool Peasant::IsTaxActive()
{
	throw std::runtime_error("Peasant called TaxEvader function");
}

void Peasant::SetTaxActive(bool state)
{
	throw std::runtime_error("Peasant called TaxEvader function");
}

const bool Peasant::IsSoothActive()
{
	throw std::runtime_error("Peasant called Soothsayer function");
}

void Peasant::SetSoothState(bool state)
{
	throw std::runtime_error("Peasant called Soothsayer function");
}



