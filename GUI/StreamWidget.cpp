#include "StreamWidget.h"
#include <QGridLayout>
#include <QLabel>

StreamWidget::StreamWidget(int number, QWidget *parent)
    : QGroupBox(parent),
      streamNumber(number)
{
    QGridLayout* layout = new QGridLayout(this);

    this->setTitle("Stream number " + QString::number(streamNumber));

    this->setLayout(layout);

    layout->addWidget(new QLabel("Transport layer protocol: ", this),1,1);
    layout->addWidget(new QLabel("Client game tick (ms):", this),4,1);
    layout->addWidget(new QLabel("Server game tick (ms):", this),5,1);

    tcp = new QRadioButton("TCP", this);
    udp = new QRadioButton("UDP", this);
    nagle = new QCheckBox("Nagle's algorithm", this);
    appProto = new QCheckBox("Enable application protocol", this);
    ordered = new QCheckBox("Ordered transfer");
    clientGameTick = new QSpinBox(this);
    serverGameTick = new QSpinBox(this);

    layout->addWidget(udp, 1,2);
    layout->addWidget(tcp, 1,3);
    layout->addWidget(appProto, 2,2);
    layout->addWidget(nagle, 2, 3);
    layout->addWidget(ordered, 3, 2);
    layout->addWidget(clientGameTick, 4, 2);
    layout->addWidget(serverGameTick, 5,2);

}
