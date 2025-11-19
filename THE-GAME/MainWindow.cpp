#include "MainWindow.h"
#include "HelpDialog.h"
#include <QPixmap>
#include <QMessageBox>
#include <QShortcut>
#include <QKeySequence>
#include <QResizeEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass())
{
    ui->setupUi(this);
    setupMenuStyle();

    // Connect buttons to slots
    connect(ui->newGameButton, &QPushButton::clicked, this, &MainWindow::onNewGameClicked);
    connect(ui->exitGameButton, &QPushButton::clicked, this, &MainWindow::onExitClicked);
    connect(ui->helpButton, &QPushButton::clicked, this, &MainWindow::onHelpClicked);

    // Shortcut for fullscreen toggle (F11)
    QShortcut* fsShortcut = new QShortcut(QKeySequence(Qt::Key_F11), this);
    connect(fsShortcut, &QShortcut::activated, this, &MainWindow::toggleFullScreen);

    // Load the title image once and cache it. We'll scale on resizeEvent.
    QPixmap titlePixmap("Assets/TitleCard.png"); // adjust path if necessary
    if (!titlePixmap.isNull()) {
        m_titlePixmap = titlePixmap;
        // set an initial scaled pixmap according to the current label size
        QPixmap scaled = m_titlePixmap.scaled(ui->titleLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->titleLabel->setPixmap(scaled);
        ui->titleLabel->setAlignment(Qt::AlignCenter);
    }
    else {
        qWarning() << "Failed to load Assets/TitleCard.png — check the path or use a resource (qrc)";
    }

    // Ensure central widget can expand — defensive; Designer already set policies
    ui->centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMenuStyle()
{
    // Set background for central widget
    ui->centralWidget->setStyleSheet("background-color: #208a3a;");

    // Button styles (image-based)
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

void MainWindow::toggleFullScreen()
{
    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    // Rescale the cached title pixmap to the new label size, preserving aspect ratio
    if (!m_titlePixmap.isNull() && ui->titleLabel) {
        QSize labelSize = ui->titleLabel->size();
        if (labelSize.width() > 0 && labelSize.height() > 0) {
            QPixmap scaled = m_titlePixmap.scaled(labelSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->titleLabel->setPixmap(scaled);
            ui->titleLabel->setAlignment(Qt::AlignCenter);
        }
    }

    // If later you add a QGraphicsView canvas, call fitInView here as well.

    QMainWindow::resizeEvent(event);
}