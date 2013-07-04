#ifndef XMLSERIALIZER_H
#define XMLSERIALIZER_H

#include <QString>
#include <QList>


class XmlElement
{

public:
    virtual QString getElementString(int intend = 0) const = 0;   //intend == number of tabulators
    virtual ~XmlElement(){}

protected:
    QString elementName;

};

class XmlStruct : public XmlElement
{

public:
    QString getElementString(int intend) const;

private:
    QList<XmlElement*> values;
};

class XmlValue : public XmlElement
{

public:
    QString getElementString(int intend) const;

private:
    QString value;

};

class XmlSerializer
{

public:
    XmlSerializer(QString fileName);
    void writeToFile();

private:

    QString fileName;
    void writeValue();

};

#endif // XMLSERIALIZER_H
