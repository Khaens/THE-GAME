#include "Game.h"
#include "MainWindow.h"
#include <QApplication>

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);

	//Game g(3);
	MainWindow w;
	//w.setGame(&g);
	w.show();

	return app.exec();
}