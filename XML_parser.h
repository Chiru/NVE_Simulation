/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef XML_PARSER_H
#define XML_PARSER_H


#include "utilities.h"
#include "ApplicationProtocol.h"
#include "DataGenerator.h"
#include "Messages.h"
#include "StatisticsCollector.h"
#include <string>
#include <vector>
#include <typeinfo>



class XMLParser{

     struct Client{
        uint16_t clientNumber;
        double uplink;
        double downlink;
        double loss;
        int delay;
    };

public:
    XMLParser(std::string filename);
    ~XMLParser();

    bool isFileCorrect(){return correctFile;}
    uint16_t getNumberOfClients()const {return numberOfClients;}
    bool getStreams(DataGenerator**&, bool client);
    uint16_t getNumberOfStreams()const {return numberOfStreams;}
    bool getApplicationProtocol(ApplicationProtocol*&);
    bool getClientStats(uint16_t clientIndex, uint16_t &clientNumber, int &delay, double &uplink, double &downlink, double &loss);

private:
    bool parseClients(std::string& file);
    bool parseStreams(std::string& file);
    bool parseStream(std::string& streamElement, DataGenerator*& stream);
    bool parseMessages(std::string& messagesElement, std::vector<Message*>& messages);
    bool parseApplicationProtocol(std::string& file);
    uint16_t countStreams(std::string& file);
    template <class T> bool readValue(const std::string& file, const std::string& variable, T& result, size_t position = 0);
    bool getRunningValue(const std::string& value, uint16_t &from, uint16_t &to);
    bool getElement(const std::string& file, size_t position,const  std::string& start, const std::string& end, std::string &result);

    std::string filename;
    bool correctFile;
    bool appProtoExists;
    ApplicationProtocol* appProto;
    DataGenerator **streams;
    uint16_t numberOfClients;
    uint16_t numberOfStreams;

    std::vector<struct XMLParser::Client*> clients;

    //std::ifstream &fileStream;

};

//class XMLParser function definitions

XMLParser::XMLParser(std::string filename): filename(filename), correctFile(true), appProto(0), streams(0), numberOfClients(0), numberOfStreams(0), clients(0){

    std::ifstream filestream(filename.c_str());
    std::string xmlFile;
    std::string token;

    if(filestream == 0){
        PRINT_ERROR( "XML file \"" << filename << "\" could not be opened." << std::endl);
        correctFile = false;
        return;
    }

    while(!filestream.eof()){
        filestream >> token;
        xmlFile.append(token);
    }

    filestream.close();

    correctFile = parseClients(xmlFile);
    correctFile = parseApplicationProtocol(xmlFile);
    correctFile = parseStreams(xmlFile);

}

XMLParser::~XMLParser(){

    std::vector<XMLParser::Client*>::iterator it = clients.begin();
    while(it != clients.end()){
        delete (*it);
        it++;
    }

    for(int i = 0; i < numberOfStreams; i++){
        if(streams[i] != 0)
            delete streams[i];
    }

    if(streams != 0)
         delete[] streams;

    if(appProto != 0){
        delete appProto;
    }
}

bool XMLParser::getElement(const std::string &file, size_t position, const std::string &start, const std::string &end, std::string &result){

    size_t end_position;
    end_position = file.find(end, position);

    if((position = file.find(start, position)) == std::string::npos)
        return false;

    if(end_position == std::string::npos){
        return false;
    }

    if(file.find(start, position+1) < end_position){
        return false;
    }

    result = file.substr(position, end_position - position +1);

    return true;
}

template <class T> bool XMLParser::readValue(const std::string &file, const std::string &variable, T &result, size_t position){

    std::string tempVariable = "";
    std::stringstream stream;
    size_t variable_begin;
    tempVariable.append(variable);
    tempVariable.append("=\"");

    variable_begin = file.find(tempVariable, position);
    if(variable_begin == std::string::npos)
        return false;

    variable_begin += tempVariable.length();
    while(file.at(variable_begin) != '\"'){
        stream << file.at(variable_begin);
        variable_begin++;
    }

    stream >> result;
    PRINT_INFO(tempVariable <<  " " <<  result << std::endl);

    if(stream.fail())
        return false;

    return true;
}

bool XMLParser::getRunningValue(const std::string &value, uint16_t &from, uint16_t &to){

    std::stringstream stream;
    char delim;
    stream << value;

    if(value.find(":") != std::string::npos){
        stream >> from >> delim >> to;
        if(stream.fail())
           return false;
    }else{
        stream >> from;
        to = from;
        if(stream.fail())
            return false;
    }

    return true;
}

bool XMLParser::parseClients(std::string &file){

    size_t latest_token = 0, temp_position = 0;
    std::string token;
    std::string value;
    XMLParser::Client* tempClient;

    int count = 0;
    uint16_t from = 0, to = 0;

    if((temp_position = file.find("<clients>")) == std::string::npos){
        PRINT_ERROR( "Incorrect format in XML file: no <clients> tag found" << std::endl);
        return false;
    }

    while((temp_position = file.find("<client>", temp_position+1)) != std::string::npos){

        if(!getElement(file, temp_position, "<client>", "</client>", token)){
            PRINT_ERROR( "Incorrect format in client specifications" << std::endl);
            return false;
        }

        latest_token = token.find("no=");
        value = "";
        if(readValue<std::string>(token, "no", value, latest_token)){

            if(getRunningValue(value, from, to) && to-from+1 > 0){
                count += to-from+1;

                for(int i = 0; i < to-from+1; i++){
                    tempClient = new XMLParser::Client();
                    clients.push_back(tempClient);

                    tempClient->clientNumber = from + i;

                    if(!readValue<int>(token, "delay", tempClient->delay, latest_token)){
                        PRINT_ERROR( "Incorrect format in client parameters" << std::endl);
                        return false;
                    }


                    if(!readValue<double>(token, "uplink", tempClient->uplink, latest_token)){
                        PRINT_ERROR( "Incorrect format in client parameters" << std::endl);
                        return false;
                    }


                    if(!readValue<double>(token, "downlink", tempClient->downlink, latest_token)){
                        PRINT_ERROR( "Incorrect format in client parameters" << std::endl);
                        return false;
                    }


                    if(!readValue<double>(token, "loss", tempClient->loss, latest_token)){
                        PRINT_ERROR( "Incorrect format in client parameters" << std::endl);
                        return false;
                    }

                }

            }else{
                PRINT_ERROR( "Incorrect format in XML file." << value << std::endl);
                return false;
            }
        }else{
            PRINT_ERROR( "Incorrect format in XML file." << value << std::endl);
            return false;
        }



    }

    if((latest_token = file.find("</clients>", latest_token)) == std::string::npos){
        PRINT_ERROR( "Incorrect format in XML file: no </clients> tag found or clients defined after it" << std::endl);
        return false;
    }

    if(count == 0){
        PRINT_ERROR( "No clients specified" << std::endl);
        return false;
    }

    numberOfClients = count;

    return true;
}

uint16_t XMLParser::countStreams(std::string &file){

    uint16_t count;
    size_t position = 0;

    for(count = 0;(position = file.find("<stream>", position+1)) != std::string::npos; count++);

    return count;

}

bool XMLParser::parseStream(std::string &streamElement, DataGenerator* &stream){

    int stream_number;
    DataGenerator::Protocol proto;
    ApplicationProtocol* appProto;
    uint16_t position;
    std::string type;
    std::string nagle("");
    std::string useAppProto("");
    std::string messagesElement("");
    std::vector<Message*> messages;

    if(!readValue<int>(streamElement, "no", stream_number, 0)){
        PRINT_ERROR( "No stream number specified." << std::endl);
        return false;
    }

    if(!readValue<std::string>(streamElement, "type", type, 0)){
        PRINT_ERROR( "No stream type specified in stream number: " << stream_number << std::endl);
        return false;
    }

    readValue<std::string>(streamElement, "nagle", nagle, 0);
    if(nagle.compare("yes") == 0 && type.compare("tcp") == 0){
        proto = DataGenerator::TCP_NAGLE_ENABLED;
    }else if(type.compare("tcp") == 0){
        proto = DataGenerator::TCP_NAGLE_DISABLED;
    }else if(type.compare("udp") == 0)
        proto = DataGenerator::UDP;
    else {
        PRINT_ERROR( "Invalid type in stream number: " << stream_number << std::endl);
    }

    readValue<std::string>(streamElement, "appproto", useAppProto, 0);

    if(useAppProto.compare("yes")== 0 && type.compare("udp") == 0){
        if(!getApplicationProtocol(appProto)){
            appProto = 0;
        }
    }else appProto = 0;

    if((position = streamElement.find("<messages>")) == std::string::npos){
        PRINT_ERROR( "No messages specified in stream number " << stream_number << std::endl);
        delete appProto;
        return false;
    }

    if(!getElement(streamElement, position, "<messages>", "</messages>", messagesElement)){
        PRINT_ERROR( streamElement << std::endl);
        PRINT_ERROR( "Incorrect format in message specification in stream number " << stream_number << std::endl);
        delete appProto;
        return false;
    }

    if(!parseMessages(messagesElement, messages)){
        PRINT_ERROR( "Incorrect format in message specifications." << std::endl);
        delete appProto;
        for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
            delete *it;
        }
        return false;
    }


    stream = new ClientDataGenerator(stream_number, proto, appProto, messages);

    return true;
}

bool XMLParser::parseStreams(std::string &file){

    size_t latest_token = 0, temp_position = 0;
    int count = 0;
    std::string streams;
    std::string streamElement;

    if((latest_token = file.find("<streams>")) == std::string::npos){
        PRINT_ERROR( "Incorrect format in XML file: no <streams> tag found" << std::endl);
        return false;
    }

    if(!getElement(file, latest_token, "<streams>", "</streams>", streams)){
        PRINT_ERROR( "Incorrect format in streams specifications" << std::endl);
        return false;
    }

    numberOfStreams = countStreams(streams);
    this->streams = new DataGenerator*[numberOfStreams];

    for(int i = 0; i < numberOfStreams; i++){
        this->streams[i] = 0;
    }

    latest_token = 0;

    while((temp_position = streams.find("<stream>", latest_token+1)) != std::string::npos){
        latest_token = temp_position;
        if(!getElement(streams, latest_token, "<stream>", "</stream>", streamElement)){
            PRINT_ERROR( "Incorrect format in stream specifications." << std::endl);
            return false;
        }

        if(!parseStream(streamElement, this->streams[count])){
            PRINT_ERROR( "Incorrect format in stream specification." << std::endl);
            return false;
        }

        count++;

    }

    if((latest_token = file.find("</streams>", latest_token)) == std::string::npos){
        PRINT_ERROR( "Incorrect format in XML file: no </streams> tag found or streams defined after it" << std::endl);
        return false;
    }

    if(count == 0){
        PRINT_ERROR( "No streams specified" << std::endl);
        return false;
    }

    return true;

}

bool XMLParser::parseMessages(std::string &messagesElement, std::vector<Message*> &messages){

    size_t latest_token = 0;
    std::string messageElement("");
    std::string type, reliable, name;
    int size, timeInterval, timeRequirement;
    double clientsOfInterest;

    if((latest_token = messagesElement.find("<message>")) == std::string::npos){
        PRINT_ERROR( "Error in message specifications" << std::endl);
        return false;
    }

    while(getElement(messagesElement, latest_token, "<message>", "</message>", messageElement)){

        if(!readValue<std::string>(messageElement, "type", type, 0)){
            PRINT_ERROR( "Error in message type specification." << std::endl);
            return false;
        }

        if(!readValue<std::string>(messageElement, "name", name, 0)){
            PRINT_ERROR( "No message name specified." << std::endl);
            return false;
        }

        if(!readValue<std::string>(messageElement, "reliable", reliable, 0)){
            PRINT_ERROR( "Error in reliability specification." << std::endl);
            return false;
        }

        if(!readValue<int>(messageElement, "size", size, 0)){
            PRINT_ERROR( "Error in message size specification." << std::endl);
            return false;
        }

        if(!readValue<int>(messageElement, "timeinterval", timeInterval, 0)){
            PRINT_ERROR( "Error in message timeinterval specification." << std::endl);
            return false;
        }

        if(size <= 0){
            PRINT_ERROR( "Message size must be more than 0." << std::endl);
            return false;
        }

        if(timeInterval <= 0){
            PRINT_ERROR( "TimeInterval must be more than 0." << std::endl);
            return false;
        }


        if(type.compare("uam") == 0){

            if(!readValue<int>(messageElement, "timerequirement", timeRequirement, 0)){
                PRINT_ERROR( "Error in message timerequirement specification." << std::endl);
                return false;
            }

            if(timeRequirement <= 0){
                PRINT_ERROR( "TimeRequirement must be more than 0." << std::endl);
                return false;
            }

            if(!readValue<double>(messageElement, "clientsofinterest", clientsOfInterest, 0)){
                PRINT_ERROR( "Error in message clients of interest specification." << std::endl);
                return false;
            }

            if(clientsOfInterest < 0 || clientsOfInterest > 100){
                PRINT_ERROR( "ClientsOfInterest must be between 0 and 100." << std::endl);
                return false;
            }

            messages.push_back(new UserActionMessage(name, reliable.compare("no") == 0 ? false : true, timeInterval, size, clientsOfInterest, timeRequirement));

        }

        if(type.compare("odt") == 0){

            //nothing special here yet
        }

        if(type.compare("mm") == 0){

            messages.push_back(new MaintenanceMessage(name, reliable.compare("no") == 0 ? false : true, timeInterval, size));

        }

        if((latest_token = messagesElement.find("</message>", latest_token)) == std::string::npos){
            PRINT_ERROR( "Error in messages specification: missing </message> tag." << std::endl);
            return false;
        }

        latest_token++;

    }

    return true;

}

bool XMLParser::parseApplicationProtocol(std::string &file){

    size_t position;
    std::string token, value;
    int acksize, delack, retransmit;

    if((position = file.find("<appproto>")) == std::string::npos){
        appProto = 0;
        PRINT_ERROR( "No application protocol found" << std::endl);
        return true;
    }

    if(!getElement(file, position, "<appproto>", "</appproto>", token)){
        PRINT_ERROR( "Incorrect format in application protocol specifications." << std::endl);
        return false;
    }

    value = "";

    if(!readValue<int>(token, "acksize", acksize)){
        PRINT_ERROR( "Incorrect format in application protocol parameter: acksize" << std::endl);
        return false;
    }

    if(!readValue<int>(token, "delayedack", delack)){
        PRINT_ERROR( "Incorrect format in application protocol parameter: delayedack" << std::endl);
        return false;
    }

    if(!readValue<int>(token, "retransmit", retransmit)){
        PRINT_ERROR( "Incorrect format in application protocol parameter: retransmit" << std::endl);
        return false;
    }

    appProto = new ApplicationProtocol(acksize, delack, retransmit);

    return true;

}

bool XMLParser::getStreams(DataGenerator** &streams, bool isClient){

    streams = new DataGenerator*[numberOfStreams];

    for(int i = 0; i < numberOfStreams; i++)
        streams[i] = 0;

    if(this->streams[0] != 0 && isClient){
        for(int i = 0; i < numberOfStreams; i++)
            streams[i] = new ClientDataGenerator(*(this->streams[i]));
    }
    else if(this->streams[0]){
        for(int i = 0; i < numberOfStreams; i++){
            streams[i] = new ServerDataGenerator(*(this->streams[i]));
        }
    }else return false;

    return true;
}


bool XMLParser::getClientStats(uint16_t clientIndex, uint16_t &clientNumber, int &delay, double &uplink, double &downlink, double &loss){

    std::vector<XMLParser::Client*>::iterator it;
    int i;

    for(it = clients.begin(), i = 1; it != clients.end(); it++, i++){
        if(i == clientIndex){
            clientNumber = (*it)->clientNumber;
            delay = (*it)->delay;
            uplink = (*it)->uplink;
            downlink = (*it)->downlink;
            loss = (*it)->loss;
            break;
        }
        if(it == clients.end())
            return false;
    }

    return true;

}


 bool XMLParser::getApplicationProtocol(ApplicationProtocol* &proto){

    if(appProto == 0)
        return false;

    else
        proto = new ApplicationProtocol(*appProto);

    return true;

}

#endif // XML_PARSER_H
