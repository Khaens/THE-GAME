#include "Deck.h"


void Deck::InsertCard(Card* insertedCard)
{
	m_initialCards.push_back(insertedCard);
}

void Deck::ShuffleDeck()
{
	//srand(time(0)); -> va fi pus in main O SINGURA DATA!
	for (size_t i = m_initialCards.size() - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		std::swap(m_initialCards[i], m_initialCards[j]);
	}
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

