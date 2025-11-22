#include "PlayerServer.h"
#include <iostream>
#include "GameServer.h"
template <typename Ability>
Player<Ability>::Player(const std::string& username, const std::string& password) : User(username, password)
{
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
	return *std::find_if(m_hand.begin(), m_hand.end(),
		[&cardValue](Card* c) { return c->GetCardValue() == cardValue; });

}
template<typename Ability>
const std::string& Player<Ability>::GetUsername() const
{
	return User::GetUsername();
}
template <typename Ability>
void Player<Ability>::ShowHand()
{
	for (auto card : m_hand) {
		std::cout << card->GetCardValue() << " ";
	}
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

template class Player<Gambler>;
template class Player<Harry_Potter>;
