#include "WinDialog.h"
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>

WinDialog::WinDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    
    // Window Settings
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);
}

void WinDialog::setupUi()
{
    resize(500, 400);

    // Main Layout (centered)
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Background Container (to apply image)
    QWidget* container = new QWidget(this);
    container->setObjectName("container");
    container->setStyleSheet(R"(
        QWidget#container {
            border-image: url(Resources/TextBox_1-2.png);
            background-color: transparent;
        }
    )");
    
    QVBoxLayout* containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(40, 60, 40, 60);
    containerLayout->setSpacing(20);

    // Title
    m_titleLabel = new QLabel("VICTORY!", container);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet(R"(
        QLabel {
            font-family: 'Jersey 15';
            font-size: 60px;
            color: #f3d05a; 
            font-weight: bold;
            background: transparent;
        }
    )");

    // Message
    m_messageLabel = new QLabel("You have beaten The Game!", container);
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setStyleSheet(R"(
        QLabel {
            font-family: 'Jersey 15';
            font-size: 30px;
            color: #ffffff;
            background: transparent;
        }
    )");

    // Back Button
    m_backButton = new QPushButton("BACK TO MENU", container);
    m_backButton->setCursor(Qt::PointingHandCursor);
    m_backButton->setFixedHeight(60);
    m_backButton->setStyleSheet(R"(
        QPushButton {
            border-image: url(Resources/Button.png);
            font-family: 'Jersey 15';
            font-size: 28px;
            color: white;
            font-weight: bold;
            padding-bottom: 5px;
        }
        QPushButton:pressed {
            border-image: url(Resources/Button_Pressed.png);
            padding-top: 5px;
        }
    )");

    connect(m_backButton, &QPushButton::clicked, this, [this]() {
        emit backToMenuRequested();
        accept();
    });

    containerLayout->addWidget(m_titleLabel);
    containerLayout->addWidget(m_messageLabel);
    containerLayout->addStretch();
    containerLayout->addWidget(m_backButton);

    mainLayout->addWidget(container);
}
