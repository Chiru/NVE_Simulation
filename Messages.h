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

public:
    Message(std::string, bool, int, uint16_t);
    virtual ~Message();
    virtual void startDataTransfer() = 0;
    virtual Message* copyMessage() const = 0;
    void getName() const{std::cout << name << std::endl;}

protected:
    std::string name;
    bool reliable;
    int timeInterval;
    uint16_t messageSize;
    uint32_t messageNumber;
    enum MessageType{USER_ACTION, OTHER_DATA, MAINTENANCE} type;
};

class UserActionMessage : public Message{

public:
    UserActionMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize, double clientsOfInterest, int requirement);
    ~UserActionMessage();
    void startDataTransfer();
    Message* copyMessage() const;

private:
    double clientsOfInterest;
    int timeRequirement;
    void sendData();

};

class OtherDataMessage : public Message{

public:
    OtherDataMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize);
    ~OtherDataMessage();
    void startDataTransfer();
    Message* copyMessage() const;

private:
    void sendData();

};

class MaintenanceMessage : public Message{

public:
    MaintenanceMessage(std::string name, bool reliable, int timeInterval, uint16_t messageSize);
    ~MaintenanceMessage();
    void startDataTransfer();
    Message* copyMessage() const;

private:
    void sendData();

};



//Class Message function definitions

Message::Message(std::string name, bool reliable, int timeInterval, uint16_t size)
    : name(name), reliable(reliable), timeInterval(timeInterval), messageSize(size){

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

Message* UserActionMessage::copyMessage() const{

    Message *msg;
    msg = new UserActionMessage(*this);

    return msg;


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

Message* OtherDataMessage::copyMessage() const{

    Message *msg;
    msg = new OtherDataMessage(*this);

    return msg;

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

Message* MaintenanceMessage::copyMessage() const{

    Message *msg;
    msg = new MaintenanceMessage(*this);

    return msg;
}

#endif // MESSAGES_H
