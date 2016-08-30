#ifndef MYCD_H
	#define MYCD_H
	#include "MyCD.h"
#endif
	
MyCD:MyCD(int myID, int totalNodes)
{
	nodeID = myID;
	numNodes = totalNodes
	myCommunityID = myID;
	
	if((cumulativeContactDurations = (double *) malloc(numNodes * sizeof(double))) == NULL)
	{
		printf("\nError!\nUnable to allocate memory for the cumulativeContactDurations array of node %d\n\n", nodeID);
		exit(EXIT_FAILURE);
	}
	
	if((currConnectedNodes = (bool *) malloc(numNodes * sizeof(bool))) == NULL)
	{
		printf("\nError!\nUnable to allocate memory for the currConnectedNodes array of node %d\n\n", nodeID);
		exit(EXIT_FAILURE);
	}
	
	if((communityWeights = (int *) malloc(numNodes * sizeof(int))) == NULL)
	{
		printf("\nError!\nUnable to allocate memory for the communityWeights array of node %d\n\n", nodeID);
		exit(EXIT_FAILURE);
	}
	
	int i;
	for(i = 0; i < numNodes; i++)
	{
		cumulativeContactDurations[i] = 0.0;
		currConnectedNodes[i] = false;
		communityWeights[i] = 0;
	}
	
	communityWeights[nodeID] = 1;
	
	return;
}

MyCD::~MyCD()
{
	int i;
	for(i = 0; i < numNodes; i++)
	{
		free(myFamiliarSets[i]);
	}

	free(currConnectedNodes);
	free(cumulativeContactDurations);
	free(communityWeights);

	return;
}

void MyCD::connectionOccured(double currTime, int encID)
{
	//updateLCandFS(currTime);
	currConnectedNodes[encID] = true;

	return;
}


void MyCD::disconnectionOccured(double currTime, int encID)
{
	//updateLCandFS(currTime);
	currConnectedNodes[encID] = false;

	return;
}

void MyCD::updateCommunity(int encCommunity, int encID, double currentTime)
{
	communityWeights[encCommunity] += 1;
	
	if(communityWeights[encCommunity] > communityWeights[myCommunityID])
		myCommunityID = endCommunity;
		
	return;
}
	
int getCommunity()
{
	return myCommunityID;
}
