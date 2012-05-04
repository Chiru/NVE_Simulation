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


class Message{

    friend std::ostream& operator<<( std::ostream& out, const Message& msg){
        msg.printStats(out, msg);
        return out;
    }

public:
    Message(std::string, bool, int, uint16_t);
    virtual ~Message();
    virtual void startDataTransfer() = 0;
    virtual Message* copyMessage()  = 0;
    std::string getName() const{return name;}
    bool getReliable() const{return reliable;}
    int getTimeInterval() const{return timeInterval;}
    uint16_t getMessageSize() const{return messageSize;}
    uint16_t getmessagesCreated() const {return messagesCreated;}


  protected:
    Message(const Message&);
    std::string name;
    bool reliable;
    int timeInterval;
    uint16_t messageSize;
    uint16_t messageID;
    enum MessageType{USER_ACTION, OTHER_DATA, MAINTENANCE} type;
    virtual void printStats(std::ostream& out, const Message& msg)const = 0;

    static uint16_t messagesCreated;
};

class UserActionMessage : public Message{

    friend class XMLParser;

public:
    ~UserActionMessage();
    void startDataTransfer();
    Message* copyMessage();

private:
    UserActionMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, double clientsOfInterest, int requirement);

    double clientsOfInterest;
    int timeRequirement;
    void sendData();
    void printStats(std::ostream& out, const Message& msg) const;

};

class OtherDataMessage : public Message{

    friend class XMLParser;

public:
    ~OtherDataMessage();
    void startDataTransfer();
    Message* copyMessage();

private:
    OtherDataMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize);

    void sendData();
    void printStats(std::ostream& out, const Message& msg)const;

};

class MaintenanceMessage : public Message{

    friend class XMLParser;

public:
    ~MaintenanceMessage();
    void startDataTransfer();
    Message* copyMessage();

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

Message::Message(const Message &msg): name(msg.getName()), reliable(msg.getReliable()), timeInterval(msg.getTimeInterval()), messageSize(msg.getMessageSize()){

    this->messageID = ++messagesCreated;

}

Message::~Message(){

}


//Class UserActionMessage function definitions

UserActionMessage::UserActionMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, double clientsOfInterest, int requirement)
    :Message(name, reliable, timeInterval, messageSize), clientsOfInterest(clientsOfInterest), timeRequirement(requirement){

    type = USER_ACTION;

}

UserActionMessage::~UserActionMessage(){

}

void UserActionMessage::startDataTransfer(){

}

void UserActionMessage::sendData(){

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

//Class OtherDataMessage function definitions


OtherDataMessage::OtherDataMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize)
    : Message(name, reliable, timeInterval, messageSize){

    type = OTHER_DATA;

}

OtherDataMessage::~OtherDataMessage(){


}


void OtherDataMessage::startDataTransfer(){


}

void OtherDataMessage::sendData(){

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

//Class MaintenanceMessage function definitions


MaintenanceMessage::MaintenanceMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize)
    : Message(name, reliable, timeInterval, messageSize){

    type = MAINTENANCE;

}

MaintenanceMessage::~MaintenanceMessage(){


}

void MaintenanceMessage::startDataTransfer(){


}

void MaintenanceMessage::sendData(){


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



#endif // MESSAGES_H
