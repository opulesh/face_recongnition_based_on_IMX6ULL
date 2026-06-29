/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.12.9
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QLabel *videoLabel;
    QGroupBox *controlGroup;
    QPushButton *startBtn;
    QPushButton *captureBtn;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        videoLabel = new QLabel(centralwidget);
        videoLabel->setObjectName(QString::fromUtf8("videoLabel"));
        videoLabel->setGeometry(QRect(200, 280, 341, 151));
        controlGroup = new QGroupBox(centralwidget);
        controlGroup->setObjectName(QString::fromUtf8("controlGroup"));
        controlGroup->setGeometry(QRect(200, 100, 341, 151));
        startBtn = new QPushButton(controlGroup);
        startBtn->setObjectName(QString::fromUtf8("startBtn"));
        startBtn->setGeometry(QRect(30, 60, 89, 25));
        captureBtn = new QPushButton(controlGroup);
        captureBtn->setObjectName(QString::fromUtf8("captureBtn"));
        captureBtn->setGeometry(QRect(180, 60, 89, 25));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 28));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", nullptr));
        videoLabel->setText(QApplication::translate("MainWindow", "TextLabel", nullptr));
        controlGroup->setTitle(QApplication::translate("MainWindow", "\346\216\247\345\210\266\351\235\242\346\235\277", nullptr));
        startBtn->setText(QApplication::translate("MainWindow", "\345\274\200\345\247\213", nullptr));
        captureBtn->setText(QApplication::translate("MainWindow", "\346\213\215\347\205\247", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
