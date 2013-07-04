#include "MessageTemplate.h"
#include <iostream>

int MessageTemplate::messageTypeCount = 1;


MessageTemplate::MessageTemplate(QWidget *parent, bool appProtoEnabled)
    :QGroupBox(parent),
      messageName(""),
      type(ClientToServer),
      appProtoEnabled(appProtoEnabled),
      reliable(false),
      returnToSender(false),
      useReceivedMessageSize(true),
      forwardMessageSize(0)
{

    messageSize.setDist(None);
    timeInterval.setDist(None);
    clientsOfInterest = 100;

}

MessageTemplate::MessageTemplate(const MessageTemplate &c)
    :QGroupBox(c.parentWidget()),
      messageName(c.messageName),
      type(c.type),
      appProtoEnabled(c.appProtoEnabled),
      reliable(c.reliable),
      returnToSender(c.returnToSender),
      messageSize(c.messageSize),
      timeInterval(c.timeInterval),
      useReceivedMessageSize(c.useReceivedMessageSize),
      forwardMessageSize(c.forwardMessageSize),
      clientsOfInterest(c.clientsOfInterest)

{
}

void MessageTemplate::setMessageName(const QString &name)
{
    this->messageName = name;
}


void MessageTemplate::setMessageType(int index)
{
    this->type = static_cast<TYPE>(index);
}

void MessageTemplate::setAppProtoEnabled(bool enabled)
{
    this->appProtoEnabled = enabled;
}

void MessageTemplate::setReliable(bool reliable)
{
    this->reliable = reliable;
}

void MessageTemplate::setReturnToSender(bool returnToSender)
{
    this->returnToSender = returnToSender;
}

//returns false if elem is 0
bool MessageTemplate::setMessageSize(DistributionElement* elem)
{
    if(elem == 0)
        return false;

    this->messageSize = *elem;
    delete elem;
    return true;
}

bool MessageTemplate::setTimeInterval(DistributionElement* elem)
{
    if(elem == 0)
        return false;

    this->timeInterval = *elem;
    delete elem;
    return true;
}

void MessageTemplate::setForwardMessageSize(int size, bool useRecvSize)
{
    if(useRecvSize)
    {
        useReceivedMessageSize = true;
        this->forwardMessageSize = 0;
    }
    else
    {
        useReceivedMessageSize = false;
        this->forwardMessageSize = size;
    }
}


void MessageTemplate::setClientsOfInterest(double percentage)
{
    clientsOfInterest = percentage;
}


//returns true if message size of received message is used, otherwise returns false and sets size
bool MessageTemplate::getForwardMessageSize(int &size) const
{
    size = forwardMessageSize;

    return useReceivedMessageSize;
}

DistributionElement::DistributionElement()
    :dist(Constant)
{
}


DistributionElement::DistributionElement(const DistributionElement &c)
{
    this->dist = c.dist;
    this->params = c.params;
    this->filename = c.filename;
    this->percentage = c.percentage;
    this->copyDistributions(c.splitDistributions);
}

DistributionElement::~DistributionElement()
{
    DistributionElement* elem;

    foreach(elem, splitDistributions)
        delete elem;
}


DistributionElement& DistributionElement::operator=(const DistributionElement& elem)
{
    this->dist = elem.dist;
    this->params = elem.params;
    this->filename = elem.filename;
    this->percentage = elem.percentage;
    this->copyDistributions(elem.splitDistributions);

    return *this;
}


void DistributionElement::setDist(Distribution dist)
{
    this->dist = dist;
}

void DistributionElement::setParams(const QList<double>& params)
{
    this->params = params;
}

void DistributionElement::setFileName(const QString& fileName)
{
    this->filename = fileName;
}

void DistributionElement::copyDistributions(const QList<DistributionElement*>& splitDistributions)
{
    DistributionElement* dist;

    foreach(dist, splitDistributions)
        this->splitDistributions.append(new DistributionElement(*dist));
}

QString DistributionElement::getDistributionString() const
{
    QString text("");


    switch(dist)
    {
    case None:
        return "";
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

    if(dist != Empirical && dist != Split)
    {
        bool first = true;

        for(QList<double>::const_iterator it = params.begin(); it != params.end(); it++)
        {
            if(!first)
                text.append(", ");
            else
                first = false;

            text.append(QString::number(*it));
        }

    }
    else if(dist == Empirical)
    {
        text.append(filename);
    }
    else if(dist == Split)
    {

        DistributionElement* dist;

        foreach(dist, splitDistributions)
        {
            text.append(QString(" %1%2").arg(dist->getPercentage()).arg("%:"));
            text.append(dist->getDistributionString());

            if(dist != splitDistributions.last())
                text.append(", ");
        }
    }

    text.append(")");

    return text;

}


void DistributionElement::addSplitDistribution(const DistributionElement *dist)
{
    splitDistributions.append(new DistributionElement(*dist));
}


void DistributionElement::setSplitDistributionPercentages(const QList<QDoubleSpinBox*>& percentages)
{
    DistributionElement* dist;

    QList<QDoubleSpinBox*>::const_iterator it = percentages.begin();

    foreach(dist, splitDistributions)
    {
        dist->setPercentage((*it)->value());
        it++;
    }

}


void DistributionElement::setPercentage(double percentage)
{
    this->percentage = percentage;
}

