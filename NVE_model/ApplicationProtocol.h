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

#ifndef APPLICATION_PROTOCOL_H
#define APPLICATION_PROTOCOL_H



#include "ns3/applications-module.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include <vector>
#include <string>
#include <cstring>
#include <deque>
#include "utilities.h"
#include "Messages.h"


class ApplicationProtocol{

    friend class XMLParser;

public:
    ~ApplicationProtocol();
    bool sendFromClient(const Message*, uint8_t*, Ptr<Socket>, bool forward = false);
    bool sendFromClient(const std::string& buffer, Ptr<Socket>, bool reliable);
    bool sendFromServer(uint8_t*, const Message*, const Address&, Ptr<Socket>, bool forward = false);
    bool sendFromServer(const std::string& buffer, const Address& addr, Ptr<Socket> sock, bool reliable);
    void configureForStream(Callback<void, uint8_t*, uint16_t, Address&> memFunc, bool ordered);
    void recv(Ptr<Socket>);
    uint16_t getHeaderSize() const{return headerSize;}
    void addAppProtoHeader(char* buffer, bool reliable, const Address* addr = 0);
    void transmissionStopped(const Address& addr, bool isClient);


    //IP fragmentation over UDP doesn't work like a charm, so send big packets in separate datagrams
    int sendAndFragment(Ptr<Socket> socket, uint8_t* buffer, uint16_t size, bool reliable, const Address* const addr = 0);

    //recursive function used to send all messages that fit into single datagram
    static int sendFragment(const std::string& buffer, const size_t index, Ptr<Socket> sock, uint16_t maxDatagramSize, const Address* const addr = 0,
                            ApplicationProtocol* appProto = 0, bool reliable = false, uint16_t headerSize = 0);
private:
    ApplicationProtocol(uint16_t ackSize, uint16_t delayedAck, uint16_t retransmit, uint16_t headerSize);


    class ReliablePacket{
    public:
        ReliablePacket(uint32_t msgNumber, uint16_t msgSize, uint8_t* buffer, Address addr = Address()):
            addr(addr), msgNumber(msgNumber), msgSize(msgSize), buffer(buffer){}

        ~ReliablePacket(){
            free(buffer);
        }
        Address addr;
        uint32_t msgNumber;
        uint16_t msgSize;
        uint8_t* buffer;
    };

    enum AppProtoPacketType{UNRELIABLE, RELIABLE, DUPLICATE, ACK, ERROR, UNORDERED}; //distinguish between different application proto headers

    std::list<ReliablePacket*> packetsWaitingAcks;
    std::list<ReliablePacket*> packetsOutOfOrder;
    std::map<const Address, std::deque<uint32_t> > packetsToAck;  //server needs to remember also the client address for each message it has received
    std::map<const Address, std::list<uint32_t> > alreadyAcked;   //ack these again, but do not forward to application
    uint16_t ackSize;
    uint16_t delayedAck;
    uint16_t retransmit;
    uint16_t headerSize;
    uint64_t totalBytesSent;
    std::map<const Address, uint32_t> reliableMsgNumber;
    //uint32_t reliableMsgNumber;
    Ptr<Socket> socket;
    uint16_t maxDatagramSize;
    Callback<void, uint8_t*, uint16_t, Address&> forwardToApplication;
    bool ordered;
    bool isClient;
    std::map<const Address, uint32_t> lastOrderedNumber;

    void resendCheckClient(uint32_t reliableMsgNumber);   //resend data without an ack before the timer runs out
    void resendCheckServer(std::map<const Address, uint32_t>& reliableMsgNumber, const Address& addr);
    void rememberReliablePacket(uint32_t msgNumber, uint16_t msgSize, uint8_t* msgContents, void (ApplicationProtocol::*fptr)(uint32_t));
    void rememberReliablePacket(std::map<const Address, uint32_t>&, uint16_t messageSize, const uint8_t *messageContents, const Address& addr,
                                                     void (ApplicationProtocol::*fptr)(std::map<const Address, uint32_t>&, const Address&));
    AppProtoPacketType parseAppProtoHeader(uint8_t* buffer, const Address& addr, uint32_t& msgNumber);
    bool sendAck(int* messagesToAck, uint16_t numberOfMessages, const Address& addr, Ptr<Socket> sock);
    uint16_t createAck(char* ack, int* numbers, uint16_t numberOfMessages);
    void ackAllPackets();
    ReliablePacket* getAllOrdered(const Address& addr, uint32_t reliableMsgNumber);


};


#endif // APPLICATION_PROTOCOL_H
