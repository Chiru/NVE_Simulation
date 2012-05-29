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
#include "StatisticsCollector.h"

class DataGenerator;

enum MessageType{USER_ACTION, OTHER_DATA, MAINTENANCE};

class Message{

    friend std::ostream& operator<<( std::ostream& out, const Message& msg){
        msg.printStats(out, msg);
        return out;
    }

public:
    Message(std::string, bool, int, uint16_t);
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
    void cancelEvent();
    void fillMessageContents(char* buffer, int number = 0, std::string* msgName = NULL);
    virtual void messageReceivedServer(std::string& messageName) = 0;
    virtual void messageReceivedClient(std::string& messageName) = 0;

  protected:
    Message(const Message&);
    virtual void printStats(std::ostream& out, const Message& msg)const = 0;
    bool parseMessageId(std::string& messageName, int& resultId);

    std::string name;
    bool reliable;
    int timeInterval;
    uint16_t messageSize;
    uint16_t messageID;
    MessageType type;
    Callback<bool, Message*,  uint8_t*> sendFunction;
    EventId sendEvent;
    bool running;

    static uint16_t messagesCreated;
};

class UserActionMessage : public Message{

    friend class XMLParser;

public:
    ~UserActionMessage();
    Message* copyMessage();
    void scheduleSendEvent(Callback<bool, Message*, uint8_t*>);
    double getClientsOfInterest() const{return clientsOfInterest;}    
    int newMessageSent();
    void messageReceivedServer(std::string& messageName);
    void messageReceivedClient(std::string& messageName);

private:
    UserActionMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, double clientsOfInterest, int requirement);

    double clientsOfInterest;
    int timeRequirement;
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
    OtherDataMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize);

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
    MaintenanceMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize);

    void sendData();
    void printStats(std::ostream& out, const Message& msg)const;

};



//Class Message function definitions

uint16_t Message::messagesCreated = 0;

Message::Message(std::string name, bool reliable, int timeInterval, uint16_t size)
    : name(name), reliable(reliable), timeInterval(timeInterval), messageSize(size){

}

Message::Message(const Message &msg): name(msg.getName()), reliable(msg.getReliable()), timeInterval(msg.getTimeInterval()), messageSize(msg.getMessageSize()), type(msg.getType()){

    this->messageID = ++messagesCreated;

}

Message::~Message(){

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

bool Message::parseMessageId(std::string &messageName, int &resultId){

    std::stringstream str;

    for(unsigned int i = this->getName().length() + 1; i < messageName.length();  i++){
        str << messageName[i];
    }

    str >> resultId;

    return true;
}


//Class UserActionMessage function definitions

uint32_t UserActionMessage::messageInstanceCounter = 0;

UserActionMessage::UserActionMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, double clientsOfInterest, int requirement)
    :Message(name, reliable, timeInterval, messageSize), clientsOfInterest(clientsOfInterest), timeRequirement(requirement){

    type = USER_ACTION;

}

UserActionMessage::~UserActionMessage(){

}

void UserActionMessage::scheduleSendEvent(Callback<bool, Message*, uint8_t*> sendFunction){

    this->sendFunction = sendFunction;
    running = true;
    sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &UserActionMessage::sendData, this);

}

void UserActionMessage::sendData(){

    char buffer[30] = "";
    Time sentTime;
    int messageNumber;

    messageNumber = newMessageSent();

    fillMessageContents(buffer, messageNumber);

    sentTime = Simulator::Now();
    StatisticsCollector::logMessagesSendFromClient(messageNumber, sentTime);

    if(!sendFunction(this, (uint8_t*)buffer))
        PRINT_ERROR("Problems with socket buffer" << std::endl);   //TODO: socket buffer    

    if(running){
        sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &UserActionMessage::sendData, this);
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
           << clientsOfInterest << "  TimeRequirement: " << timeRequirement;

}

int UserActionMessage::newMessageSent(){

    int returnValue;

    returnValue = messageInstanceCounter;
    messageInstanceCounter++;

    return returnValue;
}

void UserActionMessage::messageReceivedServer(std::string& messageName){

    int id = 0;
    parseMessageId(messageName, id);
    StatisticsCollector::logMessageReceivedByServer(id, Simulator::Now());
}

void UserActionMessage::messageReceivedClient(std::string& messageName){

    int id = 0;

    parseMessageId(messageName, id);

    StatisticsCollector::logMessageReceivedByClient(id, Simulator::Now());
}

//Class OtherDataMessage function definitions


OtherDataMessage::OtherDataMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize)
    : Message(name, reliable, timeInterval, messageSize){

    type = OTHER_DATA;

}

OtherDataMessage::~OtherDataMessage(){


}

void OtherDataMessage::sendData(){

    char buffer[30] = "";
    fillMessageContents(buffer);

    if(!sendFunction(this, (uint8_t*)buffer))
        PRINT_ERROR("Problems with server socket sending buffer." << std::endl);

    if(running){
        sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &OtherDataMessage::sendData, this);
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

    this->sendFunction = sendFunction;
    running = true;
    sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &OtherDataMessage::sendData, this);
}

void OtherDataMessage::messageReceivedServer(std::string& messageName){

}

void OtherDataMessage::messageReceivedClient(std::string& messageName){

}


//Class MaintenanceMessage function definitions


MaintenanceMessage::MaintenanceMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize)
    : Message(name, reliable, timeInterval, messageSize){

    type = MAINTENANCE;

}

MaintenanceMessage::~MaintenanceMessage(){


}

void MaintenanceMessage::scheduleSendEvent(Callback<bool, Message*, uint8_t*> function){

    this->sendFunction = function;
    running = true;
    sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &MaintenanceMessage::sendData, this);

}

void MaintenanceMessage::sendData(){

    char buffer[30] = "";
    buffer[0] = '\"';
    strcat(buffer, name.c_str());
    strcat(buffer, "\"");

    if(!sendFunction(this, (uint8_t*)buffer))
        PRINT_ERROR("Problems with socket buffer" << std::endl);   //TODO: socket buffer

    if(running)
        sendEvent = Simulator::Schedule(Time(MilliSeconds(timeInterval)), &MaintenanceMessage::sendData, this);

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
