#include "Game.h"

Game::Game(size_t numberOfPlayers) : m_numberOfPlayers{numberOfPlayers}
{
	m_players.reserve(m_numberOfPlayers);
	if(numberOfPlayers < 2 || numberOfPlayers > 5) {
		throw std::invalid_argument("Number of players must be between 2 and 5.");
	}
	for (int i = 2; i < 100; i++) {
		Card* newCard = new Card(std::to_string(i));
		m_wholeDeck.InsertCard(newCard);
		m_wholeDeck.InsertCard(newCard);
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

bool Game::IsGameOver(const Player* currentPlayer)
{
	Card* firstCard = nullptr;
	Card* secondCard = nullptr;
	std::vector<Card*> playerHand(currentPlayer->GetHand().begin(), currentPlayer->GetHand().end());
	for (Card* card : playerHand) {
		if (m_ascPile1.CanPlaceCard(card) || m_ascPile2.CanPlaceCard(card) ||
			m_descPile1.CanPlaceCard(card) || m_descPile2.CanPlaceCard(card)) {
			if(!firstCard) {
				firstCard = card;
			}
			else {
				secondCard = card;
				break;
			}
		}
	}
	if (m_wholeDeck.IsEmpty() && firstCard) return false;
	else if (!m_wholeDeck.IsEmpty() && firstCard != secondCard) return false;
	return true;
}

void Game::StartGame()
{	
	FirstRoundDealing();
	m_wholeDeck.ShuffleDeck();
	m_currentPlayerIndex = WhoStartsFirst();
	Player& startingPlayer = m_players[m_currentPlayerIndex];
	while(!IsGameOver(&GetCurrentPlayer())) {
		// Game logic for each player's turn would go here
		NextPlayer();
	}
}




void Game::NextPlayer()
{
	m_currentPlayerIndex = (m_currentPlayerIndex + 1) % m_numberOfPlayers;
}

Player& Game::GetCurrentPlayer()
{
	return m_players[m_currentPlayerIndex];
}

void Game::FirstRoundDealing()
{
	for (size_t i = 0; i < m_numberOfPlayers; i++) {
		m_players.emplace_back("Player" + std::to_string(i + 1), "password");
		Player& currentPlayer = m_players[i];
		for (size_t j = 0; j < 6; j++) {
			// Assuming Deck has a method to deal cards
			Card* dealtCard = m_wholeDeck.DrawCard();
			currentPlayer.AddCardToHand(dealtCard);
		}
	}
}

