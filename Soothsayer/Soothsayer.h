#pragma once
#include "IAbility.h"
#include <cstdint>
class __declspec(dllexport) Soothsayer :
    public IAbility
{
public:
    void UseAbility(TurnContext& ctx, size_t currentPIndex) override;
    bool CanUseAbility(TurnContext& ctx) const override;

    const bool IsSoothActive() override;
    void SetSoothState(bool state) override;

    const bool HPActive() override;
    const bool GetHPFlag() const override;
    void SetHPFlag(bool state) override;
    void SetHPActive(bool state) override;

    const size_t GetGamblerUses() override;
    const bool GActive() override;
    void SetGActive(bool state) override;

    const bool IsTaxActive() override;
    void SetTaxActive(bool state) override;

private:
    bool m_active = false;
    uint8_t m_usesLeft = 2;
};

