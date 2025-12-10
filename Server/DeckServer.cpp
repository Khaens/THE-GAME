#include "DeckServer.h"

Deck::Deck() 
{
	m_initialCards.fill(nullptr);
}

Deck::~Deck()
{
	for (size_t i = 0; i < m_currentDeckSize; i++) {
		delete m_initialCards[i];
	}
}

void Deck::InsertCard(Card* insertedCard)
{
	if (m_currentDeckSize >= DECK_SIZE) {
		return;
	}

	m_initialCards[m_currentDeckSize++] = insertedCard;
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
		m_currentDeckSize--;
		Card* drawnCard = m_initialCards[m_currentDeckSize];
		m_initialCards[m_currentDeckSize] = nullptr;
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
		cardValues.push_back(card->GetCardValue());
	}
	std::sort(cardValues.begin(), cardValues.end());
	std::cout << "\n";
	for (const auto& value : cardValues) {
		std::cout << value << " ";
	}
	std::cout << std::endl;
}

