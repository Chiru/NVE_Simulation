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

enum MessageType{USER_ACTION, OTHER_DATA, MAINTENANCE};

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

    Message(std::string, bool, int, uint16_t, uint16_t, RandomVariable* ranvar = 0);
    virtual ~Message();
    virtual Message* copyMessage()  = 0;
    virtual void scheduleSendEvent(Callback<bool, Message*, uint8_t*>) = 0;
    std::string getName() const{return name;}
    bool getReliable() const{return reliable;}
    int getTimeInterval() const{return timeInterval;}
    uint16_t getMessageSize() const{return messageSize;}
    uint16_t getmessagesCreated() const {return messagesCreated;}
    uint16_t getMessageId() const {return messageID;}
    MessageType getType() const{return type;}
    uint16_t getStreamNumber() const{return streamNumber;}
    void cancelEvent();
    void fillMessageContents(char* buffer, int number = 0, std::string* msgName = NULL);
    virtual void messageReceivedServer(std::string& messageName) = 0;
    virtual void messageReceivedClient(std::string& messageName) = 0;
    bool parseMessageId(std::string& messageName, int& resultId) const;

    static std::map<std::string ,uint16_t, StringComparator> messageNameMap;

    static inline uint16_t getMessageNameIndex(const std::string& name){
       return Message::messageNameMap.find(name)->second;
    }
    static std::string& getMessageIndexName(uint16_t index){
        static std::map<std::string, uint16_t, StringComparator>::iterator it;
        static std::string failReturn("This should never happen!");

        for(it = Message::messageNameMap.begin(); it != Message::messageNameMap.end(); it++){
            if(it->second == index){
                return const_cast<std::string&>(it->first);
            }
        }
        return failReturn;
    }


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
    RandomVariable* ranvar;

    static uint16_t messagesCreated;
    static int newMessageNumber(uint16_t streamNumber);

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

private:
    UserActionMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, double clientsOfInterest, uint32_t clientRequirement, uint32_t serverRequirement,
                      uint16_t streamNumber, RandomVariable* ranvar = 0);

    double clientsOfInterest;
    uint32_t clientTimeRequirement;  //time requirement for messages to travel from client to client
    uint32_t serverTimeRequirement;  //time requirement for messages to reach server
    void sendData();
    void printStats(std::ostream& out, const Message& msg) const;

    static uint32_t messageInstanceCounter;  //holds the count of messages sent into wire
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


private:
    OtherDataMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, uint16_t streamNumber, RandomVariable* ranvar = 0);

    void sendData();
    void printStats(std::ostream& out, const Message& msg)const;

};

class MaintenanceMessage : public Message{

    friend class XMLParser;

public:
    ~MaintenanceMessage();
    void scheduleSendEvent(Callback<bool, Message*, uint8_t*>);
    Message* copyMessage();
    void messageReceivedServer(std::string& messageName);
    void messageReceivedClient(std::string& messageName);

private:
    MaintenanceMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, uint16_t streamNumber, RandomVariable* ranvar = 0);

    void sendData();
    void printStats(std::ostream& out, const Message& msg)const;

};


//Class Message function definitions

uint16_t Message::messagesCreated = 0;
std::map< std::string, uint16_t, Message::StringComparator> Message::messageNameMap = std::map<std::string, uint16_t, Message::StringComparator>();

Message::Message(std::string name, bool reliable, int timeInterval, uint16_t size, uint16_t streamNumber, RandomVariable* ranvar)
    : name(name), reliable(reliable), timeInterval(timeInterval), messageSize(size), streamNumber(streamNumber), ranvar(ranvar){

}

Message::Message(const Message &msg): name(msg.getName()), reliable(msg.getReliable()), timeInterval(msg.getTimeInterval()), messageSize(msg.getMessageSize()),
    type(msg.getType()), streamNumber(msg.getStreamNumber()){
    if(msg.ranvar != 0)
        this->ranvar = new RandomVariable(*msg.ranvar);

    this->messageID = ++messagesCreated;
}

Message::~Message(){
    delete ranvar;
}

void Message::cancelEvent(){

    if(sendEvent.IsRunning())
        Simulator::Cancel(sendEvent);

    running = false;

}

void Message::fillMessageContents(char *buffer, int number, std::string* msgName){

    buffer[0] = '\"';
    std::stringstream str("");

    if(msgName == NULL){
        strcat(buffer, name.c_str());
        strcat(buffer, ":");

        if(this->type == USER_ACTION){
            str << number;
            strcat(buffer, str.str().c_str());
        }
    }else{
        strcat(buffer, msgName->c_str());
    }

    strcat(buffer, "\"");
}

bool Message::parseMessageId (std::string &messageName, int &resultId)const{

    std::stringstream str;

    for(unsigned int i = this->getName().length() + 1; i < messageName.length();  i++) {
        str << messageName[i];
    }

    str >> resultId;

    return true;
}

int Message::newMessageNumber(uint16_t streamnumber){

    static std::vector<std::pair<int, int> > messageNumbersForStreams;          //every stream has separate message numbers
    static std::vector<std::pair<int, int> >::iterator it;
    static std::pair<int, int>* temp;
    bool exists = false;
    int retVal;

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


//Class UserActionMessage function definitions

uint32_t UserActionMessage::messageInstanceCounter = 0;

UserActionMessage::UserActionMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, double clientsOfInterest,
                                     uint32_t clientRequirement,  uint32_t serverRequirement, uint16_t streamNumber, RandomVariable* ranvar)
    :Message(name, reliable, timeInterval, messageSize, streamNumber, ranvar), clientsOfInterest(clientsOfInterest),
      clientTimeRequirement(clientRequirement), serverTimeRequirement(serverRequirement){

    type = USER_ACTION;

    StatisticsCollector::fnptr = &getMessageIndexName; //this has to be done to avoid problems with includes when static functions are called from both files

    if(messageNameMap.find(name) == messageNameMap.end()){
        messageNameMap.insert(std::make_pair<std::string, uint16_t>(name, messageNameMap.size()));   //every message name has an unique index
        StatisticsCollector::uamCount++;
    }

}

UserActionMessage::~UserActionMessage(){

}

void UserActionMessage::scheduleSendEvent(Callback<bool, Message*, uint8_t*> sendFunction){

    int interval = 0;

    this->sendFunction = sendFunction;
    running = true;

    if(ranvar != 0){
        interval = ranvar->GetInteger();
        if(interval <= 0)
            interval = 1;
    }

    if(ranvar == 0)
        sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &UserActionMessage::sendData, this);
    else
        sendEvent = Simulator::Schedule(Time(MilliSeconds(interval)), &UserActionMessage::sendData, this);
}

void UserActionMessage::sendData(){

    char buffer[30] = "";
    static Time sentTime;
    static int interval = 0;

    int messageNumber = Message::newMessageNumber(streamNumber);

    fillMessageContents(buffer, messageNumber);

    sentTime = Simulator::Now();

    StatisticsCollector::logMessagesSendFromClient(messageNumber, sentTime, streamNumber, clientTimeRequirement, serverTimeRequirement, Message::getMessageNameIndex(name), messageID);

    if(!sendFunction(this, (uint8_t*)buffer))
        PRINT_ERROR("Problems with socket buffer" << std::endl);   //TODO: socket buffer    

    if(ranvar != 0){
        interval = ranvar->GetInteger();
        if(interval <= 0)
            interval = 1;
    }

    if(running){
        if(ranvar == 0)
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

void UserActionMessage::printStats(std::ostream &out, const Message &msg) const{

    out << "UserActionMessage  " << "  ID:" << messageID <<  "  Name: " << name << "  Reliable: " << (reliable == true ? "yes" : "no")
           << "  Size: " << messageSize << " TimeInterval: " <<  timeInterval <<  "  ClientOfInterest: "
           << clientsOfInterest << "  ClientTimeRequirement: " << clientTimeRequirement << "   ServerTimeRequirement: " << serverTimeRequirement;

}


void UserActionMessage::messageReceivedServer(std::string& messageName){

    int id = 0;
    parseMessageId(messageName, id);
    StatisticsCollector::logMessageReceivedByServer(id, Simulator::Now(), streamNumber);
}

void UserActionMessage::messageReceivedClient(std::string& messageName){

    int id = 0;
    parseMessageId(messageName, id);
    StatisticsCollector::logMessageReceivedByClient(id, Simulator::Now(), streamNumber);
}

//Class OtherDataMessage function definitions


OtherDataMessage::OtherDataMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, uint16_t streamNumber, RandomVariable* ranvar)
    : Message(name, reliable, timeInterval, messageSize, streamNumber, ranvar){
    type = OTHER_DATA;

}

OtherDataMessage::~OtherDataMessage(){


}

void OtherDataMessage::sendData(){

    char buffer[30] = "";
    fillMessageContents(buffer);
    static int interval = 0;

    if(ranvar != 0){
        interval = ranvar->GetInteger();
        if(interval <= 0)
            interval = 1;
    }

    if(!sendFunction(this, (uint8_t*)buffer))
        PRINT_ERROR("Problems with server socket sending buffer." << std::endl);

    if(running){
        if(ranvar == 0)
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

void OtherDataMessage::printStats(std::ostream &out, const Message &msg) const{

    out << "OtherDataMessage  " << "  ID:" << messageID << "  Name: " << name << "  Reliable: " << (reliable == true ? "yes" : "no")
        << "  Size: " << messageSize << " TimeInterval: " <<  timeInterval;

}

void OtherDataMessage::scheduleSendEvent(Callback<bool, Message*, uint8_t*> sendFunction){

    int interval = 0;

    this->sendFunction = sendFunction;
    running = true;
    if(ranvar != 0){
        interval = ranvar->GetInteger();
        if(interval <= 0)
            interval = 1;
    }

    if(ranvar == 0)
        sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &OtherDataMessage::sendData, this);
    else
        sendEvent = Simulator::Schedule(Time(MilliSeconds(interval)), &OtherDataMessage::sendData, this);
}

void OtherDataMessage::messageReceivedServer(std::string& messageName){

}

void OtherDataMessage::messageReceivedClient(std::string& messageName){

}


//Class MaintenanceMessage function definitions


MaintenanceMessage::MaintenanceMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, uint16_t streamNumber, RandomVariable* ranvar)
    : Message(name, reliable, timeInterval, messageSize, streamNumber, ranvar){

    type = MAINTENANCE;

}

MaintenanceMessage::~MaintenanceMessage(){


}

void MaintenanceMessage::scheduleSendEvent(Callback<bool, Message*, uint8_t*> function){

    int interval = 0;

    this->sendFunction = function;
    running = true;

    if(ranvar != 0){
        interval = ranvar->GetInteger();
        if(interval <= 0)
            interval = 1;
    }

    if(ranvar == 0)
        sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &MaintenanceMessage::sendData, this);
    else
        sendEvent = Simulator::Schedule(Time(MilliSeconds(interval)), &MaintenanceMessage::sendData, this);
}

void MaintenanceMessage::sendData(){

    char buffer[30] = "";
    static int interval = 0;
    fillMessageContents(buffer);

    if(ranvar != 0){
        interval = ranvar->GetInteger();
        if(interval <= 0)
            interval = 1;
    }

    if(!sendFunction(this, (uint8_t*)buffer))
        PRINT_ERROR("Problems with socket buffer" << std::endl);   //TODO: socket buffer

    if(running){
        if(ranvar == 0)
            sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &MaintenanceMessage::sendData, this);
        else
            sendEvent = Simulator::Schedule(Time(MilliSeconds(interval)), &MaintenanceMessage::sendData, this);
    }

}

Message* MaintenanceMessage::copyMessage(){

    Message *msg;
    msg = new MaintenanceMessage(*this);
    return msg;
}

void MaintenanceMessage::printStats(std::ostream &out, const Message &msg) const{

    out << "MaintenanceMessage  " << "  ID:" << messageID << "  Name: " << name << "  Reliable: " << (reliable == true ? "yes" : "no")
        << "  Size: " << messageSize << " TimeInterval: " <<  timeInterval;

}

void MaintenanceMessage::messageReceivedServer(std::string& messageName){

}

void MaintenanceMessage::messageReceivedClient(std::string& messageName){

}

#endif // MESSAGES_H
