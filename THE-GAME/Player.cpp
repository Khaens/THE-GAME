#include "Player.h"

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

