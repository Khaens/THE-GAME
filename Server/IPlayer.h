#pragma once
#include <unordered_set>
#include "CardServer.h"
#include "TurnContext.h"


class IPlayer
{
public:
    virtual ~IPlayer() = default;
    virtual void UseAbility(TurnContext& ctx, size_t currentPIndex) = 0;
	virtual bool CanUseAbility(TurnContext& ctx) const = 0;
	virtual bool IsFinished() const = 0;
	virtual void SetFinished(bool state) = 0;
    
	virtual const bool IsTaxActive() = 0;
	virtual void SetTaxActive(bool state) = 0;

	virtual const bool HPActive() = 0;
	virtual const bool GetHPFlag() const = 0;
	virtual void SetHPFlag(bool state) = 0;
	virtual void SetHPActive(bool state) = 0;

	virtual const size_t GetGamblerUses() = 0;
	virtual const bool GActive() = 0;
	virtual void SetGActive(bool state) = 0;

    virtual void ShowHand() = 0;
    virtual const std::unordered_set<Card*>& GetHand() const = 0;
	virtual void AddCardToHand(Card* card) = 0;
	virtual void RemoveCardFromHand(Card* card) = 0;
	virtual Card* ChooseCard(std::string cardValue) = 0;
	virtual const std::string& GetUsername() const = 0;

	virtual bool IsSoothActive() = 0;
	virtual void SetSoothState(bool state) = 0;

	virtual void SetPlayerIndex(size_t index) = 0;
	virtual size_t GetPlayerIndex() const = 0;
};


