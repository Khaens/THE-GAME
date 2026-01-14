#pragma once
import Card;
#include "PileType.h"
#include <stack>
#include <memory>
#include <stdexcept>

class Pile
{
private:
	std::stack<std::unique_ptr<Card>> m_cards;
	PileType m_type;
	
public:
	Pile(PileType type);
	~Pile();
	const Card* GetTopCard(); 
	PileType GetPileType() const;
	void PlaceCard(std::unique_ptr<Card> c);
	size_t GetSize() const;
};

