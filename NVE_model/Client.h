/**
* Copyright (c) 2013 Center for Internet Excellence, University of Oulu, All Rights Reserved
* For conditions of distribution and use, see copyright notice in license.txt
*/


#ifndef CLIENT_H
#define CLIENT_H

#include "utilities.h"
#include "ns3/address.h"
#include "DataGenerator.h"

class XMLParser;


class Client
{

    friend std::ostream& operator<<(std::ostream &out, const Client &client)
    {
        out << "Client number: " << client.clientNumber << "  Network delay: "
            << client.networkDelay  << "  Downlink bandwidth: " << client.downlinkBandwidth
            << "  Uplink bandwidth: " << client.uplinkBandwidth
            << "  Packet loss rate: " << client.lossRate;

        for(int i = 0; i < client.numberOfStreams; i++)
        {
             if((client.streams[i]) != 0)
                out << *(client.streams[i]);
        }

        return out;
    }

public:
    Client(XMLParser&, uint16_t no, Ptr<Node> node, Address *peerAddr, Ipv4Address clientAddr);
    ~Client();

    std::string getDelayInMilliseconds() const;
    std::string getUplinkBandwidthInMegabits() const;
    std::string getDownlinkBandwidthInMegabits() const;
    double getLossRate() const {return lossRate;}
    bool pcapEnabled() const {return pcap;}
    bool graphsEnabled() const {return graph;}
    Ipv4Address getAddress() const {return addr;}
    int getRunningTime() const;
    int getJoinTime() const {return joinTime;}
    int getExitTime() const {return exitTime;}

private:
    XMLParser &parser;
    DataGenerator** streams;
    int networkDelay;
    double uplinkBandwidth;
    double downlinkBandwidth;
    double lossRate;
    uint16_t clientNumber;
    uint16_t numberOfStreams;
    Ptr<Node> node;
    Ipv4Address addr;
    bool pcap;
    bool graph;
    int joinTime;
    int exitTime;
};


#endif // CLIENT_H
