#pragma once
#include "IAbility.h"

class __declspec(dllexport) TaxEvader : public IAbility
{
public:
	void UseAbility(TurnContext& ctx, size_t CurrentPIndex) override;
	bool CanUseAbility(TurnContext& ctx) const override;

	const bool IsTaxActive() override;
	void SetTaxActive(bool state) override;

	const bool HPActive() override;
	const bool GetHPFlag() const override;
	void SetHPFlag(bool state) override;
	void SetHPActive(bool state) override;

	const size_t GetGamblerUses() override;
	const bool GActive() override;
	void SetGActive(bool state) override;

	const bool IsSoothActive() override;
	void SetSoothState(bool state) override;
private:
	size_t m_uses = 2;
	bool m_active = false;

};

