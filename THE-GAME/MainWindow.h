#pragma once

#include <QMainWindow>
#include <QPixmap>
#include "ui_MainWindow.h"
#include "HelpDialog.h"
#include "SettingsDialog.h"
#include "AccountDialog.h"
#include "LobbyDialog.h"

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

private:
    Ui::MainWindowClass* ui;
    void setupMenuStyle();

    // Overlay dialogs
    HelpDialog* m_helpDialog;
    SettingsDialog* m_settingsDialog;
    AccountDialog* m_accountDialog;
    LobbyDialog* m_lobbyDialog;

    // Cache pentru pixmap
    QPixmap m_titlePixmap;
};