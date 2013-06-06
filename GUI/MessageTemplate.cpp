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

    messageSize.dist = Constant;
    timeInterval.dist = Constant;
    forwardMessageSize.dist = Constant;
    clientsOfInterest.dist = Constant;

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
    this->messageSize.dist = dist;
    this->messageSize.params = params;
}

void MessageTemplate::setTimeInterval(Distribution dist, QList<double> params)
{
    this->timeInterval.dist = dist;
    this->timeInterval.params = params;
}

void MessageTemplate::setForwardMessageSize(Distribution dist, QList<double> params)
{
    this->forwardMessageSize.dist = dist;
    this->forwardMessageSize.params = params;
}

void MessageTemplate::setClientsOfInterest(Distribution dist, QList<double> params)
{
    this->clientsOfInterest.dist = dist;
    this->clientsOfInterest.params = params;
}
