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



#include "nve_simulator.h"
#include "XML_parser.h"
#include <fstream>
#include <sstream>
#include "ns3/point-to-point-helper.h"
#include "ns3/internet-stack-helper.h"


using namespace ns3;

//Class Client function definitions

Client::Client(XMLParser& parser, uint16_t no): parser(parser), streams(0){


   parser.getStreams(streams);

   if(!parser.getClientStats(no, clientNumber, networkDelay, uplinkBandwidth, downlinkBandwidth, lossRate))
       std::cerr << "Mysterious error while creating " << no << ". client." << std::endl;

}

Client::~Client(){

    for(int i = 0; i < parser.getNumberOfStreams(); i++){
        if(streams[i] != 0)
            delete streams[i];
    }

    if(streams != 0)
         delete[] streams;

}

std::string Client::getDelayInMilliseconds(){

    std::stringstream stream;

    stream << networkDelay << "ms";

    return stream.str();

}

//Class Server function definitions

Server::Server(XMLParser& parser): parser(parser){


}

Server::~Server(){

}


//Class DataGenerator function definitions

DataGenerator::DataGenerator(){


}

DataGenerator::~DataGenerator(){

}

void DataGenerator::StartApplication(){


}

void DataGenerator::StopApplication(){


}


//Class ClientDataGenerator function definitions

ClientDataGenerator::ClientDataGenerator(){


}

ClientDataGenerator::~ClientDataGenerator(){


}


//Class ServerDataGenerator function definitions

ServerDataGenerator::ServerDataGenerator(){


}

ServerDataGenerator::~ServerDataGenerator(){

}


//Class StatisticsCollector function definitions

StatisticsCollector::StatisticsCollector(){


}

StatisticsCollector::~StatisticsCollector(){

}


//Class ApplicationProtocol function definitions

ApplicationProtocol::ApplicationProtocol(uint16_t packetSize, uint16_t delayedAck, uint16_t retransmit)
    : packetSize(packetSize),
      delayedAck(delayedAck),
      retransmit(retransmit){


}

ApplicationProtocol::ApplicationProtocol(const ApplicationProtocol &appProto){




}

ApplicationProtocol::~ApplicationProtocol(){

}

std::string ApplicationProtocol::read(){

    return "test";

}

bool ApplicationProtocol::write(std::string &message){

    return true;

}

void ApplicationProtocol::readFromSocket(){

}

void ApplicationProtocol::writeToSocket(std::string &message){


}

void ApplicationProtocol::resend(){


}


//Class Message function definitions

Message::Message(){

}

Message::~Message(){

}


//Class UserActionMessage function definitions

UserActionMessage::UserActionMessage(){

}

UserActionMessage::~UserActionMessage(){

}

void UserActionMessage::startDataTransfer(){

}

void UserActionMessage::sendData(){

}


//Class OtherDataMessage function definitions


OtherDataMessage::OtherDataMessage(){


}

OtherDataMessage::~OtherDataMessage(){


}


void OtherDataMessage::startDataTransfer(){


}

void OtherDataMessage::sendData(){

}


//Class MaintenanceMessage function definitions


MaintenanceMessage::MaintenanceMessage(){


}

MaintenanceMessage::~MaintenanceMessage(){


}

void MaintenanceMessage::startDataTransfer(){


}

void MaintenanceMessage::sendData(){


}


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

int main(int argc, char** argv){

    int i;

    std::string XML_filename = "scratch/xmltest.txt";
    XMLParser parser = XMLParser(XML_filename);
    if(!parser.isFileCorrect()){
        std::cerr << "Terminating due to a incorrect XML format" << std::endl;
        //TODO: terminate simulation
    }

    uint16_t numberOfClients = parser.getNumberOfClients();

    Server server = Server(parser);
    Client* clients[numberOfClients];

    for(uint16_t i = 0; i < numberOfClients; i++){
        clients[i] = new Client(parser, i+1);
    }

    NodeContainer allNodes;
    allNodes.Create(numberOfClients +2);   //a node for each client, one for the router and one for the server

    NodeContainer clientRouterNodes[numberOfClients];

    for(int i = 0; i < numberOfClients; i++){
        clientRouterNodes[i] = NodeContainer(allNodes.Get(i), allNodes.Get(numberOfClients));
    }

    NodeContainer routerServerNodes = NodeContainer(allNodes.Get(numberOfClients), allNodes.Get(numberOfClients+1));

    PointToPointHelper pointToPoint[numberOfClients + 1];    //point-to-point connection for each client-router connection and one for router-server connection

    for(i = 0; i < numberOfClients; i++){
        //TODO: implement data rate configuration
        pointToPoint[i].SetChannelAttribute("Delay", StringValue(clients[i]->getDelayInMilliseconds()));
    }

    NetDeviceContainer clientRouterDevices[numberOfClients];

    for(i = 0; i < numberOfClients; i++){
        clientRouterDevices[i] = pointToPoint[i].Install(clientRouterNodes[i]);
    }

    NetDeviceContainer routerServerDevices = pointToPoint[numberOfClients].Install(routerServerNodes);

    InternetStackHelper stack;
    stack.Install(allNodes);


    for(int i = 0; i < numberOfClients; i++){
        if(clients[i] != 0)
            delete clients[i];
    }

}
