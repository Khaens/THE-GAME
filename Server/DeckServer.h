#pragma once
import Card;
#include <array>
#include <random>
#include <iostream>

class Deck
{
private:
	constexpr static size_t DECK_SIZE = 196;
	std::array<Card*, DECK_SIZE> m_initialCards;
	size_t m_currentDeckSize = 0;

public:
	Deck();
	~Deck();
	void InsertCard(Card* insertedCard);
	void ShuffleDeck();
	Card* DrawCard();
	bool IsEmpty() const;
	size_t GetSize() const;

	void ShowDeck() const;
};

