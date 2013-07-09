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
    numberOfStreams(1),
    serializer(XmlSerializer("configuration.txt"))
{

    ui->setupUi(this);

    QWidget* widget = new QWidget(ui->clientScrollArea);
    widget->setLayout(new QBoxLayout(QBoxLayout::TopToBottom, ui->clientScrollArea));

    ui->clientScrollArea->setWidget(widget);

    widget = new QWidget(ui->streamScrollArea);
    widget->setLayout(new QBoxLayout(QBoxLayout::TopToBottom, ui->streamScrollArea));

    ui->streamScrollArea->setWidget(widget);

    addClientWidgetToScrollArea();
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

    ui->message_clientsOfInterestSpinBox->setMaximum(100);
    ui->message_clientsOfInterestSpinBox->setSingleStep(0.5);

    QObject::connect(ui->addClientButton, SIGNAL(clicked()), this, SLOT(addClientWidgetToScrollArea()));
    QObject::connect(ui->removeClientButton, SIGNAL(clicked()), this, SLOT(removeClient()));
    QObject::connect(ui->addStreamButton, SIGNAL(clicked()), this, SLOT(addStream()));
    QObject::connect(ui->removeStreamButton, SIGNAL(clicked()), this, SLOT(removeStream()));
    QObject::connect(ui->message_forwardMessageSizeRadioButton_received, SIGNAL(toggled(bool)), ui->message_forwardMessageSpinBox, SLOT(setDisabled(bool)));
    QObject::connect(ui->executeButton, SIGNAL(clicked()), this, SLOT(configurationFinished()));

    ui->message_forwardMessageSpinBox->setMaximum(10000);
    ui->message_forwardMessageSpinBox->setMinimum(1);
    ui->message_forwardMessageSizeRadioButton_received->setChecked(true);

    ui->message_timeReqClientSpinBox->setMaximum(10000);
    ui->message_timeReqServerSpinBox->setMaximum(10000);

    palette = new QPalette();
    palette->setColor(QPalette::WindowText, QColor(255,0,0));
    ui->message_errorLabel->setPalette(*palette);

    enableMessageEditor(false);

}

MainWindow::~MainWindow()
{
    delete ui;
    delete palette;
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

    timeInterval->setValue(new DistributionElement(msg->getMessageTimeInterval()));
    messageSize->setValue(new DistributionElement(msg->getMessageSize()));

    ui->message_clientsOfInterestSpinBox->setValue(msg->getClientsOfInterest());

    int forwardMessageSize;

    if(msg->getForwardMessageSize(forwardMessageSize))
    {
        ui->message_forwardMessageSizeRadioButton_received->setChecked(true);
        ui->message_forwardMessageSpinBox->setEnabled(false);
    }
    else
    {
        ui->message_forwardMessageSizeRadioButton_constant->setChecked(true);
        ui->message_forwardMessageSpinBox->setEnabled(true);
    }

    ui->message_forwardMessageSpinBox->setValue(forwardMessageSize);

    if(ui->message_forwardMessageSizeRadioButton_received->isChecked())
        ui->message_forwardMessageSpinBox->setEnabled(false);

    ui->message_timeReqClientSpinBox->setValue(msg->getTimeReqClient());
    ui->message_timeReqServerSpinBox->setValue(msg->getTimeReqServer());


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

    ui->message_clientsOfInterestSpinBox->clear();
    ui->message_forwardMessageSpinBox->clear();
    ui->message_timeReqClientSpinBox->clear();
    ui->message_timeReqServerSpinBox->clear();

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

    ui->message_forwardMessageSizeFrame->setEnabled(enabled);
    ui->message_forwardMessageSize->setEnabled(enabled);
    ui->message_forwardMessageSpinBox->setEnabled(enabled);
    ui->message_forwardMessageSizeRadioButton_received->setEnabled(enabled);
    ui->message_forwardMessageSizeRadioButton_constant->setEnabled(enabled);

    ui->message_clientsOfInterestFrame->setEnabled(enabled);
    ui->message_clientsOfInterest->setEnabled(enabled);
    ui->message_clientsOfInterestSpinBox->setEnabled(enabled);

    ui->addStreamButton->setEnabled(!enabled);
    ui->removeStreamButton->setEnabled(!enabled);

    ui->message_timeRequirementFrame->setEnabled(enabled);
    ui->message_timeReqClient->setEnabled(enabled);
    ui->message_timeReqServer->setEnabled(enabled);
    ui->message_timeReqClientSpinBox->setEnabled(enabled);
    ui->message_timeReqServerSpinBox->setEnabled(enabled);

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

    if((msg->setMessageSize(messageSize->getCopyOfDistributionElement())) == 0)
       return false;

    if((msg->setTimeInterval(timeInterval->getCopyOfDistributionElement())) == 0)
        return false;

    if(msg->getMessageTimeInterval().getDist() == None || msg->getMessageSize().getDist() == None)
        return false;

    msg->setForwardMessageSize(ui->message_forwardMessageSpinBox->value(), ui->message_forwardMessageSizeRadioButton_received->isChecked());
    msg->setClientsOfInterest(ui->message_clientsOfInterestSpinBox->value());

    msg->setTimeRequirementClient(ui->message_timeReqClientSpinBox->value());
    msg->setTimeRequirementServer(ui->message_timeReqServerSpinBox->value());


    return true;

}


void MainWindow::setMsgConfigErrorMessage(const QString &error)
{
     ui->message_errorLabel->setText(error);
}


void MainWindow::configurationFinished()
{

    serializer.addSimulationParam(ui->simTime->value(), ui->animationCheckBox->isChecked());

    ClientWidget* client;

    foreach(client, previousClients)
        serializer.addClientsElement(client);

    serializer.addAppProtoElement(ui->appProto_ackSize->value(), ui->appProto_delAck->value(), ui->appProto_RTO->value(), ui->appProto_headerSize->value());

    StreamWidget* stream;

    foreach(stream, previousStreams)
        serializer.addStreamElement(stream);

    serializer.writeToFile();
}

