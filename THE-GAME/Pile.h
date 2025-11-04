#pragma once
#include "Card.h"
#include "PileType.h"
#include <stack>
#include <stdexcept>

class Pile
{
private:
	std::stack<Card*> m_cards;
	PileType m_type;
	

public:
	Pile(PileType type);
	const Card* GetTopCard();
	bool CanPlaceCard(const Card* c);
	PileType GetPileType() const;
	void PlaceCard(const Card* c);
};

