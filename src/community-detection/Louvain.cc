#ifndef LOUVAIN_H
	#define LOUVAIN_H
	#include "Louvain.h"
#endif

Louvain::Louvain(int myID, int totalNodes, God *G) : CommunityDetectModel(myID, totalNodes)
{
	int i;

	communityID = myID;

	for(i = 0; i < numNodes; i++)
		myLocalCommunity[i] = i;
		
	god = G;
}

Louvain::~Louvain()
{
	
}

void Louvain::updateLC(double* contactDurations)
{
	myContactDurations = contactDurations;
	
	// CALL GOD
	god->determineCommunities();
}

void Louvain::setCommunities(int* communities)
{
	for(int i = 0; i < numNodes; i++)
		myLocalCommunity[i] = communities[i];
		
	communityID = myLocalCommunity[nodeID];
}


