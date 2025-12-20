#include "GameServer.h"
#include "PlayerFactory.h"

Game::Game(std::vector<UserModel>& users) : m_numberOfPlayers{ users.size() }
{
	m_players.reserve(m_numberOfPlayers);
	if (users.size() < 2 || users.size() > 5) {
		throw std::invalid_argument("Number of players must be between 2 and 5.");
	}

	std::vector<AbilityType> abilities = PlayerFactory::GetRandomUniqueAbilities(users.size());
	size_t i = 0;
	for (auto& user : users) {
		std::unique_ptr<IPlayer> player = PlayerFactory::CreateFromUser(user, abilities[i]);
		player->SetPlayerIndex(i);
		m_players.push_back(std::move(player));
		i++;
	}

	for (int i = 2; i < 100; i++) {
		Card* newCard = new Card(std::to_string(i));
		Card* newCard2 = new Card(std::to_string(i));
		m_wholeDeck.InsertCard(newCard);
		m_wholeDeck.InsertCard(newCard2);
	}
	m_piles[m_pileIndex++] = new Pile{ PileType::ASCENDING };
	m_piles[m_pileIndex++] = new Pile{ PileType::ASCENDING };
	m_piles[m_pileIndex++] = new Pile{ PileType::DESCENDING };
	m_piles[m_pileIndex++] = new Pile{ PileType::DESCENDING };
}

size_t Game::WhoStartsFirst()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<size_t> dist(1, m_numberOfPlayers);

	size_t randomChoice = dist(gen);
	return randomChoice - 1;
}

bool Game::IsGameOver(IPlayer& currentPlayer)
{
	int playableCards = Round::NrOfPlayableCardsInHand(*this, m_ctx);

	if (currentPlayer.IsFinished()) return false;

	if (m_currentPlayerIndex == m_ctx.HPplayerIndex && currentPlayer.GetHPFlag()) {
		std::cout << "No one played a +/-10 card until " << currentPlayer.GetUsername() << "'s turn. All players lose!\n";
		return true;
	}
	if (playableCards + m_ctx.placedCardsThisTurn < m_ctx.currentRequired) {
		std::cout << currentPlayer.GetUsername() << " cannot play the required number of cards (" << m_ctx.currentRequired << ").\n";
		std::cout << "Game Over! " << currentPlayer.GetUsername() << " loses!\n";
		return true;
	}

	return false;
}



void Game::StartGame()
{
	m_wholeDeck.ShuffleDeck();
	Round::FirstRoundDealing(*this);
	m_currentPlayerIndex = WhoStartsFirst();
}


void Game::NextPlayer()
{
	m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_numberOfPlayers;
}

Info Game::PlaceCard(size_t playerIndex, int card, int pile)
{
	if(playerIndex != m_currentPlayerIndex) {
		return Info::NOT_CURRENT_PLAYER_TURN;
	}
	Card* chosenCard = m_players[playerIndex]->GetCardFromHand(card);
	if(!chosenCard) {
		return Info::CARD_NOT_PLAYABLE;
	}

	Pile* chosenPile = Round::GetPile(pile, m_piles);

	if (!chosenPile) {
		return Info::PILE_NOT_FOUND;
	}

	if(Round::CanPlaceCard(*this, chosenCard, chosenPile, m_ctx)) {
		if (m_ctx.HPplayerIndex != -1 && m_players[m_ctx.HPplayerIndex]->HPActive()) {
			if (std::stoi(chosenCard->GetCardValue()) == std::stoi(chosenPile->GetTopCard()->GetCardValue()) + 10 ||
				std::stoi(chosenCard->GetCardValue()) == std::stoi(chosenPile->GetTopCard()->GetCardValue()) - 10) {
				m_players[m_ctx.HPplayerIndex]->SetHPFlag(false);
			}
		}
		chosenPile->PlaceCard(chosenCard);
		m_ctx.placedCardsThisTurn++;
		m_players[playerIndex]->RemoveCardFromHand(chosenCard);
		if(IsGameOver(GetCurrentPlayer())) {
			return Info::GAME_LOST;
		}
		return Info::CARD_PLACED;
	}
	else {
		return Info::CARD_NOT_PLAYABLE;
	}
}

Info Game::EndTurn(size_t playerIndex)
{
	if(m_ctx.placedCardsThisTurn < m_ctx.currentRequired) {
		return Info::NOT_ENOUGH_PLAYED_CARDS;
	}
	if (playerIndex != m_currentPlayerIndex) {
		std::cout << "It's not your turn!\n";
		return Info::NOT_CURRENT_PLAYER_TURN;
	}
	int cardsToDraw = m_ctx.placedCardsThisTurn;
	for(int i = 0; i < cardsToDraw; i++) {
		Card* drawnCard = m_wholeDeck.DrawCard();
		if(drawnCard) {
			m_players[playerIndex]->AddCardToHand(drawnCard);
		}
	}
	if (playerIndex == m_ctx.GamblerPlayerIndex &&
		m_players[m_ctx.GamblerPlayerIndex]->GActive()) {
		GetCurrentPlayer().SetGActive(false);
	}
	NextPlayer();
	Round::UpdateContext(*this, m_ctx, GetCurrentPlayer());
	m_ctx.placedCardsThisTurn = 0;
	if(IsGameOver(GetCurrentPlayer())) {
		return Info::GAME_LOST;
	}
	if (Round::IsGameWon(*this, GetCurrentPlayer())) {
		return Info::GAME_WON;
	}
	return Info::TURN_ENDED;
}

Info Game::UseAbility(size_t playerIndex) // Sothsayer Ability logic to be implemented
{
	if(m_currentPlayerIndex != playerIndex) {
		return Info::NOT_CURRENT_PLAYER_TURN;
	}
	IPlayer& currentPlayer = GetCurrentPlayer();
	if(currentPlayer.CanUseAbility(m_ctx)) {
		currentPlayer.UseAbility(m_ctx, playerIndex);
		if (currentPlayer.GetPlayerIndex() == m_ctx.TaxEvPlayerIndex
				&& currentPlayer.IsTaxActive()) {
			return Info::TAX_ABILITY_USED;
		}
		if(m_ctx.PeasantAbilityUse) {
			m_ctx.PeasantAbilityUse = false;
			return Info::PEASANT_ABILITY_USED;
		}
		return Info::ABILITY_USED;
	}
	else {
		return Info::ABILITY_NOT_AVAILABLE;
	}
}

IPlayer& Game::GetCurrentPlayer()
{
	return *m_players[m_currentPlayerIndex];
}

Card* Game::DrawCard()
{
	return m_wholeDeck.DrawCard();
}

size_t Game::GetDeckSize() const
{
	return m_wholeDeck.GetSize();
}

const std::vector<std::unique_ptr<IPlayer>>& Game::GetPlayers()
{
	return m_players;
}

std::array<Pile*, PILES_AMOUNT> Game::GetPiles()
{
	return m_piles;
}

Deck& Game::GetDeck()
{
	return m_wholeDeck;
}

void Game::ShowCtx()
{
	std::cout << "baseReq " << m_ctx.baseRequired << "\n";
	std::cout << "currReq " << m_ctx.currentRequired << "\n";
	std::cout << "HPPIndex " << m_ctx.HPplayerIndex << "\n";
	std::cout << "GambPInd " << m_ctx.GamblerPlayerIndex << "\n";
	std::cout << "TaxEvPInd" << m_ctx.TaxEvPlayerIndex << "\n";
}
