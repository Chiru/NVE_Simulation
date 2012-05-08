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
#include "ns3/tcp-socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/address.h"
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

};

class ClientDataGenerator : public DataGenerator{

public:
    ClientDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages);
    ClientDataGenerator(const DataGenerator&);
    ~ClientDataGenerator();

    virtual void StartApplication();
    virtual void StopApplication();

private:
    void dataReceived(Ptr<Socket>);
    void moreBufferSpaceAvailable(Ptr<Socket>, uint32_t);
    void sendData(Message*, uint8_t* buffer);
};


class ServerDataGenerator : public DataGenerator{

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
};


//Class DataGenerator function definitions

DataGenerator::DataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages)
    : streamNumber(streamNumber), proto(proto), appProto(appProto), messages(messages), running(false){


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
            break;

        case TCP_NAGLE_ENABLED:
            socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
            break;

        case UDP:
            socket = Socket::CreateSocket(node, UdpSocketFactory::GetTypeId());
            break;
    }

    node->AddApplication(this);

    return true;

}


//Class ClientDataGenerator function definitions

ClientDataGenerator::ClientDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages)
    : DataGenerator(streamNumber, proto, appProto, messages){


}

ClientDataGenerator::ClientDataGenerator(const DataGenerator& stream){

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

ClientDataGenerator::~ClientDataGenerator(){


}

void ClientDataGenerator::StartApplication(){

    running = true;
    socket->Connect(peerAddr);

    socket->SetRecvCallback(MakeCallback(&ClientDataGenerator::dataReceived, this));
    socket->SetSendCallback(MakeCallback(&ClientDataGenerator::moreBufferSpaceAvailable, this));

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        (*it)->scheduleSendEvent(MakeCallback(&ClientDataGenerator::sendData, this));
    }


}

void ClientDataGenerator::StopApplication(){


}

void ClientDataGenerator::dataReceived(Ptr<Socket>){

}

void ClientDataGenerator::moreBufferSpaceAvailable(Ptr<Socket> sock, uint32_t size){

}

void ClientDataGenerator::sendData(Message *msg, uint8_t* buffer){

    socket->Send(buffer, msg->getMessageSize(), 0);

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

}

void ServerDataGenerator::StartApplication(){

    running = true;
    socket->Bind(peerAddr);
    socket->Listen();
    socket->SetAcceptCallback(MakeCallback(&ServerDataGenerator::connectionRequest,this), MakeCallback(&ServerDataGenerator::newConnectionCreated, this));
    socket->SetRecvCallback(MakeCallback(&ServerDataGenerator::dataReceived, this));

}

void ServerDataGenerator::StopApplication(){


}

void ServerDataGenerator::dataReceived(Ptr<Socket> sock){

    uint8_t buffer[1000];
    sock->Recv(buffer, 1000, 0);

    std::cout << buffer << std::endl;

}

void ServerDataGenerator::moreBufferSpaceAvailable(Ptr<Socket> sock, uint32_t size){

}

bool ServerDataGenerator::connectionRequest(Ptr<Socket> sock, const Address &addr){

    return true;

}

void ServerDataGenerator::newConnectionCreated(Ptr<Socket> sock, const Address &addr){

}

#endif // DATAGENERATOR_H
