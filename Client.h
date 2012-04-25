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

#ifndef CLIENT_H
#define CLIENT_H


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



//Class Client function definitions

Client::Client(XMLParser& parser, uint16_t no): parser(parser), streams(0){


   parser.getStreams(streams);

   if(!parser.getClientStats(no, clientNumber, networkDelay, uplinkBandwidth, downlinkBandwidth, lossRate))
       std::cerr << "Mysterious error while creating " << no << ". client." << std::endl;

}

Client::~Client(){

    for(int i = 0; i < parser.getNumberOfStreams(); i++){
        if(streams[i] != 0)
            delete streams[i];
    }

    if(streams != 0)
         delete[] streams;

}

std::string Client::getDelayInMilliseconds(){

    std::stringstream stream;

    stream << networkDelay << "ms";

    return stream.str();

}

#endif // CLIENT_H
