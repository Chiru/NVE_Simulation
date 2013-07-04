#ifndef XMLSERIALIZER_H
#define XMLSERIALIZER_H

#include <QString>

class XmlSerializer
{
public:
    XmlSerializer(QString fileName);
    writeToFile();

private:
    QString fileName;

};

#endif // XMLSERIALIZER_H
