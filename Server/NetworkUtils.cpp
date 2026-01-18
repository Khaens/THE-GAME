#include "NetworkUtils.h"
#include <iostream>
#include <regex>
#include <thread>
#include <list>
#include "GameServer.h"

NetworkUtils::NetworkUtils() {
}

void NetworkUtils::StartChatWorker() {
    std::thread([this]() {
        this->ChatWorker();
    }).detach();
}

void NetworkUtils::StartWsWorker() {
    std::thread([this]() {
        this->WsWorker();
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
    } catch (const std::exception& e) {
        std::cerr << "Error getting current player in BroadcastGameState: " << e.what() << std::endl;
        state_base["current_turn_player_id"] = -1;
        state_base["current_turn_username"] = "Unknown";
    } catch (...) {
        std::cerr << "Unknown error getting current player in BroadcastGameState" << std::endl;
        state_base["current_turn_player_id"] = -1;
        state_base["current_turn_username"] = "Unknown";
    }

    // Context Info
    state_base["current_required"] = game->GetCtx().currentRequired;

    // Players info & Hands
    std::vector<crow::json::wvalue> players_json;
    const auto& players = game->GetPlayers();
    
    for (size_t i = 0; i < players.size(); ++i) {
        try {
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

        // Add Ability
        std::string abilityName = "Unknown";
        switch (players[i]->GetAbilityType()) {
            case AbilityType::HarryPotter: abilityName = "HarryPotter"; break;
            case AbilityType::Gambler: abilityName = "Gambler"; break;
            case AbilityType::TaxEvader: abilityName = "TaxEvader"; break;
            case AbilityType::Soothsayer: abilityName = "Soothsayer"; break;
            case AbilityType::Peasant: abilityName = "Peasant"; break;
        }
        player_val["ability"] = abilityName;
        
        // Ability Active States
        player_val["is_hp_active"] = players[i]->HPActive();
        player_val["is_sooth_active"] = players[i]->IsSoothActive();
        player_val["is_tax_active"] = players[i]->IsTaxActive();
        
        // Can player use their ability? (checked against game context)
        player_val["can_use_ability"] = players[i]->CanUseAbility(game->GetCtx());
        
        // For Gambler, also send uses left
        if (players[i]->GetAbilityType() == AbilityType::Gambler) {
            player_val["gambler_uses_left"] = (int)players[i]->GetGamblerUses();
        }
        
        // For Soothsayer, also send uses left
        if (players[i]->GetAbilityType() == AbilityType::Soothsayer) {
            player_val["soothsayer_uses_left"] = (int)players[i]->GetSoothsayerUses();
        }

        // For TaxEvader, also send uses left
        if (players[i]->GetAbilityType() == AbilityType::TaxEvader) {
            player_val["tax_evader_uses_left"] = (int)players[i]->GetTaxEvaderUses();
        }

            players_json.push_back(std::move(player_val));
        } catch (const std::exception& e) {
            std::cerr << "Error serializing player " << i << " in BroadcastGameState: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown error serializing player " << i << " in BroadcastGameState" << std::endl;
        }
    }
    state_base["players"] = std::move(players_json);

    // Send to connections
    std::lock_guard<std::mutex> ws_lock(ws_mutex);
    std::string msg = state_base.dump();

    if (lobby_connections.find(lobby_id) != lobby_connections.end()) {
        if (targetConn) {
            SafeSendText(targetConn, msg);
        } 
        else {
            try {
                for (auto* conn : lobby_connections[lobby_id]) {
                    SafeSendText(conn, msg); 
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

            {
                std::lock_guard<std::mutex> ws_lock(ws_mutex);
                auto it = lobby_connections.find(msg.lobby_id);
                if (it != lobby_connections.end()) {
                    for (auto* conn : it->second) {
                        SafeSendText(conn, msg.message_json);
                    }
                }
            }

            lock.lock(); 
        }
    }
}

void NetworkUtils::SafeSendText(crow::websocket::connection* conn, const std::string& msg) {
    if (!conn) return;
    
    {
        std::lock_guard<std::mutex> lock(wsSendMutex);
        wsQueue.push({conn, msg});
    }
    wsSendCv.notify_one();
}

void NetworkUtils::WsWorker() {
    while (m_wsWorkerRunning) {
        WsMessage wsMsg;
        
        {
            std::unique_lock<std::mutex> lock(wsSendMutex);
            wsSendCv.wait(lock, [this]{ return !wsQueue.empty() || !m_wsWorkerRunning; });
            
            if (!m_wsWorkerRunning && wsQueue.empty()) break;
            
            wsMsg = std::move(wsQueue.front());
            wsQueue.pop();
        }
        
        bool isValid = false;
        {
            std::lock_guard<std::mutex> lock(m_validConnMutex);
            isValid = m_validConnections.count(wsMsg.conn) > 0;
        }
        
        if (!isValid) {
            continue;
        }
        
        try {
            if (wsMsg.conn) {
                std::string messageCopy = wsMsg.message; 
                {
                    std::lock_guard<std::mutex> lock(m_pendingMsgMutex);
                    m_pendingMessages.push_back(messageCopy);
                }
                
                wsMsg.conn->send_text(messageCopy);
                
                static int sendCount = 0;
                if (++sendCount % 100 == 0) {
                     CleanupPendingMessages();
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Error in WsWorker send: " << e.what() << std::endl;
        }
    }
}

void NetworkUtils::CleanupPendingMessages() {
    std::lock_guard<std::mutex> lock(m_pendingMsgMutex);
    while (m_pendingMessages.size() > 1000) {
        m_pendingMessages.pop_front();
    }
}
