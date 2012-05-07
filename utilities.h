#ifndef UTILITIES_H
#define UTILITIES_H

#define PRINT_INFO(a) (StatisticsCollector::getVerbose() == true ? std::cout << a : std::cout << "")
#define PRINT_ERROR(a) (std::cerr << a)

#endif // UTILITIES_H
