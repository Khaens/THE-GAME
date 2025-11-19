#include "PlayerServer.h"
#include <iostream>
Player::Player(const std::string& username, const std::string& password) : User(username, password)
{
}

void Player::AddCardToHand(Card* card)
{
	m_hand.insert(card);
}

void Player::RemoveCardFromHand(Card* card)
{
	m_hand.erase(card);
}

const std::unordered_set<Card*>& Player::GetHand() const
{
	return m_hand;
}

Card* Player::ChooseCard(std::string cardValue)
{
	return *std::find_if(m_hand.begin(), m_hand.end(),
		[&cardValue](Card* c) { return c->GetCardValue() == cardValue; });
}

void Player::ShowHand()
{
	for (auto card : m_hand) {
		std::cout << card->GetCardValue() << " ";
	}
}

