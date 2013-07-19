#include "Messages.h"


//Class Message function definitions

uint16_t Message::messagesCreated = 0;
std::map<uint16_t, std::map<MessageType, std::vector<Message::NumbersAndSizes> > >Message::numbersAndSizes;

Message::Message(std::string name, bool reliable, int timeInterval, uint16_t size, uint16_t streamNumber, uint16_t forwardSize, bool rcvSize,
                 bool forwardBack, RandomVariable* ranvarTimeInterval, RandomVariable* ranvarSize, RandomVariable *ranvarForwardSize)
    : name(name), reliable(reliable), timeInterval(timeInterval), messageSize(size), streamNumber(streamNumber),
      ranvarTimeInterval(ranvarTimeInterval), ranvarSize(ranvarSize), ranvarForwardSize(ranvarForwardSize), forwardSize(forwardSize), useRcvSize(rcvSize), forwardBack(forwardBack){

}

Message::Message(const Message &msg): name(msg.getName()), reliable(msg.getReliable()), timeInterval(msg.getTimeInterval()), messageSize(msg.getMessageSize()),
    type(msg.getType()), streamNumber(msg.getStreamNumber()), forwardSize(msg.getForwardMessageSize()), useRcvSize(msg.useRcvSize), forwardBack(msg.doForwardBack()) {

    if(msg.ranvarTimeInterval != 0)
        this->ranvarTimeInterval = new RandomVariable(*msg.ranvarTimeInterval);
    else
        ranvarTimeInterval = 0;

    if(msg.ranvarSize != 0)
        this->ranvarSize = new RandomVariable(*msg.ranvarSize);
    else
        ranvarSize = 0;

    if(msg.ranvarForwardSize != 0)
        this->ranvarForwardSize = new RandomVariable(*msg.ranvarForwardSize);
    else
        ranvarForwardSize = 0;


    this->messageID = ++messagesCreated;
}

Message::~Message(){
    delete ranvarTimeInterval;
    delete ranvarSize;
    delete ranvarForwardSize;
    messagesCreated--;
}

void Message::cancelEvent(){

    if(sendEvent.IsRunning())
        Simulator::Cancel(sendEvent);

    running = false;

}

void Message::fillMessageContents(char *buffer, int number, std::string* msgName) const{

    buffer[0] = '\"';
    std::stringstream str("");

    if(msgName == NULL){
        strcat(buffer, name.c_str());
        strcat(buffer, ":");

        str << number;
        strcat(buffer, str.str().c_str());

    }else{
        strcat(buffer, msgName->c_str());
    }

    strcat(buffer, "\"");
}

bool Message::parseMessageId (const std::string &messageName, int &resultId)const{

    std::stringstream str;

    for(unsigned int i = this->getName().length() + 1; i < messageName.length() ;  i++) {
        if(isdigit(messageName[i]))
            str << messageName[i];
    }

    str >> resultId;

    return true;
}


//Class UserActionMessage function definitions


std::map< std::string, uint16_t, Message::StringComparator> UserActionMessage::userActionMessageNameMap = std::map<std::string, uint16_t, Message::StringComparator>();

UserActionMessage::UserActionMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, double clientsOfInterest,
                                     uint32_t clientRequirement,  uint32_t serverRequirement, uint16_t streamNumber, uint16_t forwardSize, bool forwardBack,
                                     bool rcvSize, RandomVariable* ranvarTimeInterval, RandomVariable* ranvarSize, RandomVariable *ranvarForwardSize)
    :Message(name, reliable, timeInterval, messageSize, streamNumber, forwardSize, rcvSize, forwardBack, ranvarTimeInterval,ranvarSize, ranvarForwardSize), clientsOfInterest(clientsOfInterest),
      clientTimeRequirement(clientRequirement), serverTimeRequirement(serverRequirement){

    if(userActionMessageNameMap.find(name) == userActionMessageNameMap.end()){
        userActionMessageNameMap.insert(std::make_pair<std::string, uint16_t>(name, userActionMessageNameMap.size()));   //every message name has an unique index
    }

    StatisticsCollector::uam_fnptr = &getUAMIndexName; //this has to be done to avoid problems with includes when static functions are called from both files

    type = USER_ACTION;
    StatisticsCollector::userActionmessageCount++;
}

UserActionMessage::~UserActionMessage(){

}

int UserActionMessage::newMessageNumber(uint16_t streamnumber){

    static std::vector<std::pair<int, int> > messageNumbersForStreams;          //every stream has separate message numbers
    static std::vector<std::pair<int, int> >::iterator it;
    static std::pair<int, int>* temp;
    bool exists = false;
    int retVal;

    if(numbersAndSizes.count(streamnumber) == 0){
        std::map<MessageType, std::vector<NumbersAndSizes> > map;
        numbersAndSizes.insert(std::make_pair<uint16_t, std::map<MessageType, std::vector<NumbersAndSizes> > >(streamnumber, map));
    }

    if(numbersAndSizes.at(streamnumber).count(USER_ACTION) == 0){
        numbersAndSizes.at(streamnumber).insert(std::make_pair<MessageType, std::vector<NumbersAndSizes> >(USER_ACTION, std::vector<NumbersAndSizes>()));
    }

    for(it = messageNumbersForStreams.begin(); it != messageNumbersForStreams.end(); it++){
        if(it->first == streamnumber){
            exists = true;
            temp = &(*it);
            break;
        }
    }

    if(!exists){
        messageNumbersForStreams.push_back(std::make_pair<int, int>(streamnumber, 0));
        retVal = 0;
    }else{
        temp->second++;
        retVal = temp->second;
    }

    return retVal;
}

void UserActionMessage::scheduleSendEvent(Callback<bool, Message*, uint8_t*> sendFunction){

    int interval = 0;

    this->sendFunction = sendFunction;
    running = true;

    if(ranvarTimeInterval != 0){
        interval = ranvarTimeInterval->GetInteger();
        if(interval <= 0)
            interval = 1;
    }

    if(ranvarTimeInterval == 0)
        sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &UserActionMessage::sendData, this);
    else{
        sendEvent = Simulator::Schedule(Time(MilliSeconds(interval)), &UserActionMessage::sendData, this);
    }
}

void UserActionMessage::sendData(){

    char buffer[30] = "";
    static Time sentTime;
    static int interval = 0;
    NumbersAndSizes stats;

    int messageNumber = UserActionMessage::newMessageNumber(streamNumber);

    stats.number = messageNumber;

    fillMessageContents(buffer, messageNumber);


    if(ranvarSize == 0)
    {
        stats.size = this->messageSize;
    }
    else
    {
        stats.size = ranvarSize->GetInteger();
    }


    if(ranvarForwardSize == 0)
    {
        if(useRcvSize)
            stats.forwardSize = stats.size;
        else
            stats.forwardSize = this->forwardSize;
    }else
    {
        stats.forwardSize = ranvarForwardSize->GetInteger();
    }


    numbersAndSizes.at(streamNumber).at(USER_ACTION).push_back(stats);

    sentTime = Simulator::Now();

    StatisticsCollector::logMessagesSentFromClient(messageNumber, sentTime, streamNumber, clientTimeRequirement, serverTimeRequirement, UserActionMessage::getUAMNameIndex(name), messageID, stats.size);

    if(!sendFunction(this, (uint8_t*)buffer))
        PRINT_ERROR("Problems with socket buffer" << std::endl);   //TODO: socket buffer

    if(ranvarTimeInterval != 0){
        interval = ranvarTimeInterval->GetInteger();
        if(interval <= 0)
            interval = 1;
    }

    if(running){
        if(ranvarTimeInterval == 0)
            sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &UserActionMessage::sendData, this);
        else
            sendEvent = Simulator::Schedule(Time(MilliSeconds(interval)), &UserActionMessage::sendData, this);
    }
}

Message* UserActionMessage::copyMessage(){

    Message *msg;
    msg = new UserActionMessage(*this);

    return msg;

}

uint16_t UserActionMessage::getMessageSize(int msgNumber) const{

    if(msgNumber == -1){
        return this->messageSize;
    }else{
         return numbersAndSizes.at(streamNumber).at(USER_ACTION)[msgNumber].size;
    }
}

uint16_t UserActionMessage::getForwardMessageSize(int msgNumber) const{

    if(msgNumber == -1){
        return this->forwardSize;
    }else{
        return numbersAndSizes.at(streamNumber).at(USER_ACTION)[msgNumber].forwardSize;
    }
}

void UserActionMessage::printStats(std::ostream &out) const{

    out << "UserActionMessage  " << "  ID:" << messageID <<  "  Name: " << name << "  Reliable: " << (reliable == true ? "yes" : "no")
           << "  Size: " << messageSize << " TimeInterval: " <<  timeInterval <<  "  ClientOfInterest: "
           << clientsOfInterest << "  ClientTimeRequirement: " << clientTimeRequirement << "   ServerTimeRequirement: " << serverTimeRequirement;

}


void UserActionMessage::messageReceivedServer(std::string& messageName){

    int id = 0;
    parseMessageId(messageName, id);
    StatisticsCollector::logUserActionMessageReceivedByServer(id, Simulator::Now(), streamNumber);
}

void UserActionMessage::messageReceivedClient(std::string& messageName){

    int id = 0;
    parseMessageId(messageName, id);

    StatisticsCollector::logUserActionMessageReceivedByClient(id, Simulator::Now(), streamNumber);
}

//Class OtherDataMessage function definitions

std::map< std::string, uint16_t, Message::StringComparator> OtherDataMessage::otherDataMessageNameMap = std::map<std::string, uint16_t, Message::StringComparator>();


OtherDataMessage::OtherDataMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, uint16_t streamNumber, uint16_t forwardSize,
                                   bool forwardBack, bool rcvSize, uint16_t timeReq, RandomVariable* ranvarTimeInterval, RandomVariable* ranvarSize,
                                   RandomVariable *ranvarForwardSize)
    : Message(name, reliable, timeInterval, messageSize, streamNumber, forwardSize, rcvSize, forwardBack, ranvarTimeInterval, ranvarSize, ranvarForwardSize), clientTimeRequirement(timeReq)

{

    if(otherDataMessageNameMap.find(name) == otherDataMessageNameMap.end()){
        otherDataMessageNameMap.insert(std::make_pair<std::string, uint16_t>(name, otherDataMessageNameMap.size()));   //every message name has an unique index
    }

    StatisticsCollector::odm_fnptr = &getODMIndexName; //this has to be done to avoid problems with includes when static functions are called from both files

    type = OTHER_DATA;
    StatisticsCollector::otherDataMessageCount++;

}

OtherDataMessage::~OtherDataMessage(){


}

int OtherDataMessage::newMessageNumber(uint16_t streamnumber){

    static std::vector<std::pair<int, int> > messageNumbersForStreams;          //every stream has separate message numbers
    static std::vector<std::pair<int, int> >::iterator it;
    static std::pair<int, int>* temp;
    bool exists = false;
    int retVal;

    if(numbersAndSizes.count(streamnumber) == 0){
        std::map<MessageType, std::vector<NumbersAndSizes> > map;
        numbersAndSizes.insert(std::make_pair<uint16_t, std::map<MessageType, std::vector<NumbersAndSizes> > >(streamnumber, map));
    }

    if(numbersAndSizes.at(streamnumber).count(OTHER_DATA) == 0){
        numbersAndSizes.at(streamnumber).insert(std::make_pair<MessageType, std::vector<NumbersAndSizes> >(OTHER_DATA, std::vector<NumbersAndSizes>()));
    }

    for(it = messageNumbersForStreams.begin(); it != messageNumbersForStreams.end(); it++){
        if(it->first == streamnumber){
            exists = true;
            temp = &(*it);
            break;
        }
    }

    if(!exists){
        messageNumbersForStreams.push_back(std::make_pair<int, int>(streamnumber, 0));
        retVal = 0;
    }else{
        temp->second++;
        retVal = temp->second;
    }

    return retVal;
}

void OtherDataMessage::sendData(){

    char buffer[30] = "";
    static Time sentTime;
    static int interval = 0;
    NumbersAndSizes stats;

    int messageNumber = OtherDataMessage::newMessageNumber(streamNumber);

    stats.number = messageNumber;

    fillMessageContents(buffer, messageNumber);

    if(ranvarSize == 0)
    {
        stats.size = this->messageSize;
    }
    else
    {
        stats.size = ranvarSize->GetInteger();
    }  

    if(ranvarForwardSize == 0)
    {
        if(useRcvSize)
            stats.forwardSize = stats.size;
        else
            stats.forwardSize = this->forwardSize;
    }
    else{
        stats.forwardSize = ranvarForwardSize->GetInteger();
    }

    numbersAndSizes.at(streamNumber).at(OTHER_DATA).push_back(stats);

    sentTime = Simulator::Now();

    StatisticsCollector::logMessagesSentFromServer(messageNumber, sentTime, streamNumber, clientTimeRequirement, OtherDataMessage::getODMNameIndex(name), messageID, stats.size);

    if(!sendFunction(this, (uint8_t*)buffer))
        PRINT_ERROR("Problems with socket buffer" << std::endl);   //TODO: socket buffer

    if(ranvarTimeInterval != 0){
        interval = ranvarTimeInterval->GetInteger();
        if(interval <= 0)
            interval = 1;
    }

    if(running){
        if(ranvarTimeInterval == 0)
            sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &OtherDataMessage::sendData, this);
        else
            sendEvent = Simulator::Schedule(Time(MilliSeconds(interval)), &OtherDataMessage::sendData, this);
    }
}

Message* OtherDataMessage::copyMessage(){

    Message *msg;
    msg = new OtherDataMessage(*this);
    return msg;
}

uint16_t OtherDataMessage::getMessageSize(int msgNumber) const{

    if(msgNumber == -1){
        return this->messageSize;
    }else{
        return numbersAndSizes.at(streamNumber).at(OTHER_DATA)[msgNumber].size;
    }

}

uint16_t OtherDataMessage::getForwardMessageSize(int msgNumber) const{

    if(msgNumber == -1){
        return this->forwardSize;
    }else{
        return numbersAndSizes.at(streamNumber).at(OTHER_DATA)[msgNumber].forwardSize;
    }
}

void OtherDataMessage::printStats(std::ostream &out) const{

    out << "OtherDataMessage  " << "  ID:" << messageID << "  Name: " << name << "  Reliable: " << (reliable == true ? "yes" : "no")
        << "  Size: " << messageSize << " TimeInterval: " <<  timeInterval;

}

void OtherDataMessage::scheduleSendEvent(Callback<bool, Message*, uint8_t*> sendFunction){

    int interval = 0;

    this->sendFunction = sendFunction;
    running = true;
    if(ranvarTimeInterval != 0){
        interval = ranvarTimeInterval->GetInteger();
        if(interval <= 0)
            interval = 1;
    }

    if(ranvarTimeInterval == 0)
        sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &OtherDataMessage::sendData, this);
    else
        sendEvent = Simulator::Schedule(Time(MilliSeconds(interval)), &OtherDataMessage::sendData, this);
}

void OtherDataMessage::messageReceivedServer(std::string& messageName){
    (void)messageName;
    //do nothing, this is only forwarded back
}

void OtherDataMessage::messageReceivedClient(std::string& messageName){

    int id = 0;
    parseMessageId(messageName, id);

    StatisticsCollector::logServerMessageReceivedByClient(id, Simulator::Now(), streamNumber);

}


