#include "MessageTemplate.h"

int MessageTemplate::messageTypeCount = 1;


MessageTemplate::MessageTemplate(bool appProtoEnabled, QWidget *parent)
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
