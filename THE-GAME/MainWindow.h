#pragma once

#include <QMainWindow>
#include <QPixmap>
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
    // handle widget resizing to rescale pixmap / adapt views
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onNewGameClicked();
    void onExitClicked();
    void onHelpClicked();

    // toggle fullscreen (F11)
    void toggleFullScreen();

private:
    Ui::MainWindowClass* ui;
    void setupMenuStyle();

    // cache original title pixmap so we can scale it on resize
    QPixmap m_titlePixmap;
};