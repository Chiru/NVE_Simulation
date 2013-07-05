#ifndef XMLSERIALIZER_H
#define XMLSERIALIZER_H

#include <QString>
#include <QList>
#include "StreamWidget.h"


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
    bool writeToFile();
    void addClientsElement(XmlElement*);
    void addAppProtoElement(XmlElement*);
    void addStreamElement(StreamWidget* stream);
    void addSimulationParam(XmlElement*);

private:

    QString fileName;
    QList<XmlElement*> elements; //TODO: remove?
    QList<XmlElement*> clients;
    XmlElement* appProto;
    QList<XmlElement*> streams;
    QList<XmlElement*> simulationParams;

    void gatherElements();
    void addMessages(XmlStruct* stream, QList<MessageTemplate*> messages);

};

#endif // XMLSERIALIZER_H
