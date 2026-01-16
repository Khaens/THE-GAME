#pragma once
#include <string>

class Card {
private:
	std::string m_cardValue;
public:
	Card(const std::string& cardValue);
	const std::string& GetCardValue() const;
};