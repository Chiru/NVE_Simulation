#ifndef NVE_SIMULATOR_H
#define NVE_SIMULATOR_H

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


#include "ns3/applications-module.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include <vector>
#include <string>

class XMLParser;
class DataGenerator;

class Client{

public:
    Client(XMLParser&, uint16_t no);
    ~Client();

    std::string getDelayInMilliseconds();

private:
    XMLParser &parser;
    DataGenerator** streams;
    int networkDelay;
    double uplinkBandwidth;
    double downlinkBandwidth;
    double lossRate;
    uint16_t clientNumber;
};


class Server{

public:
    Server(XMLParser&);
    ~Server();

private:
    XMLParser &parser;

};


class DataGenerator : public ns3::Application{

public:
    DataGenerator();
    virtual ~DataGenerator();

    virtual void StartApplication();
    virtual void StopApplication();

};

class ClientDataGenerator : public DataGenerator{

public:
    ClientDataGenerator();
    ~ClientDataGenerator();
};


class ServerDataGenerator : public DataGenerator{

public:
    ServerDataGenerator();
    ~ServerDataGenerator();

};

class StatisticsCollector{

public:
    StatisticsCollector();
    ~StatisticsCollector();

};

class ApplicationProtocol{

public:
    ApplicationProtocol(uint16_t packetSize, uint16_t delayedAck, uint16_t retransmit);
    ApplicationProtocol(const ApplicationProtocol&);
    ~ApplicationProtocol();

    bool write(std::string&);
    std::string read();

private:
    std::vector<int> packetsWaitingAcks;
    ns3::Ptr<ns3::Socket> socket;
    uint16_t packetSize;
    uint16_t delayedAck;
    uint16_t retransmit;

    void writeToSocket(std::string&);
    void readFromSocket();
    void resend();
};


class Message{

public:
    Message();
    ~Message();
    virtual void startDataTransfer() = 0;

protected:
    bool reliable;
    ns3::Time timeInterval;
    uint16_t messageSize;
    uint32_t messageNumber;

};

class UserActionMessage : public Message{

public:
    UserActionMessage();
    ~UserActionMessage();
    void startDataTransfer();

private:
    double clientsOfInterest;
    ns3::Time timeRequirement;
    void sendData();

};

class OtherDataMessage : public Message{

public:
    OtherDataMessage();
    ~OtherDataMessage();
    void startDataTransfer();

private:
    void sendData();

};

class MaintenanceMessage : public Message{

public:
    MaintenanceMessage();
    ~MaintenanceMessage();
    void startDataTransfer();

private:
    void sendData();

};



#endif // NVE_SIMULATOR_H
