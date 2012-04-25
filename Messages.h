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

class Message{

public:
    Message();
    ~Message();
    virtual void startDataTransfer() = 0;

protected:
    bool reliable;
    ns3::Time timeInterval;
    uint16_t messageSize;
    uint32_t messageNumber;

};

class UserActionMessage : public Message{

public:
    UserActionMessage();
    ~UserActionMessage();
    void startDataTransfer();

private:
    double clientsOfInterest;
    ns3::Time timeRequirement;
    void sendData();

};

class OtherDataMessage : public Message{

public:
    OtherDataMessage();
    ~OtherDataMessage();
    void startDataTransfer();

private:
    void sendData();

};

class MaintenanceMessage : public Message{

public:
    MaintenanceMessage();
    ~MaintenanceMessage();
    void startDataTransfer();

private:
    void sendData();

};



//Class Message function definitions

Message::Message(){

}

Message::~Message(){

}


//Class UserActionMessage function definitions

UserActionMessage::UserActionMessage(){

}

UserActionMessage::~UserActionMessage(){

}

void UserActionMessage::startDataTransfer(){

}

void UserActionMessage::sendData(){

}


//Class OtherDataMessage function definitions


OtherDataMessage::OtherDataMessage(){


}

OtherDataMessage::~OtherDataMessage(){


}


void OtherDataMessage::startDataTransfer(){


}

void OtherDataMessage::sendData(){

}


//Class MaintenanceMessage function definitions


MaintenanceMessage::MaintenanceMessage(){


}

MaintenanceMessage::~MaintenanceMessage(){


}

void MaintenanceMessage::startDataTransfer(){


}

void MaintenanceMessage::sendData(){


}

#endif // MESSAGES_H
