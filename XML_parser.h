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


#include "ApplicationProtocol.h"
#include <string>
#include <vector>
#include "DataGenerator.h"


class XMLParser{

     struct Client{
        uint16_t clientNumber;
        double uplink;
        double downlink;
        double loss;
        int delay;
    };

public:
    XMLParser(std::string& filename);
    ~XMLParser();

    bool isFileCorrect(){return correctFile;}
    uint16_t getNumberOfClients()const {return numberOfClients;}
    bool getStreams(DataGenerator**&);
    uint16_t getNumberOfStreams()const {return numberOfStreams;}
    bool getApplicationProtocol(ApplicationProtocol*);
    bool getAppProtoPacketSize(uint16_t &size);
    bool getAppProtoDelayedAck(uint16_t &ack);
    bool getAppProtoRetransmit(uint16_t &retransmit);
    bool getClientStats(uint16_t clientIndex, uint16_t &clientNumber, int &delay, double &uplink, double &downlink, double &loss);

private:
    bool parseClients(std::string& file);
    bool parseStreams(std::string& file);
    template <class T> bool readValue(const std::string& file, const std::string& variable, T& result, size_t position = 0);
    bool getRunningValue(const std::string& value, uint16_t &from, uint16_t &to);
    bool getElement(const std::string& file, size_t position,const  std::string& start, const std::string& end, std::string &result);

    std::string filename;
    std::string file;
    bool correctFile;
    ApplicationProtocol* appProto;
    uint16_t numberOfClients;
    uint16_t numberOfStreams;

    std::vector<struct XMLParser::Client*> clients;

    //std::ifstream &fileStream;

};

//class XMLParser function definitions

XMLParser::XMLParser(std::string& filename): filename(filename), correctFile(true), appProto(0), clients(0){

    std::ifstream filestream(filename.c_str());
    std::string xmlFile;
    std::string token;

    if(filestream == 0)
        std::cerr << "XML file could not be opened" << std::endl;

    while(!filestream.eof()){
        filestream >> token;
        xmlFile.append(token);
    }

    //std::cout << xmlFile << std::endl;  //TODO: remove when not needed anymore
    filestream.close();

    correctFile = parseClients(xmlFile);
    correctFile = parseStreams(xmlFile);

}

XMLParser::~XMLParser(){

    std::vector<XMLParser::Client*>::iterator it = clients.begin();
    while(it != clients.end()){
        std::cout << (*it)->clientNumber <<" "<< (*it)->delay << " "<<(*it)->uplink << " " << (*it)->downlink <<" "<< (*it)->loss << std::endl;
        delete (*it);
        it++;
    }

}

bool XMLParser::getElement(const std::string &file, size_t position, const std::string &start, const std::string &end, std::string &result){

    size_t end_position;
    end_position = file.find(end, position);

    if(end_position == std::string::npos){
        return false;
    }

    if(file.find(start, position+1) < end_position){
        return false;
    }

    result = file.substr(position, end_position - position +1);

    std::cout << result << std::endl;
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
    std::stringstream stream;
    XMLParser::Client* tempClient;

    int count = 0;
    uint16_t from = 0, to = 0;

    if((temp_position = file.find("<clients>")) == std::string::npos){
        std::cerr << "Incorrect format in XML file: no <clients> tag found" << std::endl;
        return false;
    }

    while((temp_position = file.find("<client>", temp_position+1)) != std::string::npos){

        if(!getElement(file, temp_position, "<client>", "</client>", token)){
            std::cerr << "Incorrect format in client specifications" << std::endl;
            return false;
        }

        latest_token = token.find("no=");
        value = "";
        if(readValue<std::string>(token, "no", value, latest_token)){

            if(getRunningValue(value, from, to) && to-from+1 > 0){
                count += to-from+1;

                for(int i = 0; i < to-from+1; i++){
                    tempClient = new XMLParser::Client();
                    tempClient->clientNumber = from + i;

                    if(!readValue<int>(token, "delay", tempClient->delay, latest_token)){
                        std::cerr << "Incorrect format in client parameters" << std::endl;
                        return false;
                    }


                    if(!readValue<double>(token, "uplink", tempClient->uplink, latest_token)){
                        std::cerr << "Incorrect format in client parameters" << std::endl;
                        return false;
                    }


                    if(!readValue<double>(token, "downlink", tempClient->downlink, latest_token)){
                        std::cerr << "Incorrect format in client parameters" << std::endl;
                        return false;
                    }


                    if(!readValue<double>(token, "loss", tempClient->loss, latest_token)){
                        std::cerr << "Incorrect format in client parameters" << std::endl;
                        return false;
                    }


                    clients.push_back(tempClient);
                }

            }else{
                std::cerr << "Incorrect format in XML file." << value << std::endl;
                return false;
            }
        }else{
            std::cerr << "Incorrect format in XML file." << value << std::endl;
            return false;
        }



    }

    if((latest_token = file.find("</clients>", latest_token)) == std::string::npos){
        std::cerr << "Incorrect format in XML file: no </clients> tag found or clients defined after it" << std::endl;
        return false;
    }

    if(count == 0){
        std::cerr << "No clients specified" << std::endl;
        return false;
    }

    numberOfClients = count;

    return true;
}

bool XMLParser::parseStreams(std::string &file){

    size_t latest_token = 0, temp_position = 0;
    int count = 0;

    if((latest_token = file.find("<streams>")) == std::string::npos){
        std::cerr << "Incorrect format in XML file: no <streams> tag found" << std::endl;
        return false;
    }

    while((temp_position = file.find("<stream>", latest_token+1)) != std::string::npos){
        latest_token = temp_position;
        if((file.find("</stream>", latest_token)) != std::string::npos){
                count++;
        }
    }

    if((latest_token = file.find("</streams>", latest_token)) == std::string::npos){
        std::cerr << "Incorrect format in XML file: no </streams> tag found or streams defined after it" << std::endl;
        return false;
    }

    if(count == 0){
        std::cerr << "No streams specified" << std::endl;
        return false;
    }

    numberOfStreams = count;

    return true;

}

bool XMLParser::getStreams(DataGenerator** &streams){

    streams = new DataGenerator*[numberOfStreams];
    for(int i = 0; i < numberOfStreams; i++)
        streams[i] = 0;

    return true;
}

bool XMLParser::getAppProtoPacketSize(uint16_t &size){

    return true; //TODO: hard-coding, parse actual file
}

bool XMLParser::getAppProtoDelayedAck(uint16_t &ack){

    return true; //TODO: hard-coding, parse actual file

}

bool XMLParser::getAppProtoRetransmit(uint16_t &retransmit){

    return true; //TODO: hard-coding, parse actual file

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


 bool XMLParser::getApplicationProtocol(ApplicationProtocol* proto){

    uint16_t size, ack, retransmit;

    if(appProto == 0){
        if(getAppProtoPacketSize(size) && getAppProtoDelayedAck(ack) && getAppProtoRetransmit(retransmit)){
            appProto = new ApplicationProtocol(size, ack, retransmit);
        }else{
            correctFile = false;
            std::cerr << "XML file format not correct, application protocol fields incorrect." << std::endl;
            return false;
        }

    }

    proto = new ApplicationProtocol(*appProto);

    return true;

}

#endif // XML_PARSER_H
