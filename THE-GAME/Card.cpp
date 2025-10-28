#include "Card.h"

Card::Card(const std::string& cardValue) : m_cardValue(cardValue)
{
}

const std::string& Card::getCardValue() const
{
	return m_cardValue;
}


