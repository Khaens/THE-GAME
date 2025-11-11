#pragma once
#include "Card.h"
#include <vector>
#include <random>
#include <iostream>

class Deck 
{
private:
	std::vector<Card*> m_initialCards;
	
public:
	Deck() = default;
	~Deck();
	void InsertCard(Card* insertedCard);
	void ShuffleDeck();
	Card* DrawCard();
	bool IsEmpty() const;
	size_t GetSize() const;

	void ShowDeck() const;
};

