#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_FAMILIAR_SET_THRESHOLD 388800		//extract
#define DEFAULT_KAPPA 3								//extract


class BubbleCM
{
protected:
	CommunityDetection *community;
	FamiliarSets *familiarSets;
	int nodeID;
	int numNodes;
	//int kappa;						//extract
	int communityID;
	//double familiarSetThreshold;	//extract
	double prevUpdate;
	double *cumulativeContactDurations;
	bool *currConnectedNodes;
	//bool *myLocalCommunity;
	//int *myLocalCommunity;
	//bool **myFamiliarSets;			//extract

public:
	BubbleCM(int myID, int totalNodes, Settings *S);
	~BubbleCM();
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
