#pragma once
#include <iostream>
#include "UserModel.h"
#include "IPlayer.h"
#include "CardServer.h"
#include "Gambler.h"
#include "HarryPotter.h"
#include "TaxEvader.h"
#include "Soothsayer.h"
#include "Peasant.h"
#include "TurnContext.h"

template <typename Ability>
class Player :
	public UserModel, public IPlayer
{
private:
    std::unordered_set<Card*> m_hand;
    Ability ability;
    size_t m_playerIndex = 10;
public:
    Player() = default;
    Player(const UserModel& user);
    ~Player();

    void AddCardToHand(Card* card) override;
    void RemoveCardFromHand(Card* card) override;
    const std::unordered_set<Card*>& GetHand() const override;
    Card* ChooseCard(std::string cardValue) override;
    const std::string& GetUsername() const override;
    void SetPlayerIndex(size_t index) override;
	size_t GetPlayerIndex() const override;

    void ShowHand() override;
	void UseAbility(TurnContext& ctx, size_t currentPIndex) override;
    bool CanUseAbility(TurnContext& ctx) const override;

    const bool IsTaxActive() override;
    virtual void SetTaxActive(bool state) override;

    const bool HPActive() override;
    const bool GetHPFlag() const override;
    void SetHPFlag(bool state) override;
    void SetHPActive(bool state) override;

    const size_t GetGamblerUses() override;
    const bool GActive() override;
    void SetGActive(bool state) override;

    void SetSoothState(bool state) override;
	virtual bool IsSoothActive() override;
};


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

template<typename Ability>
inline void Player<Ability>::SetPlayerIndex(size_t index)
{
	m_playerIndex = index;
}

template<typename Ability>
inline size_t Player<Ability>::GetPlayerIndex() const
{
    return m_playerIndex;
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


template<typename Ability>
inline const bool Player<Ability>::IsTaxActive()
{
    return ability.IsTaxActive();
}

template<typename Ability>
inline void Player<Ability>::SetTaxActive(bool state)
{
    ability.SetTaxActive(state);
}

template<typename Ability>
inline const bool Player<Ability>::HPActive()
{
    return ability.HPActive();
}

template<typename Ability>
inline const bool Player<Ability>::GetHPFlag() const
{
    return ability.GetHPFlag();
}

template<typename Ability>
inline void Player<Ability>::SetHPFlag(bool state)
{
    ability.SetHPFlag(state);
}

template<typename Ability>
inline void Player<Ability>::SetHPActive(bool state)
{
    ability.SetHPActive(state);
}

template<typename Ability>
inline const size_t Player<Ability>::GetGamblerUses()
{
    return ability.GetGamblerUses();
}

template<typename Ability>
inline const bool Player<Ability>::GActive()
{
    return ability.GActive();
}

template<typename Ability>
inline void Player<Ability>::SetGActive(bool state)
{
    ability.SetGActive(state);
}

template<typename Ability>
inline void Player<Ability>::SetSoothState(bool state)
{
	ability.SetSoothState(state);
}

template<typename Ability>
inline bool Player<Ability>::IsSoothActive()
{
	return ability.IsSoothActive();
}
