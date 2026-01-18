#include "Round.h"


void Round::FirstRoundDealing(Game& game)
{
	const auto& m_players = game.GetPlayers();
	Deck& deck = game.GetDeck();
	for (size_t i = 0; i < m_players.size(); i++) {
		int counter = 0;
		for (size_t j = 0; j < STARTING_HAND_SIZE; j++) {
			std::unique_ptr<Card> dealtCard = deck.DrawCard();
			if (dealtCard) {
				m_players[i]->AddCardToHand(std::move(dealtCard));
			}
			counter++;
			if (counter == 2) {
				counter = 0;
				deck.ShuffleDeck();
			}
		}
	}
}

bool Round::CanPlaceCard(Game& game, const Card* card, Pile* pile, TurnContext& m_ctx)
{
	try {
        if (!pile || !pile->GetTopCard() || !card) return false;

		std::string pileStr = pile->GetTopCard()->GetCardValue();
		std::string cardStr = card->GetCardValue();
		
		int top = std::stoi(pileStr);
		int value = std::stoi(cardStr);
		if (m_ctx.HPplayerIndex != -1
			&& game.GetPlayers()[m_ctx.HPplayerIndex]->HPActive()) return true;

		if (pile->GetPileType() == PileType::ASCENDING)
			return (value > top) || (value == top - 10);
		else
			return (value < top) || (value == top + 10);
	}
	catch (...) {
		std::cout << "CRASH AVOIDED in CanPlaceCard! Invalid Data." << std::endl;
		return false;
	}
}

Pile* Round::GetPile(int pileChoice, const std::array<std::unique_ptr<Pile>, PILES_AMOUNT>& piles)
{
	if (pileChoice == 1) return piles[0].get();
	else if (pileChoice == 2) return piles[1].get();
	else if (pileChoice == 3) return piles[2].get();
	else if (pileChoice == 4) return piles[3].get();
	return nullptr;
}

int Round::GetNrOfPlayableCardsInHand(Game& game, TurnContext& m_ctx)
{
	IPlayer& currentPlayer = game.GetCurrentPlayer();
	std::array<Pile*, PILES_AMOUNT> piles = game.GetPiles();
	const auto& m_players = game.GetPlayers();

	int count = 0;
	for (const auto& cardPtr : currentPlayer.GetHand()) {
		Card* card = cardPtr.get();
		if ((CanPlaceCard(game, card, piles[0], m_ctx) || CanPlaceCard(game, card, piles[1], m_ctx) ||
			CanPlaceCard(game, card, piles[2], m_ctx) || CanPlaceCard(game, card, piles[3], m_ctx)) ||
			(m_ctx.HPplayerIndex != -1 && m_players[m_ctx.HPplayerIndex]->HPActive())) {
			count++;
		}
	}
	return count;
}

void Round::UpdateContext(Game& game, TurnContext& m_ctx, IPlayer& currentPlayer)
{
	if (game.GetDeckSize() == 0) m_ctx.endgame = true;
	if (m_ctx.endgame || currentPlayer.GActive()) m_ctx.baseRequired = 1;
	else m_ctx.baseRequired = 2;
	if (m_ctx.endgame && currentPlayer.GetPlayerIndex() == m_ctx.GamblerPlayerIndex) {
		if (m_ctx.GamblerEndgamePenaltyTurns > 0 && currentPlayer.GetHand().size() > 1) {
			m_ctx.currentRequired = 2;
		}
	}
	else if (m_ctx.TaxEvPlayerIndex != currentPlayer.GetPlayerIndex() &&
		game.GetPlayers()[m_ctx.TaxEvPlayerIndex]->IsTaxActive()) {
		m_ctx.currentRequired = m_ctx.baseRequired * 2;
		game.GetPlayers()[m_ctx.TaxEvPlayerIndex]->SetTaxActive(false);
	}
	else m_ctx.currentRequired = m_ctx.baseRequired;
}
bool Round::IsGameWon(Game& game, IPlayer& currentPlayer)
{
	if (game.GetCtx().endgame && currentPlayer.GetHand().size() == 0) currentPlayer.SetFinished(true);
	bool gameWon = true;
	for (size_t i = 0; i < game.GetPlayers().size(); i++) {
		if (!game.GetPlayers()[i]->IsFinished()) {
			gameWon = false;
		}
	}
	if (gameWon) return gameWon;
	size_t checks = 0;
	size_t numPlayers = game.GetPlayers().size();
	while (game.GetCurrentPlayer().IsFinished() && checks < numPlayers) {
		game.NextPlayer();
		checks++;
	}
	UpdateContext(game, game.GetCtx(), game.GetCurrentPlayer());
	return gameWon;
}
