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
#include <string.h>

#define DEFAULT_FAMILIAR_SET_THRESHOLD 38800
#define DEFAULT_KAPPA 3

 
class CommunityDetection
{
protected:
	int nodeID;
	int numNodes;
	int kappa;
	double familiarSetThreshold;
	double prevUpdate;
	double *cumulativeContactDurations;
	bool *currConnectedNodes;
	bool *myLocalCommunity;
	bool **myFamiliarSets;

public:
	CommunityDetection(int myID, int totalNodes);
	~CommunityDetection();
	void setKappa(int value);
	void setFamiliarSetThreshold(double threshold);
	void connectionOccured(double currTime, int encID);
	void disconnectionOccured(double currTime, int encID);
	void updateLCandFS(double currentTime);
	void updateBubble(bool *encLocalCommunity, bool **encFamiliarSets, int encID, double currentTime);
	bool *cloneLocalCommunity(double currentTime);
	bool **cloneFamiliarSets(double currentTime);

	int getContactsNumHelper(bool * destCommunity);
	void BidirectionalCheck(bool *encCom,int encID);
	int getCommunityMembersAmount(void);
	bool *getCommunityFromFamiliarSet(int NodeID);

	
};
