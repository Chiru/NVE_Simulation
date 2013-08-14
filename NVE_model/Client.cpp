#include "Client.h"
#include "XML_parser.h"

//Class Client function definitions

Client::Client(XMLParser& parser, uint16_t no, Ptr<Node> node, Address *peerAddr, Ipv4Address clientAddr)
    : parser(parser),
      streams(0),
      node(node),
      addr(clientAddr)
{
   parser.getStreams(streams, true, no);
   numberOfStreams = parser.getNumberOfStreams();

   if(!parser.getClientStats(no, clientNumber, networkDelay, uplinkBandwidth, downlinkBandwidth, lossRate, pcap, graph, joinTime, exitTime))
       PRINT_ERROR( "Mysterious error while creating " << no << ". client." << std::endl);

   for(int i = 0; i < numberOfStreams; i++)
   {
       streams[i]->SetStartTime(Seconds(joinTime));
       streams[i]->SetStopTime(Seconds(exitTime));
       streams[i]->setupStream(node, peerAddr[i]);
       node->AddApplication(streams[i]);
   }
}


Client::~Client()
{
    uint64_t bytesSent = 0L;

    for(int i = 0; i < parser.getNumberOfStreams(); i++)
    {
        if(streams != 0 && streams[i] != 0)
        {
            bytesSent += streams[i]->getBytesSent();
            delete streams[i];
        }
    }

    if(streams != 0)
         delete[] streams;

    CLIENT_INFO("Client number: " << clientNumber << " finishing, sent " << bytesSent << " bytes in total." << std::endl);
}


std::string Client::getDelayInMilliseconds() const
{
    std::stringstream stream;

    stream << networkDelay << "ms";

    return stream.str();
}


std::string Client::getDownlinkBandwidthInMegabits() const
{

    std::stringstream stream;

    stream << downlinkBandwidth << "Mbps";

    return stream.str();
}


std::string Client::getUplinkBandwidthInMegabits() const
{

    std::stringstream stream;

    stream << uplinkBandwidth << "Mbps";

    return stream.str();
}


int Client::getRunningTime() const
{
    return exitTime - joinTime;
}


