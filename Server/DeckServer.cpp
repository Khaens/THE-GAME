#include "DeckServer.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>

Deck::Deck() 
{
}

Deck::~Deck()
{
}

void Deck::InsertCard(std::unique_ptr<Card> insertedCard)
{
	if (m_currentDeckSize >= DECK_SIZE) {
		return;
	}

	m_initialCards[m_currentDeckSize++] = std::move(insertedCard);
}

void Deck::ShuffleDeck()
{
	for (int i = 0; i < 5; i++) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::shuffle(m_initialCards.begin(), m_initialCards.begin() + m_currentDeckSize, gen);
	}
}

std::unique_ptr<Card> Deck::DrawCard()
{
	if (!m_initialCards.empty() && m_currentDeckSize > 0) {
		m_currentDeckSize--;
		std::unique_ptr<Card> drawnCard = std::move(m_initialCards[m_currentDeckSize]);
		return drawnCard;
	}
	else {
		return nullptr;
	}
}

bool Deck::IsEmpty() const
{
	return m_currentDeckSize == 0;
}

size_t Deck::GetSize() const
{
	return m_currentDeckSize;
}

void Deck::ShowDeck() const
{
	std::vector<std::string> cardValues;
	for (const auto& card : m_initialCards) {
		if (card) {
			cardValues.push_back(card->GetCardValue());
		}
	}
	std::sort(cardValues.begin(), cardValues.end());
	std::cout << "\n";
	for (const auto& value : cardValues) {
		std::cout << value << " ";
	}
	std::cout << std::endl;
}

