#pragma once
import Card;
#include "PileType.h"
#include <stack>
#include <stdexcept>

class Pile
{
private:
	std::stack<Card*> m_cards;
	PileType m_type;
	Card* m_initialCard;

public:
	Pile(PileType type);
	~Pile();
	const Card* GetTopCard();
	PileType GetPileType() const;
	void PlaceCard(const Card* c);
	size_t GetSize() const;
};

