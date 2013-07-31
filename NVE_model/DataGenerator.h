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
            (stream.appProto == 0 ? "UDP  Application protocol: no" :  stream.isOrdered()  ? "UDP  Application protocol: yes   ordering: ordered" :
                                                                                             "UDP  Application protocol: no   ordering: unordered" ));

        for(std::vector<Message*>::const_iterator it = stream.messages.begin(); it != stream.messages.end(); it++){
            out << "\n\t\t";
            out << (**it);
        }

        return out;
    }


public:
    enum Protocol{TCP_NAGLE_DISABLED, TCP_NAGLE_ENABLED, UDP};
    DataGenerator() :sender(appProto, gameTick){}
    DataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int gametick, bool ordered);
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
    void sendBackToSender(const Message*, const Address&, const Ptr<Socket>, std::string& messageName, bool isClient);
    bool sendImmediately() const {return immediateSend;}
    bool isOrdered() const {return ordered;}

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
    bool ordered;
    DataSender sender;
    bool immediateSend;

};

class ClientDataGenerator : public DataGenerator{

public:
    ClientDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int gametick, bool ordered);
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
    bool connectionInitialized;

    void dataReceivedTcp(Ptr<Socket>);
    void dataReceivedUdp(Ptr<Socket>);
    void readReceivedData(uint8_t *, uint16_t, Address&);
    void moreBufferSpaceAvailable(Ptr<Socket>, uint32_t);
    bool sendData(Message*, uint8_t* buffer);
    void initializeUdpConnection();
    void finishUdpConnection();
    void startSendingData();

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
    ServerDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int gametick, bool ordered);
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
    void acceptUdpConnection(const Address& addr, bool addressExists);
    void finishUdpConnection(const Address& addr);
    void clientExited(Ptr<Socket> sock);

    std::vector<ServerDataGenerator::ClientConnection*> clientConnections;    //for tcp clients
    std::vector<Address*> udpClients;                                         //for UDP clients
    std::vector<std::pair<Address, std::pair<std::string, Message*> > > udpMessages;
    UniformVariable probability;

};


#endif // DATAGENERATOR_H
