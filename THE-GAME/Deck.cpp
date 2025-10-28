#include "Deck.h"


void Deck::InsertCard(Card* insertedCard)
{
	m_initialCards.push_back(*insertedCard);
}

void Deck::ShuffleCards()
{
	//srand(time(0)); -> va fi pus in main O SINGURA DATA!
	for (int i = m_initialCards.size() - 1; i > 0; i--) {
		int j = rand() % (i + 1);
		std::swap(m_initialCards[i], m_initialCards[j]);
	}
}
