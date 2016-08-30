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

#define DEFAULT_FAMILIAR_SET_THRESHOLD 388800		//extract
#define DEFAULT_KAPPA 3								//extract


class CommunityDetection
{
protected:
	int nodeID;
	int numNodes;
	int kappa;						//extract
	int communityID;
	double familiarSetThreshold;	//extract
	double prevUpdate;
	double *cumulativeContactDurations;
	bool *currConnectedNodes;
	//bool *myLocalCommunity;
	int *myLocalCommunity;
	bool **myFamiliarSets;			//extract

public:
	CommunityDetection(int myID, int totalNodes);
	~CommunityDetection();
	void setKappa(int value);
	void setFamiliarSetThreshold(double threshold);
	void connectionOccured(double currTime, int encID);
	void disconnectionOccured(double currTime, int encID);
	void updateLCandFS(double currentTime);
	void updateBubble(int *encLocalCommunity, bool **encFamiliarSets, int encID, double currentTime);
	int *cloneLocalCommunity(double currentTime);
	bool **cloneFamiliarSets(double currentTime);
	int getMyCommunity(){return communityID;};
};
