#include "Pile.h"

Pile::Pile(PileType type) : m_type{ type }
{
    if (type == PileType::ASCENDING) {
        m_initialCard = new Card("1");
    }
    else {
        m_initialCard = new Card("100");
    }
    m_cards.push(m_initialCard);
}

Pile::~Pile()
{
    delete m_initialCard;
}

const Card* Pile::GetTopCard()
{
	return m_cards.top();
}


PileType Pile::GetPileType() const
{
    return m_type;
}

void Pile::PlaceCard(const Card* c)
{
    m_cards.push(const_cast<Card*>(c));
}
