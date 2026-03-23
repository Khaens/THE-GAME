#include "AlgorithmCardSelection.h"
#include "GameServer.h"

CardChoice AlgorithmCardSelection::ChooseCardAndPile(Game& game, const std::array<Pile*, 4>& piles, const std::vector<std::unique_ptr<Card>>& m_hand)
{
	CardChoice choice;
	int minDifference = std::numeric_limits<int>::max();
	for(int i = 0; i < 4; i++) {
		for(const auto& card : m_hand) {
			TurnContext ctx = game.GetCtx();
			Card* cardInHand = card.get();
			if (!Round::CanPlaceCard(game, cardInHand, piles[i], ctx)) continue;
			int difference = std::abs(std::stoi(piles[i]->GetTopCard()->GetCardValue()) - std::stoi(card->GetCardValue()));
			if(difference < minDifference) {
				minDifference = difference;
				choice.cardNum = std::stoi(card->GetCardValue());
				choice.pileNum = i;
			}
		}
	}
	return choice;
}
