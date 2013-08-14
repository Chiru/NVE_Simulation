/**
* Copyright (c) 2013 Center for Internet Excellence, University of Oulu, All Rights Reserved
* For conditions of distribution and use, see copyright notice in license.txt
*/


#ifndef SERVER_H
#define SERVER_H
#include "utilities.h"
#include "DataGenerator.h"
#include "ns3/address.h"


class XMLParser;

class Server
{

public:
    Server(XMLParser&, int, Ptr<Node>, Address*);
    ~Server();
    bool pcapEnabled() const {return pcap;}
    double getStartSeconds() const;

private:
    XMLParser &parser;
    uint16_t numberOfStreams;
    DataGenerator** streams;
    int runningTime;
    Ptr<Node> node;
    Address* address;
    bool pcap;

};


#endif // SERVER_H
