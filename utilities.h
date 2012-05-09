#ifndef UTILITIES_H
#define UTILITIES_H

using namespace ns3;

#define PRINT_INFO(a) (StatisticsCollector::getVerbose() == true ? std::cout << a : std::cout << "")
#define PRINT_ERROR(a) (std::cerr << a)

#define SERVER_INFO(a) (StatisticsCollector::getServerLog() == true ? std::cout << "Server: " << a :std::cout << "")
#define CLIENT_INFO(a) (StatisticsCollector::getClientLog() == true ? std::cout << "Client: " << a :std::cout << "")

#endif // UTILITIES_H
