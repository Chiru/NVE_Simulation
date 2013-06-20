#include "MessageTemplate.h"

int MessageTemplate::messageTypeCount = 1;


MessageTemplate::MessageTemplate(QWidget *parent, bool appProtoEnabled)
    :QGroupBox(parent),
      messageName(""),
      type(ClientToServer),
      appProtoEnabled(appProtoEnabled),
      reliable(false),
      returnToSender(false)
{

    messageSize.setDist(Constant);
    timeInterval.setDist(Constant);
    forwardMessageSize.setDist(Constant);
    clientsOfInterest.setDist(Constant);

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

void MessageTemplate::setMessageSize(Distribution dist, QList<double> params)
{
    this->messageSize.setDist(dist);
    this->messageSize.setParams(params);
}

void MessageTemplate::setTimeInterval(Distribution dist, QList<double> params)
{
    this->timeInterval.setDist(dist);
    this->timeInterval.setParams(params);
}

void MessageTemplate::setForwardMessageSize(Distribution dist, QList<double> params)
{
    this->forwardMessageSize.setDist(dist);
    this->forwardMessageSize.setParams(params);
}

void MessageTemplate::setClientsOfInterest(Distribution dist, QList<double> params)
{
    this->clientsOfInterest.setDist(dist);
    this->clientsOfInterest.setParams(params);
}


DistributionElement::DistributionElement()
    :dist(Constant)
{
}


DistributionElement::~DistributionElement()
{
    DistributionElement* elem;

    foreach(elem, splitDistributions)
        delete elem;
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

void DistributionElement::setDistributions(const QList<DistributionElement*>& splitDistributions)
{
    this->splitDistributions = splitDistributions;
}


