#pragma once
#include "IAbility.h"
#include <iostream>
#include <exception>
class __declspec(dllexport) Peasant :
    public IAbility
{
private:
	AbilityType m_type = AbilityType::Peasant;
public:
	AbilityType GetAbilityType() const override;

    void UseAbility(TurnContext& ctx, size_t currentPIndex) override;
	bool CanUseAbility(TurnContext& ctx) const override;

	const bool HPActive() override;
	const bool GetHPFlag() const override;
	void SetHPFlag(bool state) override;
	void SetHPActive(bool state) override;

	const size_t GetGamblerUses() override;
	const bool GActive() override;
	void SetGActive(bool state) override;

	const bool IsTaxActive() override;
	void SetTaxActive(bool state) override;

	const bool IsSoothActive() override;
	void SetSoothState(bool state) override;
	const size_t GetSoothsayerUses() override;
};

