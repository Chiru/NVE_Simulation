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

#ifndef STATISTICSCOLLECTOR_H
#define STATISTICSCOLLECTOR_H

#include <iostream>
#include "utilities.h"


class StatisticsCollector{

public:

    ~StatisticsCollector();
    static bool getVerbose() {return verbose;}
    static StatisticsCollector* createStatisticsCollector(bool);

private:
    StatisticsCollector(bool);
    static bool verbose;
    static bool collectorCreated;

};



//Class StatisticsCollector function definitions

bool StatisticsCollector::collectorCreated = false;
bool StatisticsCollector::verbose = false;

StatisticsCollector* StatisticsCollector::createStatisticsCollector(bool verbose){

    if(!collectorCreated)
        return new StatisticsCollector(verbose);

    else {
        PRINT_ERROR( "Already one StatisticsCollector exists." << std::endl);
        return NULL;
    }


}

StatisticsCollector::StatisticsCollector(bool verbose){

    StatisticsCollector::verbose = verbose;

}

StatisticsCollector::~StatisticsCollector(){

}



#endif // STATISTICSCOLLECTOR_H
