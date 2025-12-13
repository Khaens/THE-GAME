#pragma once

#include "ui_GameWindow.h"

class GameWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget* parent = nullptr);
    ~GameWindow() = default;

    void showOverlay();
    void hideOverlay();

signals:
    void backToMenuRequested();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void onBackButtonClicked();

private:
    Ui::Form* ui;
	void loadGameImage();

    QPixmap m_backgroundPixmap;
    QPixmap m_scaledBackground;
};  