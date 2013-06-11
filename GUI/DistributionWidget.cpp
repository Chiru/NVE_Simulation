#include "DistributionWidget.h"

DistributionWidget::DistributionWidget(const QString& label, QWidget *parent)
    :QFrame(parent)
{
    distributionName = new QLabel("", this);
    layout = new QGridLayout(this);
    config = new QPushButton("Configure", this);
    description = new QLabel(label, this);

    layout->addWidget(description);
    layout->addWidget(config);
    layout->addWidget(distributionName);

    connectSignals();
}


DistributionWidget::DistributionWidget(QLabel* label, QPushButton* button, QLabel* resultDist, QLayout* layout, QWidget *parent)
    : QFrame(parent),
      layout(layout),
      description(label),
      distributionName(resultDist),
      config(button)
{
    connectSignals();
}


void DistributionWidget::connectSignals()
{
    QObject::connect(config, SIGNAL(clicked()), this, SLOT(createDistributionDialog()));
}


void DistributionWidget::enableWidget(bool enabled)
{
    this->setEnabled(enabled);
    description->setEnabled(enabled);
    distributionName->setEnabled(enabled);
    config->setEnabled(enabled);
}


void DistributionWidget::createDistributionDialog()
{

}


void DistributionWidget::finishDistributionDialog()
{


}


