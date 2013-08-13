#ifndef XMLSERIALIZER_H
#define XMLSERIALIZER_H

#include <QString>
#include <QList>
#include "StreamWidget.h"
#include "ClientWidget.h"


class XmlElement
{

public:
    XmlElement(const QString& name);
    virtual QString getElementString(int intend = 0) const = 0;   //intend == number of tabulators
    virtual ~XmlElement(){}

protected:
    QString elementName;

};

class XmlStruct : public XmlElement
{

public:
    XmlStruct(const QString& name);
    ~XmlStruct();
    QString getElementString(int intend) const;
    void addElement(XmlElement*);

private:
    QList<XmlElement*> values;
};

class XmlValue : public XmlElement
{

public:
    XmlValue(const QString& name, const QString& value);
    QString getElementString(int intend) const;

private:
    QString value;

};

class XmlSerializer
{

public:
    XmlSerializer(QString fileName);
    ~XmlSerializer();
    bool writeToFile();
    void addClientsElement(const ClientWidget *);
    void addAppProtoElement(uint ackSize, uint delayedAck, uint retransmit, uint headerSize);
    void addStreamElement(const StreamWidget *stream);
    void addSimulationParam(uint simTime, bool pcap);
    void flush();

private:

    QString fileName;
    QList<XmlElement*> elements;
    QList<XmlElement*> clients;
    XmlStruct* appProto;
    QList<XmlElement*> streams;
    QList<XmlElement*> simulationParams;
    uint clientCount;
    uint simulationTime;

    void gatherElements();
    void addMessages(XmlStruct *stream, const QList<MessageTemplate *>& messages);

};

#endif // XMLSERIALIZER_H
