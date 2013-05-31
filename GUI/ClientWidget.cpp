#include "ClientWidget.h"
#include <QGridLayout>
#include <QLabel>

ClientWidget::ClientWidget(int number, QWidget *parent, const ClientWidget* previous)
    : QGroupBox(parent),
      clientNumber(number)
{

    QGridLayout* layout = new QGridLayout(this);

    this->setTitle("Client type " + QString::number(clientNumber));

    this->setLayout(layout);

    layout->addWidget(new QLabel("Network delay (ms):", this), 1,1);
    layout->addWidget(new QLabel("Packet loss %:", this), 1,3);
    layout->addWidget(new QLabel("Uplink bandwidth (Mbps):", this), 2,1);
    layout->addWidget(new QLabel("Downlink bandwidth (Mbps):", this), 2,3);
    layout->addWidget(new QLabel("Join time (seconds):", this), 3,1);
    layout->addWidget(new QLabel("Exit time (seconds):", this), 3,3);

    delay = new QSpinBox(this);
    loss = new QDoubleSpinBox(this);
    uplink = new QDoubleSpinBox(this);
    downlink = new QDoubleSpinBox(this);
    arrive = new QSpinBox(this);
    exit = new QSpinBox(this);

    if(previous != 0)
    {
        delay->setValue(previous->delay->value());
        loss->setValue(previous->loss->value());
        uplink->setValue(previous->uplink->value());
        downlink->setValue(previous->downlink->value());
        arrive->setValue(previous->arrive->value());
        exit->setValue(previous->exit->value());
    }

    layout->addWidget(delay, 1,2);
    layout->addWidget(loss, 1,4);
    layout->addWidget(uplink, 2,2);
    layout->addWidget(downlink, 2,4);
    layout->addWidget(arrive, 3,2);
    layout->addWidget(exit, 3,4);

    delay->setMaximum(10000);
    arrive->setMaximum(100000);
    exit->setMaximum(100000);

    delay->setFocusPolicy(Qt::ClickFocus);


}


