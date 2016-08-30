#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_KAPPA 3								//extract
#define DEFAULT_FAMILIAR_SET_THRESHOLD 388800		//extract

class CommunityDetectModel
{
protected:
	int nodeID;
	int numNodes;
	int communityID;
	int *myLocalCommunity;

public:
	CommunityDetectModel(int myID, int totalNodes);
	~CommunityDetectModel();
	
	virtual void updateLC(double* contactDurations);
	
	int *cloneLocalCommunity();
	
	int getMyCommunity();
	
	virtual void encounterCommunity(int *encLocalCommunity, int encID);
};
