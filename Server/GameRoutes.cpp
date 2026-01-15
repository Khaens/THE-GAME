#include "GameRoutes.h"
#include <iostream>
#include <algorithm>

GameRoutes::GameRoutes(crow::SimpleApp& app, Database* db, NetworkUtils& networkUtils) {
    RegisterRoutes(app, db, networkUtils);
}

void GameRoutes::RegisterRoutes(crow::SimpleApp& app, Database* db, NetworkUtils& networkUtils) {
    
    CROW_WEBSOCKET_ROUTE(app, "/ws/game")
        .onopen([](crow::websocket::connection& conn) {
            std::cout << "New Game WebSocket connection" << std::endl;
        })
        .onclose([&networkUtils](crow::websocket::connection& conn, const std::string& reason, uint16_t) {
            std::cout << "Game WebSocket closed" << std::endl;
            std::lock_guard<std::mutex> lock(networkUtils.ws_mutex);
             for (auto& [lobby_id, connections] : networkUtils.lobby_connections) {
             for (auto& [lobby_id, connections] : networkUtils.lobby_connections) {
                 for (size_t i = 0; i < connections.size(); ++i) {
                     if (connections[i] == &conn) {
                         if (i != connections.size() - 1) {
                             connections[i] = connections.back();
                         }
                         connections.pop_back();
                         break; 
                     }
                 }
             }
             }
        })
        .onmessage([&networkUtils](crow::websocket::connection& conn, const std::string& data, bool is_binary) {
             auto body = crow::json::load(data);
             if(!body) return;

             if (body.has("lobby_id")) {
                 std::string lid = body["lobby_id"].s();
                 std::string type = body.has("type") ? (std::string)body["type"].s() : "";
                 
                 {
                     std::lock_guard<std::mutex> lock(networkUtils.ws_mutex);
                     bool found = false;
                     for(auto* c : networkUtils.lobby_connections[lid]) {
                         if(c == &conn) found = true;
                     }
                     if(!found) networkUtils.lobby_connections[lid].push_back(&conn);
                 }

                 if (type == "chat") {
                     std::lock_guard<std::mutex> lock(networkUtils.lobby_mutex);
                     if (networkUtils.lobbies.find(lid) != networkUtils.lobbies.end()) {
                         Lobby& lobby = *networkUtils.lobbies[lid];
                         Game* game = lobby.GetGame();
                         if (game) {
                             int uid = body["user_id"].i();
                             std::string username = "Unknown";
                             
                             const auto& players = game->GetPlayers();
                             for(const auto& p : players) {
                                 if (p->GetID() == uid) {
                                     username = std::string(p->GetUsername());
                                     break;
                                 }
                             }
                             
                             std::string raw_msg = body["message"].s();
                             std::string censored_msg = networkUtils.CensorMessage(raw_msg);

                             crow::json::wvalue chat_val;
                             chat_val["type"] = "chat";
                             chat_val["lobby_id"] = lid;
                             chat_val["user_id"] = uid;
                             chat_val["username"] = username;
                             chat_val["message"] = censored_msg;
                             
                             // Enqueue instead of direct send
                             {
                                 std::lock_guard<std::mutex> qLock(networkUtils.chatMutex);
                                 networkUtils.chatQueue.push({lid, chat_val.dump()});
                             }
                             networkUtils.chatCv.notify_one();
                         }
                     }
                     return;
                 }
                 
                 if (type == "game_action") {
                     std::string action = body["action"].s();
                     int user_id = body["user_id"].i();
                     
                     std::lock_guard<std::mutex> lock(networkUtils.lobby_mutex);
                     if (networkUtils.lobbies.find(lid) == networkUtils.lobbies.end()) return;
                     Lobby& lobby = *networkUtils.lobbies[lid];
                     Game* game = lobby.GetGame();
                     if (!game) return;
                     
                     int player_idx = -1;
                     const auto& players = game->GetPlayers();
                     for(size_t i=0; i<players.size(); ++i) {
                         if(players[i]->GetID() == user_id) {
                             player_idx = i;
                             break;
                         }
                     }
                     
                     if (player_idx == -1) return; 
                     
                     Info result = Info::TURN_ENDED; 
                     bool state_changed = false;

                     if (action == "play_card") {
                         int card_val = body["card_value"].i();
                         int pile_idx = body["pile_index"].i();
                         result = game->PlaceCard(player_idx, card_val, pile_idx);
                         if (result == Info::CARD_PLACED || result == Info::TURN_ENDED || result == Info::GAME_WON || result == Info::GAME_LOST) {
                             state_changed = true;
                         }
                     } 
                     else if (action == "use_ability") {
                         result = game->UseAbility(player_idx);
                         if (result == Info::ABILITY_USED || result == Info::TAX_ABILITY_USED || result == Info::PEASANT_ABILITY_USED || result == Info::TURN_ENDED) {
                             state_changed = true;
                         }
                     }
                     else if (action == "end_turn") {
                         result = game->EndTurn(player_idx);
                         state_changed = true;
                     }

                     if (result == Info::GAME_WON || result == Info::GAME_LOST) {
                         crow::json::wvalue over_msg;
                         over_msg["type"] = "game_over";
                         over_msg["lobby_id"] = lid;
                         if (result == Info::GAME_WON) {
                             over_msg["winner_id"] = user_id;
                             over_msg["result"] = "won";
                         } else {
                             over_msg["result"] = "lost"; 
                         }
                         
                         std::string msg = over_msg.dump();
                         std::lock_guard<std::mutex> ws_lock(networkUtils.ws_mutex);
                         for(auto* c : networkUtils.lobby_connections[lid]) {
                             if(c) c->send_text(msg);
                         }
                     }
                     
                     if (result == Info::CARD_NOT_PLAYABLE || result == Info::NOT_CURRENT_PLAYER_TURN) {
                         crow::json::wvalue err;
                         err["type"] = "error";
                         if (result == Info::CARD_NOT_PLAYABLE) err["message"] = "Card not playable";
                         if (result == Info::NOT_CURRENT_PLAYER_TURN) err["message"] = "Not your turn";
                         conn.send_text(err.dump());
                     }
                     // Only broadcast game state if the game is NOT over (to avoid conflict with game_over message)
                     // and to prevent unnecessary traffic
                     if (result != Info::GAME_WON && result != Info::GAME_LOST) {
                         // Use Locked version because lobby_mutex is already held at line 85
                         networkUtils.BroadcastGameStateLocked(lid);
                     }
                 }
                 
                 if (type == "join_game") {
                     // For "join_game", only send state to the requesting client
                     // This prevents N*N broadcasts when N players join simultaneously
                     networkUtils.BroadcastGameState(lid, &conn);
                 }
             }
         });
}
