#include "TaxEvader.h"
#include <stdexcept>

//ENDGAME!
// skips his turn (doesn't need to play any cards) but the next player has to place 2x
//minimum cards reqired

AbilityType TaxEvader::GetAbilityType() const
{
    return m_type;
}

void TaxEvader::UseAbility(TurnContext& ctx, size_t CurrentPIndex)
{
    if (!CanUseAbility(ctx)) return;
    m_uses--;
    m_active = true;
    ctx.TaxEvPlayerIndex = CurrentPIndex;
    ctx.currentRequired = 0;
}

bool TaxEvader::CanUseAbility(TurnContext& ctx) const
{
    return (ctx.endgame && m_uses > 0);
}

const bool TaxEvader::IsTaxActive()
{
    return m_active;
}

void TaxEvader::SetTaxActive(bool state)
{
    m_active = state;
}

const bool TaxEvader::HPActive()
{
	return false;
}

const bool TaxEvader::GetHPFlag() const
{
	return false;
}

void TaxEvader::SetHPFlag(bool state)
{
	// No-op
}

void TaxEvader::SetHPActive(bool state)
{
	// No-op
}

const size_t TaxEvader::GetGamblerUses()
{
	return 0;
}

const bool TaxEvader::GActive()
{
	return false;
}

void TaxEvader::SetGActive(bool state)
{
	// No-op
}

const bool TaxEvader::IsSoothActive()
{
	return false;
}

void TaxEvader::SetSoothState(bool state)
{
	// No-op
}

const size_t TaxEvader::GetSoothsayerUses()
{
	return 0; // TaxEvader doesn't have soothsayer uses
}

const size_t TaxEvader::GetTaxEvaderUses()
{
	return m_uses;
}

bool TaxEvader::GetSameTurn()
{
	return false;
}

void TaxEvader::SetSameTurn(bool sameTurn)
{
 
}
