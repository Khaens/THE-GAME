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
    state_base["turn_count"] = game->GetTurnCount();
    
    // Turn info
    try {
        Player& current_player = game->GetCurrentPlayer();
        state_base["current_turn_player_id"] = current_player.GetId();
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
            player_val["user_id"] = players[i].GetId();
            player_val["username"] = std::string(players[i].GetUsername());
            player_val["hand_count"] = (int)players[i].GetHand().size();
            player_val["is_finished"] = players[i].IsFinished();
            player_val["is_active"] = players[i].IsPlayerActive();
            player_val["player_index"] = players[i].GetPlayerIndex();
            
            std::vector<std::string> hand_cards;
            for (const auto& card : players[i].GetHand()) {
                if (card) {
                    hand_cards.push_back(std::string(card->GetCardValue()));
                }
            }
            player_val["hand"] = std::move(hand_cards); 

        // Add Ability
        std::string abilityName = "Unknown";
        switch (players[i].GetAbilityType()) {
            case AbilityType::HarryPotter: abilityName = "HarryPotter"; break;
            case AbilityType::Gambler: abilityName = "Gambler"; break;
            case AbilityType::TaxEvader: abilityName = "TaxEvader"; break;
            case AbilityType::Soothsayer: abilityName = "Soothsayer"; break;
            case AbilityType::Peasant: abilityName = "Peasant"; break;
        }
        player_val["ability"] = abilityName;
        
        // Ability Active States
        player_val["is_hp_active"] = players[i].HPActive();
        player_val["is_sooth_active"] = players[i].IsSoothActive();
        player_val["is_tax_active"] = players[i].IsTaxActive();
        
        // Can player use their ability? (checked against game context)
        player_val["can_use_ability"] = players[i].CanUseAbility(game->GetCtx());
        
        // For Gambler, also send uses left
        if (players[i].GetAbilityType() == AbilityType::Gambler) {
            player_val["gambler_uses_left"] = (int)players[i].GetGamblerUses();
        }
        
        // For Soothsayer, also send uses left
        if (players[i].GetAbilityType() == AbilityType::Soothsayer) {
            player_val["soothsayer_uses_left"] = (int)players[i].GetSoothsayerUses();
        }

        // For TaxEvader, also send uses left
        if (players[i].GetAbilityType() == AbilityType::TaxEvader) {
            player_val["tax_evader_uses_left"] = (int)players[i].GetTaxEvaderUses();
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

void NetworkUtils::StartPingWorker() {
    std::thread([this]() {
        this->PingWorker();
    }).detach();
}

void NetworkUtils::PingWorker() {
    while (m_wsWorkerRunning) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        auto now = std::chrono::steady_clock::now();
        std::vector<std::pair<std::string, int>> disconnected_users;

        // Check for timeouts
        {
            std::lock_guard<std::mutex> lock(m_clientStatesMutex);
            for (auto it = m_clientStates.begin(); it != m_clientStates.end(); ) {
                auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.last_activity).count();
                if (duration > 30) { 
                    disconnected_users.push_back({it->second.lobby_id, it->second.user_id});
                    it = m_clientStates.erase(it);
                } else {
                    ++it;
                }
            }
        }

        for (const auto& user : disconnected_users) {
            std::cout << "PingWorker detected timeout for user: " << user.second << " in lobby: " << user.first << std::endl;
            HandlePlayerDisconnect(user.first, user.second);
        }

        static auto last_ping_broadcast = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - last_ping_broadcast).count() >= 10) {
            last_ping_broadcast = now;
            std::cout << "[PingWorker] Broadcasting Ping to all active games..." << std::endl;
            
            crow::json::wvalue ping_msg;
            ping_msg["type"] = "ping";
            std::string msg_str = ping_msg.dump();

            std::lock_guard<std::mutex> ws_lock(ws_mutex);
            for (auto& [lobby_id, connections] : lobby_connections) {
                for (auto* conn : connections) {
                    SafeSendText(conn, msg_str);
                }
            }
        }
    }
}

void NetworkUtils::HandlePlayerDisconnect(const std::string& lobby_id, int user_id) {
    std::lock_guard<std::mutex> lobby_lock(lobby_mutex);
    if (lobbies.find(lobby_id) == lobbies.end()) return;
    
    Lobby& lobby = *lobbies[lobby_id];
    Game* game = lobby.GetGame();
    if (!game) return;
    
    int p_index = -1;
    {
        std::lock_guard<std::mutex> game_lock(game->GetStateMutex());
        auto& players = game->GetPlayers();
        for (size_t i = 0; i < players.size(); ++i) {
            if (players[i].GetId() == user_id) {
                players[i].SetActive(false);
                p_index = (int)i;
                break;
            }
        }
    }
    
    if (p_index == -1) return;

    if (lobby.IsOwner(user_id)) {
        crow::json::wvalue update;
        update["type"] = "lobby_closed";
        update["lobby_id"] = lobby_id;
        update["reason"] = "Host left the game";
        std::string update_msg = update.dump();

        {
            std::lock_guard<std::mutex> ws_lock(lobby_ws_mutex);
            for (auto* conn : lobby_update_connections[lobby_id]) {
                SafeSendText(conn, update_msg);
            }
            lobby_update_connections.erase(lobby_id);
        }
        {
            std::lock_guard<std::mutex> ws_lock(ws_mutex);
            if (lobby_connections.count(lobby_id)) {
                for (auto* conn : lobby_connections[lobby_id]) {
                    SafeSendText(conn, update_msg);
                }
                lobby_connections.erase(lobby_id);
            }
        }

        lobbies.erase(lobby_id);
        std::cout << "Lobby " << lobby_id << " destroyed (host left)" << std::endl;
        return;
    }

    // Remove from lobby users list as well
    lobby.LeaveLobby(user_id);

    if (lobby.GetCurrentPlayers() == 0) {
        std::cout << "Lobby " << lobby_id << " destroyed (all players disconnected)" << std::endl;
        lobbies.erase(lobby_id);
        return;
    }

    bool state_changed = false;
    Info result = Info::TURN_ENDED;

    ProcessInactiveTurns(game, result, state_changed);

    if (result == Info::GAME_WON || result == Info::GAME_LOST) {
        game->UnlockAchievements();
        crow::json::wvalue over_msg;
        over_msg["type"] = "game_over";
        over_msg["lobby_id"] = lobby_id;
        if (result == Info::GAME_WON) {
            int winner = -1;
            {
                std::lock_guard<std::mutex> game_lock(game->GetStateMutex());
                winner = game->GetCurrentPlayer().GetID();
            }
            over_msg["winner_id"] = winner;
            over_msg["result"] = "won";
        } else {
            over_msg["result"] = "lost"; 
        }
        
        std::string msg = over_msg.dump();
        {
            std::lock_guard<std::mutex> ws_lock(ws_mutex);
            if (lobby_connections.find(lobby_id) != lobby_connections.end()) {
                for(auto* c : lobby_connections[lobby_id]) {
                    SafeSendText(c, msg);
                }
                lobby_connections.erase(lobby_id);
            }
        }
        
        {
            std::lock_guard<std::mutex> l_ws_lock(lobby_ws_mutex);
            if (lobby_update_connections.find(lobby_id) != lobby_update_connections.end()) {
                lobby_update_connections.erase(lobby_id);
            }
        }

        std::cout << "Game finished for lobby " << lobby_id << " due to disconnect. Destroying." << std::endl;
        lobbies.erase(lobby_id);
        return;
    }

    if (state_changed) {
        BroadcastGameStateLocked(lobby_id);
        game->UnlockAchievements();
    }
}

void NetworkUtils::ProcessInactiveTurns(Game* game, Info& result, bool& state_changed) {
    while (true) {
        size_t current_idx = 0;
        {
            std::lock_guard<std::mutex> game_lock(game->GetStateMutex());
            current_idx = game->GetCurrentPlayer().GetPlayerIndex();
            if (game->GetCurrentPlayer().IsPlayerActive()) {
                break;
            }
            if (game->IsGameOver(game->GetCurrentPlayer())) {
                break;
            }
        }
        
        Info ai_result = game->InactivePlayerTurn(current_idx);

        if (ai_result == Info::GAME_WON || ai_result == Info::GAME_LOST) {
            result = ai_result;
            state_changed = true;
            break;
        }
        // If AI turn made no progress (e.g. bad state), stop to avoid infinite loop
        if (ai_result != Info::TURN_ENDED) {
            std::cout << "[ProcessInactiveTurns] AI turn returned unexpected state, stopping." << std::endl;
            break;
        }
        state_changed = true;
    }
}
