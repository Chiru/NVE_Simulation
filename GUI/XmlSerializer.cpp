#include "XmlSerializer.h"
#include "StreamWidget.h"
#include <QFile>


XmlElement::XmlElement(const QString &name)
    :elementName(name)
{
}


XmlStruct::XmlStruct(const QString &name)
    :XmlElement(name)
{
}

XmlStruct::~XmlStruct()
{
    XmlElement* elem;

    foreach(elem, values)
        delete elem;

}


XmlValue::XmlValue(const QString &name, const QString &value)
    :XmlElement(name),
      value(value)
{
}


QString XmlStruct::getElementString(int intend) const
{
    QString result("");
    XmlElement* elem;

    for(int i = 0; i < intend; i++)
        result.append("\t");

    result.append("<" + elementName + ">\n");

    foreach(elem, values)
        result.append(elem->getElementString(intend + 1));

    for(int i = 0; i < intend; i++)
        result.append("\t");

    result.append("</" + elementName + ">\n");

    return result;
}

void XmlStruct::addElement(XmlElement* elem)
{
    values.append(elem);
}


QString XmlValue::getElementString(int intend) const
{
    QString result("");

    for(int i = 0; i < intend; i++)
        result.append("\t");

    result.append("<" + elementName + "\"" + value + "\"/\n");

    return result;
}


XmlSerializer::XmlSerializer(QString fileName)
    : fileName(fileName)
{

}


void XmlSerializer::addClientsElement(XmlElement* client)
{
    this->clients.append(client);
}


void XmlSerializer::addAppProtoElement(XmlElement* appProto)
{
    this->appProto = appProto;
}


void XmlSerializer::addStreamElement(StreamWidget* stream)
{
    XmlStruct* streamElement = new XmlStruct("stream");

    if(stream->tcpUsed())
        streamElement->addElement(new XmlValue("type", "tcp"));
    else
        streamElement->addElement(new XmlValue("type", "udp"));

    if(stream->appProto->isChecked())
        streamElement->addElement(new XmlValue("appproto", "yes"));
    else
        streamElement->addElement(new XmlValue("appproto", "no"));

    streamElement->addElement(new XmlValue("servergametick", QString::number(stream->serverGameTick->value())));
    streamElement->addElement(new XmlValue("clientgametick", QString::number(stream->clientGameTick->value())));

    addMessages(streamElement, stream->messages);

}


void XmlSerializer::addMessages(XmlStruct* stream, QList<MessageTemplate*> messages)
{
    XmlStruct* messageStruct = new XmlStruct("messages");

    MessageTemplate* msg;

    foreach(msg, messages)
    {
        XmlStruct* singleMessage = new XmlStruct("message");

        singleMessage->addElement(new XmlValue("type", msg->type == MessageTemplate::ServerToClient ? "odt" : "uam"));  //TODO: these names could be changed?
        singleMessage->addElement(new XmlValue("name",msg->messageName));
        singleMessage->addElement(new XmlValue("size", msg->messageSize.getDistributionString()));
        singleMessage->addElement(new XmlValue("reliable", msg->reliable == true ? "yes" : "no"));
        singleMessage->addElement(new XmlValue("timeinterval", msg->timeInterval.getDistributionString()));

        if(msg->useReceivedMessageSize)
            singleMessage->addElement(new XmlValue("forwardmessagesize", "rcv"));   //rcv means "same as received"
        else
            singleMessage->addElement(new XmlValue("forwardmessagesize", QString::number(msg->forwardMessageSize)));

        singleMessage->addElement(new XmlValue("returntosender", msg->returnToSender ? "yes" : "no"));    //TODO: change from "forwardback" to "returntosender" in xml file parsing
        singleMessage->addElement(new XmlValue("timerequirementclient", QString::number(msg->timeRequirementClient)));
        singleMessage->addElement(new XmlValue("timerequirementserver", QString::number(msg->timeRequirementServer)));
        singleMessage->addElement(new XmlValue("clientsofinterest", QString::number(msg->clientsOfInterest/100)));

        messageStruct->addElement(singleMessage);
    }

    stream->addElement(messageStruct);
}


void XmlSerializer::addSimulationParam(XmlElement* param)
{
    this->simulationParams.append(param);
}


void XmlSerializer::gatherElements()
{
    XmlElement* elem;

    foreach(elem, simulationParams)
        elements.append(elem);

    XmlStruct* clientElement = new XmlStruct("clients");

    foreach(elem, clients)
        clientElement->addElement(elem);

    elements.append(clientElement);
    elements.append(appProto);

    XmlStruct* streamElement = new XmlStruct("streams");

    foreach(elem, streams)
        streamElement->addElement(elem);

    elements.append(streamElement);
}


bool XmlSerializer::writeToFile()
{

    QFile file(fileName);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return false;

    XmlElement* elem;
    QString contents("<xml>\n");

    gatherElements();

    foreach(elem, elements)
        contents = elem->getElementString(1);  //TODO: write separate xml-values?

    contents.append("</xml>");

    file.write(contents.toAscii());

    file.close();

    return true;
}
