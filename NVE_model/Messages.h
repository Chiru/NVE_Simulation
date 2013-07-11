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
        msg.printStats(out);
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
    virtual void printStats(std::ostream& out)const = 0;

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
    void printStats(std::ostream& out) const;
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
    void printStats(std::ostream& out)const;
    static int newMessageNumber(uint16_t streamNumber);

};


#endif // MESSAGES_H
