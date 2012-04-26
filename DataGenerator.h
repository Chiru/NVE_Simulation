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

class DataGenerator : public ns3::Application{

    friend std::ostream& operator<<(std::ostream& out, const DataGenerator &stream){

        out << "\n\tStream: " << stream.streamNumber << "  protocol: "
            << (stream.proto == TCP_NAGLE_DISABLED ? "TCP  Nagle's disabled" : stream.proto == TCP_NAGLE_ENABLED ? "TCP  Nagle's enabled" :
            (stream.appProto == 0 ? "UDP  Application protocol: no" : "UDP  Application protocol: yes"));
        return out;

    }


public:
    enum Protocol{TCP_NAGLE_DISABLED, TCP_NAGLE_ENABLED, UDP};
    DataGenerator(){}
    DataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto);
    virtual ~DataGenerator();

    virtual void StartApplication();
    virtual void StopApplication();
    uint16_t getStreamNumber() const{return streamNumber;}
    Protocol getProtocol() const{return proto;}
    ApplicationProtocol* getApplicationProtocol() const{return appProto;}

protected:
    uint16_t streamNumber;
    Protocol proto;
    ApplicationProtocol* appProto;

};

class ClientDataGenerator : public DataGenerator{

public:
    ClientDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto);
    ClientDataGenerator(const DataGenerator&);
    ~ClientDataGenerator();
};


class ServerDataGenerator : public DataGenerator{

public:
    ServerDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto);
    ServerDataGenerator(const DataGenerator&);
    ~ServerDataGenerator();

};


//Class DataGenerator function definitions

DataGenerator::DataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto): streamNumber(streamNumber), proto(proto), appProto(appProto){


}

DataGenerator::~DataGenerator(){

    if(appProto != 0){
        delete appProto;
    }

}

void DataGenerator::StartApplication(){


}

void DataGenerator::StopApplication(){


}


//Class ClientDataGenerator function definitions

ClientDataGenerator::ClientDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto): DataGenerator(streamNumber, proto, appProto){


}

ClientDataGenerator::ClientDataGenerator(const DataGenerator& stream){

    this->streamNumber = stream.getStreamNumber();

    if(stream.getApplicationProtocol() != 0)
        this->appProto = new ApplicationProtocol((*(stream.getApplicationProtocol())));
    else this->appProto = 0;

    this->proto = stream.getProtocol();

}

ClientDataGenerator::~ClientDataGenerator(){


}



//Class ServerDataGenerator function definitions

ServerDataGenerator::ServerDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto): DataGenerator(streamNumber, proto, appProto){


}

ServerDataGenerator::ServerDataGenerator(const DataGenerator& stream){

}

ServerDataGenerator::~ServerDataGenerator(){

}

#endif // DATAGENERATOR_H
