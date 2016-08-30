#ifndef KCLIQUE_H
	#define KCLIQUE_H
	#include "KClique.h"
#endif

KClique::KClique(int myID, int totalNodes) : CommunityDetectModel(myID, totalNodes)
{
	int i;

	communityID = 1;

	kappa = DEFAULT_KAPPA;
	familiarSetThreshold = DEFAULT_FAMILIAR_SET_THRESHOLD;

	for(i = 0; i < numNodes; i++)
	{
		if(i != nodeID)
		{
			myLocalCommunity[i] = 0;
		}
		else
		{
			myLocalCommunity[i] = 1;
		}
	}
}

KClique::~KClique()
{
	
}

void KClique::setKappa(int value)
{
	kappa = value;

	return;
}

void KClique::setFamiliarSetThreshold(double threshold)
{
	familiarSetThreshold = threshold;
	
	return;
}

void KClique::updateLC(double* contactDurations)
{
	for(int i = 0; i < numNodes; i++)
	{
		if((contactDurations[i] > familiarSetThreshold))
		{
			myLocalCommunity[i] = 1;
		}
	}
}

void KClique::encounterCommunity(int *encLocalCommunity, int encID)
{
	int i = 0;
	int j = 0;
	int commonMembers = 0;
	/* Check if the encounter node is not in my local community */
	if(myLocalCommunity[encID] == 0)
	{
		commonMembers = 0;

		for(i = 0; i < numNodes; i++)
		{
			if((encLocalCommunity[i] == 1) && (myLocalCommunity[i] == 1))
			{
				commonMembers++;
			}
		}


		if(commonMembers >= kappa - 1)
		{
			myLocalCommunity[encID] = 1;
		}
	}


	/* Check if the encounter node is in my local community */
	if(myLocalCommunity[encID] == 1)
	{
		for(i = 0; i < numNodes; i++)
		{
			/* Check the node in the local community of the encountered node */
			if((encLocalCommunity[i] == 1) && (myLocalCommunity[i] == 0))
			{
				commonMembers = 0;

				for(j = 0; j < numNodes; j++)
				{
					if((encLocalCommunity[j] == 1) && (myLocalCommunity[j] == 1))
					{
						commonMembers++;
					}
				}

				if(commonMembers >= kappa - 1)
				{
					myLocalCommunity[i] = 1;
				}
			}
		}
	}
}
