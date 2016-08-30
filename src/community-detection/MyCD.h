#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class MyCD
{
protected:
	int nodeID;
	int numNodes;
	int myCommunityID;
	int *communityWeights;
	double *cumulativeContactDurations;
	bool *currConnectedNodes;
	//bool **myFamiliarSets;
	
public:
	MyCD(int myID, int totalNodes);
	~MyCD();
	void connectionOccured(double currTime, int encID);
	void disconnectionOccured(double currTime, int encID);
	void updateCommunity(int encCommunity, int encID, double currentTime);
	int getCommunity();
	
	/*
	void updateLCandFS(double currentTime);
	void updateBubble(bool *encLocalCommunity, bool **encFamiliarSets, int encID, double currentTime);
	bool *cloneLocalCommunity(double currentTime);
	bool **cloneFamiliarSets(double currentTime);
	*/
};
