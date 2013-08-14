/**
* Copyright (c) 2013 Center for Internet Excellence, University of Oulu, All Rights Reserved
* For conditions of distribution and use, see copyright notice in license.txt
*/


#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <ns3/socket.h>
#include "ns3/address.h"

using namespace ns3;

#define PRINT_RESULT(a) (std::cout << a)
#define PRINT_INFO(a) (StatisticsCollector::getVerbose() == true ? std::cout << a : std::cout << "")
#define PRINT_ERROR(a) (std::cerr << a)

#define SERVER_INFO(a) (StatisticsCollector::getServerLog() == true ? std::cout << "Server: " << a :std::cout << "")
#define CLIENT_INFO(a) (StatisticsCollector::getClientLog() == true ? std::cout << "Client: " << a :std::cout << "")

#endif // UTILITIES_H
