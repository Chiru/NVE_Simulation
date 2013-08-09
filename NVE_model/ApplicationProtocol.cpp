#include "ApplicationProtocol.h"

//Class ApplicationProtocol function definitions

ApplicationProtocol::ApplicationProtocol(uint16_t packetSize, uint16_t delayedAck, uint16_t retransmit, uint16_t headerSize)
    : ackSize(packetSize),
      delayedAck(delayedAck),
      retransmit(retransmit),
      headerSize(headerSize),
      socket(0),
      maxDatagramSize(1400) //TODO: hard coding
{
}

ApplicationProtocol::~ApplicationProtocol(){

    for(std::list<ReliablePacket*>::iterator it = packetsWaitingAcks.begin(); it != packetsWaitingAcks.end(); it++){
        delete(*it);
    }

}


bool ApplicationProtocol::sendFromClient(const Message *msg, uint8_t *buffer, Ptr<Socket> socket, bool forward){

    if(this->socket == 0){
        this->socket = socket;
    }

    int bytesSent = 0;

    uint16_t messageSize;
    int msgId;

    msg->parseMessageId(std::string((char*)buffer), msgId);

    if(!forward)
        messageSize = msg->getMessageSize(msgId);
    else
        messageSize = msg->getForwardMessageSize(msgId);

    char msgContents[messageSize + headerSize];
    msgContents[0] = '\0';

    addAppProtoHeader(msgContents, msg->getReliable());
    memcpy(msgContents + headerSize, (char*)buffer, messageSize);







    if(socket->GetTxAvailable() < messageSize + headerSize){        //TODO: how to remember messages when buffer overflows
        return false;
    }

    if((bytesSent = sendAndFragment(socket, (uint8_t*)msgContents, messageSize + headerSize, msg->getReliable(), 0)) == -1){
        return false;
    }
/*
    if(msg->getReliable())
    {
        rememberReliablePacket(reliableMsgNumber.begin()->second, messageSize, (uint8_t*)msgContents, &ApplicationProtocol::resendCheckClient);
    }

    if(msg->getReliable()){
        uint8_t* tempMsg;
        tempMsg = (uint8_t*)malloc(messageSize + headerSize);
        memcpy(tempMsg, msgContents, messageSize + headerSize);
        packetsWaitingAcks.push_back(new ApplicationProtocol::ReliablePacket((reliableMsgNumber.begin())->second, msg->getMessageSize(msgId), tempMsg));
        Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, (reliableMsgNumber.begin())->second);
    }*/

    totalBytesSent += bytesSent;

    return true;
}

bool ApplicationProtocol::sendFromClient(const std::string &buffer, Ptr<Socket> socket, bool reliable){

    if(this->socket == 0)
        this->socket = socket;

    char* msgContents = new char[buffer.length() + headerSize];
    msgContents[0] = 0;

    addAppProtoHeader(msgContents, reliable);
    memcpy(msgContents + headerSize, buffer.c_str(), buffer.length());

    if(socket->GetTxAvailable() < buffer.length() + headerSize){        //TODO: how to remember messages when buffer overflows
        delete [] msgContents;
        return false;
    }

    if(sendAndFragment(socket, (uint8_t*)msgContents, buffer.length() + headerSize, reliable, 0) == -1){
        delete [] msgContents;
        return false;
    }

    /* if(reliable)
    {
        rememberReliablePacket(reliableMsgNumber.begin()->second, buffer.length() + headerSize, (uint8_t*)msgContents, &ApplicationProtocol::resendCheckClient);
    }

   if(reliable){
        uint8_t* tempMsg;
        tempMsg = (uint8_t*)malloc(buffer.length() + headerSize);
        memcpy(tempMsg, msgContents, buffer.length() + headerSize);
        packetsWaitingAcks.push_back(new ApplicationProtocol::ReliablePacket((reliableMsgNumber.begin())->second, buffer.length(), tempMsg));
        Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, (reliableMsgNumber.begin())->second);
    }*/

    delete [] msgContents;

    return true;
}

void ApplicationProtocol::recv(Ptr<Socket> socket){

    if(this->socket == 0)
        this->socket = socket;

    Address addr;
    uint8_t* buffer = 0;
    uint16_t bufferSize;
    bufferSize = socket->GetRxAvailable();
    buffer = (uint8_t*)calloc(bufferSize, 1);
    socket->RecvFrom(buffer, bufferSize, 0, addr);

    uint32_t msgNumber;
    ReliablePacket* packet = 0;

    switch(parseAppProtoHeader(buffer, addr, msgNumber)){

    case UNRELIABLE:
    case RELIABLE:

                forwardToApplication(buffer + headerSize, bufferSize - headerSize, addr);
                free(buffer);
                while((packet = getAllOrdered(addr, ++msgNumber))){
                    forwardToApplication(packet->buffer + headerSize, packet->msgSize - headerSize, addr);
                    lastOrderedNumber[addr] = msgNumber;
                    packetsToAck[addr].push_back(msgNumber);
                    delete packet;
                }

                if(delayedAck == 0){
                    ackAllPackets();
                }

            break;

        case UNORDERED:
            packetsOutOfOrder.push_back(new ReliablePacket(msgNumber, bufferSize, buffer, addr));

            break;

        case DUPLICATE: //this is already handled
        case ACK:  //this is only ack, do not forward to application
            break;

        case ERROR:
            PRINT_ERROR("Mysterious error occured in application layer protocol." << std::endl);
            break;
    }


}


ApplicationProtocol::ReliablePacket* ApplicationProtocol::getAllOrdered(const Address& addr, uint32_t reliableMsgNumber){

    ReliablePacket* retval = 0;

    for(std::list<ReliablePacket*>::iterator it = packetsOutOfOrder.begin(); it != packetsOutOfOrder.end(); it++){
        if((*it)->addr == addr && (*it)->msgNumber == reliableMsgNumber){
            retval = (*it);
            packetsOutOfOrder.remove(*it);
            break;
        }
    }

    return retval;

}


bool ApplicationProtocol::sendFromServer(uint8_t *buffer, const Message* msg, const Address& addr, Ptr<Socket> socket, bool forward){

    uint16_t size;
    int msgId;

    msg->parseMessageId(std::string((char*)buffer), msgId);

    if(forward)
        size = (msg)->getForwardMessageSize(msgId);
    else
        size = msg->getMessageSize(msgId);

    if(this->socket == 0)
        this->socket = socket;

    int bytesSent = 0;

    char msgContents[size + headerSize]; //TODO: hard-coded message size

    msgContents[0] = '\0';

    addAppProtoHeader(msgContents, msg->getReliable(), &addr);
    strncpy(msgContents + headerSize, (char*)buffer, size);

    if(socket->GetTxAvailable() < size + headerSize){        //TODO: how to remember messages when buffer overflows
        return false;
    }

    if((bytesSent =  sendAndFragment(socket,(uint8_t*)msgContents,size + headerSize, msg->getReliable(),&addr)) == -1){
        return false;
    }

    /*if(msg->getReliable())
    {
        rememberReliablePacket(reliableMsgNumber, size, (uint8_t*)msgContents, addr, &ApplicationProtocol::resendCheckServer);

    }

    if(msg->getReliable()){
        uint8_t* tempMsg;
        tempMsg = (uint8_t*)malloc(size + headerSize);
        memcpy(tempMsg, msgContents, size + headerSize);
        packetsWaitingAcks.push_back(new ApplicationProtocol::ReliablePacket(reliableMsgNumber[addr], size, tempMsg, addr));
        Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckServer, this, reliableMsgNumber, addr);
    }*/

    totalBytesSent += bytesSent;

    return true;
}

bool ApplicationProtocol::sendFromServer(const std::string &buffer, const Address &addr, Ptr<Socket> sock, bool reliable){

    uint16_t size = buffer.length();

    if(this->socket == 0)
        this->socket = sock;

    char* msgContents = new char[size + headerSize];
    msgContents[0] = 0;

    addAppProtoHeader(msgContents, reliable, &addr);
    memcpy(msgContents + headerSize, buffer.c_str(), size);

    if(sock->GetTxAvailable() < size + headerSize){        //TODO: how to remember messages when buffer overflows
        delete [] msgContents;
        return false;
    }

    if(sendAndFragment(sock,(uint8_t*)msgContents,size + headerSize, reliable, &addr) == -1){
        delete [] msgContents;
        return false;
    }

    /*if(reliable)
    {
        rememberReliablePacket(reliableMsgNumber, size, (uint8_t*)msgContents, addr, &ApplicationProtocol::resendCheckServer);

    }
    if(reliable){
        uint8_t* tempMsg;
        tempMsg = (uint8_t*)malloc(size + headerSize);
        memcpy(tempMsg, msgContents, size + headerSize);
        packetsWaitingAcks.push_back(new ApplicationProtocol::ReliablePacket(reliableMsgNumber[addr], size, tempMsg, addr));
        Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckServer, this, reliableMsgNumber, addr);
    }*/

    delete [] msgContents;

    return true;
}

void ApplicationProtocol::configureForStream(Callback<void, uint8_t*, uint16_t, Address&> memFunc, bool ordered){
    Simulator::Schedule(Time(MilliSeconds(delayedAck)), &ApplicationProtocol::ackAllPackets, this);
    this->ordered = ordered;
    forwardToApplication = memFunc;
}


void ApplicationProtocol::resendCheckClient(uint32_t reliableMsgNumber){

    for(std::list<ApplicationProtocol::ReliablePacket*>::const_iterator it = packetsWaitingAcks.begin(); it != packetsWaitingAcks.end(); it++){
        if((**it).msgNumber == reliableMsgNumber){
            if(socket->GetTxAvailable() < (**it).msgSize){
                Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, reliableMsgNumber);
                return;
            }
            //sendAndFragment(socket, (**it).buffer,(**it).msgSize + headerSize, true);
            socket->Send((**it).buffer, (**it).msgSize, 0);
            Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, reliableMsgNumber);
            break;
        }
    }
}

void ApplicationProtocol::resendCheckServer(std::map<const Address, uint32_t>& reliableMsgNumber, const Address& addr){

    for(std::list<ApplicationProtocol::ReliablePacket*>::const_iterator it = packetsWaitingAcks.begin(); it != packetsWaitingAcks.end(); it++){
        if((**it).msgNumber == reliableMsgNumber[addr] && (**it).addr == addr){
            if(socket->GetTxAvailable() < (**it).msgSize){
                Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckServer, this, reliableMsgNumber, addr);
                return;
            }
            //sendAndFragment(socket, (**it).buffer, (**it).msgSize + headerSize, true, &(**it).addr);
            socket->SendTo((**it).buffer, (**it).msgSize, 0, (**it).addr);
            Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckServer, this, reliableMsgNumber, addr);
            break;
        }
    }
}

void ApplicationProtocol::addAppProtoHeader(char *buffer, bool reliable, const Address* addr){

    std::stringstream str("");
    str << "\"app:";

    if(addr == 0){

        if(reliableMsgNumber.empty()){
            reliableMsgNumber.insert(std::pair<Address, uint32_t>(Address(), 0));
        }

        if(reliable)
            str << ++(reliableMsgNumber.begin()->second);
        else
            str << 0;
    }else{

        if(reliableMsgNumber.count(*addr) == 0){
            reliableMsgNumber.insert(std::pair<Address, uint32_t>(*addr, 0));

        }

        if(reliable)
            str << ++(reliableMsgNumber[*addr]);
        else
            str << 0;
    }

    str << "\"";

    strncpy(buffer, str.str().c_str(), headerSize);
}

ApplicationProtocol::AppProtoPacketType ApplicationProtocol::parseAppProtoHeader(uint8_t* buffer, const Address& addr, uint32_t &msgNumber){

    if(strncmp((char*)buffer, "\"app:0\"", 7) == 0)
        return UNRELIABLE;

    if(strncmp((char*)buffer, "\"app:", 5) == 0){
        char* numberStr = strndup((char*)buffer + 5, strlen((char*)buffer) - 5);
        msgNumber = atoi(numberStr);
        free(numberStr);

        if(packetsToAck.count(addr) == 1){
            for(std::list<uint32_t>::const_reverse_iterator it = alreadyAcked[addr].rbegin(); it != alreadyAcked[addr].rend(); it++){
                if(*it == msgNumber){
                    packetsToAck[addr].push_back(msgNumber);
                    return DUPLICATE;
                }
            }


            for(std::deque<uint32_t>::const_reverse_iterator it = packetsToAck[addr].rbegin();  it != packetsToAck[addr].rend(); it++){
                if(*it == msgNumber){
                    return DUPLICATE;
                }
            }

            if(ordered){
                if(lastOrderedNumber[addr] == msgNumber -1){
                    lastOrderedNumber[addr] = msgNumber;
                    packetsToAck[addr].push_back(msgNumber);
                    return RELIABLE;

                }else{
                    return UNORDERED;
                }

            }else{
                packetsToAck[addr].push_back(msgNumber);
                return RELIABLE;
            }


        }else{
            packetsToAck.insert(std::make_pair<Address, std::deque<uint32_t> >(addr, std::deque<uint32_t>()));
            lastOrderedNumber.insert(std::make_pair<const Address&, uint32_t> (addr, 0));
            if(ordered){
                if(lastOrderedNumber[addr] == msgNumber-1){
                    lastOrderedNumber[addr] = msgNumber;
                    packetsToAck[addr].push_back(msgNumber);
                    return RELIABLE;
                }else{
                    return UNORDERED;
                }

            }else{
                packetsToAck[addr].push_back(msgNumber);
            }
        }

        return RELIABLE;
    }

    if(strncmp((char*)buffer, "\"ack:", 5) == 0){
        char* numberStr = strndup((char*)buffer + 5, strlen((char*)buffer) - 5);
        std::stringstream stream;
        uint32_t msgNumber;
        char c;
        stream << numberStr;
        std::list<ApplicationProtocol::ReliablePacket*>::iterator it;

        do{
            stream >> msgNumber;
            for(it = packetsWaitingAcks.begin(); it != packetsWaitingAcks.end(); it++){
                if((**it).msgNumber == msgNumber && ((**it).addr.IsInvalid() || (**it).addr == addr)){
                    delete *it;
                    packetsWaitingAcks.erase(it);
                    break;
                }
            }
        }while(stream >> c, c == ',');

        free(numberStr);
        return ACK;
    }

    return ERROR; //if this happens, there's a bug
}

void ApplicationProtocol::ackAllPackets(){
    int* messages = 0;
    int i;

    for(std::map<Address, std::deque<uint32_t> >::iterator it = packetsToAck.begin(); it != packetsToAck.end(); it++){
        if(it->second.empty())
            continue;

        messages = new int[it->second.size()];
        i = 0;

        while(!(it->second.empty())){
            messages[i++] = it->second.front();
            it->second.pop_front();
        }

        if(sendAck(messages, i, it->first, socket)){    //TODO: if there's problems with socket buffer, no ack is sent and packet gets "lost"
            if(alreadyAcked.count(it->first) == 0){
                alreadyAcked.insert(std::make_pair<Address, std::list<uint32_t> >(it->first, std::list<uint32_t>()));
                for(int h = 0; h < i; h++){
                    alreadyAcked[it->first].push_back(messages[h]);
                }

            }else{
                for(int h = 0; h < i; h++){
                    alreadyAcked[it->first].push_back(messages[h]);
                }
            }
        }
        delete [] messages;
    }

    if(delayedAck > 0)
        Simulator::Schedule(Time(MilliSeconds(delayedAck)), &ApplicationProtocol::ackAllPackets, this);

}

bool ApplicationProtocol::sendAck(int *messagesToAck, uint16_t numberOfMessages, const Address &addr, Ptr<Socket> sock){

    char ack[ackSize * numberOfMessages];

    uint16_t ackSize = createAck(ack, messagesToAck, numberOfMessages);

    if(sock->GetTxAvailable() < (uint32_t)(ackSize))
        return false;

    if(sock->SendTo((uint8_t*)ack, ackSize, 0, addr) == -1)
        return false;

    return true;
}


void ApplicationProtocol::transmissionStopped(const Address &addr, bool isClient)
{
    std::list<ReliablePacket*>::iterator it;

    for(it = packetsWaitingAcks.begin(); it != packetsWaitingAcks.end(); it++)
    {
        if(isClient || (*it)->addr == addr)
        {
            delete *it;
            it = packetsWaitingAcks.erase(it);
            it--;
        }
    }

    for(it = packetsOutOfOrder.begin(); it != packetsOutOfOrder.end(); it++)
    {
        if(isClient ||(*it)->addr == addr)
        {
            delete *it;
            it = packetsOutOfOrder.erase(it);
            it--;

        }
    }

    packetsToAck.erase(addr);
    alreadyAcked.erase(addr);

}


uint16_t ApplicationProtocol::createAck(char *ack, int* number, uint16_t numberOfMessages){

    std::stringstream str;
    str << "\"ack:";
    for(int i = 0; i < numberOfMessages; i++){
        str << number[i];
        if((i + 1) < numberOfMessages){
            str << ",";
        }
    }
    str << "\"";

    uint16_t ackSize;

    if(str.str().length() <= this->ackSize)
        ackSize = this->ackSize;
    else
        ackSize = str.str().length();

    strncpy(ack, str.str().c_str(), ackSize);

    return ackSize;
}

int ApplicationProtocol::sendAndFragment(Ptr<Socket> socket, uint8_t *buffer, uint16_t size, bool reliable, const Address *const addr){

    if(size > maxDatagramSize){

        std::string tempBuffer((char*)buffer, size);
        return sendFragment(tempBuffer, 0, socket, maxDatagramSize, addr, this, reliable, headerSize);

    }else{

        if(addr == 0)
        {
            if(reliable)
            {
                rememberReliablePacket(reliableMsgNumber.begin()->second, size, buffer, &ApplicationProtocol::resendCheckClient);
            }

            return socket->Send(buffer, size, 0);

        }else
        {
            if(reliable)
            {
                rememberReliablePacket(reliableMsgNumber, size, buffer, *addr, &ApplicationProtocol::resendCheckServer);
            }

            return socket->SendTo(buffer, size, 0, *addr);
        }
    }

    return -1;  //we should never get here
}



int ApplicationProtocol::sendFragment(const std::string& buffer, const size_t index, Ptr<Socket> sock, uint16_t maxDatagramSize, const Address* const addr, ApplicationProtocol* appProto, bool reliable,
                                      uint16_t headerSize){

    size_t tempIndex = 0, tempIndex2 = 0;
    tempIndex = buffer.find('"', index);   //find first "
    tempIndex = buffer.find('"', tempIndex + 1);  // find second "
    if((tempIndex = buffer.find('"', tempIndex + 1)) == std::string::npos){  //see if there's third
        if(addr == 0)
        {
            if(reliable)
                appProto->rememberReliablePacket(appProto->reliableMsgNumber.begin()->second, buffer.length(), (uint8_t*)buffer.substr(0, buffer.length()).c_str(), &ApplicationProtocol::resendCheckClient);

            return sock->Send((uint8_t*)buffer.substr(0, buffer.length()).c_str(), buffer.length(), 0);
        }
        else
        {
            if(reliable)
               appProto->rememberReliablePacket(appProto->reliableMsgNumber, buffer.length(), (uint8_t*)buffer.substr(0, buffer.length()).c_str(), *addr, &ApplicationProtocol::resendCheckServer);

            return sock->SendTo((uint8_t*)buffer.substr(0, buffer.length()).c_str(), buffer.length(), 0, *addr);
        }
    }else{
        if( tempIndex >= maxDatagramSize || (tempIndex2 = buffer.find('"', tempIndex + 1)) >= maxDatagramSize || buffer.length() > maxDatagramSize){
            int retval = 0;
            char header[headerSize];
            std::string tempString;
            std::string sendString;

            if(buffer.length() > maxDatagramSize)
            {
                if(buffer[maxDatagramSize-1] != 0)
                {
                    tempIndex = buffer.substr(0, maxDatagramSize-1).find_last_of('"');
                    tempIndex = buffer.substr(0, tempIndex).find_last_of('"');
                }
                else
                {
                   tempIndex = buffer.substr(0, maxDatagramSize).find_last_of('"');
                }
                tempIndex = buffer.substr(0, tempIndex).find_last_of('"');
                sendString = buffer.substr(0, tempIndex);
            }
            else
            {
                sendString = buffer.substr(0, tempIndex);
            }

            if(addr == 0){

                if(reliable)
                    appProto->rememberReliablePacket(appProto->reliableMsgNumber.begin()->second, tempIndex, (uint8_t*)sendString.c_str(), &ApplicationProtocol::resendCheckClient);

                retval = sock->Send((uint8_t*)sendString.c_str(), tempIndex, 0);

                if(appProto){
                    appProto->addAppProtoHeader(header, reliable, addr);
                    tempString.assign(header, headerSize);
                }

                tempString.append(buffer.substr(tempIndex, buffer.length()-tempIndex));


                if(retval == -1){
                    return -1;
                }

                return (sendFragment(tempString, 0, sock,maxDatagramSize,  addr, appProto, reliable, headerSize) + retval);
            }
            else{

                if(reliable)
                    appProto->rememberReliablePacket(appProto->reliableMsgNumber, tempIndex, (uint8_t*)sendString.c_str(), *addr, &ApplicationProtocol::resendCheckServer);


                retval = sock->SendTo((uint8_t*)sendString.c_str(), tempIndex, 0, *addr);

                if(appProto){
                    appProto->addAppProtoHeader(header, reliable, addr);
                    tempString.assign(header, headerSize);
                }

                tempString.append(buffer.substr(tempIndex, buffer.length()-tempIndex));

                if(retval == -1){
                    return -1;
                }

                return (sendFragment(tempString, 0, sock, maxDatagramSize, addr, appProto, reliable, headerSize) + retval);

            }
        }else{
            return sendFragment(buffer, tempIndex, sock, maxDatagramSize, addr, appProto, reliable, headerSize);
        }

    }

    return -1; //we should never get here
}


void ApplicationProtocol::rememberReliablePacket(uint32_t messageNumber, uint16_t messageSize, uint8_t *messageContents, void (ApplicationProtocol::*fptr)(uint32_t))
{
    uint8_t* tempMsg;
    tempMsg = (uint8_t*)malloc(messageSize + headerSize);
    memcpy(tempMsg, messageContents, messageSize + headerSize);
    packetsWaitingAcks.push_back(new ApplicationProtocol::ReliablePacket(messageNumber, messageSize, tempMsg));
    Simulator::Schedule(Time(MilliSeconds(retransmit)), fptr, this, messageNumber);
}

void ApplicationProtocol::rememberReliablePacket(std::map<const Address, uint32_t> & reliableMsgNumber, uint16_t messageSize, const uint8_t *messageContents, const Address &addr,
                                                 void (ApplicationProtocol::*fptr)(std::map<const Address, uint32_t>&, const Address&))
{
    uint8_t* tempMsg;
    tempMsg = (uint8_t*)malloc(messageSize + headerSize);
    memcpy(tempMsg, messageContents, messageSize + headerSize);
    packetsWaitingAcks.push_back(new ApplicationProtocol::ReliablePacket(reliableMsgNumber[addr], messageSize, tempMsg, addr));
    Simulator::Schedule(Time(MilliSeconds(retransmit)), fptr, this, reliableMsgNumber, addr);
}

