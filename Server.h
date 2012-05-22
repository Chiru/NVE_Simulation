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

#ifndef SERVER_H
#define SERVER_H
#include "utilities.h"
#include "DataGenerator.h"
#include "ns3/address.h"


class XMLParser;

class Server{

public:
    Server(XMLParser&, int, Ptr<Node>, Address*);
    ~Server();

private:
    XMLParser &parser;
    uint16_t numberOfStreams;
    DataGenerator** streams;
    int runningTime;
    Ptr<Node> node;
    Address* address;

};


//Class Server function definitions

Server::Server(XMLParser& parser, int runningTime, Ptr<Node> node, Address* addr): parser(parser), runningTime(runningTime), node(node), address(addr){

    parser.getStreams(streams, false);
    numberOfStreams = parser.getNumberOfStreams();

    for(int i = 0; i < numberOfStreams; i++){
        streams[i]->SetStartTime(Seconds(0));
        streams[i]->SetStopTime(Seconds(runningTime));
        streams[i]->setupStream(node, address[i]);
        node->AddApplication(streams[i]);
    }

}

Server::~Server(){

    uint64_t bytesSent = 0L;

    for(int i = 0; i < numberOfStreams; i++){
        bytesSent += streams[i]->getBytesSent();
        delete streams[i];
    }

    delete[] streams;

    SERVER_INFO("Server finishing, sent " << bytesSent << " bytes in total." << std::endl);

}

#endif // SERVER_H
