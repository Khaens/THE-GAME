#include "Deck.h"

Deck::~Deck()
{
	for (Card* card : m_initialCards)
		delete card;
}

void Deck::InsertCard(Card* insertedCard)
{
	m_initialCards.push_back(insertedCard);
}

void Deck::ShuffleDeck()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(m_initialCards.begin(), m_initialCards.end(), gen);
}

Card* Deck::DrawCard()
{
	if(!m_initialCards.empty()) {
		Card* drawnCard = m_initialCards.back();
		m_initialCards.pop_back();
		return drawnCard;
	}
	else {
		return nullptr;
	}
}

bool Deck::IsEmpty() const
{
	return m_initialCards.empty();
}

