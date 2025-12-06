#include "Gambler.h"
#include <stdexcept>


void Gambler::UseAbility(TurnContext& ctx, size_t currentPIndex) //special round rules for said ability
{
	if (m_usesLeft > 0 && !ctx.endgame) {
		m_usesLeft--;
		ctx.GamblerPlayerIndex = currentPIndex;
		ctx.currentRequired = ctx.baseRequired - 1;
		m_uses++;
		m_active = true;
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
	if (m_uses > 0) m_uses--;
	return m_uses > 0 ? m_uses + 1 : m_uses;
}

const bool Gambler::HPActive()
{
	throw std::runtime_error("Gambler called HP function");
}

const bool Gambler::GetHPFlag() const
{
	throw std::runtime_error("Gambler called HP function");
	return false;
}

void Gambler::SetHPFlag(bool state)
{
	throw std::runtime_error("Gambler called HP function");
}

void Gambler::SetHPActive(bool state)
{
	throw std::runtime_error("Gambler called HP function");
}

const bool Gambler::IsTaxActive()
{
	throw std::runtime_error("Gambler called TaxEvader function");
}

void Gambler::SetTaxActive(bool state)
{
	throw std::runtime_error("Gambler called TaxEvader function");
}

const bool Gambler::IsSoothActive()
{
	throw std::runtime_error("Gambler called Soothsayer function");
}

void Gambler::SetSoothState(bool state)
{
	throw std::runtime_error("Gambler called Soothsayer function");
}
