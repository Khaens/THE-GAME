#pragma once
#include "Card.h"
#include <vector>
#include <ctime>
#include <cstdlib>

class Deck 
{
private:
	std::vector<Card> m_initialCards;
	
public:
	Deck() = default;
	void InsertCard(Card* insertedCard);
	void ShuffleCards();
};

