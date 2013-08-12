#include "DataSender.h"


bool DataSender::send(bool sendNow, uint8_t* buffer, const Message* msg, const Ptr<Socket> sock, bool forward, bool isClient){

    uint16_t messageSize;

    int msgId;
    msg->parseMessageId(std::string((char*)buffer), msgId);

    if(forward)
        messageSize = msg->getForwardMessageSize(msgId);
    else
        messageSize = msg->getMessageSize(msgId);


    if(sendNow){

        std::string tempString;
        tempString.append((char*)buffer);
        tempString.resize(messageSize);

        if(sock->Send((uint8_t*)tempString.c_str(), messageSize, 0) == -1){
            PRINT_ERROR("Problems with server socket buffer." << std::endl);
            return false;
        }
    }else{
        std::string tempString;
        tempString.append((char*)buffer);
        tempString.resize(messageSize);
        tcpBuffer[sock].append(tempString);
        if(isClient && !forward)
            clientMessagesInTcpBuffer.push_back(MessageInfo(msg->getStreamNumber(), msgId, msg->getName()));
        else if(!isClient && !forward)
                  serverMessagesInTcpBuffer.push_back(MessageInfo(msg->getStreamNumber(), msgId, msg->getName()));
    }

    return true;
}

bool DataSender::sendTo(bool sendNow, uint8_t* buffer, const Message* msg, const Address& addr, bool forward, bool isClient, const Ptr<Socket> sock){

    uint16_t messageSize;

    if(msg->getReliable())
        udpBuffer[addr].second = true;

    int msgId;
    msg->parseMessageId(std::string((char*)buffer), msgId);

    if(forward)
        messageSize = msg->getForwardMessageSize(msgId);
    else
        messageSize = msg->getMessageSize(msgId);

    if(sendNow){

        std::string tempString;
        tempString.append((char*)buffer);
        tempString.resize(messageSize);

        if(appProto){
            if(isClient){
                if(!appProto->sendFromClient(msg, (uint8_t*)tempString.c_str(), sock, forward)){
                    PRINT_ERROR("Problems with server socket buffer." << std::endl);
                    return false;
                }
            }else{
                if(!appProto->sendFromServer((uint8_t*)tempString.c_str(), msg, addr, sock, forward)){
                    PRINT_ERROR("Problems with server socket buffer." << std::endl);
                    return false;
                }
            }
        }else{
            if(sendAndFragment(sock, (uint8_t*)tempString.c_str(), messageSize, msg->getReliable(), &addr) == -1){
                PRINT_ERROR("Problems with server socket buffer." << std::endl);
                return false;
            }
        }
    }else{
        std::string tempString;
        tempString.append((char*)buffer);
        tempString.resize(messageSize);
        udpBuffer[addr].first.append(tempString);
        if(isClient && !forward)
            clientMessagesInUdpBuffer.push_back(MessageInfo(msg->getStreamNumber(), msgId, msg->getName()));
        else if(!isClient && !forward)
                 serverMessagesInUdpBuffer.push_back(MessageInfo(msg->getStreamNumber(), msgId, msg->getName()));
    }

    return true;
}


bool DataSender::flushTcpBuffer(bool isClient){

    Time interval;

    if(isClient){
        for(std::vector<MessageInfo>::iterator msgIt = clientMessagesInTcpBuffer.begin(); msgIt != clientMessagesInTcpBuffer.end(); msgIt++){
            interval = getMessageSendInterval( msgIt->name);
        }
        clientMessagesInTcpBuffer.clear();
    }else{
        for(std::vector<MessageInfo>::iterator msgIt = serverMessagesInTcpBuffer.begin(); msgIt != serverMessagesInTcpBuffer.end(); msgIt++){
            interval = getMessageSendInterval(msgIt->name);
        }
        serverMessagesInTcpBuffer.clear();
    }

    for(std::map<Ptr<Socket>, std::string>::iterator it = tcpBuffer.begin(); it != tcpBuffer.end(); it++){

        if(it->first->Send((uint8_t*)it->second.c_str(), it->second.length(), 0) == -1){

            PRINT_ERROR("Problems with socket buffer. Error number: " << it->first->GetErrno()  << std::endl);
            return false;
        }

        tcpBuffer.erase(it);
    }



    Simulator::Schedule(Time(MilliSeconds(gameTick)), &DataSender::flushTcpBuffer, this, isClient);

    return true;
}

bool DataSender::flushUdpBuffer(Ptr<Socket> sock, bool isClient){

    Time interval;

    if(isClient){
        for(std::vector<MessageInfo>::iterator it = clientMessagesInUdpBuffer.begin(); it != clientMessagesInUdpBuffer.end(); it++){

            interval = getMessageSendInterval(it->name);
        }
        clientMessagesInUdpBuffer.clear();
    }else{
        for(std::vector<MessageInfo>::iterator it = serverMessagesInUdpBuffer.begin(); it != serverMessagesInUdpBuffer.end(); it++){

            interval = getMessageSendInterval(it->name);
        }
        serverMessagesInUdpBuffer.clear();
    }


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

            if(sendAndFragment(sock,(uint8_t*)it->second.first.c_str(), it->second.first.length(), false, &(it->first)) == -1){
                PRINT_ERROR("Problems with socket buffer." << std::endl);
                return false;
            }
            it->second.second = false;
        }
    }

    udpBuffer.clear();
    Simulator::Schedule(Time(MilliSeconds(gameTick)), &DataSender::flushUdpBuffer, this, sock, isClient);
    return true;
}

int DataSender::sendAndFragment(Ptr<Socket> socket, uint8_t *buffer, uint16_t size, bool reliable, const Address *const addr){

    if(size > maxDatagramSize){

        std::string tempBuffer((char*)buffer, size);
        uint16_t headerSize = 0;
        if(appProto == 0)
            headerSize = 0;
        else
            headerSize = appProto->getHeaderSize();

        return ApplicationProtocol::sendFragment(tempBuffer, 0, socket, maxDatagramSize, addr, appProto, reliable, headerSize);

    }else{

        if(addr == 0){
            return socket->Send(buffer, size, 0);
        }else{
            return socket->SendTo(buffer, size, 0, *addr);
        }
    }

    return -1;  //we should never get here
}

Time DataSender::getMessageSendInterval(const std::string& name){

    Time interval;

    if(lastSendTimes.count(name) == 0){
        lastSendTimes.insert(std::make_pair<std::string, Time>(std::string(name), Time("0ms")));
    }

    if(lastSendTimes.at(name).IsZero()){
        lastSendTimes.at(name) = Simulator::Now();
        interval = Time("0ms");
    }else{
        interval = Simulator::Now() -  lastSendTimes.at(name);
        lastSendTimes.at(name)= Simulator::Now();
    }

    return interval;

}


