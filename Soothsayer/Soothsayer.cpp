#include "Soothsayer.h"
#include <stdexcept>

AbilityType Soothsayer::GetAbilityType() const
{
	return m_type;
}

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
	return false;
}

const bool Soothsayer::GetHPFlag() const
{
	return false;
}

void Soothsayer::SetHPFlag(bool state)
{
	// No-op
}

void Soothsayer::SetHPActive(bool state)
{
	// No-op
}

const size_t Soothsayer::GetGamblerUses()
{
	return 0;
}

const bool Soothsayer::GActive()
{
	return false;
}

void Soothsayer::SetGActive(bool state)
{
	// No-op
}

const bool Soothsayer::IsTaxActive()
{
	return false;
}

void Soothsayer::SetTaxActive(bool state)
{
	// No-op
}

const size_t Soothsayer::GetSoothsayerUses()
{
	return m_usesLeft;
}

bool Soothsayer::GetSameTurn()
{
	return false;
}

void Soothsayer::SetSameTurn(bool sameTurn)
{
	
}
