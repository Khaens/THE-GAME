#include "Pile.h"

Pile::Pile(PileType type) : m_type{ type }
{
    std::unique_ptr<Card> initialCard;
    if (type == PileType::ASCENDING) {
        initialCard = std::make_unique<Card>("1");
    }
    else {
        initialCard = std::make_unique<Card>("100");
    }
    m_cards.push(std::move(initialCard));
}

Pile::~Pile()
{
}

const Card* Pile::GetTopCard()
{
    if (m_cards.empty()) {
        return nullptr;
    }
    return m_cards.top().get();
}


PileType Pile::GetPileType() const
{
    return m_type;
}


void Pile::PlaceCard(std::unique_ptr<Card> c)
{
    m_cards.push(std::move(c));
}

size_t Pile::GetSize() const
{
    return m_cards.size();
}
