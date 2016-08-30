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


#ifndef CENTRALITYAPPROXIMATION_H
	#define CENTRALITYAPPROXIMATION_H
	#include "CentralityApproximation.h"
#endif


CentralityApproximation::CentralityApproximation(int myID, int totalNodes,God *G)
{
	int i;

	simGod=G;
	nodeID = myID;
	numNodes = totalNodes;
	prevTimePeriod = 0;
	currTimePeriod = 0;
	prevLP=0;
	currLP=0;
	timeUnit = SIX_HOURS;

	if((prevTimeSlot = (bool *) malloc(numNodes * sizeof(bool))) == NULL)
    {
        printf("\nError!\nUnable to allocate memory for the previous time slot array of node %d\n\n", nodeID);
        exit(EXIT_FAILURE);
    }

	if((currTimeSlot = (bool *) malloc(numNodes * sizeof(bool))) == NULL)
    {
        printf("\nError!\nUnable to allocate memory for the current time slot array of node %d\n\n", nodeID);
        exit(EXIT_FAILURE);
    }

	if((currConnectedNodes = (bool *) malloc(numNodes * sizeof(bool))) == NULL)
    {
        printf("\nError!\nUnable to allocate memory for the currently connected nodes array of node %d\n\n", nodeID);
        exit(EXIT_FAILURE);
    }

	for(i = 0; i < numNodes; i++)
	{
		prevTimeSlot[i] = false;
		currTimeSlot[i] = false;
		currConnectedNodes[i] = false;
	}

	return;
}


CentralityApproximation::~CentralityApproximation()
{
	free(prevTimeSlot);
	free(currTimeSlot);
	free(currConnectedNodes);

	return;
}
void CentralityApproximation::resetEpoch(int epoch)
{
	timeUnit=epoch;
}

void CentralityApproximation::connectionOccured(double currTime, int encID)
{
	updateTimeSlots(currTime);//check if it is in next time period(next 6 hours)

	currConnectedNodes[encID] = true;
	currTimeSlot[encID] = true;//??

	return;
}


void CentralityApproximation::disconnectionOccured(double currTime, int encID)
{
	updateTimeSlots(currTime);

	currConnectedNodes[encID] = false;
	if(fmod(currTime, timeUnit) > 0.0)// it doesn't make any sense ???
	{
		currTimeSlot[encID] = true;//??? still in the current time period (e.g. 10 mod 6 = 4)
	}
	else
	{	
		printf("seems it just never happen\n");
		currTimeSlot[encID] = false;//??? remainder is 0? next time period (e.g. 12 mod 6 =0 )  those are just my assumption
	}
 
	return;
}


void CentralityApproximation::updateTimeSlots(double currTime)// get the connection period ( e.g. 2 time units for last connection. 3 time units for this connection)
{
	int i;
	unsigned long int newTimePeriod;


	newTimePeriod = (unsigned long int) (currTime / timeUnit);

	if(newTimePeriod == currTimePeriod)
	{
		return;
	}
	else if(newTimePeriod > currTimePeriod)//that means the dataset have to last for at least 6 hours. it will never happen in "debugging" dataset
	{
		if(newTimePeriod == currTimePeriod + 1)//when two nodes meet each other, they compare how many unique nodes they have met in the previous unit-time slot (S-Window) OR calculate the average of all pervious windows (C-Windows)
		{
			for(i = 0; i < numNodes; i++)
			{
				prevTimeSlot[i] = currTimeSlot[i];

				if(currConnectedNodes[i])//currtimeslot is always consist with currconnectednodes
				{
					currTimeSlot[i] = true;
				}
				else
				{
					currTimeSlot[i] = false;
				}
			}

			prevTimePeriod = currTimePeriod;
			currTimePeriod = newTimePeriod;
			
			prevLP=currLP;
			currLP=LocalPopularity;
			//LocalPopularity=0;
		}
		else//a gap in dataset
		{		
			for(i = 0; i < numNodes; i++)
			{ 
				if(currConnectedNodes[i])
				{
					prevTimeSlot[i] = true;
					currTimeSlot[i] = true;
				}
				else
				{
					prevTimeSlot[i] = false;
					currTimeSlot[i] = false;
				}
			}

			prevTimePeriod = newTimePeriod - 1;
			currTimePeriod = newTimePeriod;

			prevLP=currLP;
			currLP=LocalPopularity;
			//LocalPopularity=0;
	
		}

		return;
	}
	else
	{
		printf("\nError!\nTime is going backwards!\n\n");
		exit(EXIT_FAILURE);
	}
}



double CentralityApproximation::getLocalRank(double currTime, bool *myLocalCommunity)
{
	int i;
	double localRank;


	updateTimeSlots(currTime);


	/* Count my unique encounters with nodes in my local community */
	localRank = 0.0;
	if(prevTimePeriod != currTimePeriod)
	{// curr - prev = 1
		for(i = 0; i < numNodes; i++)
		{
			if((myLocalCommunity[i]) && (prevTimeSlot[i]))
			{
				localRank++;
			}
		}
	}
	else
	{//prev and curr are both "0"
		for(i = 0; i < numNodes; i++)
		{
			if((myLocalCommunity[i]) && (currTimeSlot[i]))
			{
				localRank++;
			}
		}
	}

	return localRank;
}




double CentralityApproximation::getGlobalRank(double currTime)
{
	int i;
	double globalRank;


	updateTimeSlots(currTime);


	/* Count my unique encounters with any node in the network */
	globalRank = 0.0;
	if(prevTimePeriod != currTimePeriod)
	{
		for(i = 0; i < numNodes; i++)
		{
			if(prevTimeSlot[i])
			{
				globalRank++;
			}
		}
	}
	else
	{
		for(i = 0; i < numNodes; i++)
		{
			if(currTimeSlot[i])
			{
				globalRank++;
			}
		} 
	}

	return globalRank;
}

double CentralityApproximation::getMobilityLocalRank(double currTime)
{
	updateTimeSlots(currTime);
	/* Count my unique encounters with nodes in my local community */
	if(prevTimePeriod != currTimePeriod)
	{// curr - prev = 1
			return prevLP;
	}
	else
	{
			return currLP;
	}

}







void CentralityApproximation::incrementLocalPopularity(double currTime,int NID,int *myLocalCommunity)
{
	updateTimeSlots(currTime);

	if(prevTimePeriod != currTimePeriod)//duplicated??? the else can be deleted. I guess
	{// curr - prev = 1
		
			if((myLocalCommunity[NID]==myLocalCommunity[nodeID]) && (prevTimeSlot[NID]))
			{
				this->LocalPopularity++;
			}

	}
	else
	{//prev and curr are both "0". Only for the first epoch

			if((myLocalCommunity[NID]==myLocalCommunity[nodeID]) && (currTimeSlot[NID]))
			{
				this->LocalPopularity++;
			}
			
	}

}
 
int CentralityApproximation::getContactsBetweenNodeCommunities(int *encCommunity)
{
	int NCF=0;
/*
	int counter=0;
	for(int i = 0; i < numNodes; i++)
	{
		if(encCommunity[i])
		counter++;
	}
*/
	if(true)
	{
		/*
		for(int i = 0; i < numNodes; i++)
		{
			if(currTimeSlot[i] && encCommunity[i])
			{
				NCF++;
			}
			else if(currTimeSlot[i] && encCommunity[i] && i==this->nodeID){printf("it should not be printed\n");}
		}
		*/
		for(int i = 0; i < numNodes; i++)
		{
			if(currTimeSlot[i])
			{
				if(simGod->getCommunityID(i)==encCommunity[i])
				NCF++;
			}
			else if(currTimeSlot[i] && encCommunity[i] && i==this->nodeID){printf("it should not be printed\n");}
		}
	}
	if (NCF==0)
	return 0;
	else
	return NCF+1;
}

double CentralityApproximation::getLocalRankLouvain(double currTime, int *myLocalCommunity, int myCommID)
{
	int i;
	double localRank;


	updateTimeSlots(currTime);


	/* Count my unique encounters with nodes in my local community */
	localRank = 0.0;
	if(prevTimePeriod != currTimePeriod)
	{
		for(i = 0; i < numNodes; i++)
		{
			if((myLocalCommunity[i] == myCommID) && (prevTimeSlot[i]))
			{
				localRank++;
			}
		}
	}
	else
	{
		for(i = 0; i < numNodes; i++)
		{
			if((myLocalCommunity[i] == myCommID) && (currTimeSlot[i]))
			{
				localRank++;
			}
		}
	}

	return localRank;
}
