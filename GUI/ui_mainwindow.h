/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Wed Jul 3 12:01:43 2013
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
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFormLayout>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QScrollArea>
#include <QtGui/QSpinBox>
#include <QtGui/QStatusBar>
#include <QtGui/QTextBrowser>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QScrollArea *mainScrollArea;
    QWidget *scrollAreaWidgetContents_12;
    QGridLayout *gridLayout_5;
    QGroupBox *clientsGroupBox;
    QVBoxLayout *verticalLayout;
    QScrollArea *clientScrollArea;
    QWidget *scrollAreaWidgetContents_14;
    QFrame *clientFrame;
    QHBoxLayout *horizontalLayout;
    QPushButton *addClientButton;
    QPushButton *removeClientButton;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QFrame *frame_2;
    QGridLayout *gridLayout_2;
    QLabel *label_6;
    QSpinBox *simTime;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_3;
    QFrame *frame;
    QGridLayout *gridLayout_3;
    QLabel *label_2;
    QSpinBox *appProto_ackSize;
    QLabel *label_3;
    QSpinBox *appProto_headerSize;
    QLabel *label_4;
    QSpinBox *appProto_RTO;
    QLabel *label_5;
    QSpinBox *appProto_delAck;
    QTextBrowser *textBrowser;
    QGroupBox *streamsGroupBox;
    QVBoxLayout *verticalLayout_4;
    QScrollArea *streamScrollArea;
    QWidget *scrollAreaWidgetContents_13;
    QFrame *streamFrame;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *addStreamButton;
    QPushButton *removeStreamButton;
    QGroupBox *messageEditor;
    QHBoxLayout *horizontalLayout_4;
    QFrame *message;
    QFormLayout *formLayout;
    QLabel *message_nameLabel;
    QLineEdit *message_name;
    QLabel *message_typeLabel;
    QComboBox *message_type;
    QCheckBox *message_reliable;
    QFrame *message_sizeFrame;
    QFormLayout *formLayout_2;
    QLabel *message_size;
    QPushButton *message_configMessageSize;
    QLineEdit *message_sizeDistribution;
    QFrame *message_timeIntervalFrame;
    QFormLayout *formLayout_3;
    QLabel *message_timeInterval;
    QPushButton *message_configTimeInterval;
    QLineEdit *message_timeIntervalDistribution;
    QCheckBox *message_returnToSender;
    QFrame *message_forwardMessageSizeFrame;
    QGridLayout *gridLayout_4;
    QLabel *message_forwardMessageSize;
    QPushButton *message_configForwardMessageSize;
    QLabel *label_9;
    QFrame *message_clientsOfInterestFrame;
    QFormLayout *formLayout_4;
    QLabel *message_clientsOfInterest;
    QDoubleSpinBox *message_clientsOfInterestSpinBox;
    QLabel *message_errorLabel;
    QPushButton *message_add;
    QPushButton *message_cancel;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QMenuBar *menuBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1197, 863);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        mainScrollArea = new QScrollArea(centralWidget);
        mainScrollArea->setObjectName(QString::fromUtf8("mainScrollArea"));
        mainScrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents_12 = new QWidget();
        scrollAreaWidgetContents_12->setObjectName(QString::fromUtf8("scrollAreaWidgetContents_12"));
        scrollAreaWidgetContents_12->setGeometry(QRect(0, 0, 1161, 914));
        gridLayout_5 = new QGridLayout(scrollAreaWidgetContents_12);
        gridLayout_5->setSpacing(6);
        gridLayout_5->setContentsMargins(11, 11, 11, 11);
        gridLayout_5->setObjectName(QString::fromUtf8("gridLayout_5"));
        clientsGroupBox = new QGroupBox(scrollAreaWidgetContents_12);
        clientsGroupBox->setObjectName(QString::fromUtf8("clientsGroupBox"));
        verticalLayout = new QVBoxLayout(clientsGroupBox);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        clientScrollArea = new QScrollArea(clientsGroupBox);
        clientScrollArea->setObjectName(QString::fromUtf8("clientScrollArea"));
        clientScrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents_14 = new QWidget();
        scrollAreaWidgetContents_14->setObjectName(QString::fromUtf8("scrollAreaWidgetContents_14"));
        scrollAreaWidgetContents_14->setGeometry(QRect(0, 0, 790, 264));
        clientScrollArea->setWidget(scrollAreaWidgetContents_14);

        verticalLayout->addWidget(clientScrollArea);

        clientFrame = new QFrame(clientsGroupBox);
        clientFrame->setObjectName(QString::fromUtf8("clientFrame"));
        clientFrame->setFrameShape(QFrame::StyledPanel);
        clientFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(clientFrame);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        addClientButton = new QPushButton(clientFrame);
        addClientButton->setObjectName(QString::fromUtf8("addClientButton"));

        horizontalLayout->addWidget(addClientButton);

        removeClientButton = new QPushButton(clientFrame);
        removeClientButton->setObjectName(QString::fromUtf8("removeClientButton"));

        horizontalLayout->addWidget(removeClientButton);


        verticalLayout->addWidget(clientFrame);


        gridLayout_5->addWidget(clientsGroupBox, 0, 0, 2, 2);

        groupBox_2 = new QGroupBox(scrollAreaWidgetContents_12);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_2 = new QVBoxLayout(groupBox_2);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        frame_2 = new QFrame(groupBox_2);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        gridLayout_2 = new QGridLayout(frame_2);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_6 = new QLabel(frame_2);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout_2->addWidget(label_6, 0, 0, 1, 1);

        simTime = new QSpinBox(frame_2);
        simTime->setObjectName(QString::fromUtf8("simTime"));

        gridLayout_2->addWidget(simTime, 0, 1, 1, 1);


        verticalLayout_2->addWidget(frame_2);


        gridLayout_5->addWidget(groupBox_2, 0, 2, 1, 1);

        groupBox = new QGroupBox(scrollAreaWidgetContents_12);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_3 = new QVBoxLayout(groupBox);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        frame = new QFrame(groupBox);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        gridLayout_3 = new QGridLayout(frame);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label_2 = new QLabel(frame);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_3->addWidget(label_2, 0, 0, 1, 1);

        appProto_ackSize = new QSpinBox(frame);
        appProto_ackSize->setObjectName(QString::fromUtf8("appProto_ackSize"));

        gridLayout_3->addWidget(appProto_ackSize, 0, 1, 1, 1);

        label_3 = new QLabel(frame);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout_3->addWidget(label_3, 1, 0, 1, 1);

        appProto_headerSize = new QSpinBox(frame);
        appProto_headerSize->setObjectName(QString::fromUtf8("appProto_headerSize"));

        gridLayout_3->addWidget(appProto_headerSize, 1, 1, 1, 1);

        label_4 = new QLabel(frame);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout_3->addWidget(label_4, 2, 0, 1, 1);

        appProto_RTO = new QSpinBox(frame);
        appProto_RTO->setObjectName(QString::fromUtf8("appProto_RTO"));

        gridLayout_3->addWidget(appProto_RTO, 2, 1, 1, 1);

        label_5 = new QLabel(frame);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout_3->addWidget(label_5, 3, 0, 1, 1);

        appProto_delAck = new QSpinBox(frame);
        appProto_delAck->setObjectName(QString::fromUtf8("appProto_delAck"));

        gridLayout_3->addWidget(appProto_delAck, 3, 1, 1, 1);

        textBrowser = new QTextBrowser(frame);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));

        gridLayout_3->addWidget(textBrowser, 4, 0, 1, 2);


        verticalLayout_3->addWidget(frame);


        gridLayout_5->addWidget(groupBox, 1, 2, 1, 1);

        streamsGroupBox = new QGroupBox(scrollAreaWidgetContents_12);
        streamsGroupBox->setObjectName(QString::fromUtf8("streamsGroupBox"));
        streamsGroupBox->setStyleSheet(QString::fromUtf8(""));
        verticalLayout_4 = new QVBoxLayout(streamsGroupBox);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        streamScrollArea = new QScrollArea(streamsGroupBox);
        streamScrollArea->setObjectName(QString::fromUtf8("streamScrollArea"));
        streamScrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents_13 = new QWidget();
        scrollAreaWidgetContents_13->setObjectName(QString::fromUtf8("scrollAreaWidgetContents_13"));
        scrollAreaWidgetContents_13->setGeometry(QRect(0, 0, 750, 444));
        streamScrollArea->setWidget(scrollAreaWidgetContents_13);

        verticalLayout_4->addWidget(streamScrollArea);

        streamFrame = new QFrame(streamsGroupBox);
        streamFrame->setObjectName(QString::fromUtf8("streamFrame"));
        streamFrame->setFrameShape(QFrame::StyledPanel);
        streamFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout_2 = new QHBoxLayout(streamFrame);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        addStreamButton = new QPushButton(streamFrame);
        addStreamButton->setObjectName(QString::fromUtf8("addStreamButton"));

        horizontalLayout_2->addWidget(addStreamButton);

        removeStreamButton = new QPushButton(streamFrame);
        removeStreamButton->setObjectName(QString::fromUtf8("removeStreamButton"));

        horizontalLayout_2->addWidget(removeStreamButton);


        verticalLayout_4->addWidget(streamFrame);


        gridLayout_5->addWidget(streamsGroupBox, 2, 0, 1, 1);

        messageEditor = new QGroupBox(scrollAreaWidgetContents_12);
        messageEditor->setObjectName(QString::fromUtf8("messageEditor"));
        messageEditor->setEnabled(true);
        horizontalLayout_4 = new QHBoxLayout(messageEditor);
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        message = new QFrame(messageEditor);
        message->setObjectName(QString::fromUtf8("message"));
        message->setEnabled(false);
        message->setFrameShape(QFrame::StyledPanel);
        message->setFrameShadow(QFrame::Raised);
        formLayout = new QFormLayout(message);
        formLayout->setSpacing(6);
        formLayout->setContentsMargins(11, 11, 11, 11);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        message_nameLabel = new QLabel(message);
        message_nameLabel->setObjectName(QString::fromUtf8("message_nameLabel"));
        message_nameLabel->setEnabled(false);

        formLayout->setWidget(0, QFormLayout::LabelRole, message_nameLabel);

        message_name = new QLineEdit(message);
        message_name->setObjectName(QString::fromUtf8("message_name"));
        message_name->setEnabled(false);

        formLayout->setWidget(0, QFormLayout::FieldRole, message_name);

        message_typeLabel = new QLabel(message);
        message_typeLabel->setObjectName(QString::fromUtf8("message_typeLabel"));
        message_typeLabel->setEnabled(false);

        formLayout->setWidget(2, QFormLayout::LabelRole, message_typeLabel);

        message_type = new QComboBox(message);
        message_type->setObjectName(QString::fromUtf8("message_type"));
        message_type->setEnabled(false);

        formLayout->setWidget(2, QFormLayout::FieldRole, message_type);

        message_reliable = new QCheckBox(message);
        message_reliable->setObjectName(QString::fromUtf8("message_reliable"));
        message_reliable->setEnabled(false);

        formLayout->setWidget(3, QFormLayout::FieldRole, message_reliable);

        message_sizeFrame = new QFrame(message);
        message_sizeFrame->setObjectName(QString::fromUtf8("message_sizeFrame"));
        message_sizeFrame->setFrameShape(QFrame::StyledPanel);
        message_sizeFrame->setFrameShadow(QFrame::Raised);
        formLayout_2 = new QFormLayout(message_sizeFrame);
        formLayout_2->setSpacing(6);
        formLayout_2->setContentsMargins(11, 11, 11, 11);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        formLayout_2->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        message_size = new QLabel(message_sizeFrame);
        message_size->setObjectName(QString::fromUtf8("message_size"));
        message_size->setEnabled(false);

        formLayout_2->setWidget(0, QFormLayout::LabelRole, message_size);

        message_configMessageSize = new QPushButton(message_sizeFrame);
        message_configMessageSize->setObjectName(QString::fromUtf8("message_configMessageSize"));

        formLayout_2->setWidget(0, QFormLayout::FieldRole, message_configMessageSize);

        message_sizeDistribution = new QLineEdit(message_sizeFrame);
        message_sizeDistribution->setObjectName(QString::fromUtf8("message_sizeDistribution"));

        formLayout_2->setWidget(1, QFormLayout::SpanningRole, message_sizeDistribution);


        formLayout->setWidget(4, QFormLayout::SpanningRole, message_sizeFrame);

        message_timeIntervalFrame = new QFrame(message);
        message_timeIntervalFrame->setObjectName(QString::fromUtf8("message_timeIntervalFrame"));
        message_timeIntervalFrame->setFrameShape(QFrame::StyledPanel);
        message_timeIntervalFrame->setFrameShadow(QFrame::Raised);
        formLayout_3 = new QFormLayout(message_timeIntervalFrame);
        formLayout_3->setSpacing(6);
        formLayout_3->setContentsMargins(11, 11, 11, 11);
        formLayout_3->setObjectName(QString::fromUtf8("formLayout_3"));
        formLayout_3->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        message_timeInterval = new QLabel(message_timeIntervalFrame);
        message_timeInterval->setObjectName(QString::fromUtf8("message_timeInterval"));
        message_timeInterval->setEnabled(false);

        formLayout_3->setWidget(0, QFormLayout::LabelRole, message_timeInterval);

        message_configTimeInterval = new QPushButton(message_timeIntervalFrame);
        message_configTimeInterval->setObjectName(QString::fromUtf8("message_configTimeInterval"));

        formLayout_3->setWidget(0, QFormLayout::FieldRole, message_configTimeInterval);

        message_timeIntervalDistribution = new QLineEdit(message_timeIntervalFrame);
        message_timeIntervalDistribution->setObjectName(QString::fromUtf8("message_timeIntervalDistribution"));

        formLayout_3->setWidget(1, QFormLayout::SpanningRole, message_timeIntervalDistribution);


        formLayout->setWidget(5, QFormLayout::SpanningRole, message_timeIntervalFrame);

        message_returnToSender = new QCheckBox(message);
        message_returnToSender->setObjectName(QString::fromUtf8("message_returnToSender"));
        message_returnToSender->setEnabled(false);

        formLayout->setWidget(6, QFormLayout::LabelRole, message_returnToSender);

        message_forwardMessageSizeFrame = new QFrame(message);
        message_forwardMessageSizeFrame->setObjectName(QString::fromUtf8("message_forwardMessageSizeFrame"));
        message_forwardMessageSizeFrame->setFrameShape(QFrame::StyledPanel);
        message_forwardMessageSizeFrame->setFrameShadow(QFrame::Raised);
        gridLayout_4 = new QGridLayout(message_forwardMessageSizeFrame);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        message_forwardMessageSize = new QLabel(message_forwardMessageSizeFrame);
        message_forwardMessageSize->setObjectName(QString::fromUtf8("message_forwardMessageSize"));
        message_forwardMessageSize->setEnabled(false);

        gridLayout_4->addWidget(message_forwardMessageSize, 0, 0, 1, 1);

        message_configForwardMessageSize = new QPushButton(message_forwardMessageSizeFrame);
        message_configForwardMessageSize->setObjectName(QString::fromUtf8("message_configForwardMessageSize"));

        gridLayout_4->addWidget(message_configForwardMessageSize, 0, 1, 1, 1);

        label_9 = new QLabel(message_forwardMessageSizeFrame);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout_4->addWidget(label_9, 1, 0, 1, 2);


        formLayout->setWidget(7, QFormLayout::SpanningRole, message_forwardMessageSizeFrame);

        message_clientsOfInterestFrame = new QFrame(message);
        message_clientsOfInterestFrame->setObjectName(QString::fromUtf8("message_clientsOfInterestFrame"));
        message_clientsOfInterestFrame->setEnabled(false);
        message_clientsOfInterestFrame->setFrameShape(QFrame::StyledPanel);
        message_clientsOfInterestFrame->setFrameShadow(QFrame::Raised);
        formLayout_4 = new QFormLayout(message_clientsOfInterestFrame);
        formLayout_4->setSpacing(6);
        formLayout_4->setContentsMargins(11, 11, 11, 11);
        formLayout_4->setObjectName(QString::fromUtf8("formLayout_4"));
        message_clientsOfInterest = new QLabel(message_clientsOfInterestFrame);
        message_clientsOfInterest->setObjectName(QString::fromUtf8("message_clientsOfInterest"));
        message_clientsOfInterest->setEnabled(false);

        formLayout_4->setWidget(0, QFormLayout::LabelRole, message_clientsOfInterest);

        message_clientsOfInterestSpinBox = new QDoubleSpinBox(message_clientsOfInterestFrame);
        message_clientsOfInterestSpinBox->setObjectName(QString::fromUtf8("message_clientsOfInterestSpinBox"));

        formLayout_4->setWidget(0, QFormLayout::FieldRole, message_clientsOfInterestSpinBox);


        formLayout->setWidget(8, QFormLayout::SpanningRole, message_clientsOfInterestFrame);

        message_errorLabel = new QLabel(message);
        message_errorLabel->setObjectName(QString::fromUtf8("message_errorLabel"));
        QPalette palette;
        QBrush brush(QColor(0, 0, 0, 255));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
        QBrush brush1(QColor(255, 0, 0, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Button, brush1);
        QBrush brush2(QColor(255, 127, 127, 255));
        brush2.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Light, brush2);
        QBrush brush3(QColor(255, 63, 63, 255));
        brush3.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Midlight, brush3);
        QBrush brush4(QColor(127, 0, 0, 255));
        brush4.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Dark, brush4);
        QBrush brush5(QColor(170, 0, 0, 255));
        brush5.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Active, QPalette::Text, brush);
        QBrush brush6(QColor(255, 255, 255, 255));
        brush6.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::BrightText, brush6);
        palette.setBrush(QPalette::Active, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Active, QPalette::Base, brush6);
        palette.setBrush(QPalette::Active, QPalette::Window, brush1);
        palette.setBrush(QPalette::Active, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Active, QPalette::AlternateBase, brush2);
        QBrush brush7(QColor(255, 255, 220, 255));
        brush7.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::ToolTipBase, brush7);
        palette.setBrush(QPalette::Active, QPalette::ToolTipText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Button, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Light, brush2);
        palette.setBrush(QPalette::Inactive, QPalette::Midlight, brush3);
        palette.setBrush(QPalette::Inactive, QPalette::Dark, brush4);
        palette.setBrush(QPalette::Inactive, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Inactive, QPalette::Text, brush);
        palette.setBrush(QPalette::Inactive, QPalette::BrightText, brush6);
        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush6);
        palette.setBrush(QPalette::Inactive, QPalette::Window, brush1);
        palette.setBrush(QPalette::Inactive, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush2);
        palette.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush7);
        palette.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush);
        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::Button, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Light, brush2);
        palette.setBrush(QPalette::Disabled, QPalette::Midlight, brush3);
        palette.setBrush(QPalette::Disabled, QPalette::Dark, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::Mid, brush5);
        palette.setBrush(QPalette::Disabled, QPalette::Text, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::BrightText, brush6);
        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush4);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Window, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::Shadow, brush);
        palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush1);
        palette.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush7);
        palette.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush);
        message_errorLabel->setPalette(palette);

        formLayout->setWidget(9, QFormLayout::SpanningRole, message_errorLabel);

        message_add = new QPushButton(message);
        message_add->setObjectName(QString::fromUtf8("message_add"));
        message_add->setEnabled(false);

        formLayout->setWidget(10, QFormLayout::LabelRole, message_add);

        message_cancel = new QPushButton(message);
        message_cancel->setObjectName(QString::fromUtf8("message_cancel"));

        formLayout->setWidget(10, QFormLayout::FieldRole, message_cancel);


        horizontalLayout_4->addWidget(message);


        gridLayout_5->addWidget(messageEditor, 2, 1, 1, 2);

        mainScrollArea->setWidget(scrollAreaWidgetContents_12);

        gridLayout->addWidget(mainScrollArea, 0, 0, 1, 1);

        MainWindow->setCentralWidget(centralWidget);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1197, 25));
        MainWindow->setMenuBar(menuBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        clientsGroupBox->setTitle(QApplication::translate("MainWindow", "Client definitions", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        addClientButton->setToolTip(QApplication::translate("MainWindow", "Add defined amount of new client types. Copies values from previous client type.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        addClientButton->setText(QApplication::translate("MainWindow", "Add new client type", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        removeClientButton->setToolTip(QApplication::translate("MainWindow", "Remove client types from list.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        removeClientButton->setText(QApplication::translate("MainWindow", "Remove clients", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("MainWindow", "Simulation configuration", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("MainWindow", "Simulation time (seconds):", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        simTime->setToolTip(QApplication::translate("MainWindow", "Defines the simulation execution time.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        groupBox->setTitle(QApplication::translate("MainWindow", "Application layer protocol", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Acknowledgement size (bytes):", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        appProto_ackSize->setToolTip(QApplication::translate("MainWindow", "The acknowledgement payload size", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_3->setText(QApplication::translate("MainWindow", "Header size (bytes):", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        appProto_headerSize->setToolTip(QApplication::translate("MainWindow", "Size of the application protocol header.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_4->setText(QApplication::translate("MainWindow", "Retransmission timeout (ms):", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        appProto_RTO->setToolTip(QApplication::translate("MainWindow", "The time limit when the message is retransmitted.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label_5->setText(QApplication::translate("MainWindow", "Delayed ack value (ms):", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        appProto_delAck->setToolTip(QApplication::translate("MainWindow", "Waiting time before an acknowledgement is sent.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        textBrowser->setHtml(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">The application protocol must be enabled separately for streams that use it, otherwise these configurations have no effect.</p></body></html>", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        streamsGroupBox->setToolTip(QApplication::translate("MainWindow", "Different streams can be added here.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        streamsGroupBox->setTitle(QApplication::translate("MainWindow", "Stream definitions", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        addStreamButton->setToolTip(QApplication::translate("MainWindow", "Add new stream to the list.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        addStreamButton->setText(QApplication::translate("MainWindow", "Add stream", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        removeStreamButton->setToolTip(QApplication::translate("MainWindow", "Remove previous stream from the list.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        removeStreamButton->setText(QApplication::translate("MainWindow", "Remove stream", 0, QApplication::UnicodeUTF8));
        messageEditor->setTitle(QApplication::translate("MainWindow", "Message definition", 0, QApplication::UnicodeUTF8));
        message_nameLabel->setText(QApplication::translate("MainWindow", "Message name:", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        message_name->setToolTip(QApplication::translate("MainWindow", "Each message must have unique name. ", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        message_typeLabel->setText(QApplication::translate("MainWindow", "Message type:", 0, QApplication::UnicodeUTF8));
        message_type->clear();
        message_type->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "Client to server", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "Server to client", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        message_type->setToolTip(QApplication::translate("MainWindow", "Changes message type.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        message_reliable->setToolTip(QApplication::translate("MainWindow", "If application protocol is enabled in UDP stream, messages can be reliable or unreliable.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        message_reliable->setText(QApplication::translate("MainWindow", "Reliable", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        message_sizeFrame->setToolTip(QApplication::translate("MainWindow", "Message size definition in bytes.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        message_size->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        message_size->setText(QApplication::translate("MainWindow", "Message size (bytes):", 0, QApplication::UnicodeUTF8));
        message_configMessageSize->setText(QApplication::translate("MainWindow", "Configure", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        message_timeIntervalFrame->setToolTip(QApplication::translate("MainWindow", "Time interval of two consecutive messages in milliseconds.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        message_timeInterval->setText(QApplication::translate("MainWindow", "Message time interval (ms):", 0, QApplication::UnicodeUTF8));
        message_configTimeInterval->setText(QApplication::translate("MainWindow", "Configure", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        message_returnToSender->setToolTip(QApplication::translate("MainWindow", "Defines whether the receiver returns the message.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        message_returnToSender->setText(QApplication::translate("MainWindow", "Return to sender", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        message_forwardMessageSizeFrame->setToolTip(QApplication::translate("MainWindow", "The size of the forwarded (or returned) message in bytes.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        message_forwardMessageSize->setText(QApplication::translate("MainWindow", "Forward message size (bytes):", 0, QApplication::UnicodeUTF8));
        message_configForwardMessageSize->setText(QApplication::translate("MainWindow", "Configure", 0, QApplication::UnicodeUTF8));
        label_9->setText(QString());
#ifndef QT_NO_TOOLTIP
        message_clientsOfInterestFrame->setToolTip(QApplication::translate("MainWindow", "Defines the percentage of the other clients that receive the  forwarded message from the server. ", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        message_clientsOfInterest->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        message_clientsOfInterest->setText(QApplication::translate("MainWindow", "Clients of interest (%):", 0, QApplication::UnicodeUTF8));
        message_errorLabel->setText(QString());
        message_add->setText(QApplication::translate("MainWindow", "Ok", 0, QApplication::UnicodeUTF8));
        message_cancel->setText(QApplication::translate("MainWindow", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
