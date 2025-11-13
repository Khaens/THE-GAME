#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"
#include "Game.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; };
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

private slots:
	void onNewGameClicked();
	void onExitClicked();
	void onHelpClicked();

private:
	Ui::MainWindowClass* ui;
	void setupMenuStyle();
};