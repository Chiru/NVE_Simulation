#include "ClientWidget.h"
#include <QGridLayout>
#include <QLabel>

ClientWidget::ClientWidget(int number, QWidget *parent, const ClientWidget* previous)
    : QGroupBox(parent),
      clientNumber(number)
{
    configClientWidget();


    if(previous != 0)
    {
        delay->setValue(previous->delay->value());
        loss->setValue(previous->loss->value());
        uplink->setValue(previous->uplink->value());
        downlink->setValue(previous->downlink->value());
        arrive->setValue(previous->arrive->value());
        exit->setValue(previous->exit->value());
        pcap->setChecked(previous->pcap->isChecked());
        graph->setChecked(previous->graph->isChecked());
        clientCount->setValue(previous->clientCount->value());
    }

}

ClientWidget::ClientWidget(int number, int count, int delay, double loss, double uplink, double downlink, int arriveTime, int exitTime, bool pcap, bool graph, QWidget *parent)
    : QGroupBox(parent),
      clientNumber(number)
{
    configClientWidget();

    this->delay->setValue(delay);
    this->loss->setValue(loss);
    this->uplink->setValue(uplink);
    this->downlink->setValue(downlink);
    this->arrive->setValue(arriveTime);
    this->exit->setValue(exitTime);
    this->pcap->setChecked(pcap);
    this->graph->setChecked(graph);
    this->clientCount->setValue(count);

}

void ClientWidget::configClientWidget()
{

    QGridLayout* layout = new QGridLayout(this);

    this->setTitle("Client type " + QString::number(clientNumber));
    this->setLayout(layout);

    layout->addWidget(new QLabel("Number of clients:", this), 1,1);
    layout->addWidget(new QLabel("Network delay (ms):", this), 2,1);
    layout->addWidget(new QLabel("Packet loss %:", this), 2,3);
    layout->addWidget(new QLabel("Uplink bandwidth (Mbps):", this), 3,1);
    layout->addWidget(new QLabel("Downlink bandwidth (Mbps):", this), 3,3);
    layout->addWidget(new QLabel("Join time (seconds):", this), 4,1);
    layout->addWidget(new QLabel("Exit time (seconds):", this), 4,3);

    clientCount = new QSpinBox(this);
    delay = new QSpinBox(this);
    loss = new QDoubleSpinBox(this);
    uplink = new QDoubleSpinBox(this);
    downlink = new QDoubleSpinBox(this);
    arrive = new QSpinBox(this);
    exit = new QSpinBox(this);
    pcap = new QCheckBox("Enable pcap", this);
    graph = new QCheckBox("Enable graphs", this);

    clientCount->setValue(1);
    clientCount->setMinimum(1);

    layout->addWidget(clientCount, 1, 2);
    layout->addWidget(delay, 2,2);
    layout->addWidget(loss, 2,4);
    layout->addWidget(uplink, 3,2);
    layout->addWidget(downlink, 3,4);
    layout->addWidget(arrive, 4,2);
    layout->addWidget(exit, 4,4);
    layout->addWidget(pcap, 5,1);
    layout->addWidget(graph, 5,3);

    clientCount->setToolTip("Number of clients of this type");
    delay->setToolTip("Network delay of this client type");
    loss->setToolTip("Packet loss of this client type");
    uplink->setToolTip("Uplink bandwidth of this client type");
    downlink->setToolTip("Downlink bandwidth of this client type");
    arrive->setToolTip("Simulation time when this client type starts the connection");
    exit->setToolTip("Simulation time when this client type closes the connection");
    pcap->setToolTip("Enable pcap-file creation for this client type (makes the simulation slower)");
    graph->setToolTip("Enable graphs for this client type");


    delay->setMaximum(10000);
    arrive->setMaximum(100000);
    exit->setMaximum(100000);

}


