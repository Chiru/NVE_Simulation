#ifndef DISTRIBUTIONWIDGET_H
#define DISTRIBUTIONWIDGET_H

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include "MessageTemplate.h"

class DistributionWidget : public QFrame
{

    Q_OBJECT

public:
    DistributionWidget(const QString& label, QWidget* parent = 0);
    DistributionWidget(QLabel* label, QPushButton* button, QLabel* resultDist, QLayout* layout, QWidget* parent = 0);

    void enableWidget(bool enabled);

private:
    DistributionElement* distribution;
    QLayout* layout;
    QLabel* description;
    QLabel* distributionName;
    QPushButton* config;

    void connectSignals();

private slots:
    void createDistributionDialog();
    void finishDistributionDialog();

};

#endif // DISTRIBUTIONWIDGET_H
