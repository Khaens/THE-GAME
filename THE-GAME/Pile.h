#pragma once
#include "Card.h"
#include "PileType.h"
#include <vector>

class Pile
{
private:
	//std::vector<Card> m_wholePile; ??? nu cred ca ar trebui aici, ci lasat in Deck
	PileType m_type;
	//probabil ar trebui sa avem 4 membri pt fiecare gramada de joc

public:
	const Card& GetTopCard();
	bool CanPlaceCard(const Card& c);
	// void placeCard(const Card& c);
};

