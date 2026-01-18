#include "AuthRoutes.h"
#include <iostream>

AuthRoutes::AuthRoutes(crow::SimpleApp& app, Database* db, NetworkUtils& networkUtils) {
    RegisterRoutes(app, db, networkUtils);
}

void AuthRoutes::RegisterRoutes(crow::SimpleApp& app, Database* db, NetworkUtils& networkUtils) {
    
    // REGISTER
    CROW_ROUTE(app, "/api/register")
        .methods(crow::HTTPMethod::POST)
    ([db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string password = body["password"].s();
        std::cout << "Register attempt for user: " << username << std::endl;

        if (db->UserExists(username)) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error"] = "User already exists";
            return crow::response(409, response);
        }

        try {
            UserModel user{-1, username, password};
            int id = db->InsertUser(user);
            
            crow::json::wvalue response;
            response["success"] = true;
            response["user_id"] = id;
            return crow::response(201, response);
        } catch (const std::exception& e) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error"] = e.what();
            return crow::response(500, response);
        }
    });

    // LOGIN
    CROW_ROUTE(app, "/api/login")
        .methods(crow::HTTPMethod::POST)
        ([db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            return crow::response(400, "Invalid JSON");
        }

        std::string username = body["username"].s();
        std::string password = body["password"].s();
        std::cout << "Login attempt for user: " << username << std::endl;

        try {
            UserModel user = db->GetUserByUsername(username);
            if (db->VerifyLogin(username,password)) {
                crow::json::wvalue response;
                response["success"] = true;
                response["user_id"] = user.GetId();
                response["username"] = std::string(user.GetUsername());
                return crow::response(200, response);
            }

            crow::json::wvalue response;
            response["success"] = false;
            response["error"] = "Invalid password";
            return crow::response(401, response);
        }
        catch (const std::exception& e) {
            crow::json::wvalue response;
            response["success"] = false;
            response["error"] = "User not found";
            return crow::response(401, response);
        }
    });

    // PROFILE PICTURE UPLOAD
    CROW_ROUTE(app, "/api/user/<int>/profile-picture")
        .methods(crow::HTTPMethod::POST)
        ([db](const crow::request& req, int user_id) {
        std::vector<char> imageData(req.body.begin(), req.body.end());
        
        if (imageData.empty()) {
             crow::json::wvalue response;
             response["success"] = false;
             response["error"] = "Empty image data";
             return crow::response(400, response);
        }

        try {
            if (db->UpdateProfileImage(user_id, imageData)) {
                crow::json::wvalue response;
                response["success"] = true;
                return crow::response(200, response);
            } else {
                 crow::json::wvalue response;
                 response["success"] = false;
                 response["error"] = "Failed to update profile image (User not found?)";
                 return crow::response(500, response);
            }
        } catch (const std::exception& e) {
             crow::json::wvalue response;
             response["success"] = false;
             response["error"] = e.what();
             return crow::response(500, response);
        }
    });

    // PROFILE PICTURE GET
    CROW_ROUTE(app, "/api/user/<int>/profile-picture")
         .methods(crow::HTTPMethod::GET)
         ([db](int user_id) {
         try {
             std::vector<char> img = db->GetProfileImage(user_id);
             if (img.empty()) {
                 return crow::response(404, "No profile image");
             }
             std::string imgStr(img.begin(), img.end());
             crow::response res(imgStr);
             res.add_header("Content-Type", "image/png");
             return res;
         } catch (...) {
             return crow::response(500, "Error retrieving image");
         }
    });

    // CHECK PROFILE PICTURE
    CROW_ROUTE(app, "/api/user/<int>/has-profile-picture")
        .methods(crow::HTTPMethod::GET)
        ([db](int user_id) {
        bool has = db->HasProfileImage(user_id);
        crow::json::wvalue response;
        response["has_image"] = has;
        return crow::response(200, response);
    });

    // DELETE PROFILE PICTURE
    CROW_ROUTE(app, "/api/user/<int>/profile-picture")
#ifdef DELETE
#undef DELETE
#endif
        .methods(crow::HTTPMethod::DELETE)
        ([db](int user_id) {
        db->DeleteProfileImage(user_id);
        crow::json::wvalue response;
        response["success"] = true;
        return crow::response(200, response);
    });

    // GET ACHIEVEMENTS
    CROW_ROUTE(app, "/api/user/<int>/achievements")
        .methods(crow::HTTPMethod::GET)
        ([db](int user_id) {
        try {
            AchievementsModel ach = db->GetAchievementsByUserId(user_id);
            
            crow::json::wvalue response;
            response["success"] = true;
            response["all_on_red"] = ach.GetAllOnRed();
            response["harry_potter"] = ach.GetHarryPotter();
            response["soothsayer"] = ach.GetSoothsayer();
            response["tax_evader"] = ach.GetTaxEvader();
            response["gambler"] = ach.GetGambler();
            response["peasant"] = ach.GetPeasant();
            response["serious_player"] = ach.GetSeriousPlayer();
            response["talented_player"] = ach.GetTalentedPlayer();
            response["jack"] = ach.GetJack();
            response["zero_effort"] = ach.GetZeroEffort();
            response["vanilla_w"] = ach.GetVanillaW();
            response["high_risk"] = ach.GetHighRisk();
            response["perfect_game"] = ach.GetPerfectGame();
            response["six_seven"] = ach.GetSixSeven();
            
            return crow::response(200, response);
        } catch (const std::exception& e) {
             crow::json::wvalue response;
             response["success"] = false;
             response["error"] = e.what();
             return crow::response(500, response);
        }
    });

    // GET STATISTICS
    CROW_ROUTE(app, "/api/user/<int>/statistics")
        .methods(crow::HTTPMethod::GET)
        ([db](int user_id) {
        try {
            StatisticsModel stats = db->GetStatisticsByUserId(user_id);
            
            crow::json::wvalue response;
            response["success"] = true;
            response["games_won"] = stats.GetGamesWon();
            response["games_played"] = stats.GetGamesPlayed();
            response["win_rate"] = stats.GetWinRate();
            response["performance_score"] = stats.GetPerformanceScore();
            
            return crow::response(200, response);
        } catch (const std::exception& e) {
             crow::json::wvalue response;
             response["success"] = false;
             response["error"] = e.what();
             return crow::response(500, response);
        }
    });

    // GET PLAYTIME
    CROW_ROUTE(app, "/api/user/<int>/playtime")
        .methods(crow::HTTPMethod::GET)
        ([db](int user_id) {
        try {
            PlaytimeModel playtime = db->GetPlaytimeByUserId(user_id);
            
            crow::json::wvalue response;
            response["success"] = true;
            response["seconds"] = playtime.GetSeconds();
            response["hours"] = playtime.GetTotalHours();
            
            return crow::response(200, response);
        } catch (const std::exception& e) {
             crow::json::wvalue response;
             response["success"] = false;
             response["error"] = e.what();
             return crow::response(500, response);
        }
    });

}
