#ifndef CLIENTWIDGET_H
#define CLIENTWIDGET_H

#include <QGroupBox>
#include <QDoubleSpinBox>
#include <QSpinBox>

class ClientWidget : public QGroupBox
{
public:
    ClientWidget(int number, QWidget* parent = 0, const ClientWidget* previous = 0);

private:

    int clientNumber;

    QSpinBox* delay;
    QDoubleSpinBox* loss;
    QDoubleSpinBox* uplink;
    QDoubleSpinBox* downlink;
    QSpinBox* arrive;
    QSpinBox* exit;

};

#endif // CLIENTWIDGET_H
