/********************************************************************************
** Form generated from reading UI file 'MainWindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindowClass
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_2;
    QLabel *titleLabel;
    QVBoxLayout *verticalLayout;
    QPushButton *newGameButton;
    QPushButton *settingsButton;
    QPushButton *exitGameButton;
    QSpacerItem *verticalSpacerBottom;
    QHBoxLayout *horizontalLayoutBottom;
    QSpacerItem *bottomSpacer;
    QPushButton *accountButton;
    QPushButton *helpButton;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindowClass)
    {
        if (MainWindowClass->objectName().isEmpty())
            MainWindowClass->setObjectName("MainWindowClass");
        MainWindowClass->resize(1080, 720);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindowClass->sizePolicy().hasHeightForWidth());
        MainWindowClass->setSizePolicy(sizePolicy);
        MainWindowClass->setMinimumSize(QSize(800, 600));
        centralWidget = new QWidget(MainWindowClass);
        centralWidget->setObjectName("centralWidget");
        verticalLayout_2 = new QVBoxLayout(centralWidget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName("verticalLayout_2");
        titleLabel = new QLabel(centralWidget);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setEnabled(true);
        sizePolicy.setHeightForWidth(titleLabel->sizePolicy().hasHeightForWidth());
        titleLabel->setSizePolicy(sizePolicy);
        titleLabel->setMinimumSize(QSize(400, 400));
        titleLabel->setScaledContents(false);
        titleLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

        verticalLayout_2->addWidget(titleLabel);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(3);
        verticalLayout->setObjectName("verticalLayout");
        newGameButton = new QPushButton(centralWidget);
        newGameButton->setObjectName("newGameButton");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::MinimumExpanding, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(newGameButton->sizePolicy().hasHeightForWidth());
        newGameButton->setSizePolicy(sizePolicy1);
        newGameButton->setMinimumSize(QSize(150, 75));

        verticalLayout->addWidget(newGameButton, 0, Qt::AlignmentFlag::AlignHCenter);

        settingsButton = new QPushButton(centralWidget);
        settingsButton->setObjectName("settingsButton");
        sizePolicy1.setHeightForWidth(settingsButton->sizePolicy().hasHeightForWidth());
        settingsButton->setSizePolicy(sizePolicy1);
        settingsButton->setMinimumSize(QSize(150, 75));

        verticalLayout->addWidget(settingsButton, 0, Qt::AlignmentFlag::AlignHCenter);

        exitGameButton = new QPushButton(centralWidget);
        exitGameButton->setObjectName("exitGameButton");
        sizePolicy1.setHeightForWidth(exitGameButton->sizePolicy().hasHeightForWidth());
        exitGameButton->setSizePolicy(sizePolicy1);
        exitGameButton->setMinimumSize(QSize(150, 75));

        verticalLayout->addWidget(exitGameButton, 0, Qt::AlignmentFlag::AlignHCenter);


        verticalLayout_2->addLayout(verticalLayout);

        verticalSpacerBottom = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_2->addItem(verticalSpacerBottom);

        horizontalLayoutBottom = new QHBoxLayout();
        horizontalLayoutBottom->setSpacing(6);
        horizontalLayoutBottom->setObjectName("horizontalLayoutBottom");
        bottomSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayoutBottom->addItem(bottomSpacer);

        accountButton = new QPushButton(centralWidget);
        accountButton->setObjectName("accountButton");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(accountButton->sizePolicy().hasHeightForWidth());
        accountButton->setSizePolicy(sizePolicy2);
        accountButton->setMinimumSize(QSize(50, 50));
        accountButton->setMaximumSize(QSize(50, 50));

        horizontalLayoutBottom->addWidget(accountButton);

        helpButton = new QPushButton(centralWidget);
        helpButton->setObjectName("helpButton");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(helpButton->sizePolicy().hasHeightForWidth());
        helpButton->setSizePolicy(sizePolicy3);
        helpButton->setMinimumSize(QSize(50, 50));

        horizontalLayoutBottom->addWidget(helpButton);


        verticalLayout_2->addLayout(horizontalLayoutBottom);

        MainWindowClass->setCentralWidget(centralWidget);
        mainToolBar = new QToolBar(MainWindowClass);
        mainToolBar->setObjectName("mainToolBar");
        MainWindowClass->addToolBar(Qt::ToolBarArea::LeftToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindowClass);
        statusBar->setObjectName("statusBar");
        MainWindowClass->setStatusBar(statusBar);

        retranslateUi(MainWindowClass);

        QMetaObject::connectSlotsByName(MainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowClass)
    {
        MainWindowClass->setWindowTitle(QCoreApplication::translate("MainWindowClass", "MainWindow", nullptr));
        titleLabel->setText(QString());
        newGameButton->setText(QString());
        settingsButton->setText(QCoreApplication::translate("MainWindowClass", "SETTINGS", nullptr));
        exitGameButton->setText(QString());
        accountButton->setText(QString());
        helpButton->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindowClass: public Ui_MainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
