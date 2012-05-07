#ifndef UTILITIES_H
#define UTILITIES_H

using namespace ns3;

#define PRINT_INFO(a) (StatisticsCollector::getVerbose() == true ? std::cout << a : std::cout << "")
#define PRINT_ERROR(a) (std::cerr << a)

#endif // UTILITIES_H
