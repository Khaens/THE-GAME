#include "CreateLobbyDialog.h"
#include <QRandomGenerator>

CreateLobbyDialog::CreateLobbyDialog(int userId, QWidget* parent)
    : QDialog(parent)
    , m_contentContainer(nullptr)
    , m_nameInput(nullptr)
    , m_playersSpinBox(nullptr)
    , m_maxPlayers(4)
    , m_userId(userId)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    setupUI();
    setupStyle();
}

void CreateLobbyDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Content container
    m_contentContainer = new QWidget(this);
    m_contentContainer->setObjectName("createLobbyContainer");
    m_contentContainer->setFixedSize(600, 400);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(20);
    containerLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    QLabel* titleLabel = new QLabel("CREATE LOBBY", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #f3d05a;");
    containerLayout->addWidget(titleLabel);

    // Input fields
    QGridLayout* inputLayout = new QGridLayout();
    inputLayout->setSpacing(15);

    // Lobby Name
    QLabel* nameLabel = new QLabel("Lobby Name:", m_contentContainer);
    nameLabel->setStyleSheet("font-size: 14px; color: #ffffff; font-weight: bold;");
    m_nameInput = new QLineEdit(m_contentContainer);
    m_nameInput->setText(QString("Lobby_") + QString::number(m_userId));
    m_nameInput->setStyleSheet(R"(
        QLineEdit {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 10px;
            font-size: 14px;
            color: #2C3E50;
        }
        QLineEdit:focus {
            border: 2px solid #f3d05a;
        }
    )");
    inputLayout->addWidget(nameLabel, 0, 0);
    inputLayout->addWidget(m_nameInput, 0, 1);

    // Max Players
    QLabel* playersLabel = new QLabel("Max Players (2-5):", m_contentContainer);
    playersLabel->setStyleSheet("font-size: 14px; color: #ffffff; font-weight: bold;");
    m_playersSpinBox = new QSpinBox(m_contentContainer);
    m_playersSpinBox->setRange(2, 5);
    m_playersSpinBox->setValue(4);
    m_playersSpinBox->setStyleSheet(R"(
        QSpinBox {
            background-color: #deaf11;
            border: 2px solid #654b1f;
            border-radius: 8px;
            padding: 10px;
            padding-right: 30px;
            font-size: 14px;
            color: #2C3E50;
        }
        QSpinBox::up-button {
            subcontrol-origin: border;
            subcontrol-position: right top;
            width: 25px;
            height: 20px;
            background-color: #654b1f;
            border-top-right-radius: 6px;
            border-left: 2px solid #8e273b;
        }
        QSpinBox::up-button:hover {
            background-color: #4a3f1e;
        }
        QSpinBox::up-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-bottom: 6px solid #f3d05a;
            width: 0;
            height: 0;
        }
        QSpinBox::down-button {
            subcontrol-origin: border;
            subcontrol-position: right bottom;
            width: 25px;
            height: 20px;
            background-color: #654b1f;
            border-bottom-right-radius: 6px;
            border-left: 2px solid #8e273b;
        }
        QSpinBox::down-button:hover {
            background-color: #4a3f1e;
        }
        QSpinBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 6px solid #f3d05a;
            width: 0;
            height: 0;
        }
    )");
    inputLayout->addWidget(playersLabel, 1, 0);
    inputLayout->addWidget(m_playersSpinBox, 1, 1);

    containerLayout->addLayout(inputLayout);

    // Info label about code generation
    QLabel* infoLabel = new QLabel("A unique code will be generated for this lobby", m_contentContainer);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("font-size: 12px; color: #f3d05a; font-style: italic;");
    containerLayout->addWidget(infoLabel);

    containerLayout->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    QPushButton* createButton = new QPushButton("CREATE", m_contentContainer);
    createButton->setFixedSize(150, 45);
    createButton->setCursor(Qt::PointingHandCursor);
    createButton->setStyleSheet(R"(
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
    connect(createButton, &QPushButton::clicked, this, &CreateLobbyDialog::onAccept);

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
    connect(cancelButton, &QPushButton::clicked, this, &CreateLobbyDialog::onCancel);

    buttonLayout->addStretch();
    buttonLayout->addWidget(createButton);
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

void CreateLobbyDialog::setupStyle()
{
    m_contentContainer->setStyleSheet(R"(
        #createLobbyContainer {
            background-color: transparent;
            border-image: url(Resources/TextBox_1-2.png);
        }
    )"
    );
}

void CreateLobbyDialog::onAccept()
{
    m_lobbyName = m_nameInput->text().trimmed();
    m_maxPlayers = m_playersSpinBox->value();

    if (m_lobbyName.isEmpty()) {
        return;
    }

    // Always generate a password since all lobbies require codes
    m_generatedPassword = generateRandomPassword();

    accept();
}

void CreateLobbyDialog::onCancel()
{
    reject();
}

QString CreateLobbyDialog::generateRandomPassword()
{
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString password;

    for (int i = 0; i < 6; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        password.append(chars[index]);
    }

    return password;
}