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