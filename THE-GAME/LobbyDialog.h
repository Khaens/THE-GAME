#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QFrame>
#include <QVector>
#include <QSpinBox>
#include <QMap> 

#include "NetworkManager.h" 

struct LobbyInfo {
    std::string lobby_id;
    int current_players;
    int max_players;
    bool is_private;
    bool game_started;
};

class LobbyDialog : public QWidget
{
    Q_OBJECT

public:
    explicit LobbyDialog(QWidget* parent = nullptr);
    ~LobbyDialog() = default;

    void showOverlay();
    void hideOverlay();
    void setUserId(int userId) { m_userId = userId; }

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void setupStyle();

    void onCreateLobbyClicked();
    void onJoinLobbyClicked(); // Noul slot

    QString generateRandomPassword();

    QWidget* m_contentContainer;
   
    QPushButton* m_createLobbyButton;
    QPushButton* m_joinLobbyButton;

    int m_userId;
    NetworkManager* m_networkManager;
    QMap<QString, QString> m_lobbyPasswords;
};