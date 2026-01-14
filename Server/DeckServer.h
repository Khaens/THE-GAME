#pragma once
import Card;
#include <array>
#include <random>
#include <iostream>
#include <memory>

class Deck
{
private:
	constexpr static size_t DECK_SIZE = 196;
	std::array<std::unique_ptr<Card>, DECK_SIZE> m_initialCards;
	size_t m_currentDeckSize = 0;

public:
	Deck();
	~Deck();
    
    // Rule of 5 for Deck (needed because of user-defined Dtor blocking implicit moves)
    Deck(Deck&&) = default;
    Deck& operator=(Deck&&) = default;
    Deck(const Deck&) = delete;
    Deck& operator=(const Deck&) = delete;

	void InsertCard(std::unique_ptr<Card> insertedCard);
	void ShuffleDeck();
	std::unique_ptr<Card> DrawCard();
	bool IsEmpty() const;
	size_t GetSize() const;

	void ShowDeck() const;
};

