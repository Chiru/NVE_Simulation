#include "XmlSerializer.h"

XmlSerializer::XmlSerializer(QString fileName)
    : fileName(fileName)
{

}


void XmlSerializer::writeToFile()
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


QString XmlValue::getElementString(int intend) const
{
    QString result("");

    for(int i = 0; i < intend; i++)
        result.append("\t");

    result.append("<" + elementName + "\"" + value + "\"/\n");

    return result;
}


