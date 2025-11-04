#include "Pile.h"

const Card& Pile::GetTopCard()
{
    //return cartea de sus; //work in progress...
}

bool Pile::CanPlaceCard(const Card& c)
{
    int top = std::stoi(GetTopCard().GetCardValue());
    int value = std::stoi(c.GetCardValue());

    if (m_type == PileType::ASCENDING)
        return (value > top) || (value == top - 10);
    else
        return (value < top) || (value == top + 10);
}
