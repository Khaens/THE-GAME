module;

#include <string>

export module Card;

export class Card
{
private:
	std::string m_cardValue;
public:
	Card(const std::string& cardValue);
	const std::string& GetCardValue() const;
};

