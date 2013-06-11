#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ClientWidget.h"
#include "StreamWidget.h"
#include "MessageTemplate.h"
#include <QBoxLayout>
#include <QScrollBar>
#include <iostream>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    numberOfClients(1),
    numberOfStreams(1)
{

    ui->setupUi(this);

    QWidget* widget = new QWidget(ui->clientScrollArea);
    widget->setLayout(new QBoxLayout(QBoxLayout::TopToBottom, ui->clientScrollArea));

    ui->clientScrollArea->setWidget(widget);

    ui->clientCountSpinBox->setMinimum(1);

    widget = new QWidget(ui->streamScrollArea);
    widget->setLayout(new QBoxLayout(QBoxLayout::TopToBottom, ui->streamScrollArea));

    ui->streamScrollArea->setWidget(widget);

    addClient();
    addStream();

    ui->appProto_ackSize->setMaximum(500);
    ui->appProto_delAck->setMaximum(10000);
    ui->appProto_headerSize->setMaximum(500);
    ui->appProto_RTO->setMaximum(10000);

    ui->simTime->setMaximum(100000);
    ui->simTime->setSingleStep(10);

    messageSize = new DistributionWidget(ui->message_size, ui->message_configMessageSize, ui->message_sizeDistribution,
                                         ui->message_sizeFrame->layout(), this);
    timeInterval = new DistributionWidget(ui->message_timeInterval, ui->message_configTimeInterval, ui->message_timeIntervalDistribution,
                                          ui->message_timeIntervalFrame->layout(), this);

    ui->message_clientsOfInterestSpinBox->setMaximum(1);
    ui->message_clientsOfInterestSpinBox->setSingleStep(0.05);

    QObject::connect(ui->addClientButton, SIGNAL(clicked()), this, SLOT(addClient()));
    QObject::connect(ui->removeClientButton, SIGNAL(clicked()), this, SLOT(removeClient()));
    QObject::connect(ui->addStreamButton, SIGNAL(clicked()), this, SLOT(addStream()));
    QObject::connect(ui->removeStreamButton, SIGNAL(clicked()), this, SLOT(removeStream()));

    //QObject::connect(ui->message_configMessageSize, SIGNAL(clicked()), this, SLOT(createDistributionDialog()));

    palette = new QPalette();
    palette->setColor(QPalette::WindowText, QColor(255,0,0));
    ui->message_errorLabel->setPalette(*palette);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete palette;
}


void MainWindow::addClient()
{

    int count = ui->clientCountSpinBox->value();

    for(int i = 0; i < count; i++)
    {
        addClientWidgetToScrollArea();
    }
}


void MainWindow::addClientWidgetToScrollArea()
{
    ClientWidget* client;

    if(numberOfClients == 1)
        client = new ClientWidget(numberOfClients++, ui->clientScrollArea->widget());
    else
        client = new ClientWidget(numberOfClients++, ui->clientScrollArea->widget(), previousClients.top());

    QFrame* line = new QFrame(ui->clientScrollArea->widget());

    ui->clientScrollArea->widget()->layout()->addWidget(client);
    previousClients.push(client);

    line->setFrameStyle(QFrame::HLine | QFrame::Plain);
    ui->clientScrollArea->widget()->layout()->addWidget(line);
    previousClientsLines.push(line);
}


void MainWindow::removeClient()
{
    if(!previousClients.isEmpty())
    {
        ClientWidget* previousClient = previousClients.pop();
        QFrame* previousLine = previousClientsLines.pop();

        ui->clientScrollArea->widget()->layout()->removeWidget(previousClient);
        delete previousClient;

        ui->clientScrollArea->widget()->layout()->removeWidget(previousLine);
        delete previousLine;

        numberOfClients--;
    }
}


void MainWindow::addStream()
{
    StreamWidget* stream = new StreamWidget(numberOfStreams++, this, ui->streamScrollArea->widget());

    QObject::connect(stream, SIGNAL(setupMessageEditor(const MessageTemplate*, StreamWidget*)),
                     this, SLOT(setMessage(const MessageTemplate*, StreamWidget*)));

    QFrame* line = new QFrame(ui->streamScrollArea->widget());

    ui->streamScrollArea->widget()->layout()->addWidget(stream);
    previousStreams.push(stream);

    line->setFrameStyle(QFrame::HLine | QFrame::Plain);
    ui->streamScrollArea->widget()->layout()->addWidget(line);
    previousStreamsLines.push(line);

}

void MainWindow::removeStream()
{
    if(!previousStreams.isEmpty())
    {
        StreamWidget* previousStream = previousStreams.pop();
        QFrame* previousLine = previousStreamsLines.pop();

        ui->streamScrollArea->widget()->layout()->removeWidget(previousStream);
        delete previousStream;

        ui->streamScrollArea->widget()->layout()->removeWidget(previousLine);
        delete previousLine;

        numberOfStreams--;
    }
}

void MainWindow::setMessage(const MessageTemplate * msg, StreamWidget* caller)
{
    enableMessageEditor(true);
    caller->enableStreamWidgets(false);
    ui->message_name->setText(msg->getMessageName());
    ui->message_type->setCurrentIndex(msg->getType());

    if(!msg->isAppProtoEnabled())
    {
        if(caller->tcpUsed())
            ui->message_reliable->setChecked(true);

        ui->message_reliable->setDisabled(true);

    }
    else
    {
        ui->message_reliable->setEnabled(true);
        ui->message_reliable->setChecked(msg->isReliable());
    }

    ui->message_returnToSender->setChecked(msg->isReturnedToSender());

    QObject::connect(ui->message_add, SIGNAL(clicked()), caller, SLOT(newMessageAdded()));
    QObject::connect(ui->message_cancel, SIGNAL(clicked()), caller, SLOT(editorClosed()));

}


void MainWindow::finishEditor()
{
    QObject::disconnect(ui->message_add, 0, 0, 0);
    QObject::disconnect(ui->message_cancel, 0, 0, 0);
    enableMessageEditor(false);

    ui->message_name->setText("");
    setMsgConfigErrorMessage("");
    ui->message_reliable->setChecked(false);
    ui->message_returnToSender->setChecked(false);
    ui->message_type->setCurrentIndex(0);

}


void MainWindow::createDistributionDialog()
{

}


void MainWindow::finishDistributionDialog()
{

}



void MainWindow::enableMessageEditor(bool enabled)
{
    ui->message->setEnabled(enabled);
    ui->message_nameLabel->setEnabled(enabled);
    ui->message_name->setEnabled(enabled);
    ui->message_typeLabel->setEnabled(enabled);
    ui->message_type->setEnabled(enabled);
    ui->message_reliable->setEnabled(enabled);
    ui->message_returnToSender->setEnabled(enabled);
    ui->message_cancel->setEnabled(enabled);
    ui->message_add->setEnabled(enabled);
    ui->addStreamButton->setEnabled(!enabled);
    ui->removeStreamButton->setEnabled(!enabled);
    ui->message_clientsOfInterestFrame->setEnabled(enabled);
    ui->message_clientsOfInterest->setEnabled(enabled);
    ui->message_clientsOfInterestSpinBox->setEnabled(enabled);

    messageSize->enableWidget(enabled);
    timeInterval->enableWidget(enabled);

}


bool MainWindow::configMessageFromEditor(MessageTemplate* msg)
{

    if(ui->message_name->text() == "")
        return false;

    msg->setMessageName(ui->message_name->text());
    msg->setMessageType(ui->message_type->currentIndex());
    msg->setReliable(ui->message_reliable->isChecked());
    msg->setReturnToSender(ui->message_returnToSender->isChecked());

     //TODO: add these later
    // msg->setMessageSize(Constant, 0);
   // msg->setTimeInterval(Constant, 0);
  // msg->setForwardMessageSize(Constant, 0);
 // msg->setClientsOfInterest(Constant, 0);

    return true;

}


void MainWindow::setMsgConfigErrorMessage(const QString &error)
{
     ui->message_errorLabel->setText(error);
}



