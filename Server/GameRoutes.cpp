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
            std::cout << "Game WebSocket closed: " << &conn << std::endl;
            
            std::string lid;
            int uid = -1;
            {
                std::lock_guard<std::mutex> lock(networkUtils.ws_mutex);
                if (networkUtils.connection_to_lobby.count(&conn)) lid = networkUtils.connection_to_lobby[&conn];
                if (networkUtils.connection_to_user.count(&conn)) uid = networkUtils.connection_to_user[&conn];

                for (auto& pair : networkUtils.lobby_connections) {
                    auto& vec = pair.second;
                    auto it = std::find(vec.begin(), vec.end(), &conn);
                    if (it != vec.end()) {
                        vec.erase(it);
                    }
                }
                networkUtils.connection_to_user.erase(&conn);
                networkUtils.connection_to_lobby.erase(&conn);
            }

            if (!lid.empty() && uid != -1) {
                std::lock_guard<std::mutex> lock(networkUtils.lobby_mutex);
                if (networkUtils.lobbies.count(lid)) {
                    Lobby& lobby = *networkUtils.lobbies[lid];
                    if (lobby.IsOwner(uid)) {
                        std::cout << "Host disconnected. Closing lobby: " << lid << std::endl;
                        
                        crow::json::wvalue update;
                        update["type"] = "lobby_closed";
                        update["lobby_id"] = lid;
                        update["reason"] = "Host disconnected";
                        std::string msg = update.dump();

                        {
                            std::lock_guard<std::mutex> ws_lock(networkUtils.ws_mutex);
                            if (networkUtils.lobby_connections.count(lid)) {
                                for (auto* c : networkUtils.lobby_connections[lid]) {
                                    if (c) c->send_text(msg);
                                }
                                networkUtils.lobby_connections.erase(lid);
                            }
                        }
                        {
                            std::lock_guard<std::mutex> l_ws_lock(networkUtils.lobby_ws_mutex);
                            networkUtils.lobby_update_connections.erase(lid);
                        }
                        networkUtils.lobbies.erase(lid);
                    } else {
                        lobby.LeaveLobby(uid);
                        networkUtils.BroadcastGameStateLocked(lid);
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

                 networkUtils.connection_to_lobby[&conn] = lid;
                 if (body.has("user_id")) {
                     networkUtils.connection_to_user[&conn] = static_cast<int>(body["user_id"].i());
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
                         
                         Card tempCard(std::to_string(card_val));
                         result = game->PlaceCard(player_idx, tempCard, pile_idx);
                         if (result == Info::CARD_PLACED || result == Info::TURN_ENDED || result == Info::GAME_WON || result == Info::GAME_LOST) {
                             state_changed = true;
                         }
                     } 
                     else if (action == "use_ability") {
                         result = game->UseAbility(player_idx);
                          if (result == Info::ABILITY_USED || result == Info::TAX_ABILITY_USED || 
                              result == Info::PEASANT_ABILITY_USED || result == Info::SOOTHSAYER_ABILITY_USED ||
                              result == Info::GAMBLER_ABILITY_USED || result == Info::HARRY_POTTER_ABILITY_USED ||
                              result == Info::TURN_ENDED) {
                              state_changed = true;
                          }
                     }
                     else if (action == "end_turn") {
                         if (players[player_idx]->IsSoothActive()) {
                             players[player_idx]->SetSoothState(false);
                         }
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
                         over_msg["clear_chat"] = true;
                         
                         std::string msg = over_msg.dump();
                         
                         {
                             std::lock_guard<std::mutex> ws_lock(networkUtils.ws_mutex);
                             if (networkUtils.lobby_connections.find(lid) != networkUtils.lobby_connections.end()) {
                                 for(auto* c : networkUtils.lobby_connections[lid]) {
                                     if(c) c->send_text(msg);
                                 }
                                 networkUtils.lobby_connections.erase(lid);
                             }
                         }
                         
                         {
                            std::lock_guard<std::mutex> l_ws_lock(networkUtils.lobby_ws_mutex);
                            if (networkUtils.lobby_update_connections.find(lid) != networkUtils.lobby_update_connections.end()) {
                                networkUtils.lobby_update_connections.erase(lid);
                            }
                         }

                         std::cout << "Game finished for lobby " << lid << ". Destroying lobby." << std::endl;
                         
                         networkUtils.lobbies.erase(lid);
                     }
                     
                     if (result == Info::CARD_NOT_PLAYABLE || result == Info::NOT_CURRENT_PLAYER_TURN) {
                         crow::json::wvalue err;
                         err["type"] = "error";
                         if (result == Info::CARD_NOT_PLAYABLE) err["message"] = "Card not playable";
                         if (result == Info::NOT_CURRENT_PLAYER_TURN) err["message"] = "Not your turn";
                         conn.send_text(err.dump());
                     }

                     if (result != Info::GAME_WON && result != Info::GAME_LOST) {
                         networkUtils.BroadcastGameStateLocked(lid);
                     }

                     // Check for and broadcast newly unlocked achievements
                     auto newlyUnlocked = game->UnlockAchievements();
                     for (const auto& pair : newlyUnlocked) {
                         networkUtils.BroadcastAchievement(lid, pair.first, pair.second);
                     }
                 }
                 
                 if (type == "join_game") {
                     networkUtils.BroadcastGameState(lid, &conn);
                 }
             }
         });
}
