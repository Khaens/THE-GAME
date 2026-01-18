#pragma once
#include "IAbility.h"

//(only in earlygame) minimum required cards = 1. Can only use it 3 times.
//in late game, the number of times he used this ability = the number of rounds he has to place
// 2 times the minimum required cards

class __declspec(dllexport) Gambler : public IAbility
{
public:
	AbilityType GetAbilityType() const override;

	void UseAbility(TurnContext& ctx, size_t currentPIndex) override;
	bool CanUseAbility(TurnContext& ctx) const override;

	void SetGActive(bool state) override;
	const bool GActive() override;
	const size_t GetGamblerUses() override;

	const bool HPActive() override;
	const bool GetHPFlag() const override;
	void SetHPFlag(bool state) override;
	void SetHPActive(bool state) override;

	const bool IsTaxActive() override;
	void SetTaxActive(bool state) override;

	const bool IsSoothActive() override;
	void SetSoothState(bool state) override;
	const size_t GetSoothsayerUses() override;

	const size_t GetTaxEvaderUses() override;

	bool GetSameTurn() override;
	void SetSameTurn(bool sameTurn) override;
private:
	size_t m_usesLeft = 3;
	bool m_active = false;
	AbilityType m_type = AbilityType::Gambler;
};

