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
#include "ns3/random-variable.h"
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
    bool getStreams(DataGenerator**&, bool client, uint16_t clientNumber = 0);
    uint16_t getNumberOfStreams()const {return numberOfStreams;}
    bool getApplicationProtocol(ApplicationProtocol*&);
    bool getClientStats(uint16_t clientIndex, uint16_t &clientNumber, int &delay, double &uplink, double &downlink, double &loss);
    uint16_t getRunningTime() const{return runningTime;}

private:

    typedef union{
        double doubleVal;
        long longIntVal;
        uint32_t uintVal;
    } RanvarValues;   //these are the different variables that are passed as a parameter to ns-3 distribution functions

    //distributions supported by ns-3
    typedef enum{Uniform = 0, Constant, Sequential, Exponential, Pareto, Weibull, Normal, Lognormal, Gamma, Erlang, Zipf, Zeta, Triangular, Empirical, Extreme, Split, None} Distributions;


    //convenience class to operate with distribution enum types
    class DistributionEnum{

    public:
        DistributionEnum();
        explicit DistributionEnum(Distributions dist, XMLParser* owner = 0);
        explicit DistributionEnum(int dist, XMLParser* owner = 0);

        DistributionEnum(const std::string& distStr, XMLParser* owner = 0);
        operator int() const;
        bool operator==(const Distributions &d);
        bool operator<(const Distributions &d);
        bool operator>(const Distributions &d);
        void setDistribution(Distributions dist);
        Distributions getDistribution() const;
        Distributions getDistribution(int i) const;
        RandomVariable* constructRandomVariable(const std::string& params, const std::string& fileName);

    private:
        Distributions distribution;
        static const std::string distributionStrings[];
        static const int distCount;

        template <class T, class U, class V, class W> bool readCommaSeparatedString(const std::string& csvString, int paramCount, T& val1, U& val2, V& val3, W& val4);
        bool readEmpiricalDataFile(const std::string& filename, EmpiricalVariable& ranvar);
        RandomVariable* readSplitDistribution(const std::string& params);
        bool readAndRemovePercentage(std::string& dist, double& result);

        XMLParser* owner;

    }; //end of nested class


    bool parseClients(std::string& file);
    bool parseStreams(std::string& file);
    bool parseStream(std::string& streamElement, DataGenerator*& clientStream, DataGenerator*& serverStream);
    bool parseMessages(std::string& messagesElement, std::vector<Message*>& messages, uint16_t stream_number);
    bool parseApplicationProtocol(std::string& file);
    bool parseGameTick(std::string& streamElement, int& serverGameTick, int& clientGameTick);
    bool parseRunningTime(std::string& file);
    uint16_t countStreams(std::string& file);
    template <class T> bool readValue(const std::string& file, const std::string& variable, T& result, size_t position = 0);
    bool getRunningValue(const std::string& value, uint16_t &from, uint16_t &to);
    bool getElement(const std::string& file, size_t position,const  std::string& start, const std::string& end, std::string &result);
    bool readRandomVariable(const std::string& element, RandomVariable*& ranvar, DistributionEnum& distribution, const std::string& variableName);

    std::string filename;
    bool correctFile;
    bool appProtoExists;
    ApplicationProtocol* appProto;
    DataGenerator **clientStreams;
    DataGenerator **serverStreams;
    uint16_t numberOfClients;
    uint16_t numberOfStreams;
    int serverGameTick;
    int clientGameTick;
    int runningTime;

    std::vector<struct XMLParser::Client*> clients;

};



//class XMLParser function definitions

XMLParser::XMLParser(std::string filename): filename(filename), correctFile(true), appProto(0), clientStreams(0),serverStreams(0), numberOfClients(0), numberOfStreams(0), clients(0){

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

    if(!(correctFile = parseClients(xmlFile)))
        return;

    if(!(correctFile = parseApplicationProtocol(xmlFile)))
        return;

    if(!(correctFile = parseStreams(xmlFile)))
        return;


    if(!(correctFile = parseRunningTime(xmlFile)))
        return;
}

XMLParser::~XMLParser(){

    std::vector<XMLParser::Client*>::iterator it = clients.begin();
    while(it != clients.end()){
        delete (*it);
        it++;
    }

    for(int i = 0; i < numberOfStreams; i++){
        delete clientStreams[i];
      //  delete serverStreams[i];
    }

    delete[] serverStreams;
    delete[] clientStreams;

    delete appProto;

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

bool XMLParser::parseStream(std::string &streamElement, DataGenerator* &clientStream, DataGenerator* &serverStream){

    static uint16_t stream_number = 0;
    DataGenerator::Protocol proto = DataGenerator::UDP;
    ApplicationProtocol* appProto;
    uint16_t position;
    std::string type;
    std::string nagle("");
    std::string useAppProto("");
    std::string messagesElement("");
    std::vector<Message*> messages;
    int serverGameTick = 0, clientGameTick = 0;

    stream_number++;

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


    if(!parseGameTick(streamElement, serverGameTick, clientGameTick)){
        delete appProto;
        return false;
    }

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

    if(!parseMessages(messagesElement, messages, stream_number)){
        PRINT_ERROR( "Incorrect format in message specifications." << std::endl);
        delete appProto;
        for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
            delete *it;
        }
        return false;
    }

    clientStream = new ClientDataGenerator(stream_number, proto, appProto, messages, clientGameTick);
    serverStream = new ServerDataGenerator(stream_number, proto, appProto, messages, serverGameTick);

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
    this->clientStreams = new DataGenerator*[numberOfStreams];
    this->serverStreams = new DataGenerator*[numberOfStreams];

    for(int i = 0; i < numberOfStreams; i++){
        this->clientStreams[i] = 0;
        this->serverStreams[i] = 0;
    }

    latest_token = 0;

    while((temp_position = streams.find("<stream>", latest_token+1)) != std::string::npos){
        latest_token = temp_position;
        if(!getElement(streams, latest_token, "<stream>", "</stream>", streamElement)){
            PRINT_ERROR( "Incorrect format in stream specifications." << std::endl);
            return false;
        }

        if(!parseStream(streamElement, this->clientStreams[count], this->serverStreams[count])){
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

bool XMLParser::parseMessages(std::string &messagesElement, std::vector<Message*> &messages, uint16_t stream_number){

    size_t latest_token = 0;
    std::string messageElement("");
    std::string type, reliable, name, forwardBack;
    int size = 0, timeInterval = 0, clientTimeRequirement = 0, serverTimeRequirement = 0, forwardSize = 0;
    double clientsOfInterest;
    RandomVariable* ranvarTimeInterval = 0;
    RandomVariable* ranvarSize = 0;
    RandomVariable* ranvarForwardSize = 0;
    DistributionEnum distribution;

    if((latest_token = messagesElement.find("<message>")) == std::string::npos){
        PRINT_ERROR( "Error in message specifications" << std::endl);
        return false;
    }

    while(getElement(messagesElement, latest_token, "<message>", "</message>", messageElement)){

        ranvarTimeInterval = ranvarSize = ranvarForwardSize = 0;

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

        if(!readRandomVariable(messageElement, ranvarSize, distribution, "size")){                  //if no distribution is specified, read simple timeinterval
            if(!readValue<int>(messageElement, "size", size, 0)){
                PRINT_ERROR( "Error in message size specification." << std::endl);
                return false;
            }
       }

        if(!readRandomVariable(messageElement, ranvarTimeInterval, distribution, "timeinterval")){          //if no distribution is specified, read simple timeinterval
            if(!readValue<int>(messageElement, "timeinterval", timeInterval, 0)){
                PRINT_ERROR( "Error in message timeinterval specification." << std::endl);
                return false;
            }
        }

        if(!readValue<std::string>(messageElement, "forwardback", forwardBack, 0)){
            PRINT_ERROR( "Error in forwardback specification." << std::endl);
            return false;
        }

        if(!readRandomVariable(messageElement, ranvarForwardSize, distribution, "forwardmessagesize")){                  //if no distribution is specified, read simple timeinterval
            if(!readValue<int>(messageElement, "forwardmessagesize", forwardSize, 0)){
                PRINT_ERROR( "Error in message forwardmessagesize specification." << std::endl);
                return false;
            }

        }

        if(size <= 0 && ranvarSize == 0){
            PRINT_ERROR( "Message size must be more than 0 ot distribution must be specified." << std::endl);
            return false;
        }

        if(timeInterval <= 0 && ranvarTimeInterval == 0){
            PRINT_ERROR( "TimeInterval value must be either more than 0 or distribution must be specified." << std::endl);
            return false;
        }

        if(!readValue<int>(messageElement, "timerequirementclient", clientTimeRequirement, 0)){
            PRINT_ERROR( "Error in message timerequirementclient specification." << std::endl);
            return false;
        }

        if(type.compare("uam") == 0){


            if(!readValue<int>(messageElement, "timerequirementserver", serverTimeRequirement, 0)){
                PRINT_ERROR( "Error in message timerequirementserver specification." << std::endl);
                return false;
            }

            if(serverTimeRequirement <= 0 || clientTimeRequirement <= 0){
                PRINT_ERROR( "TimeRequirement must be more than 0." << std::endl);
                return false;
            }

            if(!readValue<double>(messageElement, "clientsofinterest", clientsOfInterest, 0)){
                PRINT_ERROR( "Error in message clients of interest specification." << std::endl);
                return false;
            }

            if(clientsOfInterest < 0 || clientsOfInterest > 1){
                PRINT_ERROR( "ClientsOfInterest must be between 0 and 1." << std::endl);
                return false;
            }

           messages.push_back(new UserActionMessage(name, reliable.compare("no") == 0 ? false : true, timeInterval, size, clientsOfInterest, clientTimeRequirement,
                                                         serverTimeRequirement, stream_number, forwardSize, forwardBack.compare("no") == 0 ? false : true,
                                                         ranvarTimeInterval, ranvarSize, ranvarForwardSize));
        }

        else if(type.compare("odt") == 0){

            if(clientTimeRequirement <= 0){
                PRINT_ERROR( "TimeRequirement must be more than 0." << std::endl);
                return false;
            }

           messages.push_back(new OtherDataMessage(name, reliable.compare("no") == 0 ? false : true, timeInterval, size, stream_number, forwardSize,
                                                   forwardBack.compare("no") == 0 ? false : true, clientTimeRequirement, ranvarTimeInterval, ranvarSize, ranvarForwardSize));
        }

        else{
            PRINT_ERROR("Unknown message type: " << type <<std::endl);
            return false;
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
    int acksize, delack, retransmit, headerSize;

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

    if(!readValue<int>(token, "headersize", headerSize)){
        PRINT_ERROR("Incorrect format in application protocol parameter: headersize" << std::endl);
        return false;
    }

    appProto = new ApplicationProtocol(acksize, delack, retransmit, headerSize);

    return true;

}

bool XMLParser::getStreams(DataGenerator** &streams, bool isClient, uint16_t clientNumber){

    streams = new DataGenerator*[numberOfStreams];

    for(int i = 0; i < numberOfStreams; i++)
        streams[i] = 0;

    if(this->clientStreams[0] != 0 && isClient){
        for(int i = 0; i < numberOfStreams; i++){
            ((ClientDataGenerator*)this->clientStreams[i])->setClientNumber(clientNumber);
            streams[i] = new ClientDataGenerator(*(this->clientStreams[i]));
        }
    }
    else if(this->serverStreams[0]){
        for(int i = 0; i < numberOfStreams; i++){
            streams[i] = new ServerDataGenerator(*(this->serverStreams[i]));
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

bool XMLParser::parseGameTick(std::string& streamElement, int& serverGameTick, int& clientGameTick){

    if(!readValue<int>(streamElement, "<servergametick", serverGameTick) || serverGameTick < 0){
        PRINT_ERROR(" Incorrect servergametick value." << std::endl);
        return false;
    }

    if(!readValue<int>(streamElement, "<clientgametick", clientGameTick) || clientGameTick < 0){
        PRINT_ERROR(" Incorrect clientgametick value." << std::endl);
        return false;
    }

    return true;
}

bool XMLParser::parseRunningTime(std::string &file){

    if(!readValue<int>(file, "<runningtime", runningTime) || runningTime <= 0){
        PRINT_ERROR(" Incorrect runningtime value." << std::endl);
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

 bool XMLParser::readRandomVariable(const std::string& element, RandomVariable*& ranvar, DistributionEnum& distribution, const std::string& variableName){

     std::string result;
     std::string filename("");
     if(variableName.empty()){
        result = element;
     }else{
        readValue<std::string>(element, variableName, result, 0);
     }

     std::string distName(result.substr(0, result.find('(')));

     if(distName.compare("empirical") == 0){
         getElement(result, 0, "(", ")", filename);
         filename = filename.substr(1, filename.length()-2);
     }

     distribution = DistributionEnum(distName, this);
     if(distribution == None)
         return false;

     ranvar = distribution.constructRandomVariable(result.substr(distName.length(), result.length() - distName.length()), filename);
     if(ranvar == 0)
         return false;

     return true;
 }


 //nested class DistributionEnum definitions

 const int XMLParser::DistributionEnum::distCount = 17;
 const std::string XMLParser::DistributionEnum::distributionStrings[XMLParser::DistributionEnum::distCount] = {"uniform", "constant", "sequential", "exponential", "pareto", "weibull", "normal",
                                                                                                               "lognormal", "gamma", "erlang", "zipf", "zeta",
                                                                                                               "triangular", "empirical", "extreme", "split", "none"};

 XMLParser::DistributionEnum::DistributionEnum(): distribution(XMLParser::Uniform){}

 XMLParser::DistributionEnum::DistributionEnum(XMLParser::Distributions dist, XMLParser* owner) : owner(owner){
    setDistribution(dist);
 }

XMLParser::DistributionEnum::DistributionEnum(int dist, XMLParser* owner): owner(owner){
     setDistribution(getDistribution(dist));
 }

  XMLParser::DistributionEnum::DistributionEnum(const std::string& distStr, XMLParser* owner) : owner(owner){

      for(distribution = XMLParser::Uniform; this->operator<(XMLParser::None); distribution = getDistribution(((int)distribution+1))){
         if(distributionStrings[distribution].compare(distStr) == 0){
             break;
         }
     }
 }

  XMLParser::DistributionEnum::operator int() const{
     for(int i = 0; i < distCount; i++){
         if(i == this->distribution)
             return i;
     }
     return XMLParser::None;
 }

 bool  XMLParser::DistributionEnum::operator==(const XMLParser::Distributions &d){
     if(d == this->distribution)
         return true;
     else
         return false;
 }

 bool  XMLParser::DistributionEnum::operator<(const XMLParser::Distributions &d){
     if(d > distribution)
         return true;
     else
         return false;
 }

 bool  XMLParser::DistributionEnum::operator>(const XMLParser::Distributions &d){
     if(d < distribution)
         return true;
     else
         return false;
 }

 void  XMLParser::DistributionEnum::setDistribution(XMLParser::Distributions dist){
     distribution = dist;
 }

 XMLParser::Distributions  XMLParser::DistributionEnum::getDistribution() const{
     return distribution;
 }

 XMLParser::Distributions  XMLParser::DistributionEnum::getDistribution(int i) const{

     switch(i){
         case 0: return XMLParser::Uniform;
         case 1: return XMLParser::Constant;
         case 2: return XMLParser::Sequential;
         case 3: return XMLParser::Exponential;
         case 4: return XMLParser::Pareto;
         case 5: return XMLParser::Weibull;
         case 6: return XMLParser::Normal;
         case 7: return XMLParser::Lognormal;
         case 8: return XMLParser::Gamma;
         case 9: return XMLParser::Erlang;
         case 10: return XMLParser::Zipf;
         case 11: return XMLParser::Zeta;
         case 12: return XMLParser::Triangular;
         case 13: return XMLParser::Empirical;
         case 14: return XMLParser::Extreme;
         case 15: return XMLParser::Split;
         case 16: return XMLParser::None;
         default: return XMLParser::None;
     }
}

 template <class T, class U, class V, class W> bool XMLParser::DistributionEnum::readCommaSeparatedString(const std::string &csvString, int paramCount, T& val1, U& val2, V& val3, W& val4){

      std::stringstream stream;
      stream << csvString;

      char c, c2,c3;
      stream >> c;
      if(c != '(' || stream.fail())
          return false;

      switch(paramCount){
      case 1:
          stream >> val1;
          if(stream.fail())
              return false;
          break;
      case 2:
          stream >> val1;
          stream >> c;
          stream >> val2;
          if(stream.fail() || c != ',')
              return false;
          break;
      case 3:
          stream >> val1;
          stream >> c;
          stream >> val2;
          stream >> c2;
          stream >> val3;
          if(stream.fail() || c != ',' || c2 != ',')
              return false;
          break;
      case 4:
          stream >> val1;
          stream >> c;
          stream >> val2;
          stream >> c2;
          stream >> val3;
          stream >> c3;
          stream >> val4;
          if(stream.fail() || c != ',' || c2 != ',' || c3 != ',')
              return false;
          break;
      }

      stream >> c;

      if(c != ')' || stream.fail())
          return false;

     return true;
 }



RandomVariable* XMLParser::DistributionEnum::constructRandomVariable(const std::string &params, const std::string& fileName){

    RandomVariable* retVal = 0;
    XMLParser::RanvarValues v1, v2, v3, v4;

     switch(distribution){

     case XMLParser::Uniform:
         if(!readCommaSeparatedString<double, double, uint32_t, uint32_t>(params, 2, v1.doubleVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new UniformVariable(v1.doubleVal, v2.doubleVal);
         }
         break;
     case XMLParser::Constant:
         if(!readCommaSeparatedString<double, uint32_t, uint32_t, uint32_t>(params, 1, v1.doubleVal, v2.uintVal, v3.uintVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new ConstantVariable(v1.doubleVal);
         }
         break;
     case XMLParser::Sequential:
         if(!readCommaSeparatedString<double, double, double, uint32_t>(params, 4, v1.doubleVal, v2.doubleVal, v3.doubleVal, v3.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new SequentialVariable(v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.uintVal);
         }
         break;

     case XMLParser::Exponential:
         if(!readCommaSeparatedString<double, double, uint32_t, uint32_t>(params, 2, v1.doubleVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new ExponentialVariable(v1.doubleVal, v2.doubleVal);
         }
         break;
     case XMLParser::Pareto:
         if(!readCommaSeparatedString<double, double, double, uint32_t>(params, 3, v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new ParetoVariable(v1.doubleVal, v2.doubleVal, v3.doubleVal);
         }
         break;
     case XMLParser::Weibull:
         if(!readCommaSeparatedString<double, double, double, double>(params, 4, v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.doubleVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new WeibullVariable(v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.doubleVal);
         }
         break;
     case XMLParser::Normal:
         if(!readCommaSeparatedString<double, double, uint32_t, uint32_t>(params, 2, v1.doubleVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new NormalVariable(v1.doubleVal, v2.doubleVal);
         }
         break;
     case XMLParser::Lognormal:
         if(!readCommaSeparatedString<double, double, uint32_t, uint32_t>(params, 2, v1.doubleVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new LogNormalVariable(v1.doubleVal, v2.doubleVal);
         }
         break;
     case XMLParser::Gamma:
         if(!readCommaSeparatedString<double, double, uint32_t, uint32_t>(params, 2, v1.doubleVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new GammaVariable(v1.doubleVal, v2.doubleVal);
         }
         break;
     case XMLParser::Erlang:
         if(!readCommaSeparatedString<uint32_t, double, uint32_t, uint32_t>(params, 2, v1.uintVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new ErlangVariable(v1.uintVal, v2.doubleVal);
         }
         break;
     case XMLParser::Zipf:
         if(!readCommaSeparatedString<long, double, uint32_t, uint32_t>(params, 2, v1.longIntVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new ZipfVariable(v1.longIntVal, v2.doubleVal);
         }
         break;
     case XMLParser::Zeta:
         if(!readCommaSeparatedString<double, uint32_t, uint32_t, uint32_t>(params, 1, v1.doubleVal, v2.uintVal, v3.uintVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new ZetaVariable(v1.doubleVal);
         }
         break;
     case XMLParser::Triangular:
         if(!readCommaSeparatedString<double, double, double, uint32_t>(params, 3, v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new TriangularVariable(v1.doubleVal, v2.doubleVal, v3.doubleVal);
         }
         break;
         //TODO: get the empirical data from a file
     case XMLParser::Empirical:
         retVal = new EmpiricalVariable();

         if(!readEmpiricalDataFile(fileName, *((EmpiricalVariable*)retVal))){
             PRINT_ERROR("Couldn't read empirical values from file: " << fileName << std::endl);
             delete retVal;
             retVal = 0;
         }
         break;

     case XMLParser::Extreme:
         if(!readCommaSeparatedString<double, double, double, uint32_t>(params, 3, v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.uintVal)){
             PRINT_ERROR("Error in timeinterval distribution parameters" << std::endl);
         }else{
             retVal = new ExtremeVariable(v1.doubleVal, v2.doubleVal, v3.doubleVal);
         }
         break;

     case XMLParser::Split:
            retVal = readSplitDistribution(params);
         break;

     case XMLParser::None:
     default:
         return 0;
     }

     return retVal;
}

bool XMLParser::DistributionEnum::readEmpiricalDataFile(const std::string &filename, EmpiricalVariable &ranvar){
//data file must contain each the amount of packets for each x-value (time) {1, ... , n} where n is the max time value
    std::ifstream file(filename.c_str(), std::ios_base::in);

    if(file.fail())
        return false;

    char c;
    double yval;
    double xval = 1;
    double totalValues = 0;
    double valuesThisFar = 0;  //these exist in order to calculate the probabilities


    do{
        file >> yval;

        if(!file.eof() && !file.fail()){
            totalValues += yval;
        }

    }while(file >> c, !file.fail() && !file.eof());

    file.clear(std::ios_base::goodbit);

    file.seekg(std::ios_base::beg);

    do{
        file >> yval;
        valuesThisFar += yval;

        if(yval > 0){
            ranvar.CDF(xval, valuesThisFar/totalValues);
        }

        xval += 1;
    }while(file >> c, !file.fail() && !file.eof());

    file.close();
    return true;
}

RandomVariable* XMLParser::DistributionEnum::readSplitDistribution(const std::string& params){

    std::stringstream stream;
    stream << params;
    SplitDistribution* retVal = 0;
    RandomVariable* ranvar;
    DistributionEnum distribution;
    char c;
    size_t pos = 0;
    size_t previous_pos = 0;
    bool cont = true;
    std::string temp;
    int distCount = 0;
    double percentage = 0;

    stream >> c;

    if(c != '(' || stream.fail())
        return 0;

    retVal = new SplitDistribution();

    stream >> temp;

    std::string dist = "";

    while(cont){

        pos = temp.find_first_of(',', temp.find_first_of(')', pos));

        if(pos == std::string::npos){
            dist = temp.substr(previous_pos, temp.find_first_of(')', previous_pos)- (previous_pos - 1));
            cont = false;
        }else{
            dist = temp.substr(previous_pos, pos - previous_pos);
        }

        if(!readAndRemovePercentage(dist, percentage)){
            delete retVal;
            return 0;
        }

        owner->readRandomVariable(dist, ranvar, distribution, "");

        if(ranvar != 0){
            retVal->AddDistribution(*ranvar, percentage);
            distCount++;
        }

        previous_pos = pos + 1;

    }

    if(distCount == 0){
        delete retVal;
        retVal = 0;

    }

    return retVal;

}

bool XMLParser::DistributionEnum::readAndRemovePercentage(std::string &dist, double &result){

    std::stringstream stream;

    stream << dist;

    stream >> result;

    if(stream.fail())
        return false;

    char c;

    stream >> c;

    if(stream.fail() || c != ':')
        return false;

    stream >> dist;

    return true;

}


#endif // XML_PARSER_H
