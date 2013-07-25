#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QCheckBox>

class ClientWidget : public QGroupBox
{
    Q_OBJECT

    friend class XmlSerializer;

public:
    ClientWidget(int number, int simTime, QWidget* parent = 0, const ClientWidget* previous = 0);
    ClientWidget(int number, int count, int delay, double loss, double uplink, double downlink, int arriveTime, int exitTime, bool pcap, bool graph, QWidget* parent = 0);

public slots:
    void simTimeChanged(int);

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
    int lastConfiguredArriveValue;
    int lastConfiguredExitValue;
    bool autoValueChange;

    void configClientWidget();

private slots:
    void exitValueChangedManually(int);
    void arriveValueChangedManually(int);

};

#endif // CLIENTWIDGET_H
