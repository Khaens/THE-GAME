#include "DeckServer.h"

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
	if (!m_initialCards.empty()) {
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

size_t Deck::GetSize() const
{
	return m_initialCards.size();
}

void Deck::ShowDeck() const
{
	std::vector<std::string> cardValues;
	for (const auto& card : m_initialCards) {
		cardValues.push_back(card->GetCardValue());
	}
	std::sort(cardValues.begin(), cardValues.end());
	std::cout << "\n";
	for (const auto& value : cardValues) {
		std::cout << value << " ";
	}
	std::cout << std::endl;
}

