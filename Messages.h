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

#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include "ns3/event-id.h"
#include "utilities.h"
#include "ns3/random-variable.h"
#include "StatisticsCollector.h"

class DataGenerator;

enum MessageType{USER_ACTION, OTHER_DATA};

class Message{

    friend std::ostream& operator<<( std::ostream& out, const Message& msg){
        msg.printStats(out, msg);
        return out;
    }


public:

    struct StringComparator{
        bool operator() (const std::string& s1, const std::string& s2) const{

            if(s1.compare(s2) > 0)
                return true;
            else
                return false;
        }
    };

    Message(std::string, bool, int, uint16_t, uint16_t, uint16_t forwardSize, bool forwardBack, RandomVariable* ranvarTimeInterval = 0, RandomVariable* ranvarSize = 0, RandomVariable* ranvarForwardSize = 0);
    virtual ~Message();
    virtual Message* copyMessage()  = 0;
    virtual void scheduleSendEvent(Callback<bool, Message*, uint8_t*>) = 0;
    std::string getName() const{return name;}
    bool getReliable() const{return reliable;}
    int getTimeInterval() const{return timeInterval;}
    virtual uint16_t getMessageSize(int msgNumber = -1) const = 0;
    uint16_t getmessagesCreated() const {return messagesCreated;}
    uint16_t getMessageId() const {return messageID;}
    MessageType getType() const{return type;}
    uint16_t getStreamNumber() const{return streamNumber;}
    void cancelEvent();
    void fillMessageContents(char* buffer, int number = 0, std::string* msgName = NULL) const;
    virtual void messageReceivedServer(std::string& messageName) = 0;
    virtual void messageReceivedClient(std::string& messageName) = 0;
    bool parseMessageId(const std::string& messageName, int& resultId) const;
    bool doForwardBack() const {return forwardBack;}
    virtual uint16_t getForwardMessageSize(int msgNumber = -1) const = 0;


  protected:
    Message(const Message&);
    virtual void printStats(std::ostream& out, const Message& msg)const = 0;

    std::string name;
    bool reliable;
    int timeInterval;
    uint16_t messageSize;
    uint16_t messageID;
    MessageType type;
    Callback<bool, Message*,  uint8_t*> sendFunction;
    EventId sendEvent;
    bool running;
    uint16_t streamNumber;
    RandomVariable* ranvarTimeInterval;
    RandomVariable* ranvarSize;
    RandomVariable* ranvarForwardSize;
    uint16_t forwardSize;
    bool forwardBack;

    typedef struct{    //these must be remembered in order to get messagesizes when they are generated with random values
        uint16_t number;
        uint16_t size;
        uint16_t forwardSize;
    }NumbersAndSizes;

    static uint16_t messagesCreated;
    static std::map<uint16_t, std::map<MessageType, std::vector<NumbersAndSizes> > >numbersAndSizes;   //map contains element for each stream, each stream has vector for each message type

};


class UserActionMessage : public Message{

    friend class XMLParser;

public:
    ~UserActionMessage();
    Message* copyMessage();
    void scheduleSendEvent(Callback<bool, Message*, uint8_t*>);
    double getClientsOfInterest() const{return clientsOfInterest;}
    uint32_t getServerTimeRequirement() const{return serverTimeRequirement;}
    uint32_t getClientTimeRequirement() const{return clientTimeRequirement;}
    void messageReceivedServer(std::string& messageName);
    void messageReceivedClient(std::string& messageName);
    virtual uint16_t getForwardMessageSize(int msgNumber = -1) const;
    virtual uint16_t getMessageSize(int msgNumber = -1) const;

    static std::map<std::string ,uint16_t, StringComparator> userActionMessageNameMap;

    static inline uint16_t getUAMNameIndex(const std::string& name){
       return UserActionMessage::userActionMessageNameMap.find(name)->second;
    }
    static std::string& getUAMIndexName(uint16_t index){
        static std::map<std::string, uint16_t, StringComparator>::iterator it;
        static std::string failReturn("This should never happen!");

        for(it = UserActionMessage::userActionMessageNameMap.begin(); it != UserActionMessage::userActionMessageNameMap.end(); it++){
            if(it->second == index){
                return const_cast<std::string&>(it->first);
            }
        }
        return failReturn;
    }

private:
    UserActionMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, double clientsOfInterest, uint32_t clientRequirement, uint32_t serverRequirement,
                      uint16_t streamNumber, uint16_t forwardSize, bool forwardBack, RandomVariable* ranvarTimeInterval = 0, RandomVariable* ranvarSize = 0, RandomVariable* ranvarForwardSize = 0);

    double clientsOfInterest;
    uint32_t clientTimeRequirement;  //time requirement for messages to travel from client to client
    uint32_t serverTimeRequirement;  //time requirement for messages to reach server
    void sendData();
    void printStats(std::ostream& out, const Message& msg) const;
    static int newMessageNumber(uint16_t streamNumber);

};

class OtherDataMessage : public Message{

    friend class XMLParser;

public:
    ~OtherDataMessage();
    void startDataTransfer();
    Message* copyMessage();
    void scheduleSendEvent(Callback<bool, Message*, uint8_t*>);
    void messageReceivedServer(std::string& messageName);
    void messageReceivedClient(std::string& messageName);
    virtual uint16_t getForwardMessageSize(int msgNumber = -1) const;
    virtual uint16_t getMessageSize(int msgNumber = -1) const;

    static std::map<std::string ,uint16_t, StringComparator> otherDataMessageNameMap;

    static inline uint16_t getODMNameIndex(const std::string& name){
       return OtherDataMessage::otherDataMessageNameMap.find(name)->second;
    }
    static std::string& getODMIndexName(uint16_t index){
        static std::map<std::string, uint16_t, StringComparator>::iterator it;
        static std::string failReturn("This should never happen!");

        for(it = OtherDataMessage::otherDataMessageNameMap.begin(); it != OtherDataMessage::otherDataMessageNameMap.end(); it++){
            if(it->second == index){
                return const_cast<std::string&>(it->first);
            }
        }
        return failReturn;
    }

private:
    OtherDataMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, uint16_t streamNumber, uint16_t forwardSize, bool forwardBack, uint16_t timeReq,
                     RandomVariable* ranvarTimeInterval = 0, RandomVariable* ranvarSize = 0, RandomVariable* ranvarForwardSize = 0);

    uint16_t clientTimeRequirement;

    void sendData();
    void printStats(std::ostream& out, const Message& msg)const;
    static int newMessageNumber(uint16_t streamNumber);

};


//Class Message function definitions

uint16_t Message::messagesCreated = 0;
std::map<uint16_t, std::map<MessageType, std::vector<Message::NumbersAndSizes> > >Message::numbersAndSizes;

Message::Message(std::string name, bool reliable, int timeInterval, uint16_t size, uint16_t streamNumber, uint16_t forwardSize, bool forwardBack,
                 RandomVariable* ranvarTimeInterval, RandomVariable* ranvarSize, RandomVariable* ranvarForwardSize)
    : name(name), reliable(reliable), timeInterval(timeInterval), messageSize(size), streamNumber(streamNumber),
      ranvarTimeInterval(ranvarTimeInterval), ranvarSize(ranvarSize), ranvarForwardSize(ranvarForwardSize), forwardSize(forwardSize), forwardBack(forwardBack){

}

Message::Message(const Message &msg): name(msg.getName()), reliable(msg.getReliable()), timeInterval(msg.getTimeInterval()), messageSize(msg.getMessageSize()),
    type(msg.getType()), streamNumber(msg.getStreamNumber()), forwardSize(msg.getForwardMessageSize()), forwardBack(msg.doForwardBack()){

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
                                     RandomVariable* ranvarTimeInterval, RandomVariable* ranvarSize, RandomVariable* ranvarForwardSize)
    :Message(name, reliable, timeInterval, messageSize, streamNumber, forwardSize, forwardBack, ranvarTimeInterval, ranvarSize, ranvarForwardSize), clientsOfInterest(clientsOfInterest),
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

    if(ranvarSize == 0){
        stats.size = this->getMessageSize();
    }
    else{
        stats.size = ranvarSize->GetInteger();
    }

    if(ranvarForwardSize == 0){
        stats.forwardSize = this->getForwardMessageSize();
    }else{
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

void UserActionMessage::printStats(std::ostream &out, const Message &msg) const{

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
                                   bool forwardBack, uint16_t req, RandomVariable* ranvarTimeInterval, RandomVariable* ranvarSize, RandomVariable* ranvarforwardSize)
    : Message(name, reliable, timeInterval, messageSize, streamNumber, forwardSize, forwardBack, ranvarTimeInterval, ranvarSize, ranvarforwardSize), clientTimeRequirement(req){

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

    if(ranvarSize == 0){
        stats.size = this->getMessageSize();
    }
    else{
        stats.size = ranvarSize->GetInteger();
    }

    if(ranvarForwardSize == 0){
        stats.forwardSize = this->getForwardMessageSize();
    }else{
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

void OtherDataMessage::printStats(std::ostream &out, const Message &msg) const{

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
    //do nothing, this is only forwarded back
}

void OtherDataMessage::messageReceivedClient(std::string& messageName){

    int id = 0;
    parseMessageId(messageName, id);

    StatisticsCollector::logServerMessageReceivedByClient(id, Simulator::Now(), streamNumber);

}


#endif // MESSAGES_H
