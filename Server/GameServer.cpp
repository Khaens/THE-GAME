#include "GameServer.h"
#include "PileType.h"
#include "AbilityType.h"

import PlayerFactory;

Game::Game(std::vector<UserModel>& users, Database& db) : 
    m_numberOfPlayers{ users.size() }, 
    m_currentPlayerIndex(0), 
    m_pileIndex(0), 
    m_database(db)
{
	m_players.reserve(m_numberOfPlayers);
	if (users.size() < MIN_PLAYERS || users.size() > MAX_PLAYERS) {
		throw std::invalid_argument("Number of players must be between 2 and 5.");
	}

	std::vector<AbilityType> abilities = PlayerFactory::GetRandomUniqueAbilities(users.size());
	size_t i = 0;
	for (auto& user : users) {
		std::unique_ptr<IPlayer> player = PlayerFactory::CreateFromUser(user, abilities[i]);
		player->SetPlayerIndex(i);
		
		if (abilities[i] == AbilityType::HarryPotter) m_ctx.HPplayerIndex = i;
		else if (abilities[i] == AbilityType::Gambler) m_ctx.GamblerPlayerIndex = i;
		else if (abilities[i] == AbilityType::TaxEvader) m_ctx.TaxEvPlayerIndex = i;
		else if (abilities[i] == AbilityType::Soothsayer) m_ctx.SoothPlayerIndex = i;
		else if (abilities[i] == AbilityType::Peasant) m_ctx.PeasantPlayerIndex = i;

		m_players.push_back(std::move(player));
		i++;
	}

	for (int i = 2; i < CARD_SET; i++) {
		m_wholeDeck.InsertCard(std::make_unique<Card>(std::to_string(i)));
		m_wholeDeck.InsertCard(std::make_unique<Card>(std::to_string(i)));
	}
	m_piles[m_pileIndex++] = std::make_unique<Pile>(PileType::ASCENDING);
	m_piles[m_pileIndex++] = std::make_unique<Pile>(PileType::ASCENDING);
	m_piles[m_pileIndex++] = std::make_unique<Pile>(PileType::DESCENDING);
	m_piles[m_pileIndex++] = std::make_unique<Pile>(PileType::DESCENDING);

	m_gameStats.reserve(m_numberOfPlayers);
	for (int i = 0; i < m_numberOfPlayers; i++) {
		m_gameStats.insert({ m_players[i]->GetID(), GameStatistics() });
	}
}

Game::~Game()
{
}

Game::Game(Game&& other) noexcept
    : m_numberOfPlayers(other.m_numberOfPlayers),
    m_currentPlayerIndex(other.m_currentPlayerIndex),
    m_pileIndex(other.m_pileIndex),
    m_players(std::move(other.m_players)),
    m_piles(std::move(other.m_piles)),
    m_wholeDeck(std::move(other.m_wholeDeck)), 
    m_ctx(std::move(other.m_ctx)),
    m_database(other.m_database),
    m_gameStats(std::move(other.m_gameStats))
{
}

Game& Game::operator=(Game&& other) noexcept
{
    if (this != &other) {
        m_numberOfPlayers = other.m_numberOfPlayers;
        m_currentPlayerIndex = other.m_currentPlayerIndex;
        m_pileIndex = other.m_pileIndex;
        m_players = std::move(other.m_players);
        m_piles = std::move(other.m_piles);
        m_wholeDeck = std::move(other.m_wholeDeck);
        m_ctx = std::move(other.m_ctx);
        m_gameStats = std::move(other.m_gameStats);
    }
    return *this;
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
	int playableCards = Round::GetNrOfPlayableCardsInHand(*this, m_ctx);

	if (currentPlayer.IsFinished()) return false;

    const std::string& username = currentPlayer.GetUsername();

	if (m_currentPlayerIndex == static_cast<size_t>(m_ctx.HPplayerIndex) && !currentPlayer.GetSameTurn()) {
		if (currentPlayer.GetHPFlag()) {
			std::cout << "No one played a +/-10 card until " << username << "'s turn. All players lose!\n";
			return true;
		}
	}
	if (m_ctx.endgame && currentPlayer.GetHand().size() == 1 && m_ctx.currentRequired == 2 && playableCards == 1) {
	}
	else if (playableCards + m_ctx.placedCardsThisTurn < m_ctx.currentRequired) {
		std::cout << username << " cannot play the required number of cards (" << m_ctx.currentRequired << ").\n";
		std::cout << "Game Over! " << username << " loses!\n";
		return true;
	}
	return false;
}


void Game::StartGame()
{
    std::lock_guard<std::mutex> lock(m_stateMutex);
	m_wholeDeck.ShuffleDeck();
	Round::FirstRoundDealing(*this);
	m_currentPlayerIndex = WhoStartsFirst();
}


void Game::NextPlayer()
{
	m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_numberOfPlayers;
}

Info Game::PlaceCard(size_t playerIndex, const Card& card, int pile)
{
    std::lock_guard<std::mutex> lock(m_stateMutex);
	if (playerIndex != m_currentPlayerIndex) {
		return Info::NOT_CURRENT_PLAYER_TURN;
	}
	Card* chosenCard = m_players[playerIndex]->GetCardFromHand(card.GetCardValue());
	if (!chosenCard) {
		return Info::CARD_NOT_PLAYABLE;
	}

	std::string chosenCardValue = chosenCard->GetCardValue();
	
	Pile* chosenPile = Round::GetPile(pile, m_piles);

	if (!chosenPile) {
		return Info::PILE_NOT_FOUND;
	}

	if (Round::CanPlaceCard(*this, chosenCard, chosenPile, m_ctx)) {
        const Card* pileTopCard = chosenPile->GetTopCard();
        if (!pileTopCard) {
             return Info::PILE_NOT_FOUND;
        }
        
        std::string pileTopCardValue = pileTopCard->GetCardValue();

		if (m_ctx.HPplayerIndex != -1 && m_players[static_cast<size_t>(m_ctx.HPplayerIndex)]->GetHPFlag()) {
            try {
			    if (std::stoi(chosenCardValue) == std::stoi(pileTopCardValue) + 10 &&
					chosenPile->GetPileType() == PileType::DESCENDING) {
				    m_players[static_cast<size_t>(m_ctx.HPplayerIndex)]->SetHPFlag(false);
			    }
				if (std::stoi(chosenCardValue) == std::stoi(pileTopCardValue) - 10 &&
					chosenPile->GetPileType() == PileType::ASCENDING) {
					m_players[static_cast<size_t>(m_ctx.HPplayerIndex)]->SetHPFlag(false);
				}	
			}
			catch (...) {}
		}
		int diff = 0;
		try {
			diff = std::abs(std::stoi(chosenCardValue) - std::stoi(pileTopCardValue));
		} catch(...) {
			std::cout << "Error calculating diff in PlaceCard." << std::endl;
		}
		if (diff > 3) {
			m_gameStats[GetCurrentPlayer().GetID()].perfectGame = false;
		}
		if (chosenCardValue == "6") m_gameStats[GetCurrentPlayer().GetID()].placed6 = true;
		if (chosenCardValue == "7") m_gameStats[GetCurrentPlayer().GetID()].placed7 = true;
		if (m_gameStats[GetCurrentPlayer().GetID()].placed6 && m_gameStats[GetCurrentPlayer().GetID()].placed7)
			m_gameStats[GetCurrentPlayer().GetID()].placed6And7InSameRound = true;
		chosenPile->PlaceCard(m_players[playerIndex]->RemoveCardFromHand(chosenCard)); 
		m_ctx.placedCardsThisTurn++;

		if (m_ctx.HPplayerIndex == playerIndex && m_players[playerIndex]->HPActive())
			m_players[playerIndex]->SetHPActive(false);

		if (Round::IsGameWon(*this, GetCurrentPlayer())) {
			UpdateGameStats(true);
			return Info::GAME_WON;
		}

		if (IsGameOver(GetCurrentPlayer())) {
			UpdateGameStats(false);
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
    std::lock_guard<std::mutex> lock(m_stateMutex);
	if (playerIndex != m_currentPlayerIndex) {
		std::cout << "It's not your turn!\n";
		return Info::NOT_CURRENT_PLAYER_TURN;
	}

	bool canEndWithFewerCards = m_ctx.endgame && 
	                             m_players[playerIndex]->GetHand().size() == 0 &&
	                             m_ctx.placedCardsThisTurn < m_ctx.currentRequired &&
	                             m_ctx.currentRequired == 2;
	
	if (m_ctx.placedCardsThisTurn < m_ctx.currentRequired && !canEndWithFewerCards) {
		std::cout << "Cannot end turn: Played " << m_ctx.placedCardsThisTurn << " cards, required " << m_ctx.currentRequired << "\n";
		return Info::NOT_ENOUGH_PLAYED_CARDS;
	}
	
	int cardsToDraw = m_ctx.placedCardsThisTurn;
	for (int i = 0; i < cardsToDraw; i++) {
		std::unique_ptr<Card> drawnCard = m_wholeDeck.DrawCard();
		if (drawnCard) {
			m_players[playerIndex]->AddCardToHand(std::move(drawnCard));
		}
	}
	if (playerIndex == m_ctx.GamblerPlayerIndex &&
		m_players[m_ctx.GamblerPlayerIndex]->GActive()) {
		GetCurrentPlayer().SetGActive(false);
	}
	if (m_ctx.endgame && playerIndex == m_ctx.GamblerPlayerIndex && m_ctx.GamblerEndgamePenaltyTurns > 0) {
		m_ctx.GamblerEndgamePenaltyTurns--;
	}
	if (m_players[playerIndex]->IsSoothActive()) {
		m_players[playerIndex]->SetSoothState(false);
	}
	if (m_players[playerIndex]->GetHPFlag()) {
		m_players[playerIndex]->SetSameTurn(false);
		m_players[playerIndex]->SetHPActive(false);
	}
	CheckAchievements(GetCurrentPlayer());
	m_gameStats[m_players[playerIndex]->GetID()].placed7 = false;
	m_gameStats[m_players[playerIndex]->GetID()].placed6 = false;
	NextPlayer();
	Round::UpdateContext(*this, m_ctx, GetCurrentPlayer());
	m_ctx.placedCardsThisTurn = 0;
	if (IsGameOver(GetCurrentPlayer())) {
		UpdateGameStats(false);
		return Info::GAME_LOST;
	}
	if (Round::IsGameWon(*this, GetCurrentPlayer())) {
		UpdateGameStats(true);
		return Info::GAME_WON;
	}

	m_wholeDeck.ShuffleDeck();
	
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
	{"highRisk", [](const IPlayer& p, const GameStatistics& s, const StatisticsModel&) {
		IPlayer& playerRef = const_cast<IPlayer&>(p); 
		return s.usedGambler && playerRef.GetGamblerUses() == 0;
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

		if (winRate <= 0.0f || gamesWon <= 0) return false;
		int totalGames = dbStats.GetGamesPlayed();
		return totalGames == 10 && winRate > 80.0f;
	}},
	{"jack", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.playedWithAllAbilities;
	}}
};

std::vector<std::pair<int, std::string>> Game::UnlockAchievements()
{
    std::vector<std::pair<int, std::string>> allNewlyUnlocked;
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
            std::vector<std::string> newAchs = m_database.UnlockAchievements(userId, achievementConditions);
            for (const auto& achKey : newAchs) {
                allNewlyUnlocked.push_back({ userId, achKey });
            }
            if (!newAchs.empty()) {
                std::cout << "Unlocked " << newAchs.size()
                    << " new achievement(s) for " << player->GetUsername() << std::endl;
            }
        }
    }
    return allNewlyUnlocked;
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
	if (m_ctx.endgame && currentPlayer.GetPlayerIndex() == m_ctx.GamblerPlayerIndex) {
		if (m_ctx.placedCardsThisTurn < 2) {
			stats.atLeastTwoCardsInEndgame = false;
		}
	}
}

void Game::UpdateGameStats(bool won)
{
	for (const auto& player : m_players) {
		int userId = player->GetID();
        const std::string& username = player->GetUsername();
		try {
			StatisticsModel stats = m_database.GetStatisticsByUserId(userId);
			
			stats.SetGamesPlayed(stats.GetGamesPlayed() + 1);
			if (won) stats.SetGamesWon(stats.GetGamesWon() + 1);
			if (stats.GetGamesPlayed() > 0) {
				float rawWinRate = ((float)stats.GetGamesWon() / stats.GetGamesPlayed()) * 100.0f;
				stats.SetWinRate(std::round(rawWinRate * 100.0f) / 100.0f);
			}

			m_database.UpdateStatistics(stats);
			std::cout << "Updated statistics for user " << username 
				<< ": Games Played=" << stats.GetGamesPlayed() 
				<< ", Games Won=" << stats.GetGamesWon() 
				<< ", Win Rate=" << stats.GetWinRate() << std::endl;

		}
		catch (const std::exception& e) {
			std::cerr << "Error updating statistics for user " << userId << ": " << e.what() << std::endl;
		}
	}
}

Info Game::UseAbility(size_t playerIndex)
{
    std::lock_guard<std::mutex> lock(m_stateMutex);
	if (m_currentPlayerIndex != playerIndex) {
		return Info::NOT_CURRENT_PLAYER_TURN;
	}
	IPlayer& currentPlayer = GetCurrentPlayer();
	if (currentPlayer.CanUseAbility(m_ctx)) {
		m_gameStats[currentPlayer.GetID()].usedAnyAbility = true;
		currentPlayer.UseAbility(m_ctx, playerIndex);
		if (currentPlayer.GetPlayerIndex() == m_ctx.TaxEvPlayerIndex
			&& currentPlayer.IsTaxActive()) {
			m_gameStats[currentPlayer.GetID()].taxEvaderUses++;
			return Info::TAX_ABILITY_USED;
		}
		if (m_ctx.PeasantAbilityUse) {
			m_ctx.PeasantAbilityUse = false;
			return Info::PEASANT_ABILITY_USED;
		}
		if (currentPlayer.GetPlayerIndex() == m_ctx.SoothPlayerIndex
			&& currentPlayer.IsSoothActive()) {
			return Info::SOOTHSAYER_ABILITY_USED;
		}
		if (currentPlayer.GetPlayerIndex() == m_ctx.HPplayerIndex
			&& currentPlayer.HPActive()) {
			return Info::HARRY_POTTER_ABILITY_USED;
		}
		if(currentPlayer.GetPlayerIndex() == m_ctx.GamblerPlayerIndex) {
			m_gameStats[currentPlayer.GetID()].gamblerAbilityUses++;
			return Info::GAMBLER_ABILITY_USED;
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
    if (m_currentPlayerIndex >= m_players.size()) {
        throw std::out_of_range("Current player index out of range");
    }
	return *m_players[m_currentPlayerIndex];
}

TurnContext& Game::GetCtx()
{
	return m_ctx;
}

std::unique_ptr<Card> Game::DrawCard()
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
	std::array<Pile*, PILES_AMOUNT> rawPiles;
    for (size_t i = 0; i < PILES_AMOUNT; ++i) {
        rawPiles[i] = m_piles[i].get();
    }
    return rawPiles;
}

Deck& Game::GetDeck()
{
	return m_wholeDeck;
}
