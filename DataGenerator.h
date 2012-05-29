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

#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include "ApplicationProtocol.h"
#include "Messages.h"
#include "utilities.h"
#include "StatisticsCollector.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/address.h"
#include "ns3/inet-socket-address.h"
#include <vector>

class DataGenerator : public ns3::Application{

    friend std::ostream& operator<<(std::ostream& out, const DataGenerator &stream){

        out << "\n\tStream: " << stream.streamNumber << "  protocol: "
            << (stream.proto == TCP_NAGLE_DISABLED ? "TCP  Nagle's disabled" : stream.proto == TCP_NAGLE_ENABLED ? "TCP  Nagle's enabled" :
            (stream.appProto == 0 ? "UDP  Application protocol: no" : "UDP  Application protocol: yes"));

        for(std::vector<Message*>::const_iterator it = stream.messages.begin(); it != stream.messages.end(); it++){
            out << "\n\t\t";
            out << (**it);
        }

        return out;
    }


public:
    enum Protocol{TCP_NAGLE_DISABLED, TCP_NAGLE_ENABLED, UDP};
    DataGenerator(){}
    DataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int gametick = 10);   //TODO: configure gametick
    virtual ~DataGenerator();
    virtual void StartApplication() = 0;
    virtual void StopApplication() = 0;
    virtual void dataReceivedTcp(Ptr<Socket>) = 0;
    virtual void dataReceivedUdp(Ptr<Socket>) = 0;
    virtual void moreBufferSpaceAvailable(Ptr<Socket>, uint32_t) = 0;
    bool setupStream(Ptr<Node> node, Address addr);
    uint64_t getBytesSent() const{return totalBytesSent;}
    uint16_t getStreamNumber() const{return streamNumber;}
    Protocol getProtocol() const{return proto;}
    ApplicationProtocol* getApplicationProtocol() const{return appProto;}
    virtual std::vector<Message*> getMessages()const {return messages;}
    int getGameTick() const{return gameTick;}

protected:
    enum ReadMsgNameReturnValue{READ_FAILED = 0, READ_SUCCESS, NAME_CONTINUES};
    ReadMsgNameReturnValue readMessageName(std::string& name, uint8_t* buffer, uint16_t charLeft, bool nameContinues = false);

    uint16_t streamNumber;
    Protocol proto;
    ApplicationProtocol* appProto;
    std::vector<Message*> messages;
    Ptr<Socket> socket;
    bool running;
    Address peerAddr;
    uint64_t totalBytesSent;
    int gameTick;
};

class ClientDataGenerator : public DataGenerator{

public:
    ClientDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int gametick = 10); //TODO: configure gametick
    ClientDataGenerator(const DataGenerator&);
    ~ClientDataGenerator();

    virtual void StartApplication();
    virtual void StopApplication();
    void setClientNumber(uint16_t clientNumber);
    uint16_t getClientNumber()const {return ownerClient;}

private:
    uint16_t ownerClient;
    uint16_t bytesLeftToRead;
    bool dataLeft;
    bool nameLeft;
    std::string messageNamePart;
    std::string fullMessageName;

    void dataReceivedTcp(Ptr<Socket>);
    void dataReceivedUdp(Ptr<Socket>);
    void moreBufferSpaceAvailable(Ptr<Socket>, uint32_t);
    bool sendData(Message*, uint8_t* buffer);
};


class ServerDataGenerator : public DataGenerator{

    class ClientConnection{

    public:
        ClientConnection(Ptr<Socket>);
        ~ClientConnection();

        void forwardUserActionMessage(std::pair<std::string, Message*>&);

        Ptr<Socket> clientSocket;
        std::vector<std::pair<Ptr<Socket>, std::pair<std::string, Message*> > > messageBuffer;
        uint16_t bytesLeftToRead;
        bool dataLeft;
        bool nameLeft;
        std::string  messageNamePart;
        std::string fullMessageName;

    };

public:
    ServerDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int gametick = 10);     //TODO: configure gametick
    ServerDataGenerator(const DataGenerator&);
    ~ServerDataGenerator();

    virtual void StartApplication();
    virtual void StopApplication();

private:
    void dataReceivedTcp(Ptr<Socket>);
    void dataReceivedUdp(Ptr<Socket>);
    void moreBufferSpaceAvailable(Ptr<Socket>, uint32_t);
    bool connectionRequest(Ptr<Socket>, const Address&);
    void newConnectionCreated(Ptr<Socket>, const Address&);
    void forwardData();
    void sendToRandomClients(std::pair<Ptr<Socket>, std::pair<std::string, Message*> >&);   //for TCP streams
    void sendToRandomClients(std::pair<Address, std::pair<std::string, Message*> >&);       //for UDP streams
    void forwardUserActionMessageOverUdp(std::pair<std::string, Message*>, Address&);
    bool sendData(Message*, uint8_t*);

    std::vector<ServerDataGenerator::ClientConnection*> clientConnections;
    std::vector<Address*> udpClients;
    std::vector<std::pair<Address, std::pair<std::string, Message*> > > udpMessages;
    UniformVariable probability;

};


//Class DataGenerator function definitions

DataGenerator::DataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int tick)
    : streamNumber(streamNumber), proto(proto), appProto(appProto), messages(messages), running(false), totalBytesSent(0), gameTick(tick){


}

DataGenerator::~DataGenerator(){

    if(appProto != 0){
        delete appProto;
    }

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        delete *it;
    }

}


bool DataGenerator::setupStream(Ptr<Node> node, Address addr){

    peerAddr = addr;

    switch(proto){

        case TCP_NAGLE_DISABLED:
            socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
            socket->SetAttribute("TcpNoDelay", BooleanValue(true));
            socket->SetAttribute("SegmentSize", UintegerValue(1400));
            break;

        case TCP_NAGLE_ENABLED:
            socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
            socket->SetAttribute("TcpNoDelay", BooleanValue(false));
            socket->SetAttribute("SegmentSize", UintegerValue(1400));
            break;

        case UDP:
            socket = Socket::CreateSocket(node, UdpSocketFactory::GetTypeId());
            break;
    }

    node->AddApplication(this);

    return true;

}

DataGenerator::ReadMsgNameReturnValue DataGenerator::readMessageName(std::string &name, uint8_t *buffer, uint16_t charLeft, bool nameContinues){

   if(charLeft <= 1){
        return NAME_CONTINUES;      //read only "-character
    }


    if(nameContinues){
        int i;

        if(name.length() == 0)   //this means there's extra " in the beginning of the buffer
            i = 1;
        else i = 0;

        for(; (char)buffer[i] != '\"'; i++){
            name += (char)buffer[i];
            if((i+1) == charLeft){
                return NAME_CONTINUES;
            }

        }
    }
    else{
        if((char)buffer[0] != '\"'){
            return READ_FAILED;
        }

        for(int i = 1; (char)buffer[i] != '\"'; i++){
            name += (char)buffer[i];
            if((i+1) == charLeft){
                return NAME_CONTINUES;
            }
        }
    }

    return READ_SUCCESS;
}

//Class ClientDataGenerator function definitions

ClientDataGenerator::ClientDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int tick)
    : DataGenerator(streamNumber, proto, appProto, messages, tick), ownerClient(0), bytesLeftToRead(0), dataLeft(false), nameLeft(false){

}

ClientDataGenerator::ClientDataGenerator(const DataGenerator& stream) : bytesLeftToRead(0), dataLeft(false), nameLeft(false){

    this->streamNumber = stream.getStreamNumber();
    this->ownerClient = (dynamic_cast<const ClientDataGenerator&>(stream)).getClientNumber();
    this->gameTick = stream.getGameTick();

    if(stream.getApplicationProtocol() != 0)
        this->appProto = new ApplicationProtocol((*(stream.getApplicationProtocol())));
    else this->appProto = 0;

    this->proto = stream.getProtocol();

    std::vector<Message*> messages = stream.getMessages();

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        this->messages.push_back((*it)->copyMessage());
    }
}

ClientDataGenerator::~ClientDataGenerator(){

     if(socket != 0)
         socket->Close();
     CLIENT_INFO("Closed client socket for stream number: " << this->streamNumber << std::endl);

}

void ClientDataGenerator::setClientNumber(uint16_t clientNumber){
    ownerClient = clientNumber;
}

void ClientDataGenerator::StartApplication(){

    running = true;

    switch(proto){

        case TCP_NAGLE_DISABLED:
        case TCP_NAGLE_ENABLED:
            socket->Connect(peerAddr);
            socket->SetRecvCallback(MakeCallback(&ClientDataGenerator::dataReceivedTcp, this));
            break;

    case UDP:
            socket->Connect(peerAddr);
            socket->SetRecvCallback(MakeCallback(&ClientDataGenerator::dataReceivedUdp, this));
            break;
    }

    CLIENT_INFO("Client number: " << ownerClient <<  " is starting stream no: " << this->streamNumber << std::endl);

    socket->SetSendCallback(MakeCallback(&ClientDataGenerator::moreBufferSpaceAvailable, this));

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        if((*it)->getType() == USER_ACTION || (*it)->getType() == MAINTENANCE){
            (*it)->scheduleSendEvent(MakeCallback(&ClientDataGenerator::sendData, this));
        }
    }

}

void ClientDataGenerator::StopApplication(){

    running = false;

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        (*it)->cancelEvent();
    }

    if(socket){
       socket->ShutdownSend();
       socket->Close();
    }
}

void ClientDataGenerator::dataReceivedTcp(Ptr<Socket> sock){

    uint8_t* buffer = 0;
    uint16_t bufferSize = 0;
    uint16_t bytesRead = 0;
    uint16_t messageSize = 0;
    Message* message;
    ReadMsgNameReturnValue retVal;
    std::string messageName;

    bufferSize = sock->GetRxAvailable();
    buffer = (uint8_t*) calloc(sizeof(uint8_t), bufferSize);
    sock->Recv(buffer, bufferSize, 0);

    if(running){

        if(dataLeft){
            if(nameLeft){
                messageName.assign(messageNamePart);
            }

            if(bytesLeftToRead > bufferSize){
                dataLeft = true;
                bytesLeftToRead -= bufferSize;
            }else{
                if(!nameLeft){
                    bytesRead += bytesLeftToRead;
                    messageNamePart.clear();
                    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                        if(fullMessageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && fullMessageName[(*it)->getName().length()] == ':'){
                            message = *it;
                            break;
                        }
                    }
                    message->messageReceivedClient(fullMessageName);
                }

                if(bytesRead >= bufferSize){
                    dataLeft = false;
                }

                while(bytesRead < bufferSize){

                    if((retVal = readMessageName(messageName, buffer + bytesRead, bufferSize-bytesRead, nameLeft)) == READ_SUCCESS){

                        for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                            if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
                                message = *it;
                                break;
                            }
                        }
                        messageSize = message->getMessageSize();

                         if((bufferSize - bytesRead) <  messageSize - messageNamePart.length() - (nameLeft ==true ? 1 : 0)){   // -1 because of the "-character in the beginning of the name

                             if(nameLeft)
                                bytesLeftToRead = messageSize-(bufferSize - bytesRead) -(messageNamePart.length() +1);
                            else
                                bytesLeftToRead = messageSize-(bufferSize - bytesRead);

                            nameLeft = false;
                            dataLeft = true;
                            bytesRead = bufferSize;
                            fullMessageName.assign(messageName);
                            messageNamePart.assign((""));
                        }else{  //if we get here, the whole message has been read
                             if(nameLeft){
                                 bytesRead += messageSize - (messageNamePart.length() +1);
                             }else{
                                 bytesRead += messageSize;
                             }

                             nameLeft = false;
                             message->messageReceivedClient(messageName);
                             dataLeft = false;
                             bytesLeftToRead = 0;
                             messageNamePart.assign("");
                        }

                        messageName.assign("");

                    }
                    else if(retVal == NAME_CONTINUES){

                        if(nameLeft)
                            messageNamePart.append(messageName.substr(messageNamePart.length(), bufferSize-bytesRead));
                        else
                            messageNamePart.assign(messageName.substr(0, bufferSize-bytesRead));

                        bytesLeftToRead = 0;
                        nameLeft = true;
                        dataLeft = true;
                        bytesRead = bufferSize;
                    }
                    else if(retVal == READ_FAILED)
                        PRINT_ERROR("This should never happen, check message names!4  " <<std::endl);

                }
            }
        }else{

            while(bytesRead < bufferSize){
                if((retVal = readMessageName(messageName, buffer + bytesRead, bufferSize-bytesRead)) == READ_SUCCESS){
                    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                        if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
                            message = *it;
                            break;
                        }
                    }
                    messageSize = message->getMessageSize();

                    if((bufferSize - bytesRead) < messageSize){   //if this is true, the message continues in the next TCP segment
                        dataLeft = true;
                        bytesLeftToRead = messageSize-(bufferSize - bytesRead);
                        bytesRead = bufferSize;
                        fullMessageName.assign(messageName);
                        messageNamePart.assign((""));
                    }else{  //if we get here, the whole message has been read
                        bytesRead += messageSize;
                        message->messageReceivedClient(messageName);
                        dataLeft = false;
                        bytesLeftToRead = 0;
                    }

                    messageName.assign("");
                }
                else if(retVal == NAME_CONTINUES){
                    nameLeft = true;
                    dataLeft = true;
                    messageNamePart.assign(messageName.substr(0, bufferSize-bytesRead));
                    bytesLeftToRead = 0;
                    bytesRead = bufferSize;
                }
                else if(retVal == READ_FAILED)
                    PRINT_ERROR("This should never happen, check message names!" << std::endl);
            }
        }
    }

    if(buffer != 0)
        free(buffer);
}

void ClientDataGenerator::dataReceivedUdp(Ptr<Socket> sock){

    Address addr;
    sock->RecvFrom(addr);
}


void ClientDataGenerator::moreBufferSpaceAvailable(Ptr<Socket> sock, uint32_t size){

}

bool ClientDataGenerator::sendData(Message *msg, uint8_t* buffer){

    uint16_t bytesSent;

    if(running){
        if(socket->GetTxAvailable() < msg->getMessageSize())   {        //TODO: how to remember messages when buffer overflows
            return false;
        }

        if((bytesSent = socket->Send(buffer, msg->getMessageSize(), 0)) == -1){
            return false;
        }

        totalBytesSent += bytesSent;
    }

    return true;
}


//Class ServerDataGenerator function definitions

ServerDataGenerator::ServerDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int tick)
    : DataGenerator(streamNumber, proto, appProto, messages, tick){

    probability = UniformVariable(0,1);

}

ServerDataGenerator::ServerDataGenerator(const DataGenerator& stream){

    this->probability = UniformVariable(0,1);
    this->gameTick = stream.getGameTick();
    this->streamNumber = stream.getStreamNumber();

    if(stream.getApplicationProtocol() != 0)
        this->appProto = new ApplicationProtocol((*(stream.getApplicationProtocol())));
    else this->appProto = 0;

    this->proto = stream.getProtocol();

    std::vector<Message*> messages = stream.getMessages();

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        this->messages.push_back((*it)->copyMessage());

    }

}

ServerDataGenerator::~ServerDataGenerator(){

    for(std::vector<ServerDataGenerator::ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
        (*it)->clientSocket->Close();
        delete (*it);
        SERVER_INFO("Closed server socket for stream number: " << this->getStreamNumber() << std::endl);
    }

    for(std::vector<Address*>::iterator it = udpClients.begin(); it != udpClients.end(); it++)
        delete (*it);

}

void ServerDataGenerator::StartApplication(){

    running = true;
    socket->Bind(peerAddr);

    SERVER_INFO("Starting server stream no: " << this->streamNumber << std::endl);

    switch(proto){
        case TCP_NAGLE_DISABLED:
        case TCP_NAGLE_ENABLED:
            socket->Listen();
            socket->SetAcceptCallback(MakeCallback(&ServerDataGenerator::connectionRequest,this), MakeCallback(&ServerDataGenerator::newConnectionCreated, this));
            break;

        case UDP:
            socket->SetRecvCallback(MakeCallback(&ServerDataGenerator::dataReceivedUdp, this));
            break;
    }

    Simulator::Schedule(Time(MilliSeconds(gameTick)), &ServerDataGenerator::forwardData, this);

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        if((*it)->getType() == OTHER_DATA)
            (*it)->scheduleSendEvent(MakeCallback(&ServerDataGenerator::sendData, this));
    }

}

void ServerDataGenerator::StopApplication(){

    running = false;

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        (*it)->cancelEvent();
    }

    if(socket){
        socket->Close();
    }

    for(std::vector<ServerDataGenerator::ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
        (*it)->clientSocket->ShutdownRecv();
    }

}

void ServerDataGenerator::dataReceivedTcp(Ptr<Socket> sock){

    ClientConnection* client = 0;
    std::string messageName;
    uint8_t* buffer = 0;
    uint16_t bytesRead = 0;
    Message* message = 0;
    uint16_t messageSize = 0;
    uint16_t bufferSize = 0;
    ReadMsgNameReturnValue retVal;

    if(running){

        for(std::vector<ServerDataGenerator::ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
            if(sock == (*it)->clientSocket){
                client = (*it);
                break;
            }
        }

        bufferSize = sock->GetRxAvailable();
        buffer = (uint8_t*)calloc(sizeof(uint8_t), bufferSize);
        sock->Recv(buffer, bufferSize, 0);

        if(client->dataLeft){

            if(client->nameLeft){
                messageName.assign(client->messageNamePart);
            }

            if(client->bytesLeftToRead > bufferSize){
                client->dataLeft = true;
                client->bytesLeftToRead -= bufferSize;
            }else{
                if(!client->nameLeft){
                    bytesRead += client->bytesLeftToRead;               
                    client->messageNamePart.clear();
                    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                        if(client->fullMessageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && client->fullMessageName[(*it)->getName().length()] == ':'){
                            message = *it;
                            break;
                        }
                    }
                    client->messageBuffer.push_back(std::make_pair<Ptr<Socket>, std::pair<std::string, Message*> >(sock, std::make_pair<std::string, Message*>(client->fullMessageName, message)));
                    message->messageReceivedServer(client->fullMessageName);
                }

                if(bytesRead >= bufferSize){
                    client->dataLeft = false;
                }

                while(bytesRead < bufferSize){

                    if((retVal = readMessageName(messageName, buffer + bytesRead, bufferSize-bytesRead, client->nameLeft)) == READ_SUCCESS){

                        for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                            if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
                                message = *it;
                                break;
                            }
                        }
                        messageSize = message->getMessageSize();
                         if((bufferSize - bytesRead) <  messageSize -client->messageNamePart.length() - (client->nameLeft ==true ? 1 : 0)){   // -1 because of the "-character in the beginning of the name

                             if(client->nameLeft)
                                client->bytesLeftToRead = messageSize-(bufferSize - bytesRead) -( client->messageNamePart.length() +1);
                            else
                                client->bytesLeftToRead = messageSize-(bufferSize - bytesRead);

                            client->nameLeft = false;
                            client->dataLeft = true;

                            bytesRead = bufferSize;
                            client->messageNamePart.assign((""));
                            client->fullMessageName.assign(messageName);
                        }else{  //if we get here, the whole message has been read
                             if(client->nameLeft){
                                 bytesRead += messageSize - (client->messageNamePart.length() +1);
                             }else{
                                 bytesRead += messageSize;
                             }
                            client->nameLeft = false;
                            client->messageBuffer.push_back(std::make_pair<Ptr<Socket>, std::pair<std::string, Message*> >(sock, std::make_pair<std::string, Message*>(messageName, message)));
                            message->messageReceivedServer(messageName);
                            client->dataLeft = false;
                            client->bytesLeftToRead = 0;
                          //  client->messageNamePart.assign(messageName);
                        }

                        messageName.assign("");

                    }
                    else if(retVal == NAME_CONTINUES){

                        if(client->nameLeft)
                            client->messageNamePart.append(messageName.substr(client->messageNamePart.length(), bufferSize-bytesRead));
                        else
                            client->messageNamePart.assign(messageName.substr(0, bufferSize-bytesRead));

                        client->bytesLeftToRead = 0;
                        client->nameLeft = true;
                        client->dataLeft = true;
                        bytesRead = bufferSize;
                    }
                    else if(retVal == READ_FAILED)
                        PRINT_ERROR("This should never happen, check message names!1  " <<std::endl);

                }
            }
        }else{
            while(bytesRead < bufferSize){
                if((retVal = readMessageName(messageName, buffer + bytesRead, bufferSize-bytesRead)) == READ_SUCCESS){
                    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                        if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
                            message = *it;
                            break;
                        }
                    }
                    messageSize = message->getMessageSize();

                    if((bufferSize - bytesRead) < messageSize){   //if this is true, the message continues in the next TCP segment
                        client->dataLeft = true;
                        client->bytesLeftToRead = messageSize-(bufferSize - bytesRead);
                        bytesRead = bufferSize;
                        client->messageNamePart.assign((""));
                        client->fullMessageName.assign(messageName);
                    }else{  //if we get here, the whole message has been read
                        bytesRead += messageSize;
                        client->messageBuffer.push_back(std::make_pair<Ptr<Socket>, std::pair<std::string, Message*> >(sock, std::make_pair<std::string, Message*>(messageName, message)));
                        message->messageReceivedServer(messageName);
                        client->dataLeft = false;
                        client->bytesLeftToRead = 0;
                    }

                    messageName.assign("");
                }
                else if(retVal == NAME_CONTINUES){
                    client->nameLeft = true;
                    client->dataLeft = true;
                    client->messageNamePart.assign(messageName.substr(0, bufferSize-bytesRead));
                    client->bytesLeftToRead = 0;
                    bytesRead = bufferSize;
                }
                else if(retVal == READ_FAILED){
                    PRINT_ERROR("This should never happen, check message names!" << std::endl);sleep(10);}
            }
        }
    }

        if(buffer != 0)
            free(buffer);
}


void ServerDataGenerator::dataReceivedUdp(Ptr<Socket> sock){

    uint16_t bufferSize = 0;
    std::string messageName;
    Message* message = 0;
    uint8_t* buffer = 0;
    uint16_t bytesRead = 0;
    Address clientAddr;
    ReadMsgNameReturnValue retVal;
    bool addressExists = false;

    if(running){

        bufferSize = sock->GetRxAvailable();
        buffer = (uint8_t*)calloc(sizeof(uint8_t), bufferSize);

        sock->RecvFrom(buffer, bufferSize, 0, clientAddr);

        if(udpClients.empty()){
            udpClients.push_back(new Address(clientAddr));
        }

        for(std::vector<Address*>::iterator it = udpClients.begin(); it != udpClients.end(); it++){
            if((**it) == clientAddr){
                addressExists = true;
                break;
            }
        }

        if(!addressExists){
            udpClients.push_back(new Address(clientAddr));
        }

        while(bytesRead < bufferSize){

            if((retVal = readMessageName(messageName, buffer, bufferSize-bytesRead)) == READ_SUCCESS){
                for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                    if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0){
                        message = (*it);
                        break;
                    }
                }
                bytesRead += message->getMessageSize();
                udpMessages.push_back(std::make_pair<Address, std::pair<std::string, Message*> >(Address(clientAddr), std::make_pair<std::string, Message*>(messageName, message)));
                message->messageReceivedServer(messageName);
            }
            else if(retVal == NAME_CONTINUES){
                PRINT_ERROR("This should never happen!" << std::endl);
            }
            else if(retVal == READ_FAILED)
                PRINT_ERROR("This should never happen, check message names! " << std::endl);
        }

        if(buffer != 0)
            free(buffer);
    }

}

void ServerDataGenerator::moreBufferSpaceAvailable(Ptr<Socket> sock, uint32_t size){

}

bool ServerDataGenerator::connectionRequest(Ptr<Socket> sock, const Address &addr){

    SERVER_INFO("Connection request from: " << addr << std::endl);   //TODO: getting ip-addresses impossible???
    return true;

}

void ServerDataGenerator::newConnectionCreated(Ptr<Socket> sock, const Address &addr){

    SERVER_INFO("Connection accepted from: " << addr << " in stream number: " << streamNumber << "   " << Simulator::Now() << std::endl);
    clientConnections.push_back(new ServerDataGenerator::ClientConnection(sock));
    sock->SetRecvCallback(MakeCallback(&ServerDataGenerator::dataReceivedTcp, this));
}

void ServerDataGenerator::forwardData(){

    if(running){
        switch(proto){

            case TCP_NAGLE_DISABLED:
            case TCP_NAGLE_ENABLED:

                for(std::vector<ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
                    for(std::vector<std::pair<Ptr<Socket>, std::pair<std::string, Message*> > >::iterator messages = (*it)->messageBuffer.begin(); messages != (*it)->messageBuffer.end(); messages++){
                        if((*messages).second.second->getType() == USER_ACTION){
                            sendToRandomClients(*messages);
                        }
                    }
                    (*it)->messageBuffer.clear();
                }
                break;

            case UDP:

                for(std::vector<std::pair<Address, std::pair<std::string, Message*> > >::iterator it = udpMessages.begin(); it != udpMessages.end(); it++){
                    if((*it).second.second->getType() == USER_ACTION){
                        sendToRandomClients(*it);
                    }
                }
                udpMessages.clear();

                break;
        }
        Simulator::Schedule(Time(MilliSeconds(gameTick)), &ServerDataGenerator::forwardData, this);
    }
}

void ServerDataGenerator::sendToRandomClients(std::pair<Ptr<Socket>, std::pair<std::string, Message*> > &msg){

    double clientsToSend = ((UserActionMessage*)msg.second.second)->getClientsOfInterest();

    for(std::vector<ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
        if(msg.first == (*it)->clientSocket)
            continue;

        if(clientsToSend >= probability.GetValue()){
            (*it)->forwardUserActionMessage(msg.second);
        }
    }
}

void ServerDataGenerator::sendToRandomClients(std::pair<Address, std::pair<std::string, Message*> > &msg){

    double clientsToSend = ((UserActionMessage*)msg.second.second)->getClientsOfInterest();

    for(std::vector<Address*>::iterator it = udpClients.begin(); it != udpClients.end(); it++){
        if((**it) == msg.first)
            continue;

        if(clientsToSend >= probability.GetValue()){
            forwardUserActionMessageOverUdp(msg.second, (**it));
        }
    }
}

void ServerDataGenerator::forwardUserActionMessageOverUdp(std::pair<std::string, Message*> msg, Address& addr){

    char buffer[30] = "";
    msg.second->fillMessageContents(buffer, 0, &msg.first);

    if(socket->SendTo((uint8_t*)buffer, msg.second->getMessageSize(), 0, addr) == -1)
        PRINT_ERROR("Problems with server socket buffer." << std::endl);
}

bool ServerDataGenerator::sendData(Message *msg, uint8_t *buffer){

    uint16_t bytesSent;

    if(running){

        if(this->proto == TCP_NAGLE_DISABLED || this->proto == TCP_NAGLE_ENABLED){
            for(std::vector<ServerDataGenerator::ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
                if((*it)->clientSocket->GetTxAvailable() < msg->getMessageSize())
                    return false;
                if((bytesSent = (*it)->clientSocket->Send(buffer, msg->getMessageSize(), 0)) == -1)
                    return false;

                totalBytesSent += bytesSent;
            }
        }else if(this->proto == UDP){
            for(std::vector<Address*>::iterator it = udpClients.begin(); it != udpClients.end(); it++){
                if((bytesSent = socket->SendTo(buffer, msg->getMessageSize(), 0, **it)) == -1)
                    return false;

                totalBytesSent += bytesSent;
            }
        }
    }

    return true;
}


//nested class ClientConnection function definitions

ServerDataGenerator::ClientConnection::ClientConnection(Ptr<Socket> sock): clientSocket(sock), dataLeft(false), nameLeft(false){

}

ServerDataGenerator::ClientConnection::~ClientConnection(){

    clientSocket->Close();
}

void ServerDataGenerator::ClientConnection::forwardUserActionMessage(std::pair<std::string, Message*>& msg){

    char buffer[30] = "";
    msg.second->fillMessageContents(buffer, 0, &msg.first);

    if(clientSocket->Send((uint8_t*)buffer, msg.second->getMessageSize(), 0) == -1)
        PRINT_ERROR("Problems with server socket buffer." << std::endl);

}

#endif // DATAGENERATOR_H
