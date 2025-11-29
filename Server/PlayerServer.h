#pragma once
#include "UserModel.h"
#include "IPlayer.h"
#include "CardServer.h"
#include "Gambler.h"
#include "HarryPotter.h"

#include "TurnContext.h"

template <typename Ability>
class Player :
	public UserModel, public IPlayer
{
private:
    std::unordered_set<Card*> m_hand;
	Ability ability;
public:
    Player() = default;
    Player(const UserModel& user);
    ~Player();

    void AddCardToHand(Card* card) override;
    void RemoveCardFromHand(Card* card) override;
    const std::unordered_set<Card*>& GetHand() const override;
    Card* ChooseCard(std::string cardValue) override;
    const std::string& GetUsername() const override;

    void ShowHand() override;
	void UseAbility(TurnContext& ctx, size_t currentPIndex) override;
    bool CanUseAbility(TurnContext& ctx) const override;
};

#include "PlayerServer.h"
#include <iostream>

template <typename Ability>
Player<Ability>::Player(const UserModel& user)
    : UserModel(user)
{
}

template<typename Ability>
Player<Ability>::~Player()
{
    m_hand.clear();
}

template <typename Ability>
void Player<Ability>::AddCardToHand(Card* card)
{
    m_hand.insert(card);
}

template <typename Ability>
void Player<Ability>::RemoveCardFromHand(Card* card)
{
    m_hand.erase(card);
}

template <typename Ability>
const std::unordered_set<Card*>& Player<Ability>::GetHand() const
{
    return m_hand;
}

template <typename Ability>
Card* Player<Ability>::ChooseCard(std::string cardValue)
{
    auto it = std::find_if(m_hand.begin(), m_hand.end(),
        [&cardValue](Card* c) { return c->GetCardValue() == cardValue; });

    if (it != m_hand.end()) {
        return *it;
    }
    return nullptr;
}

template<typename Ability>
const std::string& Player<Ability>::GetUsername() const
{
    return UserModel::GetUsername();
}

template <typename Ability>
void Player<Ability>::ShowHand()
{
    for (auto card : m_hand) {
        std::cout << card->GetCardValue() << " ";
    }
    std::cout << std::endl;
}

template<typename Ability>
void Player<Ability>::UseAbility(TurnContext& ctx, size_t currentPIndex)
{
    ability.UseAbility(ctx, currentPIndex);
}

template<typename Ability>
bool Player<Ability>::CanUseAbility(TurnContext& ctx) const
{
    return ability.CanUseAbility(ctx);
}
