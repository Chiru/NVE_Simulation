#include "Server.h"
#include "XML_parser.h"

//Class Server function definitions

Server::Server(XMLParser& parser, int runningTime, Ptr<Node> node, Address* addr): parser(parser), runningTime(runningTime), node(node), address(addr){

    parser.getStreams(streams, false);
    numberOfStreams = parser.getNumberOfStreams();

    for(int i = 0; i < numberOfStreams; i++){
        streams[i]->SetStartTime(Seconds(0));
        streams[i]->SetStopTime(Seconds(runningTime +1));
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

