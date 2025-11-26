#include "LobbyDialog.h"
#include <QResizeEvent>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QRandomGenerator>
#include <QMap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QCheckBox>

// ATENȚIE: Presupunem că NetworkManager.h a fost actualizat pentru a include
// structura LobbyResponse și funcția joinLobbyByCode.
// Pentru a rezolva eroarea de compilare, vom folosi 'auto' și vom accesa un membru.

LobbyDialog::LobbyDialog(QWidget* parent)
    : QWidget(parent)
    , m_contentContainer(nullptr)
    , m_createLobbyButton(nullptr)
    , m_joinLobbyButton(nullptr)
    , m_userId(-1)
    , m_networkManager(nullptr)
{
    setWindowFlags(Qt::Widget);
    setAttribute(Qt::WA_TranslucentBackground);

    m_networkManager = new NetworkManager("http://localhost:18080");

    setupUI();
    setupStyle();
    hide();
}

void LobbyDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_contentContainer = new QWidget(this);
    m_contentContainer->setFixedSize(700, 300);

    QVBoxLayout* containerLayout = new QVBoxLayout(m_contentContainer);
    containerLayout->setSpacing(15);
    containerLayout->setContentsMargins(30, 30, 30, 30);

    // Title
    QLabel* titleLabel = new QLabel("GAME LOBBIES", m_contentContainer);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #f3d05a; margin-bottom: 10px;");
    containerLayout->addWidget(titleLabel);

    // Layout pentru butoane (vertical, unul sub altul)
    QVBoxLayout* buttonsLayout = new QVBoxLayout();
    buttonsLayout->setSpacing(15);

    // 1. CREATE LOBBY Button
    m_createLobbyButton = new QPushButton("CREATE LOBBY", m_contentContainer);
    m_createLobbyButton->setFixedSize(300, 55);
    m_createLobbyButton->setCursor(Qt::PointingHandCursor);

    // 2. JOIN LOBBY Button
    m_joinLobbyButton = new QPushButton("JOIN LOBBY", m_contentContainer);
    m_joinLobbyButton->setFixedSize(300, 55);
    m_joinLobbyButton->setCursor(Qt::PointingHandCursor);

    buttonsLayout->addWidget(m_createLobbyButton, 0, Qt::AlignCenter);
    buttonsLayout->addWidget(m_joinLobbyButton, 0, Qt::AlignCenter);

    containerLayout->addLayout(buttonsLayout);

    // Buton Inapoi
    QPushButton* backButton = new QPushButton("BACK", m_contentContainer);
    backButton->setObjectName("BACK");
    backButton->setFixedSize(150, 40);
    backButton->setCursor(Qt::PointingHandCursor);
    connect(backButton, &QPushButton::clicked, this, &LobbyDialog::hideOverlay);

    QHBoxLayout* backButtonLayout = new QHBoxLayout();
    backButtonLayout->addStretch();
    backButtonLayout->addWidget(backButton);
    backButtonLayout->addStretch();
    containerLayout->addLayout(backButtonLayout);

    // Centrare
    QHBoxLayout* centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(m_contentContainer);
    centerLayout->addStretch();

    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();

    // Conectam sloturile
    connect(m_createLobbyButton, &QPushButton::clicked, this, &LobbyDialog::onCreateLobbyClicked);
    connect(m_joinLobbyButton, &QPushButton::clicked, this, &LobbyDialog::onJoinLobbyClicked);
}

void LobbyDialog::setupStyle()
{
    // Stiluri generale pentru overlay
    setStyleSheet("background-color: rgba(0, 0, 0, 150);");

    m_contentContainer->setStyleSheet(
        "background-color: #8e273b; "
        "border: 3px solid #f3d05a; "
        "border-radius: 15px;"
    );

    // Stiluri pentru butoanele de lobby
    QString lobbyButtonStyle = R"(
        QPushButton {
            background-color: #f3d05a;
            color: #2C3E50;
            border: none;
            border-radius: 10px;
            font-size: 18px;
            font-weight: bold;
            padding: 15px;
        }
        QPushButton:hover {
            background-color: #e3b310;
        }
        QPushButton:pressed {
            background-color: #869e22;
        }
    )";
    m_createLobbyButton->setStyleSheet(lobbyButtonStyle);
    m_joinLobbyButton->setStyleSheet(lobbyButtonStyle);

    // Stil pentru butonul "BACK"
    m_contentContainer->findChild<QPushButton*>("BACK")->setStyleSheet(R"(
        QPushButton {
            background-color: #654b1f;
            color: #f3d05a;
            border: none;
            border-radius: 8px;
            font-size: 14px;
            padding: 8px 15px;
        }
        QPushButton:hover {
            background-color: #4a3f1e;
        }
    )");
}

void LobbyDialog::showOverlay()
{
    if (parentWidget()) {
        setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    }
    show();
}

void LobbyDialog::hideOverlay()
{
    hide();
}

void LobbyDialog::onCreateLobbyClicked()
{
    if (m_userId == -1) {
        QMessageBox::warning(this, "Authentication Required", "Please log in to create a lobby.");
        return;
    }

    // 1. Creare dialog customizat
    QDialog dialog(this);
    dialog.setWindowTitle("Create New Lobby");
    dialog.setStyleSheet("background-color: #8e273b; color: #f3d05a;");

    // Layout principal
    QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
    QGridLayout* inputLayout = new QGridLayout(); // Folosim GridLayout pentru aliniere mai bună

    int row = 0;

    // Nume Lobby
    QLabel* nameLabel = new QLabel("Nume Lobby:", &dialog);
    QLineEdit* nameEdit = new QLineEdit(&dialog);
    nameEdit->setText(QString("Lobby_") + QString::number(m_userId));
    inputLayout->addWidget(nameLabel, row, 0);
    inputLayout->addWidget(nameEdit, row++, 1);

    // Număr Jucători (SpinBox)
    QLabel* playersLabel = new QLabel("Max Jucători (2-5):", &dialog);
    QSpinBox* playersSpinBox = new QSpinBox(&dialog);
    playersSpinBox->setRange(2, 5); // Limita între 2 și 5 jucători
    playersSpinBox->setValue(4);    // Valoare implicită 4
    inputLayout->addWidget(playersLabel, row, 0);
    inputLayout->addWidget(playersSpinBox, row++, 1);

    // Checkbox pentru private
    QCheckBox* privateCheckBox = new QCheckBox("Lobby Privat (Cod necesar)", &dialog);
    inputLayout->addWidget(privateCheckBox, row++, 0, 1, 2); // Ocupă 2 coloane

    dialogLayout->addLayout(inputLayout);

    // Butoane OK si Cancel
    QPushButton* okButton = new QPushButton("Creează", &dialog);
    QPushButton* cancelButton = new QPushButton("Anulează", &dialog);
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    dialogLayout->addLayout(buttonLayout);

    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    // 2. Logica de procesare la ACCEPT
    if (dialog.exec() == QDialog::Accepted) {
        QString lobbyName = nameEdit->text().trimmed();
        bool isPrivate = privateCheckBox->isChecked();
        int maxPlayers = playersSpinBox->value(); // Extragem valoarea SpinBox-ului

        if (lobbyName.isEmpty()) {
            QMessageBox::warning(this, "Input Invalid", "Numele lobby-ului nu poate fi gol.");
            return;
        }

        // Generăm parola DOAR pe client și o trimitem la server.
        QString generatedPassword = isPrivate ? generateRandomPassword() : "";

        // Apelul la NetworkManager
        // ATENTIE: NetworkManager.h și NetworkManager.cpp trebuie actualizate 
        // pentru a include și maxPlayers!
        LobbyResponse lobbyResponse = m_networkManager->createLobby(
            m_userId,
            lobbyName.toStdString(),
            maxPlayers, // <--- NOU: Max Players
            isPrivate,
            generatedPassword.toStdString() // Trimitem parola
        );

        // 3. Afișarea mesajului de succes
        if (lobbyResponse.success) {

            // Închidem overlay-ul principal al LobbyDialog-ului
            hideOverlay();

            QString successMessage = "Lobby-ul **'" + lobbyName + "'** a fost creat cu succes!\n";
            successMessage += "Număr maxim de jucători: **" + QString::number(maxPlayers) + "**";

            if (isPrivate) {
                // Afișează codul/parola generată în MESAJUL SEPARAT
                successMessage += "\n\n**Cod Lobby:** **" + generatedPassword + "**\n"
                    "Folosește acest cod pentru a intra în lobby.";
            }

            QMessageBox::information(this, "Succes", successMessage);
        }
        else {
            QString errorMessage = QString::fromStdString(
                // Asigură-te că LobbyResponse are un membru 'error_message'
                lobbyResponse.error_message.empty() ? "Eroare necunoscută la crearea lobby-ului." : lobbyResponse.error_message
            );
            QMessageBox::warning(this, "Eroare", errorMessage);
        }
    }
}

// --- FIX pentru logica JOIN LOBBY (single input) și eroarea NetworkManager ---
void LobbyDialog::onJoinLobbyClicked()
{
    if (m_userId == -1) {
        QMessageBox::warning(this, "Authentication Required", "Please log in to join a lobby.");
        return;
    }

    bool ok;
    // POPUP NOU: Cere DOAR codul/parola
    QString lobbyCodeQ = QInputDialog::getText(this, "Join Lobby",
        "Enter the Lobby Code / Password:", QLineEdit::Normal, "", &ok);

    // Formatare ca în Among Us: litere mari, fără spații
    QString lobbyCode = lobbyCodeQ.toUpper().trimmed();

    if (!ok || lobbyCode.isEmpty()) {
        return; // Anulat sau camp gol
    }

    std::string code_to_join = lobbyCode.toStdString();

    // Înlocuim apelul inexistent `joinLobbyWithPassword` cu `joinLobbyByCode`.
    // Acest cod (parola) este folosit pentru a identifica și a intra în lobby.
    // ATENTIE: NetworkManager.h trebuie sa includa o functie 'bool joinLobbyByCode(int userId, const std::string& code)'
    bool success = m_networkManager->joinLobby(m_userId, code_to_join);

    if (success) {
        QMessageBox::information(this, "Success",
            "Successfully joined the lobby using code: " + lobbyCode);
        hideOverlay();
    }
    else {
        QMessageBox::warning(this, "Error",
            "Failed to join lobby. Invalid code, full, or game already started.");
    }
}

QString LobbyDialog::generateRandomPassword()
{
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString password;

    for (int i = 0; i < 6; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        password.append(chars[index]);
    }

    return password;
}

void LobbyDialog::resizeEvent(QResizeEvent* event)
{
    if (parentWidget() && isVisible()) {
        setGeometry(0, 0, parentWidget()->width(), parentWidget()->height());
    }
    QWidget::resizeEvent(event);
}