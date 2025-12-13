#pragma once
#include "IAbility.h"
//(only in endgame) can place any card he wants, but if no other player plays a 
// +/- 10 card until his round then it's game over

class __declspec(dllexport) HarryPotter : public IAbility
{
public:
	void UseAbility(TurnContext& ctx, size_t CurrentPIndex) override;
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
private:
	bool m_canUse = true;
	bool m_active = false;
	bool m_HPFlag = false;
};

