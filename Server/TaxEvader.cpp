#include "TaxEvader.h"
#include <stdexcept>

//ENDGAME!
// skips his turn (doesn't need to play any cards) but the next player has to place 2x
//minimum cards reqired

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
    throw std::runtime_error("TaxEvader called HP function");
}

const bool TaxEvader::GetHPFlag() const
{
    throw std::runtime_error("TaxEvader called HP function");
}

void TaxEvader::SetHPFlag(bool state)
{
    throw std::runtime_error("TaxEvader called HP function");
}

void TaxEvader::SetHPActive(bool state)
{
    throw std::runtime_error("TaxEvader called HP function");
}

const size_t TaxEvader::GetGamblerUses()
{
    throw std::runtime_error("TaxEvader called Gambler function");
}

const bool TaxEvader::GActive()
{
    throw std::runtime_error("TaxEvader called Gambler function");
}

void TaxEvader::SetGActive(bool state)
{
    throw std::runtime_error("TaxEvader called Gambler function");
}
