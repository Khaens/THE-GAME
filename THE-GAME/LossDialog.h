#pragma once

#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class LossDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LossDialog(QWidget *parent = nullptr);

signals:
    void backToMenuRequested();

private:
    void setupUi();
    QPushButton* m_backButton;
    QLabel* m_titleLabel;
    QLabel* m_messageLabel;
};
