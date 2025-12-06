#include "Soothsayer.h"
#include <stdexcept>

void Soothsayer::UseAbility(TurnContext& ctx, size_t currentPIndex)
{
	ctx.SoothPlayerIndex = currentPIndex;
	m_usesLeft--;
	m_active = true;
}

bool Soothsayer::CanUseAbility(TurnContext& ctx) const
{
	return !ctx.endgame && m_usesLeft > 0;
}

const bool Soothsayer::IsSoothActive()
{
	return m_active;
}

void Soothsayer::SetSoothState(bool state)
{
	m_active = state;
}

const bool Soothsayer::HPActive()
{
	throw std::runtime_error("Soothsayer called HP function");
}

const bool Soothsayer::GetHPFlag() const
{
	throw std::runtime_error("Soothsayer called HP function");
}

void Soothsayer::SetHPFlag(bool state)
{
	throw std::runtime_error("Soothsayer called HP function");
}

void Soothsayer::SetHPActive(bool state)
{
	throw std::runtime_error("Soothsayer called HP function");
}

const size_t Soothsayer::GetGamblerUses()
{
	throw std::runtime_error("Soothsayer called Gambler function");
}

const bool Soothsayer::GActive()
{
	throw std::runtime_error("Soothsayer called Gambler function");
}

void Soothsayer::SetGActive(bool state)
{
	throw std::runtime_error("Soothsayer called Gambler function");
}

const bool Soothsayer::IsTaxActive()
{
	throw std::runtime_error("Soothsayer called TaxEvader function");
}

void Soothsayer::SetTaxActive(bool state)
{
	throw std::runtime_error("Soothsayer called TaxEvader function");
}
