#include "XmlSerializer.h"
#include "StreamWidget.h"
#include <QFile>
#include <iostream>


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

    result.append("<" + elementName + "=\"" + value + "\"/>\n");

    return result;
}


XmlSerializer::XmlSerializer(QString fileName)
    : fileName(fileName),
      appProto(0),
      clientCount(1)
{
}


XmlSerializer::~XmlSerializer()
{
    flush();
}

void XmlSerializer::addClientsElement(const ClientWidget *client)
{
    XmlStruct* elem = new XmlStruct("client");

    QString number("");

    if(client->clientCount->value() == 1)
    {
        number = QString::number(clientCount);
    }
    else
    {
        number = QString::number(clientCount);
        number.append(":");
        number.append(QString::number(clientCount + client->clientCount->value() - 1));
    }

    elem->addElement(new XmlValue("no", number));

    clientCount += client->clientCount->value();

    elem->addElement(new XmlValue("delay", QString::number(client->delay->value())));
    elem->addElement(new XmlValue("uplink", QString::number(client->uplink->value())));
    elem->addElement(new XmlValue("downlink", QString::number(client->downlink->value())));
    elem->addElement(new XmlValue("loss", QString::number(client->loss->value() / 100)));

    elem->addElement(new XmlValue("jointime", QString::number(client->arrive->value())));

    elem->addElement(new XmlValue("exittime", QString::number(client->exit->value())));

    elem->addElement(new XmlValue("pcap", client->pcap->isChecked() ? "yes" : "no"));
    elem->addElement(new XmlValue("graphs", client->graph->isChecked() ? "yes" : "no"));

    clients.append(elem);
}


void XmlSerializer::addAppProtoElement(uint ackSize, uint delayedAck, uint retransmit, uint headerSize)
{
    appProto = new XmlStruct("appproto");

    appProto->addElement(new XmlValue("acksize", QString::number(ackSize)));
    appProto->addElement(new XmlValue("delayedack", QString::number(delayedAck)));
    appProto->addElement(new XmlValue("retransmit", QString::number(retransmit)));
    appProto->addElement(new XmlValue("headersize", QString::number(headerSize)));
}


void XmlSerializer::addStreamElement(const StreamWidget* stream)
{

    XmlStruct* streamElement = new XmlStruct("stream");

    if(stream->tcpUsed())
        streamElement->addElement(new XmlValue("type", "tcp"));
    else
        streamElement->addElement(new XmlValue("type", "udp"));

    if(stream->nagle->isChecked())
        streamElement->addElement(new XmlValue("nagle", "yes"));
    else
        streamElement->addElement(new XmlValue("nagle", "no"));

    if(stream->appProto->isChecked())
        streamElement->addElement(new XmlValue("appproto", "yes"));
    else
        streamElement->addElement(new XmlValue("appproto", "no"));

    if(stream->ordered->isChecked())
        streamElement->addElement(new XmlValue("ordered", "yes"));
    else
        streamElement->addElement(new XmlValue("ordered", "no"));

    streamElement->addElement(new XmlValue("servergametick",
                                           QString::number(stream->serverGameTick->value())));
    streamElement->addElement(new XmlValue("clientgametick",
                                           QString::number(stream->clientGameTick->value())));

    addMessages(streamElement, stream->messages);

    streams.append(streamElement);

}


void XmlSerializer::addMessages(XmlStruct* stream, const QList<MessageTemplate*>& messages)
{
    XmlStruct* messageStruct = new XmlStruct("messages");

    MessageTemplate* msg;

    foreach(msg, messages)
    {
        XmlStruct* singleMessage = new XmlStruct("message");

        singleMessage->addElement(new XmlValue("type", msg->type == MessageTemplate::ServerToClient ? "odt" : "uam"));
        singleMessage->addElement(new XmlValue("name",msg->messageName));
        singleMessage->addElement(new XmlValue("size", msg->messageSize.getDistributionString()));
        singleMessage->addElement(new XmlValue("reliable", msg->reliable == true ? "yes" : "no"));
        singleMessage->addElement(new XmlValue("timeinterval", msg->timeInterval.getDistributionString()));

        if(msg->useReceivedMessageSize)
            singleMessage->addElement(new XmlValue("forwardmessagesize", "rcv"));
        else
            singleMessage->addElement(new XmlValue("forwardmessagesize",
                                                   QString::number(msg->forwardMessageSize)));

        singleMessage->addElement(new XmlValue("returntosender",
                                               msg->returnToSender ? "yes" : "no"));
        singleMessage->addElement(new XmlValue("timerequirementclient",
                                               QString::number(msg->timeRequirementClient)));
        singleMessage->addElement(new XmlValue("timerequirementserver",
                                               QString::number(msg->timeRequirementServer)));
        singleMessage->addElement(new XmlValue("clientsofinterest",
                                               QString::number(msg->clientsOfInterest/100)));

        messageStruct->addElement(singleMessage);
    }

    stream->addElement(messageStruct);
}


void XmlSerializer::addSimulationParam(uint simTime, bool pcap)
{
    this->simulationParams.append(new XmlValue("runningtime", QString::number(simTime)));
    this->simulationParams.append(new XmlValue("serverpcap", pcap ? "yes" : "no"));
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
        contents.append(elem->getElementString(1));

    contents.append("</xml>");

    file.write(contents.toAscii());

    file.close();

    return true;
}


void XmlSerializer::flush()
{
    XmlElement* elem;

    clientCount = 1;

    delete appProto;
    appProto = 0;

    foreach(elem, clients)
    {
        delete elem;
        clients.removeOne(elem);
    }

    foreach(elem, streams)
    {
        delete elem;
        streams.removeOne(elem);
    }

    foreach(elem, simulationParams)
    {
        delete elem;
        simulationParams.removeOne(elem);
    }

    if(!elements.empty())
    {
        foreach(elem, elements)
        {
            elements.removeOne(elem);
        }

    }
}

