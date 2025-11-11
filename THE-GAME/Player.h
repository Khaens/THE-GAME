#pragma once
#include "User.h"
#include "Card.h"
#include <unordered_set>
class Player :
    public User
{
private:
    std::unordered_set<Card*> m_hand;

public:
    Player(const std::string& username, const std::string& password);
    
    void AddCardToHand(Card* card);
    void RemoveCardFromHand(Card* card);
	const std::unordered_set<Card*>& GetHand() const;
    Card* ChooseCard(std::string cardValue);

    void ShowHand();
};

