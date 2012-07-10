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
#include "DataSender.h"
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
    DataGenerator() :sender(appProto, gameTick){}
    DataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int gametick = 10);   //TODO: configure gametick
    virtual ~DataGenerator();
    virtual void StartApplication() = 0;
    virtual void StopApplication() = 0;
    virtual void dataReceivedTcp(Ptr<Socket>) = 0;
    virtual void dataReceivedUdp(Ptr<Socket>) = 0;
    virtual void moreBufferSpaceAvailable(Ptr<Socket>, uint32_t) = 0;
    bool setupStream(Ptr<Node> node, Address addr, uint16_t gameTick = 10);
    uint64_t getBytesSent() const{return totalBytesSent;}
    uint16_t getStreamNumber() const{return streamNumber;}
    Protocol getProtocol() const{return proto;}
    ApplicationProtocol* getApplicationProtocol() const{return appProto;}
    virtual std::vector<Message*> getMessages()const {return messages;}
    int getGameTick() const{return gameTick;}
    void sendBackToSender(const Message*, const Address&, const Ptr<Socket>, std::string& messageName, bool isClient);
    bool sendImmediately() const {return immediateSend;}

protected:
    enum ReadMsgNameReturnValue{READ_FAILED = 0, READ_SUCCESS, NAME_CONTINUES};
    ReadMsgNameReturnValue readMessageName(std::string& name, uint8_t* buffer, uint16_t charLeft, bool nameContinues = false);
    virtual void readReceivedData(uint8_t*, uint16_t, Address& srcAddr) = 0;

    uint16_t streamNumber;
    Protocol proto;
    ApplicationProtocol* appProto;
    std::vector<Message*> messages;
    Ptr<Socket> socket;
    bool running;
    Address peerAddr;
    uint64_t totalBytesSent;
    int gameTick;
    DataSender sender;
    bool immediateSend;
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
    void readReceivedData(uint8_t *, uint16_t, Address&);
    void moreBufferSpaceAvailable(Ptr<Socket>, uint32_t);
    bool sendData(Message*, uint8_t* buffer);
};


class ServerDataGenerator : public DataGenerator{

    class ClientConnection{

    public:
        ClientConnection(Ptr<Socket>, DataSender&, bool);
        ~ClientConnection();

        void forwardUserActionMessage(std::pair<std::string, Message*>&);

        Ptr<Socket> clientSocket;
        std::vector<std::pair<Ptr<Socket>, std::pair<std::string, Message*> > > messageBuffer;
        uint16_t bytesLeftToRead;
        bool dataLeft;
        bool nameLeft;
        std::string  messageNamePart;
        std::string fullMessageName;
        DataSender& sender;
        bool immediateSend;

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
    void readReceivedData(uint8_t *, uint16_t, Address&);
    void moreBufferSpaceAvailable(Ptr<Socket>, uint32_t);
    bool connectionRequest(Ptr<Socket>, const Address&);
    void newConnectionCreated(Ptr<Socket>, const Address&);
    void forwardData();
    void sendToRandomClients(std::pair<Ptr<Socket>, std::pair<std::string, Message*> >&);   //for TCP streams
    void sendToRandomClients(std::pair<Address, std::pair<std::string, Message*> >&);       //for UDP streams
    void forwardUserActionMessage(std::pair<std::string, Message*>, Address&);
    bool sendData(Message*, uint8_t*);

    std::vector<ServerDataGenerator::ClientConnection*> clientConnections;
    std::vector<Address*> udpClients;
    std::vector<std::pair<Address, std::pair<std::string, Message*> > > udpMessages;
    UniformVariable probability;

};


//Class DataGenerator function definitions

DataGenerator::DataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int tick)
    : streamNumber(streamNumber), proto(proto), appProto(appProto), messages(messages), running(false), totalBytesSent(0), gameTick(tick), sender(DataSender(appProto, tick)){

    if(tick == 0)
        immediateSend = true;
    else
        immediateSend = false;

}

DataGenerator::~DataGenerator(){

    if(appProto != 0){
        delete appProto;
    }

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        delete *it;
    }

}


bool DataGenerator::setupStream(Ptr<Node> node, Address addr, uint16_t gameTick){

    peerAddr = addr;

    this->gameTick = gameTick;

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

void DataGenerator::sendBackToSender(const Message* msg, const Address& addr, const Ptr<Socket> socket, std::string& messageName, bool isClient){

    char buffer[30] = "";
    msg->fillMessageContents(buffer, 0, &messageName);

    switch(this->proto){

    case TCP_NAGLE_DISABLED:
    case TCP_NAGLE_ENABLED:
        if(!sender.send(immediateSend, (uint8_t*)buffer, msg, socket, true, isClient))
            return;

        break;

    case UDP:

        if(!sender.sendTo(immediateSend, (uint8_t*)buffer, msg, addr, true, isClient,socket))
            return;

        break;
    }
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

    this->immediateSend = stream.sendImmediately();
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
            if(!immediateSend)
                Simulator::Schedule(Time(MilliSeconds(gameTick)), &DataSender::flushTcpBuffer, &sender, true);
            socket->SetRecvCallback(MakeCallback(&ClientDataGenerator::dataReceivedTcp, this));
            break;

    case UDP:
            socket->Connect(peerAddr);
            if(!immediateSend)
                Simulator::Schedule(Time(MilliSeconds(gameTick)), &DataSender::flushUdpBuffer, &sender, socket, true);
            if(appProto){
                socket->SetRecvCallback(MakeCallback(&ApplicationProtocol::recv, appProto));
                appProto->configureForStream(MakeCallback(&ClientDataGenerator::readReceivedData, this));
            }else{
                socket->SetRecvCallback(MakeCallback(&ClientDataGenerator::dataReceivedUdp, this));

            }
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
    Message* message = 0;
    ReadMsgNameReturnValue retVal;
    std::string messageName;

    bufferSize = sock->GetRxAvailable();
    buffer = (uint8_t*) calloc(bufferSize, sizeof(uint8_t));
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

                         messageSize = (message)->getForwardMessageSize();


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
                        PRINT_ERROR("This should never happen, check message names!" <<std::endl);

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

                    messageSize = (message)->getForwardMessageSize();

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
    uint8_t* buffer = 0;
    uint16_t bufferSize = 0;

    bufferSize = sock->GetRxAvailable();
    buffer = (uint8_t*)calloc(bufferSize, sizeof(uint8_t));

    sock->RecvFrom(buffer, bufferSize, 0, addr);

    readReceivedData(buffer, bufferSize, addr);

    free(buffer);
}

void ClientDataGenerator::readReceivedData(uint8_t* buffer, uint16_t bufferSize, Address& srcAddr){

    uint16_t bytesRead = 0;
    Message* message = 0;
    std::string messageName;
    ReadMsgNameReturnValue retVal;

    if(running){

        while(bytesRead < bufferSize){

            if((retVal = readMessageName(messageName, buffer, bufferSize-bytesRead)) == READ_SUCCESS){
                for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                    if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
                        message = (*it);
                        break;
                    }
                }

                bytesRead += (message)->getForwardMessageSize();

                message->messageReceivedClient(messageName);

                if(message->getType() == OTHER_DATA && message->doForwardBack()){
                    sendBackToSender(message, srcAddr, this->socket, messageName, true);
                }
            }
            else if(retVal == NAME_CONTINUES){
                PRINT_ERROR("This should never happen!" << std::endl);
            }
            else if(retVal == READ_FAILED)
                PRINT_ERROR("This should never happen, check message names!" << std::endl);
        }
    }
}


void ClientDataGenerator::moreBufferSpaceAvailable(Ptr<Socket> sock, uint32_t size){

}

bool ClientDataGenerator::sendData(Message *msg, uint8_t* buffer){

    if(running)
        if(!sender.sendTo(immediateSend, buffer, msg, peerAddr, false, true, socket))
            return false;

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

    this->immediateSend = stream.sendImmediately();

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
        if(appProto){
            socket->SetRecvCallback(MakeCallback(&ApplicationProtocol::recv, appProto));
            appProto->configureForStream(MakeCallback(&ServerDataGenerator::readReceivedData, this));
        }else{
            socket->SetRecvCallback(MakeCallback(&ServerDataGenerator::dataReceivedUdp, this));
        }
            break;
    }
    if(!immediateSend)
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
        buffer = (uint8_t*)calloc(bufferSize, sizeof(uint8_t));
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
                        PRINT_ERROR("This should never happen, check message names!" <<std::endl);

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
                else if(retVal == READ_FAILED)
                    PRINT_ERROR("This should never happen, check message names!" << std::endl);
            }
        }
    }

        if(buffer != 0)
            free(buffer);
}

void ServerDataGenerator::dataReceivedUdp(Ptr<Socket> sock){

    Address addr;
    uint8_t* buffer = 0;
    uint16_t bufferSize = 0;

    bufferSize = sock->GetRxAvailable();
    buffer = (uint8_t*)calloc(bufferSize, sizeof(uint8_t));

    sock->RecvFrom(buffer, bufferSize, 0, addr);
    readReceivedData(buffer, bufferSize, addr);

    free(buffer);
}

void ServerDataGenerator::readReceivedData(uint8_t *buffer, uint16_t bufferSize, Address& clientAddr){

    std::string messageName;
    Message* message = 0;
    uint16_t bytesRead = 0;
    ReadMsgNameReturnValue retVal;
    bool addressExists = false;

    if(running){

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
                    if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
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
    clientConnections.push_back(new ServerDataGenerator::ClientConnection(sock, sender, immediateSend));
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
                        else if((*messages).second.second->getType() == MAINTENANCE && (*messages).second.second->doForwardBack()){
                            sendBackToSender(messages->second.second, this->peerAddr/*this is not needed over TCP*/, (*it)->clientSocket,  messages->second.first, false);
                        }

                    }
                    (*it)->messageBuffer.clear();
                }
                if(immediateSend)
                    sender.flushTcpBuffer(false);
                break;

            case UDP:

                for(std::vector<std::pair<Address, std::pair<std::string, Message*> > >::iterator it = udpMessages.begin(); it != udpMessages.end(); it++){
                    if((*it).second.second->getType() == USER_ACTION){
                        sendToRandomClients(*it);
                    }
                    else if((*it).second.second->getType() == MAINTENANCE && (*it).second.second->doForwardBack()){
                        sendBackToSender(it->second.second, it->first, this->socket,  it->second.first, false);

                    }

                }
                udpMessages.clear();
                if(immediateSend)
                    sender.flushUdpBuffer(this->socket, false);
                break;
        }
        if(!immediateSend)
            Simulator::Schedule(Time(MilliSeconds(gameTick)), &ServerDataGenerator::forwardData, this);
    }
}

void ServerDataGenerator::sendToRandomClients(std::pair<Ptr<Socket>, std::pair<std::string, Message*> > &msg){    //forwarding over TCP

    double clientsToSend = ((UserActionMessage*)msg.second.second)->getClientsOfInterest();

    for(std::vector<ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
        if(msg.first == (*it)->clientSocket && !(msg.second.second->doForwardBack()))
            continue;

        if(clientsToSend >= probability.GetValue()){
            (*it)->forwardUserActionMessage(msg.second);
        }
    }
}

void ServerDataGenerator::sendToRandomClients(std::pair<Address, std::pair<std::string, Message*> > &msg){      //forwarding over UDP

    double clientsToSend = ((UserActionMessage*)msg.second.second)->getClientsOfInterest();

    for(std::vector<Address*>::iterator it = udpClients.begin(); it != udpClients.end(); it++){
        if((**it) == msg.first && !(msg.second.second->doForwardBack()))
            continue;

        if(clientsToSend >= probability.GetValue()){
            forwardUserActionMessage(msg.second, (**it));
        }
    }
}

void ServerDataGenerator::forwardUserActionMessage(std::pair<std::string, Message*> msg, Address& addr){    //forwarding over UDP

    char buffer[30] = "";
    int messageNumber;
    msg.second->fillMessageContents(buffer, 0, &msg.first);

    if(!sender.sendTo(immediateSend, (uint8_t*)buffer, msg.second, addr, true, false, socket))
        return;

    msg.second->parseMessageId(msg.first, messageNumber);

    StatisticsCollector::logMessageForwardedByServer(messageNumber, streamNumber);
}

bool ServerDataGenerator::sendData(Message *msg, uint8_t *buffer){

    if(running){

        if(this->proto == TCP_NAGLE_DISABLED || this->proto == TCP_NAGLE_ENABLED){
            for(std::vector<ServerDataGenerator::ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
                if((*it)->clientSocket->GetTxAvailable() < msg->getMessageSize())
                    return false;

                if(!sender.send(immediateSend, buffer, msg, (*it)->clientSocket, false, false))
                    return false;

            }
        }else if(this->proto == UDP){
            for(std::vector<Address*>::iterator it = udpClients.begin(); it != udpClients.end(); it++){

                if(!sender.sendTo(immediateSend, buffer, msg, **it, false, false, socket))
                    return false;
            }
        }
    }

    return true;
}


//nested class ClientConnection function definitions

ServerDataGenerator::ClientConnection::ClientConnection(Ptr<Socket> sock, DataSender& sender, bool immediate): clientSocket(sock), dataLeft(false), nameLeft(false), sender(sender),
    immediateSend(immediate){

}

ServerDataGenerator::ClientConnection::~ClientConnection(){

    clientSocket->Close();
}

void ServerDataGenerator::ClientConnection::forwardUserActionMessage(std::pair<std::string, Message*>& msg){           //forwarding over TCP

    char buffer[30] = "";
    int messageNumber;
    msg.second->fillMessageContents(buffer, 0, &msg.first);

    if(!sender.send(immediateSend, (uint8_t*)buffer, msg.second, clientSocket, true,false))
        return;


    msg.second->parseMessageId(msg.first, messageNumber);

    StatisticsCollector::logMessageForwardedByServer(messageNumber, msg.second->getStreamNumber());
}

#endif // DATAGENERATOR_H
