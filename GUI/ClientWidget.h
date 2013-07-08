#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>

class ClientWidget : public QGroupBox
{

    friend class XmlSerializer;

public:
    ClientWidget(int number, QWidget* parent = 0, const ClientWidget* previous = 0);

private:

    int clientNumber;

    QSpinBox* clientCount;
    QSpinBox* delay;
    QDoubleSpinBox* loss;
    QDoubleSpinBox* uplink;
    QDoubleSpinBox* downlink;
    QSpinBox* arrive;
    QSpinBox* exit;
    QCheckBox* pcap;
    QCheckBox* graph;

};

#endif // CLIENTWIDGET_H
