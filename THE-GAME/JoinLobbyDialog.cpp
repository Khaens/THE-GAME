#include "JoinLobbyDialog.h"

JoinLobbyDialog::JoinLobbyDialog(QWidget* parent)
    : QDialog(parent)
    , m_contentContainer(nullptr)
    , m_codeInput(nullptr)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    setupUI();
    setupStyle();
}

void JoinLobbyDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Content container
    m_contentContainer = new QWidget(this);
    m_contentContainer->setObjectName("joinLobbyContainer");
    m_contentContainer->setFixedSize(450, 280);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(20);
    containerLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    QLabel* titleLabel = new QLabel("JOIN LOBBY", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #f3d05a;");
    containerLayout->addWidget(titleLabel);

    // Instruction
    QLabel* instructionLabel = new QLabel("Enter the lobby code:", m_contentContainer);
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setStyleSheet("font-size: 14px; color: #ffffff;");
    containerLayout->addWidget(instructionLabel);

    // Code Input
    m_codeInput = new QLineEdit(m_contentContainer);
    m_codeInput->setPlaceholderText("XXXX");
    m_codeInput->setAlignment(Qt::AlignCenter);
    m_codeInput->setMaxLength(4);
    m_codeInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 15px;
            font-size: 20px;
            font-weight: bold;
            letter-spacing: 5px;
            color: #2C3E50;
        }
        QLineEdit:focus {
            border: 2px solid #f3d05a;
        }
    )");
    containerLayout->addWidget(m_codeInput);

    containerLayout->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    QPushButton* joinButton = new QPushButton("JOIN", m_contentContainer);
    joinButton->setFixedSize(150, 45);
    joinButton->setCursor(Qt::PointingHandCursor);
    joinButton->setStyleSheet(R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #e3b310;
        }
        QPushButton:pressed {
            background-color: #869e22;
        }
    )");
    connect(joinButton, &QPushButton::clicked, this, &JoinLobbyDialog::onAccept);

    QPushButton* cancelButton = new QPushButton("CANCEL", m_contentContainer);
    cancelButton->setFixedSize(150, 45);
    cancelButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setStyleSheet(R"(
        QPushButton {
            background-color: #654b1f;
            color: #f3d05a;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #4a3f1e;
        }
        QPushButton:pressed {
            background-color: #3d431a;
        }
    )");
    connect(cancelButton, &QPushButton::clicked, this, &JoinLobbyDialog::onCancel);

    buttonLayout->addStretch();
    buttonLayout->addWidget(joinButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    containerLayout->addLayout(buttonLayout);

    // Center the container
    QVBoxLayout* centerLayout = new QVBoxLayout();
    QHBoxLayout* horizontalCenter = new QHBoxLayout();
    horizontalCenter->addStretch();
    horizontalCenter->addWidget(m_contentContainer);
    horizontalCenter->addStretch();

    centerLayout->addStretch();
    centerLayout->addLayout(horizontalCenter);
    centerLayout->addStretch();

    mainLayout->addLayout(centerLayout);
}

void JoinLobbyDialog::setupStyle()
{
    m_contentContainer->setStyleSheet(R"(
        #joinLobbyContainer {
            background-color: transparent;
            border-image: url(Resources/TextBox_1-2_Small.png);
        }
    )"
    );
}

void JoinLobbyDialog::onAccept()
{
    m_lobbyCode = m_codeInput->text().toUpper().trimmed();

    if (m_lobbyCode.isEmpty()) {
        m_codeInput->setStyleSheet(R"(
            QLineEdit {
                background-color: #deaf11;
                border: 2px solid #c0392b;
                border-radius: 8px;
                padding: 15px;
                font-size: 20px;
                font-weight: bold;
                letter-spacing: 5px;
                color: #2C3E50;
            }
        )");
        return;
    }

    accept();
}

void JoinLobbyDialog::onCancel()
{
    reject();
}