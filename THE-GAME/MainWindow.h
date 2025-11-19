#pragma once

#include <QMainWindow>
#include <QPixmap>
#include <QStackedWidget>
#include "ui_MainWindow.h"
#include "Game.h"

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
    void onBackToMenuClicked(); 
    void toggleFullScreen();

private:
    Ui::MainWindowClass* ui;
    void setupMenuStyle();

    QWidget* m_helpWidget;

    QWidget* createHelpWidget();
    QString getGameRules();

    // Cache pentru pixmap
    QPixmap m_titlePixmap;
};