#include "CreateLobbyDialog.h"
#include <QRandomGenerator>
#include <QClipboard>
#include <QApplication>

CreateLobbyDialog::CreateLobbyDialog(int userId, QWidget* parent)
    : QDialog(parent)
    , m_contentContainer(nullptr)
    , m_nameInput(nullptr)
    , m_playersSpinBox(nullptr)
    , m_codeDisplayWidget(nullptr)
    , m_codeLabel(nullptr)
    , m_createButton(nullptr)
    , m_doneButton(nullptr)
    , m_maxPlayers(4)
    , m_userId(userId)
    , m_lobbyCreated(false)
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
    mainLayout->setContentsMargins(30, 15, 30, 15);

    // Content container
    m_contentContainer = new QWidget(this);
    m_contentContainer->setObjectName("createLobbyContainer");
    m_contentContainer->setFixedSize(700, 525);

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

    // Code display area (initially hidden)
    m_codeDisplayWidget = new QWidget(m_contentContainer);
    m_codeDisplayWidget->setObjectName("codeDisplayFrame");
    QVBoxLayout* codeLayout = new QVBoxLayout(m_codeDisplayWidget);
    codeLayout->setSpacing(5);
    codeLayout->setContentsMargins(30, 20, 30, 20);

    QLabel* successLabel = new QLabel("Lobby Created Successfully!", m_codeDisplayWidget);
    successLabel->setAlignment(Qt::AlignCenter);
    successLabel->setStyleSheet("font-size: 16px; color: #27AE60; font-weight: bold;");
    codeLayout->addWidget(successLabel);

    QLabel* codeTitle = new QLabel("Your Lobby Code:", m_codeDisplayWidget);
    codeTitle->setAlignment(Qt::AlignCenter);
    codeTitle->setStyleSheet("font-size: 14px; color: #ffffff;");
    codeLayout->addWidget(codeTitle);

    // Added spacing above the code label
    codeLayout->addSpacing(5);

    m_codeLabel = new QLabel(m_codeDisplayWidget);
    m_codeLabel->setAlignment(Qt::AlignCenter);
    m_codeLabel->setMinimumHeight(50);
    m_codeLabel->setStyleSheet(R"(
    QLabel {
        background-color: transparent;
        border: none;
        padding: 10px;
        font-size: 36px;
        font-weight: bold;
        letter-spacing: 10px;
        color: #f3d05a;
    }
    )");
    codeLayout->addWidget(m_codeLabel);

    // Added spacing between the code label and the button
    codeLayout->addSpacing(5);

    QPushButton* copyButton = new QPushButton("Copy Code", m_codeDisplayWidget);
    copyButton->setFixedHeight(40);
    copyButton->setCursor(Qt::PointingHandCursor);
    copyButton->setStyleSheet(R"(
    QPushButton {
        background-color: #deaf11;
        color: #000000;
        border: none;
        border-radius: 8px;
        font-size: 14px;
        font-weight: bold;
    }
    QPushButton:hover {
        background-color: #b38c0c;
    }
    QPushButton:pressed {
        background-color: #9e7c09;
    }
    )");
    connect(copyButton, &QPushButton::clicked, [this]() {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(m_generatedPassword);
        });
    codeLayout->addWidget(copyButton);

    QLabel* shareLabel = new QLabel("Share this code with friends to join!", m_codeDisplayWidget);
    shareLabel->setAlignment(Qt::AlignCenter);
    shareLabel->setStyleSheet("font-size: 12px; color: #ffffff; font-style: italic;");
    codeLayout->addWidget(shareLabel);

    m_codeDisplayWidget->hide(); // Initially hidden
    containerLayout->addWidget(m_codeDisplayWidget);

    // Info label about code generation (shown before creation)
    QLabel* infoLabel = new QLabel("A unique code will be generated for this lobby", m_contentContainer);
    infoLabel->setObjectName("infoLabel");
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setStyleSheet("font-size: 12px; color: #f3d05a; font-style: italic;");
    containerLayout->addWidget(infoLabel);

    containerLayout->addStretch();

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    m_createButton = new QPushButton("CREATE", m_contentContainer);
    m_createButton->setFixedSize(150, 45);
    m_createButton->setCursor(Qt::PointingHandCursor);
    m_createButton->setStyleSheet(R"(
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
    connect(m_createButton, &QPushButton::clicked, this, &CreateLobbyDialog::onAccept);

    m_doneButton = new QPushButton("DONE", m_contentContainer);
    m_doneButton->setFixedSize(150, 45);
    m_doneButton->setCursor(Qt::PointingHandCursor);
    m_doneButton->setStyleSheet(R"(
        QPushButton {
            background-color: #27AE60;
            color: #ffffff;
            border: none;
            border-radius: 10px;
            font-size: 15px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #229954;
        }
        QPushButton:pressed {
            background-color: #1e8449;
        }
    )");
    connect(m_doneButton, &QPushButton::clicked, this, &QDialog::accept);
    m_doneButton->hide(); // Initially hidden

    QPushButton* cancelButton = new QPushButton("CANCEL", m_contentContainer);
    cancelButton->setFixedSize(150, 45);
    cancelButton->setCursor(Qt::PointingHandCursor);
    cancelButton->setStyleSheet(R"(
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
    connect(cancelButton, &QPushButton::clicked, this, &CreateLobbyDialog::onCancel);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_createButton);
    buttonLayout->addWidget(m_doneButton);
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

    m_codeDisplayWidget->setStyleSheet(R"(
        #codeDisplayFrame {
            margin-top: 5px; 
            margin-bottom: 5px; 
        }
    )");
}

void CreateLobbyDialog::onAccept()
{
    m_lobbyName = m_nameInput->text().trimmed();
    m_maxPlayers = m_playersSpinBox->value();

    if (m_lobbyName.isEmpty()) {
        return;
    }

    // Generate password and show it
    m_generatedPassword = generateRandomPassword();
    m_lobbyCreated = true;

    showGeneratedCode();
}

void CreateLobbyDialog::showGeneratedCode()
{
    // Set the code text
    m_codeLabel->setText(m_generatedPassword);

    // Hide input fields and create button
    m_nameInput->setEnabled(false);
    m_playersSpinBox->setEnabled(false);
    m_createButton->hide();

    // Hide info label
    QLabel* infoLabel = m_contentContainer->findChild<QLabel*>("infoLabel");
    if (infoLabel) {
        infoLabel->hide();
    }

    // Show code display and done button
    m_codeDisplayWidget->show();
    m_doneButton->show();
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