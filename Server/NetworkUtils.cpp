#include "NetworkUtils.h"
#include <iostream>
#include <regex>
#include <thread>
#include "GameServer.h"

NetworkUtils::NetworkUtils() {
}

void NetworkUtils::StartChatWorker() {
    std::thread([this]() {
        this->ChatWorker();
    }).detach();
}

void NetworkUtils::BroadcastGameState(const std::string& lobby_id, crow::websocket::connection* targetConn) {
    std::lock_guard<std::mutex> lock(lobby_mutex);
    BroadcastGameStateLocked(lobby_id, targetConn);
}

void NetworkUtils::BroadcastGameStateLocked(const std::string& lobby_id, crow::websocket::connection* targetConn) {
    if (lobbies.find(lobby_id) == lobbies.end()) return;
    
    Lobby& lobby = *lobbies[lobby_id];
    Game* game = lobby.GetGame();
    if (!game) return;

    // Lock game state to prevent concurrent modification during iteration
    std::lock_guard<std::mutex> game_lock(game->GetStateMutex());

    crow::json::wvalue state_base;
    state_base["type"] = "game_state";
    state_base["lobby_id"] = lobby_id;
    
    // Piles
    std::vector<crow::json::wvalue> piles_json;
    auto piles = game->GetPiles();
    for (size_t i = 0; i < piles.size(); ++i) {
        if (piles[i]) {
            crow::json::wvalue pile_val;
            const Card* topCard = piles[i]->GetTopCard();
            if (topCard) {
                pile_val["top_card"] = std::string(topCard->GetCardValue());
            } else {
                pile_val["top_card"] = "ERR"; 
            }
            pile_val["count"] = (int)piles[i]->GetSize();
            piles_json.push_back(std::move(pile_val));
        }
    }
    state_base["piles"] = std::move(piles_json);
    
    // Deck
    state_base["deck_count"] = game->GetDeckSize();
    
    // Turn info
    try {
        IPlayer& current_player = game->GetCurrentPlayer();
        state_base["current_turn_player_id"] = current_player.GetID();
        state_base["current_turn_username"] = std::string(current_player.GetUsername());
    } catch (...) {
        state_base["current_turn_player_id"] = -1;
    }

    // Players info & Hands
    std::vector<crow::json::wvalue> players_json;
    const auto& players = game->GetPlayers();
    
    for (size_t i = 0; i < players.size(); ++i) {
        crow::json::wvalue player_val;
        player_val["user_id"] = players[i]->GetID();
        player_val["username"] = std::string(players[i]->GetUsername());
        player_val["hand_count"] = (int)players[i]->GetHand().size();
        player_val["is_finished"] = players[i]->IsFinished();
        player_val["player_index"] = players[i]->GetPlayerIndex();
        
        std::vector<std::string> hand_cards;
        for (const auto& card : players[i]->GetHand()) {
            if (card) {
                hand_cards.push_back(std::string(card->GetCardValue()));
            }
        }
        player_val["hand"] = std::move(hand_cards); 

        players_json.push_back(std::move(player_val));
    }
    state_base["players"] = std::move(players_json);

    // Send to connections
    std::lock_guard<std::mutex> ws_lock(ws_mutex);
    std::string msg = state_base.dump();

    if (lobby_connections.find(lobby_id) != lobby_connections.end()) {
        // If targetConn is specified, send only to it
        if (targetConn) {
            targetConn->send_text(std::string(msg));
        } 
        // Otherwise broadcast to all
        else {
            try {
                for (auto* conn : lobby_connections[lobby_id]) {
                    if (conn) {
                        conn->send_text(std::string(msg)); 
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error broadcasting game state to lobby " << lobby_id << ": " << e.what() << std::endl;
            } catch (...) {
                std::cerr << "Unknown error broadcasting game state to lobby " << lobby_id << std::endl;
            }
        }
    }
}

std::string NetworkUtils::CensorMessage(const std::string& input) {
    std::string result = input;
    std::regex digits(R"(\b([1-9]|[1-9][0-9]|100)\b)");
    result = std::regex_replace(result, digits, "###");
    
    std::string pattern = R"(\b()"
        R"((douazeci|treizeci|patruzeci|cincizeci|saizeci|saptezeci|optzeci|nouazeci)\s+si\s+(unu|doi|trei|patru|cinci|sase|sapte|opt|noua)|)"
        R"(one|two|three|four|five|six|seven|eight|nine|ten|)"
        R"(eleven|twelve|thirteen|fourteen|fifteen|sixteen|seventeen|eighteen|nineteen|)"
        R"(twenty|thirty|forty|fifty|sixty|seventy|eighty|ninety|hundred|)"
        R"(unu|doi|trei|patru|cinci|sase|sapte|opt|noua|zece|)"
        R"(unsprezece|doisprezece|treisprezece|patrusprezece|cincisprezece|saisprezece|saptesprezece|optsprezece|nouasprezece|)"
        R"(douazeci|treizeci|patruzeci|cincizeci|saizeci|saptezeci|optzeci|nouazeci|suta)"
        R"()\b)";
    
    std::regex words(pattern, std::regex_constants::icase);
    result = std::regex_replace(result, words, "###");
    
    return result;
}

void NetworkUtils::ChatWorker() {
    while (true) {
        std::unique_lock<std::mutex> lock(chatMutex);
        chatCv.wait(lock, [this] { return !chatQueue.empty(); });

        while (!chatQueue.empty()) {
            ChatMessage msg = chatQueue.front();
            chatQueue.pop();
            lock.unlock(); 

            // Broadcast
            {
                std::lock_guard<std::mutex> ws_lock(ws_mutex);
                auto it = lobby_connections.find(msg.lobby_id);
                if (it != lobby_connections.end()) {
                    try {
                        for (auto* conn : it->second) {
                            if (conn) conn->send_text(msg.message_json);
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Error broadcasting chat to lobby " << msg.lobby_id << ": " << e.what() << std::endl;
                    }
                }
            }

            lock.lock(); 
        }
    }
}
