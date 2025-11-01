#pragma once
#include "Card.h"
#include "PileType.h"
#include <vector>

class Pile
{
private:
	std::vector<Card> m_wholePile;
	PileType m_type;

public:
	const Card& GetTopCard();
	bool CanPlaceCard(const Card& c);
	// void placeCard(const Card& c);
};

