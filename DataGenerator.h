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
    DataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages);
    virtual ~DataGenerator();
    virtual void StartApplication() = 0;
    virtual void StopApplication() = 0;
    virtual void dataReceived(Ptr<Socket>) = 0;
    virtual void moreBufferSpaceAvailable(Ptr<Socket>, uint32_t) = 0;
    bool setupStream(Ptr<Node> node, Address addr);
    uint64_t getBytesSent() const{return totalBytesSent;}
    uint16_t getStreamNumber() const{return streamNumber;}
    Protocol getProtocol() const{return proto;}
    ApplicationProtocol* getApplicationProtocol() const{return appProto;}
    virtual std::vector<Message*> getMessages()const {return messages;}

protected:
    uint16_t streamNumber;
    Protocol proto;
    ApplicationProtocol* appProto;
    std::vector<Message*> messages;
    Ptr<Socket> socket;
    bool running;
    Address peerAddr;
    uint64_t totalBytesSent;
    bool readMessageName(std::string& name, uint8_t* buffer);
};

class ClientDataGenerator : public DataGenerator{

public:
    ClientDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages);
    ClientDataGenerator(const DataGenerator&);
    ~ClientDataGenerator();

    virtual void StartApplication();
    virtual void StopApplication();
    void setClientNumber(uint16_t clientNumber);
    uint16_t getClientNumber()const {return ownerClient;}

private:
    uint16_t ownerClient;

    void dataReceived(Ptr<Socket>);
    void moreBufferSpaceAvailable(Ptr<Socket>, uint32_t);
    bool sendData(Message*, uint8_t* buffer);
};


class ServerDataGenerator : public DataGenerator{

    class ClientConnection{

    public:
        ClientConnection(Ptr<Socket>);
        ~ClientConnection();

        Ptr<Socket> clientSocket;
        std::vector<Message*> messageBuffer;
        uint16_t bytesLeftToRead;
        bool dataLeft;
    };

public:
    ServerDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages);
    ServerDataGenerator(const DataGenerator&);
    ~ServerDataGenerator();

    virtual void StartApplication();
    virtual void StopApplication();

private:
    void dataReceived(Ptr<Socket>);
    void moreBufferSpaceAvailable(Ptr<Socket>, uint32_t);
    bool connectionRequest(Ptr<Socket>, const Address&);
    void newConnectionCreated(Ptr<Socket>, const Address&);
    std::vector<ServerDataGenerator::ClientConnection*> clientConnections;

};


//Class DataGenerator function definitions

DataGenerator::DataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages)
    : streamNumber(streamNumber), proto(proto), appProto(appProto), messages(messages), running(false), totalBytesSent(0){


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

bool DataGenerator::readMessageName(std::string &name, uint8_t *buffer){

    if((char)buffer[0] != '\"'){
        std::cout << (char)buffer[1]<<(char)buffer[2]<<(char)buffer[3]<<(char)buffer[4]<<(char)buffer[5]<< std::endl;
        return false;
    }

    for(int i = 1; (char)buffer[i] != '\"'; i++)
        name += (char)buffer[i];

    std::cout << name << std::endl;

    return true;
}

//Class ClientDataGenerator function definitions

ClientDataGenerator::ClientDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages)
    : DataGenerator(streamNumber, proto, appProto, messages), ownerClient(0){


}

ClientDataGenerator::ClientDataGenerator(const DataGenerator& stream){

    this->streamNumber = stream.getStreamNumber();
    this->ownerClient = (dynamic_cast<const ClientDataGenerator&>(stream)).getClientNumber();

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
    socket->Connect(peerAddr);

    CLIENT_INFO("Client number: " << ownerClient <<  " is starting stream no: " << this->streamNumber << std::endl);

    socket->SetRecvCallback(MakeCallback(&ClientDataGenerator::dataReceived, this));
    socket->SetSendCallback(MakeCallback(&ClientDataGenerator::moreBufferSpaceAvailable, this));

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        (*it)->scheduleSendEvent(MakeCallback(&ClientDataGenerator::sendData, this));
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

void ClientDataGenerator::dataReceived(Ptr<Socket> sock){

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

ServerDataGenerator::ServerDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages)
    : DataGenerator(streamNumber, proto, appProto, messages){

}

ServerDataGenerator::ServerDataGenerator(const DataGenerator& stream){

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
        delete (*it);
        SERVER_INFO("Closed server socket for stream number: " << this->getStreamNumber() << std::endl);
    }

}

void ServerDataGenerator::StartApplication(){

    running = true;
    socket->Bind(peerAddr);

    SERVER_INFO("Starting server stream no: " << this->streamNumber << std::endl);

    socket->Listen();
    socket->SetAcceptCallback(MakeCallback(&ServerDataGenerator::connectionRequest,this), MakeCallback(&ServerDataGenerator::newConnectionCreated, this));
    //socket->SetRecvCallback(MakeCallback(&ServerDataGenerator::dataReceived, this));

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
        (*it)->clientSocket->Close();
    }

}

void ServerDataGenerator::dataReceived(Ptr<Socket> sock){

    ClientConnection* client = 0;
    std::string messageName;
    uint8_t* buffer = 0;
    uint16_t bytesRead = 0;
    Message* message = 0;
    uint16_t messageSize = 0;
    uint16_t bufferSize = 0;

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
            if(client->bytesLeftToRead > bufferSize){
                client->dataLeft = true;
                client->bytesLeftToRead -= bufferSize;
            }else{
                bytesRead += client->bytesLeftToRead;
                while(bytesRead < bufferSize){
                    if(readMessageName(messageName, buffer + bytesRead)){
                        for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                            if(messageName.compare((*it)->getName()) == 0){
                                message = *it;
                                break;
                            }
                        }
                        messageSize = message->getMessageSize();
                        if((bufferSize - bytesRead) < messageSize){
                            client->dataLeft = true;
                            client->bytesLeftToRead = messageSize-(bufferSize - bytesRead);
                            client->messageBuffer.push_back(message);
                            bytesRead = bufferSize;
                        }else{
                            bytesRead += messageSize;
                            client->messageBuffer.push_back(message);
                            client->dataLeft = false;
                        }
                    }
                }
            }
        }else{
            while(bytesRead < bufferSize){
                if(readMessageName(messageName, buffer + bytesRead)){
                    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                        if(messageName.compare((*it)->getName()) == 0){
                            message = *it;
                            break;
                        }
                    }
                    messageSize = message->getMessageSize();
                    if((bufferSize - bytesRead) < messageSize){
                        client->dataLeft = true;
                        client->bytesLeftToRead = messageSize-(bufferSize - bytesRead);
                        client->messageBuffer.push_back(message);
                        bytesRead = bufferSize;
                    }else{
                        bytesRead += messageSize;
                        client->messageBuffer.push_back(message);
                        client->dataLeft = false;
                    }
                }
            }
        }
    }

        if(buffer != 0)
            free(buffer);


   // uint8_t buffer[1000];
    //sock->Recv(buffer, 1000, 0);

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
    sock->SetRecvCallback(MakeCallback(&ServerDataGenerator::dataReceived, this));
}


//nested class ClientConnection function definitions

ServerDataGenerator::ClientConnection::ClientConnection(Ptr<Socket> sock): clientSocket(sock){

}

ServerDataGenerator::ClientConnection::~ClientConnection(){

    clientSocket->Close();
}

#endif // DATAGENERATOR_H
