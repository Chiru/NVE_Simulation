#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ClientWidget.h"
#include <QBoxLayout>
#include <QScrollBar>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    numberOfClients(1)
{
    ui->setupUi(this);

    QWidget* widget = new QWidget(ui->clientsScrollArea);
    widget->setLayout(new QBoxLayout(QBoxLayout::TopToBottom, ui->clientsScrollArea));

    ui->clientsScrollArea->setWidget(widget);

    ui->clientCountSpinBox->setMinimum(1);

    addClient();

    QObject::connect(ui->addClientButton, SIGNAL(clicked()), this, SLOT(addClient()));
    QObject::connect(ui->removeClientButton, SIGNAL(clicked()), this, SLOT(removeClient()));
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
        client = new ClientWidget(numberOfClients++, ui->clientsScrollArea->widget());
    else
        client = new ClientWidget(numberOfClients++, ui->clientsScrollArea->widget(), previousClients.top());

    QFrame* line = new QFrame(ui->clientsScrollArea->widget());

    ui->clientsScrollArea->widget()->layout()->addWidget(client);
    previousClients.push(client);

    line->setFrameStyle(QFrame::HLine | QFrame::Plain);
    ui->clientsScrollArea->widget()->layout()->addWidget(line);
    previousLines.push(line);
}


void MainWindow::removeClient()
{

    if(!previousClients.isEmpty())
    {
        ClientWidget* previousClient = previousClients.pop();
        QFrame* previousLine = previousLines.pop();

        ui->clientsScrollArea->widget()->layout()->removeWidget(previousClient);
        delete previousClient;

        ui->clientsScrollArea->widget()->layout()->removeWidget(previousLine);
        delete previousLine;

        numberOfClients--;
    }
}



