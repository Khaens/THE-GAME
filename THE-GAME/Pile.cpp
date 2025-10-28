#include "Pile.h"

const Card& Pile::getTopCard()
{
    return m_wholePile.back();
}

bool Pile::canPlaceCard(const Card& c)
{
    int top = std::stoi(getTopCard().getCardValue());
    int value = std::stoi(c.getCardValue());

    if (m_type == PileType::ASCENDING)
        return (value > top) || (value == top - 10);
    else
        return (value < top) || (value == top + 10);
}
