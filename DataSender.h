#ifndef DATASENDER_H
#define DATASENDER_H

#include "ns3/address.h"
#include "ns3/socket.h"
#include "ApplicationProtocol.h"
#include "Messages.h"
#include <map>
//

class DataSender{

public:
    DataSender(ApplicationProtocol* appProto, uint16_t gameTick) : appProto(appProto), gameTick(gameTick){}

    bool send(bool sendNow, uint8_t* buffer, const Message* msg, const Ptr<Socket> sock, bool forward, bool isClient);
    bool sendTo(bool sendNow, uint8_t* buffer, const Message* msg, const Address& addr, bool forward, bool isClient, const Ptr<Socket> sock = 0);
    bool flushUdpBuffer(Ptr<Socket>, bool isClient);
    bool flushTcpBuffer(bool isClient);
    void setGameTick(uint16_t gameTick){this->gameTick = gameTick;}

private:

    std::map<Ptr<Socket>, std::string> tcpBuffer;
    std::map<Address, std::pair<std::string, bool> > udpBuffer;
    ApplicationProtocol* appProto;
    bool isClient;
    uint16_t gameTick;

};

bool DataSender::send(bool sendNow, uint8_t* buffer, const Message* msg, const Ptr<Socket> sock, bool forward, bool isClient){

    uint16_t messageSize;

    if(forward)
        messageSize = msg->getForwardMessageSize();
    else
        messageSize = msg->getMessageSize();

    if(sendNow){
        if(sock->Send((uint8_t*)buffer, messageSize, 0) == -1){
            PRINT_ERROR("Problems with server socket buffer." << std::endl);
            return false;
        }
    }else{
        std::string tempString;
        tempString.append((char*)buffer);
        tempString.resize(messageSize);
        tcpBuffer[sock].append(tempString);

    }

    return true;
}

bool DataSender::sendTo(bool sendNow, uint8_t* buffer, const Message* msg, const Address& addr, bool forward, bool isClient, const Ptr<Socket> sock){

    uint16_t messageSize;

    if(msg->getReliable())
        udpBuffer[addr].second = true;

    if(forward)
        messageSize = msg->getForwardMessageSize();
    else
        messageSize = msg->getMessageSize();

    if(sendNow){
        if(appProto){
            if(isClient){
                if(!appProto->sendFromClient(msg, buffer, sock, forward)){
                    PRINT_ERROR("Problems with server socket buffer." << std::endl);
                    return false;
                }
            }else{
                if(!appProto->sendFromServer(buffer, msg, addr, sock, forward)){
                    PRINT_ERROR("Problems with server socket buffer." << std::endl);
                    return false;
                }
            }
        }else{
            if(sock->SendTo((uint8_t*)buffer, messageSize, 0, addr) == -1){
                PRINT_ERROR("Problems with server socket buffer." << std::endl);
                return false;
            }
        }
    }else{
        std::string tempString;
        tempString.append((char*)buffer);
        tempString.resize(messageSize);
        udpBuffer[addr].first.append(tempString);
    }

    return true;
}

bool DataSender::flushTcpBuffer(bool isClient){

    for(std::map<Ptr<Socket>, std::string>::iterator it = tcpBuffer.begin(); it != tcpBuffer.end(); it++){

        if(it->first->Send((uint8_t*)it->second.c_str(), it->second.length(), 0) == -1){
            PRINT_ERROR("Problems with server socket buffer." << std::endl);
            return false;
        }
        tcpBuffer.erase(it);
    }

    Simulator::Schedule(Time(MilliSeconds(gameTick)), &DataSender::flushTcpBuffer, this, isClient);

    return true;
}

bool DataSender::flushUdpBuffer(Ptr<Socket> sock, bool isClient){

    if(appProto){
        if(isClient){
            for(std::map<Address, std::pair<std::string, bool> >::iterator it = udpBuffer.begin(); it != udpBuffer.end(); it++){
                appProto->sendFromClient(it->second.first, sock, it->second.second);
                it->second.second = false;
            }

        }else{
            for(std::map<Address, std::pair<std::string, bool> >::iterator it = udpBuffer.begin(); it != udpBuffer.end(); it++){
                appProto->sendFromServer(it->second.first, it->first, sock, it->second.second);
                it->second.second = false;
            }
        }
    }else{

        for(std::map<Address, std::pair<std::string, bool> >::iterator it = udpBuffer.begin(); it != udpBuffer.end(); it++){
            if(sock->SendTo((uint8_t*)it->second.first.c_str(), it->second.first.length(), 0, it->first) == -1){
                PRINT_ERROR("Problems with server socket buffer." << std::endl);
                return false;
            }
            it->second.second = false;
        }
    }

    udpBuffer.clear();

    Simulator::Schedule(Time(MilliSeconds(gameTick)), &DataSender::flushUdpBuffer, this, sock, isClient);
    return true;
}

#endif // DATASENDER_H
