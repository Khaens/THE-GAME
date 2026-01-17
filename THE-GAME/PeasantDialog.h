#pragma once
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>

class PeasantDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PeasantDialog(QWidget* parent = nullptr);
    ~PeasantDialog() = default;
};
