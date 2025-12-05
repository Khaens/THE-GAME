#include "GameWindow.h"
#include "ui_GameWindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QResizeEvent>

GameWindow::GameWindow(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::Form())
{
    ui->setupUi(this);
    setWindowFlags(Qt::Widget);  // Important: Makes it a child widget
    setupUI();
    hide();  // Start hidden
}

void GameWindow::setupUI()
{
    // Remove any existing layout
    if (layout()) {
        delete layout();
    }

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);  // Some margin from edges
    mainLayout->setSpacing(20);

    // Title
    QLabel* titleLabel = new QLabel("GAME IN PROGRESS", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 32px;
            font-weight: bold;
            color: #f3d05a;
            padding: 20px;
            border: 2px solid #f3d05a;
            border-radius: 10px;
            background-color: rgba(101, 75, 31, 0.3);
        }
    )");
    mainLayout->addWidget(titleLabel);

    // Game content placeholder
    QLabel* contentLabel = new QLabel("Game content will go here\n\nCards, piles, player info...", this);
    contentLabel->setAlignment(Qt::AlignCenter);
    contentLabel->setStyleSheet(R"(
        QLabel {
            font-size: 18px;
            color: #ffffff;
            padding: 20px;
            background-color: rgba(101, 75, 31, 0.2);
            border-radius: 10px;
            min-height: 300px;
        }
    )");
    mainLayout->addWidget(contentLabel);

    // Back button
    QPushButton* backButton = new QPushButton("Back to Main Menu", this);
    backButton->setFixedSize(200, 45);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet(R"(
        QPushButton {
            background-color: #e03434;
            color: #ffffff;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #b82a2a;
        }
        QPushButton:pressed {
            background-color: #a12525;
        }
    )");
    connect(backButton, &QPushButton::clicked, this, &GameWindow::onBackButtonClicked);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // Semi-transparent dark background for overlay effect
    setStyleSheet(R"(
        GameWindow {
            background-color: rgba(13, 10, 71, 0.95);  /* Semi-transparent */
            border: 3px solid #f3d05a;
            border-radius: 15px;
        }
    )");
    
    ui->formBackground->setStyleSheet("background-color: #0d0a47;");
}

void GameWindow::showOverlay()
{
    if (parentWidget()) {
        setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    }
    raise();  // Bring to front
    show();
}

void GameWindow::hideOverlay()
{
    hide();
}

void GameWindow::resizeEvent(QResizeEvent* event)
{
    if (parentWidget() && isVisible()) {
        setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    }
    QWidget::resizeEvent(event);
}

void GameWindow::onBackButtonClicked()
{
    hideOverlay();
    emit backToMenuRequested();
}