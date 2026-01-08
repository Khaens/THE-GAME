#include "GameServer.h"
#include "PlayerFactory.h"

Game::Game(std::vector<UserModel>& users, Database& db) : m_numberOfPlayers{ users.size() }, m_database(db)
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

	m_gameStats.reserve(m_numberOfPlayers);
	for (int i = 0; i < m_numberOfPlayers; i++) {
		m_gameStats.insert({ m_players[i]->GetID(), GameStatistics() });
	}
}

Game::~Game()
{
	for (size_t i = 0; i < PILES_AMOUNT; i++) {
		delete m_piles[i];
	}
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
	// Add safety check before finishing turn logic to catch issues early?
	// For now, Round.cpp fix should cover the crash.
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
	if (playerIndex != m_currentPlayerIndex) {
		return Info::NOT_CURRENT_PLAYER_TURN;
	}
	Card* chosenCard = m_players[playerIndex]->GetCardFromHand(card);
	if (!chosenCard) {
		return Info::CARD_NOT_PLAYABLE;
	}

	Pile* chosenPile = Round::GetPile(pile, m_piles);

	if (!chosenPile) {
		return Info::PILE_NOT_FOUND;
	}

	if (Round::CanPlaceCard(*this, chosenCard, chosenPile, m_ctx)) {
		if (m_ctx.HPplayerIndex != -1 && m_players[m_ctx.HPplayerIndex]->HPActive()) {
			if (std::stoi(chosenCard->GetCardValue()) == std::stoi(chosenPile->GetTopCard()->GetCardValue()) + 10 ||
				std::stoi(chosenCard->GetCardValue()) == std::stoi(chosenPile->GetTopCard()->GetCardValue()) - 10) {
				m_players[m_ctx.HPplayerIndex]->SetHPFlag(false);
			}
		}
		int diff = 0;
		try {
			diff = std::abs(std::stoi(chosenCard->GetCardValue()) - std::stoi(chosenPile->GetTopCard()->GetCardValue()));
		} catch(...) {
			std::cout << "Error calculating diff in PlaceCard." << std::endl;
		}
		if (diff > 3) {
			m_gameStats[GetCurrentPlayer().GetID()].perfectGame = false;
		}
		if (chosenCard->GetCardValue() == "6") m_gameStats[GetCurrentPlayer().GetID()].placed6 = true;
		if (chosenCard->GetCardValue() == "7") m_gameStats[GetCurrentPlayer().GetID()].placed7 = true;
		if (m_gameStats[GetCurrentPlayer().GetID()].placed6 && m_gameStats[GetCurrentPlayer().GetID()].placed7)
			m_gameStats[GetCurrentPlayer().GetID()].placed6And7InSameRound = true;
		chosenPile->PlaceCard(chosenCard);
		m_ctx.placedCardsThisTurn++;
		m_players[playerIndex]->RemoveCardFromHand(chosenCard);

		if (IsGameOver(GetCurrentPlayer())) {
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
	if (playerIndex != m_currentPlayerIndex) {
		std::cout << "It's not your turn!\n";
		return Info::NOT_CURRENT_PLAYER_TURN;
	}
	if (m_ctx.placedCardsThisTurn < m_ctx.currentRequired) {
		return Info::NOT_ENOUGH_PLAYED_CARDS;
	}
	int cardsToDraw = m_ctx.placedCardsThisTurn;
	for (int i = 0; i < cardsToDraw; i++) {
		Card* drawnCard = m_wholeDeck.DrawCard();
		if (drawnCard) {
			m_players[playerIndex]->AddCardToHand(drawnCard);
		}
	}
	if (playerIndex == m_ctx.GamblerPlayerIndex &&
		m_players[m_ctx.GamblerPlayerIndex]->GActive()) {
		GetCurrentPlayer().SetGActive(false);
	}
	CheckAchievements(GetCurrentPlayer());
	m_gameStats[m_players[playerIndex]->GetID()].placed7 = false;
	m_gameStats[m_players[playerIndex]->GetID()].placed6 = false;
	UnlockAchievements();
	NextPlayer();
	Round::UpdateContext(*this, m_ctx, GetCurrentPlayer());
	m_ctx.placedCardsThisTurn = 0;
	if (IsGameOver(GetCurrentPlayer())) {
		return Info::GAME_LOST;
	}
	if (Round::IsGameWon(*this, GetCurrentPlayer())) {
		return Info::GAME_WON;
	}
	return Info::TURN_ENDED;
}

using AchievementChecker = std::function<bool(const IPlayer&, const GameStatistics&, const StatisticsModel&)>;

static const std::unordered_map<std::string, AchievementChecker> ACHIEVEMENT_CHECKS = {
	{"harryPotter", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.usedHarryPotter;
	}},
	{"soothsayer", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.usedSoothsayer;
	}},
	{"taxEvader", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.usedTaxEvader;
	}},
	{"gambler", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.usedGambler;
	}},
	{"peasant", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.usedPeasant;
	}},
	{"allOnRed", [](const IPlayer& p, const GameStatistics& s, const StatisticsModel&) {
		return s.usedGambler && s.atLeastTwoCardsInEndgame && p.IsFinished();
	}},
	{"zeroEffort", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.wonGame && s.taxEvaderUses >= 5;
	}},
	{"vanillaW", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.wonGame && !s.usedAnyAbility;
	}},
	{"highRisk", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.usedGambler && s.usedAllGamblerAbilities;
	}},
	{"perfectGame", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.wonGame && s.perfectGame;
	}},
	{"sixSeven", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.placed6And7InSameRound;
	}},
	{"seriousPlayer", [](const IPlayer&, const GameStatistics&, const StatisticsModel& dbStats) {
		return dbStats.GetGamesWon() >= 5;
	}},
	{"talentedPlayer", [](const IPlayer&, const GameStatistics&, const StatisticsModel& dbStats) {
		int gamesWon = dbStats.GetGamesWon();
		float winRate = dbStats.GetWinRate();

		if (winRate <= 0.0f || gamesWon == 0) return false;
		int totalGames = static_cast<int>(gamesWon / winRate);
		return totalGames == 10 && winRate > 0.80f;
	}},
	{"jack", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.playedWithAllAbilities;
	}}
};

void Game::UnlockAchievements()
{
	for (const auto& player : m_players) {
		int userId = player->GetID();
		const GameStatistics& stats = m_gameStats[userId];
		StatisticsModel dbStats = m_database.GetStatisticsByUserId(userId);

		std::unordered_map<std::string, bool> achievementConditions;

		for (const auto& [achName, checker] : ACHIEVEMENT_CHECKS) {
			if (checker(*player, stats, dbStats)) {
				achievementConditions[achName] = true;
			}
		}

		if (!achievementConditions.empty()) {
			m_database.UnlockAchievements(userId, achievementConditions);
			std::cout << "Unlocked " << achievementConditions.size()
				<< " achievement(s) for " << player->GetUsername() << std::endl;
		}
	}
}
void Game::CheckAchievements(IPlayer& currentPlayer)
{
	int userId = currentPlayer.GetID();
	GameStatistics& stats = m_gameStats[userId];
	if (currentPlayer.GetPlayerIndex() == m_ctx.HPplayerIndex) {
		stats.usedHarryPotter = true;
	}
	if (currentPlayer.GetPlayerIndex() == m_ctx.SoothPlayerIndex) {
		stats.usedSoothsayer = true;
	}
	if (currentPlayer.GetPlayerIndex() == m_ctx.TaxEvPlayerIndex) {
		stats.usedTaxEvader = true;
	}
	if (currentPlayer.GetPlayerIndex() == m_ctx.GamblerPlayerIndex) {
		stats.usedGambler = true;
	}
	if (currentPlayer.GetPlayerIndex() == m_ctx.PeasantPlayerIndex) {
		stats.usedPeasant = true;
	}

}

Info Game::UseAbility(size_t playerIndex) // Sothsayer Ability logic to be implemented
{
	if (m_currentPlayerIndex != playerIndex) {
		return Info::NOT_CURRENT_PLAYER_TURN;
	}
	IPlayer& currentPlayer = GetCurrentPlayer();
	m_gameStats[currentPlayer.GetID()].usedAnyAbility = true;
	if (currentPlayer.CanUseAbility(m_ctx)) {
		currentPlayer.UseAbility(m_ctx, playerIndex);
		if (currentPlayer.GetPlayerIndex() == m_ctx.TaxEvPlayerIndex
			&& currentPlayer.IsTaxActive()) {
			return Info::TAX_ABILITY_USED;
		}
		if (m_ctx.PeasantAbilityUse) {
			m_ctx.PeasantAbilityUse = false;
			return Info::PEASANT_ABILITY_USED;
		}
		return Info::ABILITY_USED;
	}
	else {
		if(playerIndex == m_ctx.GamblerPlayerIndex && currentPlayer.GetGamblerUses() == 0) {
			m_gameStats[currentPlayer.GetID()].usedAllGamblerAbilities = true;
		}
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
