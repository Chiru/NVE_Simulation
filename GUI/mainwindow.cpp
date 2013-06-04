#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ClientWidget.h"
#include "StreamWidget.h"
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

    QObject::connect(ui->addClientButton, SIGNAL(clicked()), this, SLOT(addClient()));
    QObject::connect(ui->removeClientButton, SIGNAL(clicked()), this, SLOT(removeClient()));
    QObject::connect(ui->addStreamButton, SIGNAL(clicked()), this, SLOT(addStream()));
    QObject::connect(ui->removeStreamButton, SIGNAL(clicked()), this, SLOT(removeStream()));


}

MainWindow::~MainWindow()
{
    delete ui;
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
    StreamWidget* stream = new StreamWidget(numberOfStreams++, ui->streamScrollArea->widget());

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


