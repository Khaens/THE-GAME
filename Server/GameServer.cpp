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
	for(int i = 0; i < m_numberOfPlayers; i++) {
		m_gameStats.insert({m_players[i]->GetID(), GameStatistics()});
	}
}

size_t Game::WhoStartsFirst()
{
	std::cout << "\nTake a look at your cards... Who wants to start the game? You have 1 minute to decide.\n";
	std::cout << "Pick a number 1 - " << m_numberOfPlayers << ": ";

	std::packaged_task<size_t()> task([]() {
		size_t choice;
		std::cin >> choice;
		return choice;
		});

	auto future = task.get_future();
	std::thread(std::move(task)).detach();

	if (future.wait_for(std::chrono::seconds(60)) == std::future_status::ready) {
		size_t choice = future.get();
		if (choice >= 1 && choice <= m_numberOfPlayers) {
			std::cout << "Player " << choice << " will start The Game!\n";
			return choice - 1;
		}
		else {
			std::cout << "Invalid number! We will now pick randomly...\n";
		}
	}
	else {
		std::cout << "Uh oh... Time's up! Picking randomly...\n";
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<size_t> dist(1, m_numberOfPlayers);

	size_t randomChoice = dist(gen);
	std::cout << "Player " << randomChoice << " will start The Game!\n";
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
	if (playableCards < m_ctx.currentRequired) {
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

	while (true) {
		IPlayer& currentPlayer = GetCurrentPlayer();
		Round::UpdateContext(*this, m_ctx, currentPlayer);
		if(Round::IsGameWon(*this, currentPlayer)) {
			std::cout << "Players have won the game!\n";
			break;
		}
		Round::AbilityUse(*this, m_ctx, currentPlayer);
		if (IsGameOver(currentPlayer)) {
			break;
		}
		ShowCtx();
		int playedCards = 0;
		for (int i = 0; i < m_ctx.currentRequired; i++) {
			Round::OneRound(*this, m_ctx);
			if (IsGameOver(currentPlayer)) break;
			playedCards++;
		}
		if (currentPlayer.GetPlayerIndex() == m_ctx.GamblerPlayerIndex &&
			m_players[m_ctx.GamblerPlayerIndex]->GActive()) {
			currentPlayer.SetGActive(false);
		}
		else if (currentPlayer.GetPlayerIndex() == m_ctx.TaxEvPlayerIndex
			&& currentPlayer.IsTaxActive()) {
			std::cout << "You don't have to play any cards this round!\n";
		}
		int nrOfPlayableCards = Round::NrOfPlayableCardsInHand(*this, m_ctx);
		if (nrOfPlayableCards == 0) {
			std::cout << "Nu mai poti pune nici o carte. Trecem la urmatorul jucator.\n";
			for (size_t i = 0; i < playedCards; i++) {
				Card* drawnCard = m_wholeDeck.DrawCard();
				if (drawnCard) currentPlayer.AddCardToHand(drawnCard);
			}
		}
		else {
			while (nrOfPlayableCards > 0) {
				std::cout << "Mai poti pune " << nrOfPlayableCards << " carti.\n";
				std::cout << "Doresti sa continui? (y/n): ";
				char optiune;
				std::cin >> optiune;
				if (optiune == 'n') {
					break;
				}
				else if (optiune == 'y') {
					Round::OneRound(*this, m_ctx);
					playedCards++;
				}
				nrOfPlayableCards = Round::NrOfPlayableCardsInHand(*this, m_ctx);
			}
			for (size_t i = 0; i < playedCards; i++) {
				Card* drawnCard = m_wholeDeck.DrawCard();
				if (drawnCard) currentPlayer.AddCardToHand(drawnCard);
			}
			if (m_ctx.endgame && playedCards < 2) {
				m_gameStats[currentPlayer.GetID()].atLeastTwoCardsInEndgame = false;
			}
			/*currentPlayer.ShowHand();
			m_wholeDeck.ShowDeck();*/
			NextPlayer();
		}
		// CheckAndUnlockAchievements();
	}
}

void Game::NextPlayer()
{
	m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_numberOfPlayers;
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
	{"allOnRed", [](const IPlayer&, const GameStatistics& s, const StatisticsModel&) {
		return s.usedGambler && s.atLeastTwoCardsInEndgame;
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

void Game::CheckAndUnlockAchievements()
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
