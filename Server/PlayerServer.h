#pragma once
#include "UserServer.h"
#include "IPlayer.h"
#include "CardServer.h"
#include "Gambler.h"
#include "Harry_Potter.h"

class Game;

template <typename Ability>
class Player :
	public User, public IPlayer
{
private:
    std::unordered_set<Card*> m_hand;
	Ability ability;
public:
    Player(const std::string& username, const std::string& password);

    void AddCardToHand(Card* card) override;
    void RemoveCardFromHand(Card* card) override;
    const std::unordered_set<Card*>& GetHand() const override;
    Card* ChooseCard(std::string cardValue) override;
    const std::string& GetUsername() const override;

    void ShowHand() override;
	void UseAbility(Game* game) override;
};



