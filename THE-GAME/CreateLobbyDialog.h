#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QCheckBox>

class CreateLobbyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateLobbyDialog(int userId, QWidget* parent = nullptr);
    ~CreateLobbyDialog() = default;

    QString getLobbyName() const { return m_lobbyName; }
    int getMaxPlayers() const { return m_maxPlayers; }
    bool wasLobbyCreated() const { return m_lobbyCreated; }

private:
    void setupUI();
    void setupStyle();
    void onAccept();
    void onCancel();

    QWidget* m_contentContainer;
    QLineEdit* m_nameInput;
    QSpinBox* m_playersSpinBox;
    QPushButton* m_createButton;

    QString m_lobbyName;
    int m_maxPlayers;
    int m_userId;
    bool m_lobbyCreated;
};