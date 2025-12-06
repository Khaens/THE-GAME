#pragma once

#include <QMainWindow>
#include <QPixmap>
#include <memory>
#include "ui_MainWindow.h"
#include "HelpDialog.h"
#include "SettingsDialog.h"
#include "AccountDialog.h"
#include "LobbyDialog.h"
#include "GameWindow.h"
#include "NetworkManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindowClass; };
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onNewGameClicked();
    void onExitClicked();
    void onHelpClicked();
    void onSettingsClicked();
    void onAccountClicked();
    void toggleFullScreen();
    void showGameOverlay();  // Renamed from showGameWindow

private:
    Ui::MainWindowClass* ui;
    void setupMenuStyle();

    // Overlay dialogs
    HelpDialog* m_helpDialog;
    SettingsDialog* m_settingsDialog;
    AccountDialog* m_accountDialog;
    LobbyDialog* m_lobbyDialog;
    GameWindow* m_gameWindow;  // Now an overlay

    QPixmap m_titlePixmap;

    // Network
    std::shared_ptr<NetworkManager> m_networkManager;
};