#pragma once
#include <string>

class Card
{
protected:
	std::string m_cardValue;
public:
	Card(const std::string& cardValue);
	const std::string& getCardValue() const;
};

