#include "DistributionWidget.h"
#include <iostream>

DistributionWidget::DistributionWidget(const QString& label, QWidget *parent)
    :QFrame(parent),
      distribution(0)
{
    distributionName = new QLineEdit("", this);
    layout = new QGridLayout(this);
    config = new QPushButton("Configure", this);
    description = new QLabel(label, this);

    layout->addWidget(description);
    layout->addWidget(config);
    layout->addWidget(distributionName);

    distributionName->setReadOnly(true);
    connectSignals();
}


DistributionWidget::DistributionWidget(QLabel* label, QPushButton* button, QLineEdit* resultDist, QLayout* layout, QWidget *parent)
    : QFrame(parent),
      distribution(0),
      layout(layout),
      description(label),
      distributionName(resultDist),
      config(button)
{
    distributionName->setReadOnly(true);
    connectSignals();
}


DistributionWidget::~DistributionWidget()
{
    delete distribution;
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
    DistributionDialog* dialog = new DistributionDialog(false);

    if(dialog->exec())
    {
        delete distribution;
        distribution = dialog->getDistribution();
        distributionName->setText(dialog->getDistributionString(distribution));

    }

    delete distribution;
    distribution = 0;

    delete dialog;
}



DistributionDialog::DistributionDialog(bool alreadySplit)
  :   QDialog(),
      splitDistributionButton(0),
      paramsLayout(0),
      fileName(0),
      alreadySplit(alreadySplit),
      numberOfDistributionsSplit(0)

{

    dialogLayout = new QGridLayout(this);
    dialogLayout->setSizeConstraint(QLayout::SetFixedSize);
    this->setLayout(dialogLayout);

    createDistributionComboBox();
    dialogLayout->addWidget(availableDistributions, 1,1);
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    dialogLayout->addWidget(buttonBox,3,1);

    QObject::connect(availableDistributions, SIGNAL(currentIndexChanged(int)), this, SLOT(distributionChanged(int)));
    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptDistributionDialog()));
    QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancelDistributionDialog()));

    distribution = new DistributionElement();
    distributionChanged(Constant);

}


DistributionDialog::~DistributionDialog()
{
    cleanUpDialog();
    delete distribution;
}


void DistributionDialog::acceptDistributionDialog()
{
    done(1);
}

void DistributionDialog::cancelDistributionDialog()
{
    done(0);
}


void DistributionDialog::createDistributionDialog()
{

    DistributionDialog* dialog = new DistributionDialog(true);
    if(dialog->exec())
    {
        DistributionElement* dist = dialog->getDistribution();
        splitDistributionNames.append(new QLineEdit(getDistributionString(dist)));
        paramDescriptions.append(new QLabel("Distribution " + QString::number(numberOfDistributionsSplit +1) + " "));
        paramDescriptions.append(new QLabel("Percentage "));
        splitDistributionPercentages.append(new QDoubleSpinBox());
        updateSplitDistribution();

        delete dist;
    }

    delete dialog;
}


void DistributionDialog::distributionChanged(int dist)
{

    cleanUpDialog();

    distribution->setDist((Distribution)dist);

    switch(dist)
    {
    case Uniform:
        paramDescriptions.append(new QLabel("Low end of the range"));
        paramDescriptions.append(new QLabel("High end of the range"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        break;
    case Constant:
        paramDescriptions.append(new QLabel("Constant value"));
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        break;
    case Sequential:
        paramDescriptions.append(new QLabel("First value of the sequence"));
        paramDescriptions.append(new QLabel("One more than the last value of the sequence"));
        paramDescriptions.append(new QLabel("Increment between sequence values"));
        paramDescriptions.append(new QLabel(" Number of times each member of the sequence is repeated"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        params[2]->setMaximum(10000);
        params[3]->setMaximum(10000);
        params[3]->setDecimals(0);
        break;
    case Exponential:
        paramDescriptions.append(new QLabel("Mean value of the random variable"));
        paramDescriptions.append(new QLabel("Upper bound on returned values"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        break;
    case Pareto:
        paramDescriptions.append(new QLabel("Mean value"));
        paramDescriptions.append(new QLabel("Shape parameter"));
        paramDescriptions.append(new QLabel("Upper limit of returned values"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        params[2]->setMaximum(10000);
        break;
    case Weibull:
        paramDescriptions.append(new QLabel("Mean value for the distribution"));
        paramDescriptions.append(new QLabel("Shape (alpha) parameter for the distribution"));
        paramDescriptions.append(new QLabel("Upper limit of returned values"));
        paramDescriptions.append(new QLabel("Location parameter"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        params[2]->setMaximum(10000);
        params[3]->setMaximum(10000);
        break;
    case Normal:
        paramDescriptions.append(new QLabel("Mean value"));
        paramDescriptions.append(new QLabel("Variance"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        break;
    case Lognormal:
        paramDescriptions.append(new QLabel("Log scale parameter"));
        paramDescriptions.append(new QLabel("Shape"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        params[0]->setMinimum(-10000);
        params[1]->setMinimum(-10000);
        break;
    case Gamma:
        paramDescriptions.append(new QLabel("Alpha parameter of the gamma distribution"));
        paramDescriptions.append(new QLabel("Beta parameter of the gamma distribution"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        break;
    case Erlang:
        paramDescriptions.append(new QLabel("Shape parameter of the Erlang distribution"));
        paramDescriptions.append(new QLabel("Rate parameter of the Erlang distribution"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        params[0]->setMinimum(1);
        params[0]->setDecimals(0);
        break;
    case Zipf:
        paramDescriptions.append(new QLabel("The number of possible items"));
        paramDescriptions.append(new QLabel("The alpha parameter"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        params[0]->setMinimum(1);
        params[0]->setDecimals(0);
        break;
    case Zeta:
        paramDescriptions.append(new QLabel("The alpha parameter"));
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[0]->setMinimum(1);
        break;
    case Triangular:
        paramDescriptions.append(new QLabel("Low end of the range"));
        paramDescriptions.append(new QLabel("High end of the range"));
        paramDescriptions.append(new QLabel("Mean value of the distribution"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        params[2]->setMaximum(10000);
        break;
    case Empirical:
        paramDescriptions.append(new QLabel("Filename "));
        fileName = new QLineEdit();
        break;
    case Extreme:    //TODO: check if this is supported in ns-3 version
        paramDescriptions.append(new QLabel("Location parameter"));
        paramDescriptions.append(new QLabel("Scale parameter"));
        paramDescriptions.append(new QLabel("Upper bound of returned values"));
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params.append(new QDoubleSpinBox());
        params[0]->setMaximum(10000);
        params[1]->setMaximum(10000);
        params[2]->setMaximum(10000);
        break;
    case Split: //TODO: check if this is supported in ns-3 version
        numberOfDistributionsSplit++;
        paramDescriptions.append(new QLabel("Distribution 1"));
        paramDescriptions.append(new QLabel("Percentage "));
        splitDistributionNames.append(new QLineEdit(""));
        splitDistributionNames.last()->setReadOnly(true);
        splitDistributionPercentages.append(new QDoubleSpinBox());
        splitDistributionPercentages[0]->setMaximum(1);
        splitDistributionButton = new QPushButton("Add");
        QObject::connect(splitDistributionButton, SIGNAL(clicked()), this, SLOT(createDistributionDialog()));
        break;

    }

    addParamWidgetsToDialog();

}


void DistributionDialog::cleanUpDialog()
{
    while(!params.isEmpty())
    {
        QWidget* param = params.takeFirst();
        paramsLayout->removeWidget(param);
        delete param;
    }

    while(!paramDescriptions.isEmpty())
    {
        QWidget* param = paramDescriptions.takeFirst();
        paramsLayout->removeWidget(param);
        delete param;
    }

    while(!splitDistributionNames.isEmpty())
    {
        QWidget* param = splitDistributionNames.takeFirst();
        paramsLayout->removeWidget(param);
        delete param;
    }

    while(!splitDistributionPercentages.isEmpty())
    {
        QWidget* param = splitDistributionPercentages.takeFirst();
        paramsLayout->removeWidget(param);
        delete param;
    }

    if(splitDistributionButton != 0)
    {
        paramsLayout->removeWidget(splitDistributionButton);
        delete splitDistributionButton;
        splitDistributionButton = 0;
    }

    if(paramsLayout != 0)
    {
        dialogLayout->removeItem(paramsLayout);
        delete paramsLayout;
        paramsLayout = 0;
    }

    if(fileName != 0)
    {
        delete fileName;
        fileName = 0;
    }
}


void DistributionDialog::addParamWidgetsToDialog()
{
    if(paramsLayout == 0)
    {
        paramsLayout = new QFormLayout();
        dialogLayout->addLayout(paramsLayout, 2, 1);
    }

    for(int i = paramsLayout->count()/2; i < params.length(); i++)
    {
        paramsLayout->addWidget(paramDescriptions[i]);
        paramsLayout->addWidget(params[i]);
    }

    if(fileName != 0)
    {
        paramsLayout->addWidget(paramDescriptions[0]);
        paramsLayout->addWidget(fileName);
    }

    for(int i = paramsLayout->count()/5; i < splitDistributionNames.length(); i++)
    {
        paramsLayout->addRow(paramDescriptions[2*i], paramDescriptions[2*i+1]);
        paramsLayout->addRow(splitDistributionNames[i], splitDistributionPercentages[i]);
    }

    if(splitDistributionButton != 0)
        paramsLayout->addWidget(splitDistributionButton);
}


void DistributionDialog::createDistributionComboBox()
{
    availableDistributions = new QComboBox(this);

    availableDistributions->insertItem(Constant, "Constant");
    availableDistributions->insertItem(Uniform, "Uniform");
    availableDistributions->insertItem(Sequential, "Sequential");
    availableDistributions->insertItem(Exponential, "Exponential");
    availableDistributions->insertItem(Pareto, "Pareto");
    availableDistributions->insertItem(Weibull, "Weibull");
    availableDistributions->insertItem(Normal, "Normal");
    availableDistributions->insertItem(Lognormal, "Lognormal");
    availableDistributions->insertItem(Gamma, "Gamma");
    availableDistributions->insertItem(Erlang, "Erlang");
    availableDistributions->insertItem(Zipf, "Zipf");
    availableDistributions->insertItem(Zeta, "Zeta");
    availableDistributions->insertItem(Triangular, "Triangular");
    availableDistributions->insertItem(Extreme, "Extreme");   //TODO: check if ns-3 version supports this
    if(!alreadySplit)
    {
        availableDistributions->insertItem(Empirical, "Empirical");
        availableDistributions->insertItem(Split, "Split");  //TODO: check if ns-3 version supports this
    }
}


DistributionElement* DistributionDialog::getDistribution()
{

    DistributionElement* retVal = 0;

    if(distribution->getDist() == Empirical)
    {
        retVal = new DistributionElement();
        retVal->setFileName(this->fileName->text());
        retVal->setDist(distribution->getDist());

    }
    else if(distribution->getDist() == Split)
    {
        retVal = new DistributionElement();
        retVal->setDistributions(distribution->getSplitDistributions());
        retVal->setDist(distribution->getDist());
    }
    else
    {
        retVal = new DistributionElement();
        retVal->setDist(this->distribution->getDist());

        for(QList<QDoubleSpinBox*>::const_iterator it = this->params.begin(); it != this->params.end(); it++)
        {
            retVal->getParams().append((*it)->value());
        }
    }

    return retVal;
}



QString DistributionDialog::getDistributionString(const DistributionElement* distribution)
{
    QString text("");

    if(distribution == 0)
    {
        text = "";
    }
    else
    {

        switch(distribution->getDist())
        {
        case None:
        break;
        case Constant:
            text = "Constant (";
        break;
        case Uniform:
            text = "Uniform (";
        break;
        case Sequential:
            text = "Sequential (";
        break;
        case Exponential:
            text = "Exponential (";
        break;
        case Pareto:
            text = "Pareto (";
        break;
        case Weibull:
            text = "Weibull (";
        break;
        case Normal:
            text = "Normal (";
        break;
        case Lognormal:
            text = "Lognormal (";
        break;
        case Gamma:
            text = "Gamma (";
        break;
        case Erlang:
            text = "Erlang (";
        break;
        case Zipf:
            text = "Zipf (";
        break;
        case Zeta:
            text = "Zeta (";
        break;
        case Triangular:
            text = "Triangular (";
        break;
        case Extreme:
            text = "Extreme (";
        break;
        case Empirical:
            text = "Empirical (";
        break;
        case Split:
            text = "Split (";
            break;

        }

        if(distribution->getDist() != Empirical && distribution->getDist() != Split)
        {
            bool first = true;

            for(QList<double>::const_iterator it = distribution->getParams().begin(); it != distribution->getParams().end(); it++)
            {
                if(!first)
                    text.append(", ");
                else
                    first = false;

                text.append(QString::number(*it));
            }

        }
        else if(distribution->getDist() == Empirical)
        {
            text.append(distribution->getFileName());
        }
        else if(distribution->getDist() == Split)
        {


        }
    }

    text.append(")");

    return text;

}


void DistributionDialog::updateSplitDistribution()
{

   // paramsLayout->removeWidget(splitDistributionButton);

   // while(!paramsLayout->isEmpty())
    {
     //   paramsLayout->removeItem(paramsLayout->itemAt(0));
    }

    numberOfDistributionsSplit++;
    addParamWidgetsToDialog();

}



