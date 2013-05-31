/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Fri May 31 11:47:54 2013
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSpinBox>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGroupBox *clientsGroupBox;
    QScrollArea *clientsScrollArea;
    QWidget *scrollAreaWidgetContents;
    QFrame *clientFrame;
    QLabel *label;
    QSpinBox *clientCountSpinBox;
    QPushButton *removeClientButton;
    QPushButton *addClientButton;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(784, 527);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        clientsGroupBox = new QGroupBox(centralWidget);
        clientsGroupBox->setObjectName(QString::fromUtf8("clientsGroupBox"));
        clientsGroupBox->setGeometry(QRect(10, 10, 611, 361));
        clientsScrollArea = new QScrollArea(clientsGroupBox);
        clientsScrollArea->setObjectName(QString::fromUtf8("clientsScrollArea"));
        clientsScrollArea->setGeometry(QRect(0, 20, 591, 261));
        clientsScrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 589, 259));
        clientsScrollArea->setWidget(scrollAreaWidgetContents);
        clientFrame = new QFrame(clientsGroupBox);
        clientFrame->setObjectName(QString::fromUtf8("clientFrame"));
        clientFrame->setGeometry(QRect(10, 300, 581, 51));
        clientFrame->setFrameShape(QFrame::StyledPanel);
        clientFrame->setFrameShadow(QFrame::Raised);
        label = new QLabel(clientFrame);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 20, 221, 17));
        clientCountSpinBox = new QSpinBox(clientFrame);
        clientCountSpinBox->setObjectName(QString::fromUtf8("clientCountSpinBox"));
        clientCountSpinBox->setGeometry(QRect(220, 10, 59, 27));
        removeClientButton = new QPushButton(clientFrame);
        removeClientButton->setObjectName(QString::fromUtf8("removeClientButton"));
        removeClientButton->setGeometry(QRect(450, 10, 121, 27));
        addClientButton = new QPushButton(clientFrame);
        addClientButton->setObjectName(QString::fromUtf8("addClientButton"));
        addClientButton->setGeometry(QRect(310, 10, 131, 27));
        clientsScrollArea->raise();
        clientFrame->raise();
        clientFrame->raise();
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 784, 25));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        clientsGroupBox->setTitle(QApplication::translate("MainWindow", "Client definitions", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Number of client types to add:", 0, QApplication::UnicodeUTF8));
        removeClientButton->setText(QApplication::translate("MainWindow", "Remove clients", 0, QApplication::UnicodeUTF8));
        addClientButton->setText(QApplication::translate("MainWindow", "Add new client(s)", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
