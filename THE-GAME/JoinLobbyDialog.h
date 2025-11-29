#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class JoinLobbyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JoinLobbyDialog(QWidget* parent = nullptr);
    ~JoinLobbyDialog() = default;

    QString getLobbyCode() const { return m_lobbyCode; }

private:
    void setupUI();
    void setupStyle();
    void onAccept();
    void onCancel();

    QWidget* m_contentContainer;
    QLineEdit* m_codeInput;
    QString m_lobbyCode;
};