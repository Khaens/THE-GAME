#pragma once
#include <unordered_set>
#include "CardServer.h"

class Game;

class IPlayer
{
public:
    virtual ~IPlayer() = default;
    virtual void UseAbility(Game* game) = 0;
    
    virtual void ShowHand() = 0;
    virtual const std::unordered_set<Card*>& GetHand() const = 0;
	virtual void AddCardToHand(Card* card) = 0;
	virtual void RemoveCardFromHand(Card* card) = 0;
	virtual Card* ChooseCard(std::string cardValue) = 0;
	virtual const std::string& GetUsername() const = 0;
};


