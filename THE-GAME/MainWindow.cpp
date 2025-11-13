#include "MainWindow.h"
#include "HelpDialog.h"
#include <QPixmap>
#include <QMessageBox>


MainWindow::MainWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindowClass())
{
	ui->setupUi(this);
	setupMenuStyle();

	// Conectează butoanele la slot-uri
	connect(ui->newGameButton, &QPushButton::clicked, this, &MainWindow::onNewGameClicked);
	connect(ui->exitGameButton, &QPushButton::clicked, this, &MainWindow::onExitClicked); 
	connect(ui->helpButton, &QPushButton::clicked, this, &MainWindow::onHelpClicked);

	// Încarcă imaginea pentru titlu
	QPixmap titlePixmap("Assets/TitleCard.png"); // ajustează calea
	ui->titleLabel->setPixmap(titlePixmap);
	ui->titleLabel->setScaledContents(true);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::setupMenuStyle()
{
	// Setează fundalul pentru widget-ul central
	ui->centralWidget->setStyleSheet("background-color: #2C3E50;"); // exemplu de culoare

	// Stilizează butoanele cu aspect diferit la hover și press
	QString gameButtonStyle = R"(
	QPushButton {
		border-image: url(Assets/Button_Play.png);
	}
	QPushButton:pressed {
		border-image: url(Assets/Button_Play_Pressed.png);
	}
	)";

	QString exitButtonStyle = R"(
	QPushButton {
		border-image: url(Assets/Button_Exit.png);
	}
	QPushButton:pressed {
		border-image: url(Assets/Button_Exit_Pressed.png);
	}
	)";

	QString helpButtonStyle = R"(
	QPushButton {
		border-image: url(Assets/Button_Help.png);
	}
	QPushButton:pressed {
		border-image: url(Assets/Button_Help_Pressed.png);
	}
	)";

	ui->newGameButton->setStyleSheet(gameButtonStyle);
	ui->exitGameButton->setStyleSheet(exitButtonStyle);
	ui->helpButton->setStyleSheet(helpButtonStyle);
}

void MainWindow::onNewGameClicked()
{
	// Aici vei ascunde meniul și vei afișa ecranul de joc
	// Deocamdată doar un mesaj de test
	qDebug() << "New Game clicked!";
}

void MainWindow::onExitClicked()
{
	close();
}

void MainWindow::onHelpClicked()
{
	HelpDialog helpDialog(this);
	helpDialog.exec();
}