#include "DataGenerator.h"


//Class DataGenerator function definitions

DataGenerator::DataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int tick, bool ordered)
    : streamNumber(streamNumber), proto(proto), appProto(appProto), messages(messages), running(false), totalBytesSent(0), gameTick(tick), ordered(ordered), sender(DataSender(appProto, tick)){

    if(tick == 0)
        immediateSend = true;
    else
        immediateSend = false;

}

DataGenerator::~DataGenerator(){

    if(appProto != 0){
        delete appProto;
    }

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
            delete *it;
    }
}


bool DataGenerator::setupStream(Ptr<Node> node, Address addr){

    peerAddr = addr;

    sender.setGameTick(gameTick);

    if(gameTick == 0)
        immediateSend = true;
    else
        immediateSend = false;

    switch(proto){
    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpReno"));
        case TCP_NAGLE_DISABLED:
            socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
            socket->SetAttribute("TcpNoDelay", BooleanValue(true));
            socket->SetAttribute("SegmentSize", UintegerValue(1400));
           //socket->SetAttribute("DelAckCount", UintegerValue(100));
           // socket->SetAttribute("DelAckTimeout", TimeValue(Time("47ms")));
            break;

        case TCP_NAGLE_ENABLED:
            socket = Socket::CreateSocket(node, TcpSocketFactory::GetTypeId());
            socket->SetAttribute("TcpNoDelay", BooleanValue(false));
            socket->SetAttribute("SegmentSize", UintegerValue(1400));
           // socket->SetAttribute("DelAckCount", UintegerValue(100));
           // socket->SetAttribute("DelAckTimeout", TimeValue(Time("45ms")));
            break;

        case UDP:
            socket = Socket::CreateSocket(node, UdpSocketFactory::GetTypeId());
            break;

    }

    node->AddApplication(this);

    return true;

}

DataGenerator::ReadMsgNameReturnValue DataGenerator::readMessageName(std::string &name, uint8_t *buffer, uint16_t charLeft, bool nameContinues){

    if(charLeft <= 1){
        return NAME_CONTINUES;      //read only "-character
    }

    if(nameContinues){
        int i;

    /*    if(name.length() == 0){   //this means there's extra " in the beginning of the buffer, THIS IS PROBABLY NEVER NEEDED, BUT CAUSES A BUG WHEN USED
            i = 0;
        }
        else i = 0;*/

        for(i = 0; (char)buffer[i] != '\"'; i++){
            name += (char)buffer[i];
            if((i+1) == charLeft){
                return NAME_CONTINUES;
            }
        }
    }
    else{
        if((char)buffer[0] != '\"'){
            return READ_FAILED;
        }

        for(int i = 1; (char)buffer[i] != '\"'; i++){
            name += (char)buffer[i];
            if((i+1) == charLeft){
                return NAME_CONTINUES;
            }
        }
    }

    return READ_SUCCESS;
}

void DataGenerator::sendBackToSender(const Message* msg, const Address& addr, const Ptr<Socket> socket, std::string& messageName, bool isClient){

    char buffer[30] = "";
    msg->fillMessageContents(buffer, 0, &messageName);

    switch(this->proto){

    case TCP_NAGLE_DISABLED:
    case TCP_NAGLE_ENABLED:
        if(!sender.send(immediateSend, (uint8_t*)buffer, msg, socket, true, isClient))
            return;

        break;

    case UDP:

        if(!sender.sendTo(immediateSend, (uint8_t*)buffer, msg, addr, true, isClient,socket))
            return;

        break;
    }

    if(!isClient){
        int messageId = 0;
        std::string name = msg->getName();
        msg->parseMessageId(messageName, messageId);
        StatisticsCollector::logMessageForwardedByServer(messageId, streamNumber, msg->getForwardMessageSize(messageId));
    }

}


//Class ClientDataGenerator function definitions

ClientDataGenerator::ClientDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int tick, bool ordered)
    : DataGenerator(streamNumber, proto, appProto, messages, tick, ordered), ownerClient(0), bytesLeftToRead(0), dataLeft(false), nameLeft(false){
}

ClientDataGenerator::ClientDataGenerator(const DataGenerator& stream) : bytesLeftToRead(0), dataLeft(false), nameLeft(false){

    this->streamNumber = stream.getStreamNumber();
    this->ownerClient = (dynamic_cast<const ClientDataGenerator&>(stream)).getClientNumber();
    this->gameTick = stream.getGameTick();
    this->ordered = stream.isOrdered();

    if(stream.getApplicationProtocol() != 0)
        this->appProto = new ApplicationProtocol((*(stream.getApplicationProtocol())));
    else this->appProto = 0;

    this->proto = stream.getProtocol();

    std::vector<Message*> messages = stream.getMessages();

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        this->messages.push_back((*it)->copyMessage());
    }

    this->immediateSend = stream.sendImmediately();
    sender = DataSender(appProto, gameTick);
}

ClientDataGenerator::~ClientDataGenerator(){

     if(socket != 0)
         socket->Close();
     CLIENT_INFO("Closed client socket for stream number: " << this->streamNumber << std::endl);

}

void ClientDataGenerator::setClientNumber(uint16_t clientNumber){
    ownerClient = clientNumber;
}

void ClientDataGenerator::StartApplication(){

    running = true;

    switch(proto){

        case TCP_NAGLE_DISABLED:
        case TCP_NAGLE_ENABLED:
            socket->Connect(peerAddr);
            if(!immediateSend)
                Simulator::Schedule(Time(MilliSeconds(gameTick)), &DataSender::flushTcpBuffer, &sender, true);
            socket->SetRecvCallback(MakeCallback(&ClientDataGenerator::dataReceivedTcp, this));
            break;

    case UDP:
            socket->Connect(peerAddr);
            if(!immediateSend)
                Simulator::Schedule(Time(MilliSeconds(gameTick)), &DataSender::flushUdpBuffer, &sender, socket, true);
            if(appProto){
                socket->SetRecvCallback(MakeCallback(&ApplicationProtocol::recv, appProto));
                appProto->configureForStream(MakeCallback(&ClientDataGenerator::readReceivedData, this), ordered);
            }else{
                socket->SetRecvCallback(MakeCallback(&ClientDataGenerator::dataReceivedUdp, this));

            }
            break;
    }

    CLIENT_INFO("Client number: " << ownerClient <<  " is starting stream no: " << this->streamNumber << std::endl);

    socket->SetSendCallback(MakeCallback(&ClientDataGenerator::moreBufferSpaceAvailable, this));

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        if((*it)->getType() == USER_ACTION){
            (*it)->scheduleSendEvent(MakeCallback(&ClientDataGenerator::sendData, this));
        }
    }


}

void ClientDataGenerator::StopApplication(){

    running = false;

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        (*it)->cancelEvent();
    }

    if(socket){
       socket->ShutdownSend();
       socket->Close();
    }
}

void ClientDataGenerator::dataReceivedTcp(Ptr<Socket> sock){

    uint8_t* buffer = 0;
    uint16_t bufferSize = 0;
    uint16_t bytesRead = 0;
    uint16_t messageSize = 0;
    Message* message = 0;
    ReadMsgNameReturnValue retVal;
    std::string messageName;

    bufferSize = sock->GetRxAvailable();
    buffer = (uint8_t*) calloc(bufferSize, sizeof(uint8_t));
    sock->Recv(buffer, bufferSize, 0);

    if(running){

        if(dataLeft){
            if(nameLeft){
                messageName.assign(messageNamePart);
            }

            if(bytesLeftToRead > bufferSize){
                dataLeft = true;
                bytesLeftToRead -= bufferSize;
            }else{
                if(!nameLeft){
                    bytesRead += bytesLeftToRead;
                    messageNamePart.clear();
                    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                        if(fullMessageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && fullMessageName[(*it)->getName().length()] == ':'){
                            message = *it;
                            break;
                        }
                    }

                   message->messageReceivedClient(fullMessageName);
                }

                if(bytesRead >= bufferSize){
                    dataLeft = false;
                }

                while(bytesRead < bufferSize){

                    if((retVal = readMessageName(messageName, buffer + bytesRead, bufferSize-bytesRead, nameLeft)) == READ_SUCCESS){

                        for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                            if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
                                message = *it;
                                break;
                            }
                        }

                        int msgId;
                        message->parseMessageId(messageName, msgId);

                        if(message->getType() != OTHER_DATA)
                            messageSize = message->getForwardMessageSize(msgId);
                        else
                            messageSize = message->getMessageSize(msgId);

                         if((bufferSize - bytesRead) <  messageSize - messageNamePart.length() - (nameLeft ==true ? 1 : 0)){   // -1 because of the "-character in the beginning of the name

                             if(nameLeft)
                                bytesLeftToRead = messageSize-(bufferSize - bytesRead) -(messageNamePart.length() +1);
                            else
                                bytesLeftToRead = messageSize-(bufferSize - bytesRead);

                            nameLeft = false;
                            dataLeft = true;
                            bytesRead = bufferSize;
                            fullMessageName.assign(messageName);
                            messageNamePart.assign((""));
                        }else{  //if we get here, the whole message has been read
                             if(nameLeft){
                                 bytesRead += messageSize - (messageNamePart.length() +1);
                             }else{
                                 bytesRead += messageSize;
                             }

                             nameLeft = false;

                             message->messageReceivedClient(messageName);

                             if(message->getType() == OTHER_DATA && message->doForwardBack())
                                 sendBackToSender(message, peerAddr, socket, messageName, true);

                             dataLeft = false;
                             bytesLeftToRead = 0;
                             messageNamePart.assign("");
                        }

                        messageName.assign("");

                    }
                    else if(retVal == NAME_CONTINUES){

                        if(nameLeft)
                            messageNamePart.append(messageName.substr(messageNamePart.length(), bufferSize-bytesRead));
                        else
                            messageNamePart.assign(messageName.substr(0, bufferSize-bytesRead));

                        bytesLeftToRead = 0;
                        nameLeft = true;
                        dataLeft = true;
                        bytesRead = bufferSize;
                    }
                    else if(retVal == READ_FAILED)
                        PRINT_ERROR("This should never happen, check message names!" <<std::endl);

                }
            }
        }else{

            while(bytesRead < bufferSize){
                if((retVal = readMessageName(messageName, buffer + bytesRead, bufferSize-bytesRead)) == READ_SUCCESS){
                    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                        if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
                            message = *it;
                            break;
                        }
                    }

                    int msgId;
                    message->parseMessageId(messageName, msgId);

                    if(message->getType() != OTHER_DATA)
                        messageSize = message->getForwardMessageSize(msgId);
                    else
                        messageSize = message->getMessageSize(msgId);

                    if((bufferSize - bytesRead) < messageSize){   //if this is true, the message continues in the next TCP segment
                        dataLeft = true;
                        bytesLeftToRead = messageSize-(bufferSize - bytesRead);
                        bytesRead = bufferSize;
                        fullMessageName.assign(messageName);
                        messageNamePart.assign((""));
                    }else{  //if we get here, the whole message has been read
                        bytesRead += messageSize;
                        message->messageReceivedClient(messageName);

                        if(message->getType() == OTHER_DATA && message->doForwardBack())
                            sendBackToSender(message, peerAddr, socket, messageName, true);

                        dataLeft = false;
                        bytesLeftToRead = 0;
                    }

                    messageName.assign("");
                }
                else if(retVal == NAME_CONTINUES){
                    nameLeft = true;
                    dataLeft = true;
                    messageNamePart.assign(messageName.substr(0, bufferSize-bytesRead));
                    bytesLeftToRead = 0;
                    bytesRead = bufferSize;
                }
                else if(retVal == READ_FAILED)
                    PRINT_ERROR("This should never happen, check message names!" << std::endl);
            }
        }
    }

    if(buffer != 0)
        free(buffer);
}

void ClientDataGenerator::dataReceivedUdp(Ptr<Socket> sock){

    Address addr;
    uint8_t* buffer = 0;
    uint16_t bufferSize = 0;

    bufferSize = sock->GetRxAvailable();

    buffer = (uint8_t*)calloc(bufferSize, sizeof(uint8_t));
    sock->RecvFrom(buffer, bufferSize, 0, addr);
    readReceivedData(buffer, bufferSize, addr);

    free(buffer);
}

void ClientDataGenerator::readReceivedData(uint8_t* buffer, uint16_t bufferSize, Address& srcAddr){

    uint16_t bytesRead = 0;
    Message* message = 0;
    std::string messageName;
    ReadMsgNameReturnValue retVal;

    if(running){

        while(bytesRead < bufferSize){

            if((retVal = readMessageName(messageName, buffer + bytesRead, bufferSize-bytesRead)) == READ_SUCCESS){

                for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                    if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
                        message = (*it);
                        break;
                    }
                }

                int msgId;
                message->parseMessageId(messageName, msgId);

                if(message->getType() != OTHER_DATA)
                    bytesRead += message->getForwardMessageSize(msgId);

                else
                    bytesRead += message->getMessageSize(msgId);

                message->messageReceivedClient(messageName);

                if(message->getType() == OTHER_DATA && message->doForwardBack()){
                    sendBackToSender(message, srcAddr, this->socket, messageName, true);
                }

                messageName.assign("");
            }
            else if(retVal == NAME_CONTINUES){
                PRINT_ERROR("This should never happen!" << std::endl);
            }
            else if(retVal == READ_FAILED){ //this can happen if only second part of IP fragmented packet arrives
                return;
            }
        }
    }
}


void ClientDataGenerator::moreBufferSpaceAvailable(Ptr<Socket> sock, uint32_t size){

}

bool ClientDataGenerator::sendData(Message *msg, uint8_t* buffer){

    if(running){

        if(this->proto == TCP_NAGLE_DISABLED || this->proto == TCP_NAGLE_ENABLED){
                if(!sender.send(immediateSend, buffer, msg, socket, false, true))
                    return false;

        }else if(this->proto == UDP){
            if(!sender.sendTo(immediateSend, buffer, msg, peerAddr, false, true, socket))
                return false;
            }
    }


    return true;
}


//Class ServerDataGenerator function definitions

ServerDataGenerator::ServerDataGenerator(uint16_t streamNumber, Protocol proto, ApplicationProtocol* appProto, std::vector<Message*> messages, int tick, bool ordered)
    : DataGenerator(streamNumber, proto, appProto, messages, tick, ordered){

    probability = UniformVariable(0,1);

}

ServerDataGenerator::ServerDataGenerator(const DataGenerator& stream){

    this->probability = UniformVariable(0,1);
    this->gameTick = stream.getGameTick();
    this->streamNumber = stream.getStreamNumber();
    this->ordered = stream.isOrdered();

    if(stream.getApplicationProtocol() != 0)
        this->appProto = new ApplicationProtocol((*(stream.getApplicationProtocol())));
    else this->appProto = 0;

    this->proto = stream.getProtocol();

    std::vector<Message*> messages = stream.getMessages();

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        this->messages.push_back((*it)->copyMessage());

    }

    this->immediateSend = stream.sendImmediately();

    sender = DataSender(appProto, gameTick);

}

ServerDataGenerator::~ServerDataGenerator(){

    for(std::vector<ServerDataGenerator::ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
        (*it)->clientSocket->Close();
        delete (*it);
        SERVER_INFO("Closed server socket for stream number: " << this->getStreamNumber() << std::endl);
    }

    for(std::vector<Address*>::iterator it = udpClients.begin(); it != udpClients.end(); it++)
        delete (*it);

}

void ServerDataGenerator::StartApplication(){

    running = true;
    socket->Bind(peerAddr);

    SERVER_INFO("Starting server stream no: " << this->streamNumber << std::endl);

    switch(proto){
        case TCP_NAGLE_DISABLED:
        case TCP_NAGLE_ENABLED:
            socket->Listen();
            socket->SetAcceptCallback(MakeCallback(&ServerDataGenerator::connectionRequest,this), MakeCallback(&ServerDataGenerator::newConnectionCreated, this));
            if(!immediateSend)
                Simulator::Schedule(Time(MilliSeconds(gameTick)), &DataSender::flushTcpBuffer, &sender, false);
            break;

        case UDP:
            if(!immediateSend)
                Simulator::Schedule(Time(MilliSeconds(gameTick)), &DataSender::flushUdpBuffer, &sender, socket, false);
            if(appProto){
                socket->SetRecvCallback(MakeCallback(&ApplicationProtocol::recv, appProto));
                appProto->configureForStream(MakeCallback(&ServerDataGenerator::readReceivedData, this), ordered);
            }else{
                socket->SetRecvCallback(MakeCallback(&ServerDataGenerator::dataReceivedUdp, this));
            }
                break;
    }

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        if((*it)->getType() == OTHER_DATA)
            (*it)->scheduleSendEvent(MakeCallback(&ServerDataGenerator::sendData, this));
    }

}

void ServerDataGenerator::StopApplication(){

    running = false;

    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
        (*it)->cancelEvent();
    }

    if(socket){
        socket->Close();
    }

    for(std::vector<ServerDataGenerator::ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
        (*it)->clientSocket->ShutdownRecv();
    }

}

void ServerDataGenerator::dataReceivedTcp(Ptr<Socket> sock){

    ClientConnection* client = 0;
    std::string messageName;
    uint8_t* buffer = 0;
    uint16_t bytesRead = 0;
    Message* message = 0;
    uint16_t messageSize = 0;
    uint16_t bufferSize = 0;
    ReadMsgNameReturnValue retVal;

    if(running){

        for(std::vector<ServerDataGenerator::ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
            if(sock == (*it)->clientSocket){
                client = (*it);
                break;
            }
        }

        bufferSize = sock->GetRxAvailable();
        buffer = (uint8_t*)calloc(bufferSize, sizeof(uint8_t));
        sock->Recv(buffer, bufferSize, 0);

        if(client->dataLeft){

            if(client->nameLeft){
                messageName.assign(client->messageNamePart);
            }

            if(client->bytesLeftToRead > bufferSize){
                client->dataLeft = true;
                client->bytesLeftToRead -= bufferSize;
            }else{
                if(!client->nameLeft){
                    bytesRead += client->bytesLeftToRead;
                    client->messageNamePart.clear();
                    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                        if(client->fullMessageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && client->fullMessageName[(*it)->getName().length()] == ':'){
                            message = *it;
                            break;
                        }
                    }
                    client->messageBuffer.push_back(std::make_pair<Ptr<Socket>, std::pair<std::string, Message*> >(sock, std::make_pair<std::string, Message*>(client->fullMessageName, message)));
                    message->messageReceivedServer(client->fullMessageName);
                }

                if(bytesRead >= bufferSize){
                    client->dataLeft = false;
                }

                while(bytesRead < bufferSize){

                    if((retVal = readMessageName(messageName, buffer + bytesRead, bufferSize-bytesRead, client->nameLeft)) == READ_SUCCESS){

                        for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                            if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
                                message = *it;
                                break;
                            }
                        }

                        int msgId;
                        message->parseMessageId(messageName, msgId);

                        if(message->getType() == OTHER_DATA)
                            messageSize = message->getForwardMessageSize(msgId);
                        else
                            messageSize = message->getMessageSize(msgId);

                         if((bufferSize - bytesRead) <  messageSize -client->messageNamePart.length() - (client->nameLeft ==true ? 1 : 0)){   // -1 because of the "-character in the beginning of the name

                             if(client->nameLeft)
                                client->bytesLeftToRead = messageSize-(bufferSize - bytesRead) -( client->messageNamePart.length() +1);
                            else
                                client->bytesLeftToRead = messageSize-(bufferSize - bytesRead);

                            client->nameLeft = false;
                            client->dataLeft = true;

                            bytesRead = bufferSize;
                            client->messageNamePart.assign((""));
                            client->fullMessageName.assign(messageName);
                        }else{  //if we get here, the whole message has been read
                             if(client->nameLeft){
                                 bytesRead += messageSize - (client->messageNamePart.length() +1);
                             }else{
                                 bytesRead += messageSize;
                             }
                            client->nameLeft = false;
                            client->messageBuffer.push_back(std::make_pair<Ptr<Socket>, std::pair<std::string, Message*> >(sock, std::make_pair<std::string, Message*>(messageName, message)));
                            message->messageReceivedServer(messageName);
                            client->dataLeft = false;
                            client->bytesLeftToRead = 0;
                        }

                        messageName.assign("");

                    }
                    else if(retVal == NAME_CONTINUES){

                        if(client->nameLeft)
                            client->messageNamePart.append(messageName.substr(client->messageNamePart.length(), bufferSize-bytesRead));
                        else
                            client->messageNamePart.assign(messageName.substr(0, bufferSize-bytesRead));

                        client->bytesLeftToRead = 0;
                        client->nameLeft = true;
                        client->dataLeft = true;
                        bytesRead = bufferSize;
                    }
                    else if(retVal == READ_FAILED){
                        PRINT_ERROR("This should never happen, check message names!" <<std::endl);
                    }

                }
            }
        }else{
            while(bytesRead < bufferSize){
                if((retVal = readMessageName(messageName, buffer + bytesRead, bufferSize-bytesRead)) == READ_SUCCESS){
                    for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                        if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
                            message = *it;
                            break;
                        }
                    }

                    int msgId;
                    message->parseMessageId(messageName, msgId);

                    if(message->getType() == OTHER_DATA)
                        messageSize = message->getForwardMessageSize(msgId);
                    else
                        messageSize = message->getMessageSize(msgId);

                    if((bufferSize - bytesRead) < messageSize){   //if this is true, the message continues in the next TCP segment
                        client->dataLeft = true;
                        client->bytesLeftToRead = messageSize-(bufferSize - bytesRead);
                        bytesRead = bufferSize;
                        client->messageNamePart.assign((""));
                        client->fullMessageName.assign(messageName);
                    }else{  //if we get here, the whole message has been read
                        bytesRead += messageSize;
                        client->messageBuffer.push_back(std::make_pair<Ptr<Socket>, std::pair<std::string, Message*> >(sock, std::make_pair<std::string, Message*>(messageName, message)));
                        message->messageReceivedServer(messageName);
                        client->dataLeft = false;
                        client->bytesLeftToRead = 0;
                    }

                    messageName.assign("");
                }
                else if(retVal == NAME_CONTINUES){
                    client->nameLeft = true;
                    client->dataLeft = true;
                    client->messageNamePart.assign(messageName.substr(0, bufferSize-bytesRead));
                    client->bytesLeftToRead = 0;
                    bytesRead = bufferSize;
                }
                else if(retVal == READ_FAILED){
                    PRINT_ERROR("This should never happen, check message names!" << std::endl);
                }
            }
        }
    }

    if(buffer != 0)
        free(buffer);

    forwardData();

}

void ServerDataGenerator::dataReceivedUdp(Ptr<Socket> sock){

    Address addr;
    uint8_t* buffer = 0;
    uint16_t bufferSize = 0;

    bufferSize = sock->GetRxAvailable();
    buffer = (uint8_t*)calloc(bufferSize, sizeof(uint8_t));

    sock->RecvFrom(buffer, bufferSize, 0, addr);
    readReceivedData(buffer, bufferSize, addr);

    free(buffer);
}

void ServerDataGenerator::readReceivedData(uint8_t *buffer, uint16_t bufferSize, Address& clientAddr){

    std::string messageName;
    Message* message = 0;
    uint16_t bytesRead = 0;
    ReadMsgNameReturnValue retVal;
    bool addressExists = false;

    if(running){

        if(udpClients.empty()){
            udpClients.push_back(new Address(clientAddr));
        }

        for(std::vector<Address*>::iterator it = udpClients.begin(); it != udpClients.end(); it++){
            if((**it) == clientAddr){
                addressExists = true;
                break;
            }
        }

        if(!addressExists){
            udpClients.push_back(new Address(clientAddr));
        }

        while(bytesRead < bufferSize){

            if((retVal = readMessageName(messageName, buffer + bytesRead, bufferSize-bytesRead)) == READ_SUCCESS){
                for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
                    if(messageName.compare(0, (*it)->getName().length(), (*it)->getName()) == 0 && messageName[(*it)->getName().length()] == ':'){
                        message = (*it);
                        break;
                    }
                }

                int msgId;
                message->parseMessageId(messageName, msgId);

                if(message->getType() == OTHER_DATA){
                    bytesRead += message->getForwardMessageSize(msgId);
                }
                else{
                    bytesRead += message->getMessageSize(msgId);
                }

                udpMessages.push_back(std::make_pair<Address, std::pair<std::string, Message*> >(Address(clientAddr), std::make_pair<std::string, Message*>(messageName, message)));

                message->messageReceivedServer(messageName);

                messageName.assign("");
            }
            else if(retVal == NAME_CONTINUES){
                PRINT_ERROR("This should never happen!" << std::endl);
            }
            else if(retVal == READ_FAILED){ //this can happen if only second part of IP fragmented packet arrives
                return;
            }
        }
    }

    forwardData();
}

void ServerDataGenerator::moreBufferSpaceAvailable(Ptr<Socket> sock, uint32_t size){

}

bool ServerDataGenerator::connectionRequest(Ptr<Socket> sock, const Address &addr){

    (void)sock;
    SERVER_INFO("Connection request from: " << addr << std::endl);   //TODO: getting ip-addresses impossible???
    return true;

}

void ServerDataGenerator::newConnectionCreated(Ptr<Socket> sock, const Address &addr){

    SERVER_INFO("Connection accepted from: " << addr << " in stream number: " << streamNumber << "   " << Simulator::Now() << std::endl);
    clientConnections.push_back(new ServerDataGenerator::ClientConnection(sock, sender, immediateSend));
    sock->SetRecvCallback(MakeCallback(&ServerDataGenerator::dataReceivedTcp, this));
}

void ServerDataGenerator::forwardData(){

    if(running){
        switch(proto){

            case TCP_NAGLE_DISABLED:
            case TCP_NAGLE_ENABLED:

                for(std::vector<ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
                    for(std::vector<std::pair<Ptr<Socket>, std::pair<std::string, Message*> > >::iterator messages = (*it)->messageBuffer.begin(); messages != (*it)->messageBuffer.end(); messages++){
                        if(messages->second.second->doForwardBack() && messages->second.second->getType() == USER_ACTION)
                            sendBackToSender(messages->second.second, peerAddr /*this is not really neede over TCP*/, messages->first, messages->second.first, false);

                        sendToRandomClients(*messages);
                    }
                    (*it)->messageBuffer.clear();
                }

                break;

            case UDP:

                for(std::vector<std::pair<Address, std::pair<std::string, Message*> > >::iterator it = udpMessages.begin(); it != udpMessages.end(); it++){
                    if(it->second.second->doForwardBack() && it->second.second->getType() == USER_ACTION)
                        sendBackToSender(it->second.second, it->first, socket, it->second.first, false);

                    sendToRandomClients(*it);

                }
                udpMessages.clear();

                break;
        }
    }
}

void ServerDataGenerator::sendToRandomClients(std::pair<Ptr<Socket>, std::pair<std::string, Message*> > &msg){    //forwarding over TCP

    double clientsToSend = ((UserActionMessage*)msg.second.second)->getClientsOfInterest();

    for(std::vector<ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
        if(msg.first == (*it)->clientSocket)
            continue;

        if(clientsToSend >= probability.GetValue()){
            (*it)->forwardUserActionMessage(msg.second);
        }
    }
}

void ServerDataGenerator::sendToRandomClients(std::pair<Address, std::pair<std::string, Message*> > &msg){      //forwarding over UDP

    double clientsToSend = ((UserActionMessage*)msg.second.second)->getClientsOfInterest();

    for(std::vector<Address*>::iterator it = udpClients.begin(); it != udpClients.end(); it++){
        if((**it) == msg.first)
            continue;

        if(clientsToSend >= probability.GetValue()){
            forwardUserActionMessage(msg.second, (**it));
        }
    }
}

void ServerDataGenerator::forwardUserActionMessage(std::pair<std::string, Message*> msg, Address& addr){    //forwarding over UDP

    char buffer[30] = "";
    int messageNumber = 0;
    msg.second->fillMessageContents(buffer, 0, &msg.first);

    if(!sender.sendTo(immediateSend, (uint8_t*)buffer, msg.second, addr, true, false, socket))
        return;

    msg.second->parseMessageId(msg.first, messageNumber);

    StatisticsCollector::logMessageForwardedByServer(messageNumber, streamNumber, msg.second->getForwardMessageSize(messageNumber));
}

bool ServerDataGenerator::sendData(Message *msg, uint8_t *buffer){

    if(running){

        if(this->proto == TCP_NAGLE_DISABLED || this->proto == TCP_NAGLE_ENABLED){
            for(std::vector<ServerDataGenerator::ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); it++){
                if((*it)->clientSocket->GetTxAvailable() < msg->getMessageSize())
                    return false;

                if(!sender.send(immediateSend, buffer, msg, (*it)->clientSocket, false, false))
                    return false;

                if(msg->getType() == OTHER_DATA){      //count clients who should get this message
                    int messageNumber;
                    msg->parseMessageId(std::string((char*)buffer+1), messageNumber);
                    StatisticsCollector::countMessagesSentFromServer(messageNumber, streamNumber);
                }
            }
        }else if(this->proto == UDP){
            for(std::vector<Address*>::iterator it = udpClients.begin(); it != udpClients.end(); it++){

                if(!sender.sendTo(immediateSend, buffer, msg, **it, false, false, socket))
                    return false;

                if(msg->getType() == OTHER_DATA){      //count clients who should get this message
                    int messageNumber;
                    msg->parseMessageId(std::string((char*)buffer+1), messageNumber);
                    StatisticsCollector::countMessagesSentFromServer(messageNumber, streamNumber);
                }
            }
        }
    }

    return true;
}


//nested class ClientConnection function definitions

ServerDataGenerator::ClientConnection::ClientConnection(Ptr<Socket> sock, DataSender& sender, bool immediate): clientSocket(sock), dataLeft(false), nameLeft(false), sender(sender),
    immediateSend(immediate){

}

ServerDataGenerator::ClientConnection::~ClientConnection(){

    clientSocket->Close();
}

void ServerDataGenerator::ClientConnection::forwardUserActionMessage(std::pair<std::string, Message*>& msg){           //forwarding over TCP

    char buffer[30] = "";
    int messageNumber = 0;
    msg.second->fillMessageContents(buffer, 0, &msg.first);

    if(!sender.send(immediateSend, (uint8_t*)buffer, msg.second, clientSocket, true,false))
        return;


    msg.second->parseMessageId(msg.first, messageNumber);

    StatisticsCollector::logMessageForwardedByServer(messageNumber, msg.second->getStreamNumber(), msg.second->getForwardMessageSize(messageNumber));
}

