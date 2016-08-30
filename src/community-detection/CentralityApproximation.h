/*
 *  Adyton: A Network Simulator for Opportunistic Networks
 *  Copyright (C) 2015  Nikolaos Papanikos, Dimitrios-Georgios Akestoridis,
 *  and Evangelos Papapetrou
 *
 *  This file is part of Adyton.
 *
 *  Adyton is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Adyton is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Adyton.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Written by Dimitrios-Georgios Akestoridis.
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef GOD_H
	#define GOD_H
	#include "../core/God.h"
#endif

#define SIX_HOURS 21600


class CentralityApproximation
{
protected:
	God *simGod;
	int nodeID;
	int numNodes;
	unsigned long int prevTimePeriod;
	unsigned long int currTimePeriod;
	double timeUnit;
	bool *prevTimeSlot;
	bool *currTimeSlot;
	bool *currConnectedNodes;


public:
	CentralityApproximation(int myID, int totalNodes, God *G);
	~CentralityApproximation();
	void connectionOccured(double currTime, int encID);
	void disconnectionOccured(double currTime, int encID);
	void updateTimeSlots(double currTime);
	double getLocalRank(double currTime, bool *myLocalCommunity);
	double getGlobalRank(double currTime);
	int getContactsBetweenNodeCommunities(int *encCommunity);
	void resetEpoch(int epoch);
	void incrementLocalPopularity(double currTime,int NID,int *myCommunity);
	double LocalPopularity;
	double prevLP;
	double currLP;
	double getMobilityLocalRank(double currTime);
	double getLocalRankLouvain(double currTime, int *myLocalCommunity, int myCommID);
};
