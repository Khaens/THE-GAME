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
	// m_initialCard is now inside the stack as the first element, handled by unique_ptr logic or just managed via stack
	// But destructor deleted m_initialCard explicitly. If it's in stack, stack auto-deletes.
	// Wait, constructor does `m_cards.push(m_initialCard)`. 
	// So we don't need separate m_initialCard member if we don't access it separately.
	// Initial code: m_initialCard = new...; m_cards.push(m_initialCard); 
	// So m_initialCard was just a helper.
	
public:
	Pile(PileType type);
	~Pile();
	const Card* GetTopCard(); // Returns raw pointer (observer)
	PileType GetPileType() const;
	void PlaceCard(std::unique_ptr<Card> c);
	size_t GetSize() const;
};

