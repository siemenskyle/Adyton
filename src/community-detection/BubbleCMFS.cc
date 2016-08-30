#ifndef BUBBLECMFS_H
	#define BUBBLECMFS_H
	#include "BubbleCMFS.h"
#endif


BubbleCMFS::BubbleCMFS(int myID, int totalNodes, God *G, Settings *S)
{
	int i;
	int j;

	string profileAttribute;
	nodeID = myID;
	numNodes = totalNodes;
	kappa = DEFAULT_KAPPA;
	familiarSetThreshold = DEFAULT_FAMILIAR_SET_THRESHOLD;
	prevUpdate = 0.0;
	
	if(S->ProfileExists() && (profileAttribute = S->GetProfileAttribute("communityDetect")) != "none")
		community = new Louvain(myID, totalNodes, G);
	else
		community = new KClique(myID, totalNodes);
		

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

	/*if((myLocalCommunity = (int *) malloc(numNodes * sizeof(int))) == NULL)
	{
		printf("\nError!\nUnable to allocate memory for the myLocalCommunity array of node %d\n\n", nodeID);
		exit(EXIT_FAILURE);
	}*/

	if((myFamiliarSets = (bool **) malloc(numNodes * sizeof(bool *))) == NULL)
	{
		printf("\nError!\nUnable to allocate memory for the approximatedFamiliarSets array of node %d\n\n", nodeID);
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < numNodes; i++)
	{
		if((myFamiliarSets[i] = (bool *) malloc(numNodes * sizeof(bool))) == NULL)
		{
		    printf("\nError!\nUnable to allocate memory for the approximatedFamiliarSets array of node %d\n\n", nodeID);
		    exit(EXIT_FAILURE);
		}
	}

	for(i = 0; i < numNodes; i++)
	{
		cumulativeContactDurations[i] = 0.0;
		currConnectedNodes[i] = false;

		/*if(i != nodeID)
		{
			myLocalCommunity[i] = 0;
		}
		else
		{
			myLocalCommunity[i] = 1;
		}
		*/
		for(j = 0; j < numNodes; j++)
		{
			myFamiliarSets[i][j] = false;
		}
	}

	return;
}


BubbleCMFS::~BubbleCMFS()
{
	int i;


	for(i = 0; i < numNodes; i++)
	{
		free(myFamiliarSets[i]);
	}

	free(myFamiliarSets);
	//free(myLocalCommunity);
	free(community);
	free(currConnectedNodes);
	free(cumulativeContactDurations);

	return;
}


void BubbleCMFS::setKappa(int value)
{
	kappa = value;
	((KClique *) community)->setKappa(value);

	return;
}


void BubbleCMFS::setFamiliarSetThreshold(double threshold)
{
	familiarSetThreshold = threshold;
	((KClique *) community)->setFamiliarSetThreshold(threshold);
	return;
}


void BubbleCMFS::connectionOccured(double currTime, int encID)
{
	updateLCandFS(currTime);
	currConnectedNodes[encID] = true;

	return;
}


void BubbleCMFS::disconnectionOccured(double currTime, int encID)
{
	updateLCandFS(currTime);
	currConnectedNodes[encID] = false;

	return;
}


void BubbleCMFS::updateLCandFS(double currTime)
{
	int i;

	for(i = 0; i < numNodes; i++)
	{
		if(currConnectedNodes[i])
		{
			cumulativeContactDurations[i] += currTime - prevUpdate;

			if((cumulativeContactDurations[i] > familiarSetThreshold) && (!myFamiliarSets[nodeID][i]))
			{
				myFamiliarSets[nodeID][i] = true;
				//myLocalCommunity[i] = 1;
			}
		}
	}

	community->updateLC(cumulativeContactDurations);

	prevUpdate = currTime;

	return;
}


void BubbleCMFS::updateBubble(int *encLocalCommunity, bool **encFamiliarSets, int encID, double currentTime)
{
	int i;
	int j;
	int commonMembers;


	updateLCandFS(currentTime);


	community->encounterCommunity(encLocalCommunity, encID);
	
	int* myLocalCommunity = community->cloneLocalCommunity();
	int myCommID = community->getMyCommunity();
	
	/* Check if the encounter node is not in my local community */
	if(myLocalCommunity[encID] != myCommID)
	{
		commonMembers = 0;

		for(i = 0; i < numNodes; i++)
		{
			if((encFamiliarSets[encID][i]) && (myLocalCommunity[i] == 1))
			{
				commonMembers++;
			}
		}


		/*if(commonMembers >= kappa - 1)
		{
			myLocalCommunity[encID] = 1;
		}*/
	}


	/* Check if the encounter node is in my local community */
	if(myLocalCommunity[encID] == myCommID)
	{
		for(i = 0; i < numNodes; i++)
		{
			/* Update my approximation of the familiar set of the encountered node */
			myFamiliarSets[encID][i] = encFamiliarSets[encID][i];


			/* Check the node in the local community of the encountered node */
			if((encLocalCommunity[i] == 1) && (myLocalCommunity[i] != myCommID))
			{
				commonMembers = 0;

				for(j = 0; j < numNodes; j++)
				{
					if((encFamiliarSets[i][j]) && (myLocalCommunity[j] == myCommID))
					{
						commonMembers++;
					}
				}

				if(commonMembers >= kappa - 1)
				{
					//myLocalCommunity[i] = 1;

					/* Update my approximation of the familiar set of the new community member */
					for(j = 0; j < numNodes; j++)
					{
						myFamiliarSets[i][j] = encFamiliarSets[i][j];
					}
				}
			}
		}
	}

	free(myLocalCommunity);

	return;
}


int *BubbleCMFS::cloneLocalCommunity(double currentTime)
{	
	updateLCandFS(currentTime);

	return community->cloneLocalCommunity();
}


bool **BubbleCMFS::cloneFamiliarSets(double currentTime)
{
	int i;
	bool **tmp;


	updateLCandFS(currentTime);

	if((tmp = (bool **) malloc(numNodes * sizeof(bool *))) == NULL)
	{
		printf("\nError!\nUnable to allocate memory to clone the approximatedFamiliarSets array of node %d\n\n", nodeID);
		exit(EXIT_FAILURE);
	}

	memcpy(tmp, myFamiliarSets, numNodes * sizeof(bool*));

	for(i = 0; i < numNodes; i++)
	{
		if((tmp[i] = (bool *) malloc(numNodes * sizeof(bool))) == NULL)
		{
		    printf("\nError!\nUnable to allocate memory to clone the approximatedFamiliarSets array of node %d\n\n", nodeID);
		    exit(EXIT_FAILURE);
		}

		memcpy(tmp[i], myFamiliarSets[i], numNodes * sizeof(bool));
	}

	return tmp;
}
int BubbleCMFS::getCommunityMembersAmount(void)
{
	int contacts=0;
	for(int i=0; i<numNodes;i++)
	{
		if(myFamiliarSets[nodeID][i])
		contacts++;
	}
	return contacts--;

}

bool *BubbleCMFS::getCommunityFromFamiliarSet(int NodeID)
{
	bool *tmp;
	

	if((tmp = (bool *) malloc(numNodes * sizeof(bool))) == NULL)
	{
		printf("\nError!\nUnable to allocate memory to clone the myLocalCommunity array of node %d\n\n", nodeID);
		exit(EXIT_FAILURE);
	}

	memcpy(tmp, myFamiliarSets[NodeID], numNodes * sizeof(bool));

	return tmp;
}

