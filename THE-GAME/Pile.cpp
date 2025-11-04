#include "Pile.h"

Pile::Pile(PileType type) : m_type{ type }
{}

const Card* Pile::GetTopCard()
{
	return m_cards.top();
}

bool Pile::CanPlaceCard(const Card* c)
{
    int top = std::stoi(GetTopCard()->GetCardValue());
    int value = std::stoi(c->GetCardValue());

    if (m_type == PileType::ASCENDING)
        return (value > top) || (value == top - 10);
    else
        return (value < top) || (value == top + 10);
}

PileType Pile::GetPileType() const
{
    return PileType();
}

void Pile::PlaceCard(const Card* c)
{
    if(CanPlaceCard(c))
		m_cards.push(const_cast<Card*>(c));
    else {
        throw std::invalid_argument("Cannot place card on this pile.");
    }
}
