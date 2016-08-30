#ifndef COMMUNITYDETECTMODEL_H
	#define COMMUNITYDETECTMODEL_H
	#include "CommunityDetectModel.h"
#endif

CommunityDetectModel::CommunityDetectModel(int myID, int totalNodes)
{
	nodeID = myID;
	numNodes = totalNodes;
	
	if((myLocalCommunity = (int *) malloc(numNodes * sizeof(int))) == NULL)
	{
		printf("\nError!\nUnable to allocate memory for the myLocalCommunity array of node %d\n\n", nodeID);
		exit(EXIT_FAILURE);
	}
}

CommunityDetectModel::~CommunityDetectModel()
{
	free(myLocalCommunity);
	return;
}

void CommunityDetectModel::updateLC(double* contactDurations)
{
	return;
}

void CommunityDetectModel::encounterCommunity(int *encLocalCommunity, int encID)
{
	return;
}

int* CommunityDetectModel::cloneLocalCommunity()
{
	int *tmp;
	
	if((tmp = (int *) malloc(numNodes * sizeof(int))) == NULL)
	{
		printf("\nError!\nUnable to allocate memory to clone the myLocalCommunity array of node %d\n\n", nodeID);
		exit(EXIT_FAILURE);
	}

	memcpy(tmp, myLocalCommunity, numNodes * sizeof(int));

	return tmp;
}

int CommunityDetectModel::getMyCommunity()
{
	return communityID;
}
