#pragma once

#include <QMainWindow>
#include <QPixmap>
#include "ui_MainWindow.h"
#include "Game.h"
#include "HelpDialog.h"
#include "SettingsDialog.h"

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
    void toggleFullScreen();

private:
    Ui::MainWindowClass* ui;
    void setupMenuStyle();

    // Overlay dialogs
    HelpDialog* m_helpDialog;
    SettingsDialog* m_settingsDialog;

    // Cache pentru pixmap
    QPixmap m_titlePixmap;
};