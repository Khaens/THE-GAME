#include "AlgorithmCardSelection.h"
#include "GameServer.h"

CardChoice AlgorithmCardSelection::ChooseCardAndPile(Game& game, const std::array<Pile*, 4>& piles, const std::vector<std::unique_ptr<Card>>& m_hand)
{
	CardChoice minChoice;
	CardChoice maxChoice;
	int minDifference = std::numeric_limits<int>::max();
	int maxDifference = std::numeric_limits<int>::min();
	for(int i = 0; i < 4; i++) {
		for(const auto& card : m_hand) {
			TurnContext ctx = game.GetCtx();
			Card* cardInHand = card.get();
			if (!Round::CanPlaceCard(game, cardInHand, piles[i], ctx)) continue;
			int difference = std::abs(std::stoi(piles[i]->GetTopCard()->GetCardValue()) - std::stoi(card->GetCardValue()));
			if(difference > maxDifference) {
				maxDifference = difference;
				maxChoice.cardNum = std::stoi(card->GetCardValue());
				maxChoice.pileNum = i + 1;
			}
			if(difference < minDifference) {
				minDifference = difference;
				minChoice.cardNum = std::stoi(card->GetCardValue());
				minChoice.pileNum = i + 1;
			}
		}
	}
	if (game.GetCurrentPlayer().HPActive()) return maxChoice;
	else return minChoice;
}
