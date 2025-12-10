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
		m_players.push_back(PlayerFactory::CreateFromUser(user, abilities[i]));
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

bool Game::IsGameOver(const IPlayer& currentPlayer)
{
	int playableCards = Round::NrOfPlayableCardsInHand(*this, m_ctx);


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
		if (GetDeckSize() == 0) m_ctx.endgame = true;
		if (m_ctx.endgame) m_ctx.baseRequired = 1;
		else m_ctx.baseRequired = 2;
		if (m_ctx.endgame && m_currentPlayerIndex == m_ctx.GamblerPlayerIndex) {
			if (currentPlayer.GetHand().size() > 1 &&
				currentPlayer.GetGamblerUses() > 0) {
				m_ctx.currentRequired = 2;
			}
		}
		else if (m_ctx.TaxEvPlayerIndex != -1 && m_players[m_ctx.TaxEvPlayerIndex]->IsTaxActive() &&
			m_currentPlayerIndex == (m_ctx.TaxEvPlayerIndex + 1) % m_numberOfPlayers) {
			m_ctx.currentRequired = m_ctx.baseRequired * 2;
			m_players[m_ctx.TaxEvPlayerIndex]->SetTaxActive(false);
		}
		else m_ctx.currentRequired = m_ctx.baseRequired;
		if (currentPlayer.CanUseAbility(m_ctx)) {
			std::cout << "\n" << currentPlayer.GetUsername() << ", do you want to use your ability this turn? (y/n): ";
			char useAbility;
			std::cin >> useAbility;
			if (std::tolower(useAbility) == 'y') {
				currentPlayer.UseAbility(m_ctx, m_currentPlayerIndex);
				if (m_ctx.SoothPlayerIndex != -1) {
					std::cout << "Other player's cards: \n";
					for (size_t i = 0; i < m_numberOfPlayers; i++) {
						std::cout << m_players[i]->GetUsername() << ": ";
						m_players[i]->ShowHand();
					}
				}
			}
		}
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
		if (m_currentPlayerIndex == m_ctx.GamblerPlayerIndex &&
			m_players[m_ctx.GamblerPlayerIndex]->GActive()) {
			currentPlayer.SetGActive(false);
		}
		else if (m_currentPlayerIndex == m_ctx.TaxEvPlayerIndex
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
			/*currentPlayer.ShowHand();
			m_wholeDeck.ShowDeck();*/
			NextPlayer();
		}
	}
}


void Game::NextPlayer()
{
	m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_numberOfPlayers;
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
	return std::array<Pile*, PILES_AMOUNT>();
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
