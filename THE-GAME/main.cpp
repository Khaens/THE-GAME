#include "MainWindow.h"
#include <QApplication>
#include <QFontDatabase>
#include <iostream>

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

    int jerseyFontId = QFontDatabase::addApplicationFont("Fonts/Jersey15-Regular.ttf");
    int knightFontId = QFontDatabase::addApplicationFont("Fonts/KnightWarrior-w16n8.otf");

    if (jerseyFontId == -1) {
        std::cerr << "Failed to load Jersey 15 font" << std::endl;
    }
    if (knightFontId == -1) {
        std::cerr << "Failed to load Knight Warrior font" << std::endl;
    }

	MainWindow w;
	w.showFullScreen();

	return app.exec();
}

// End of THE-GAME/main.cpp
/*

/api/register
/api/login

/api/lobby/create
/api/lobby/join
/api/lobby/<id>/status
/api/lobby/<id>/players
/api/lobby/<id>/start
/api/lobby/leave

/ws/lobby - for lobby updates

subscribe
lobby_state
player_join
player_left
game_started
lobby_closed

/ws/game - for game actions and state updates

join_game
chat
game_action - action - play_card
game_action - action - use_ability
game_action - action - end_turn
game_state
game_over
error

*/