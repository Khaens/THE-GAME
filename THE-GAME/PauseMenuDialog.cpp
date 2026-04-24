#include "PauseMenuDialog.h"
#include "SoundManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QResizeEvent>

PauseMenuDialog::PauseMenuDialog(QWidget* parent)
    : QWidget(parent)
{
    hide();
    setupUI();
    setupStyle();
}

void PauseMenuDialog::showOverlay()
{
    QWidget* parent = parentWidget();
    if (parent) {
        setGeometry(0, 0, parent->width(), parent->height());
    }
    raise();
    show();
}

void PauseMenuDialog::hideOverlay()
{
    hide();
}

void PauseMenuDialog::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_contentContainer) {
        m_contentContainer->move((width() - m_contentContainer->width()) / 2,
                                 (height() - m_contentContainer->height()) / 2);
    }
}

void PauseMenuDialog::setupUI()
{
    // Semi-transparent background for the whole overlay
    setStyleSheet("background-color: rgba(0, 0, 0, 150);");

    m_contentContainer = new QWidget(this);
    m_contentContainer->setObjectName("pauseContainer");
    m_contentContainer->setFixedSize(400, 500);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setContentsMargins(40, 50, 40, 50);
    containerLayout->setSpacing(20);

    // Title
    QLabel* titleLabel = new QLabel("PAUSED", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet(R"(
        QLabel {
            font-family: 'Knight Warrior';
            font-size: 50px;
            color: #f3d05a; 
            font-weight: bold;
            background: transparent;
        }
    )");
    containerLayout->addWidget(titleLabel);

    containerLayout->addStretch();

    // Buttons
    auto createButton = [this](const QString& text) {
        QPushButton* btn = new QPushButton(text, m_contentContainer);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedHeight(70);
        btn->setStyleSheet(R"(
            QPushButton {
                border-image: url(Resources/Button.png);
                font-family: 'Jersey 15';
                font-size: 28px;
                color: white;
                font-weight: bold;
                padding-bottom: 5px;
                background-color: transparent;
            }
            QPushButton:pressed {
                border-image: url(Resources/Button_Pressed.png);
                padding-top: 5px;
            }
        )");
        return btn;
    };

    QPushButton* resumeButton = createButton("RESUME");
    QPushButton* settingsButton = createButton("SETTINGS");
    QPushButton* leaveButton = createButton("LEAVE LOBBY");

    containerLayout->addWidget(resumeButton);
    containerLayout->addWidget(settingsButton);
    containerLayout->addWidget(leaveButton);

    containerLayout->addStretch();

    // Connections
    connect(resumeButton, &QPushButton::clicked, this, [this]() {
        SoundManager::instance()->play(SoundType::BigButtonClick);
        emit resumeRequested();
        hideOverlay();
    });

    connect(settingsButton, &QPushButton::clicked, this, [this]() {
        SoundManager::instance()->play(SoundType::BigButtonClick);
        emit settingsRequested();
        // Don't hide pause menu, just show settings on top
    });

    connect(leaveButton, &QPushButton::clicked, this, [this]() {
        SoundManager::instance()->play(SoundType::BigButtonClick);
        emit leaveLobbyRequested();
        hideOverlay();
    });
}

void PauseMenuDialog::setupStyle()
{
    m_contentContainer->setStyleSheet(R"(
        #pauseContainer {
            border-image: url(Resources/TextBox_2-1.png);
            background-color: transparent;
        }
    )");
}
