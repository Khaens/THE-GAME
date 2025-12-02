#pragma once
#include "TurnContext.h"

class IAbility {
public:
	virtual ~IAbility() = default;

	virtual void UseAbility(TurnContext& ctx, size_t CurrentPIndex) = 0;
	virtual bool CanUseAbility(TurnContext& ctx) const = 0;

	virtual const bool HPActive() = 0;
	virtual const bool GetHPFlag() const = 0;
	virtual void SetHPFlag(bool state) = 0;
	virtual void SetHPActive(bool state) = 0;

	virtual const size_t GetGamblerUses() = 0;
	virtual const bool GActive() = 0;
	virtual void SetGActive(bool state) = 0;
};