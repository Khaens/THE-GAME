#pragma once
#ifdef ABILITYCORE_EXPORTS
#define ABILITY_API __declspec(dllexport)
#else
#define ABILITY_API __declspec(dllimport)
#endif

#include "TurnContext.h"
#include "AbilityType.h"

class ABILITY_API IAbility {
protected:
	IAbility() = default;
private:
	IAbility(const IAbility&) = delete;
	IAbility& operator=(const IAbility&) = delete;
public:
	virtual ~IAbility() = default;

	virtual AbilityType GetAbilityType() const = 0;

	virtual void UseAbility(TurnContext& ctx, size_t CurrentPIndex) = 0;
	virtual bool CanUseAbility(TurnContext& ctx) const = 0;

	virtual const bool HPActive() = 0;
	virtual const bool GetHPFlag() const = 0;
	virtual void SetHPFlag(bool state) = 0;
	virtual void SetHPActive(bool state) = 0;

	virtual const size_t GetGamblerUses() = 0;
	virtual const bool GActive() = 0;
	virtual void SetGActive(bool state) = 0;

	virtual const bool IsTaxActive() = 0;
	virtual void SetTaxActive(bool state) = 0;

	virtual const bool IsSoothActive() = 0;
	virtual void SetSoothState(bool state) = 0;
	virtual const size_t GetSoothsayerUses() = 0;
};