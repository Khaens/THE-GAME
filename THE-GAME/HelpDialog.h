#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>

class HelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HelpDialog(QWidget* parent = nullptr);
    ~HelpDialog() = default;

private:
    void setupUI();
    void setupStyle();
    QString getGameRules();
};