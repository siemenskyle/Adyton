#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef COMMUNITYDETECTMODEL_H
	#define COMMUNITYDETECTMODEL_H
	#include "CommunityDetectModel.h"
#endif

#define DEFAULT_KAPPA 3								//extract
#define DEFAULT_FAMILIAR_SET_THRESHOLD 388800		//extract

class KClique : public CommunityDetectModel
{
protected:
	int kappa;						//extract
	int familiarSetThreshold;

public:
	KClique(int myID, int totalNodes);
	~KClique();
	void setKappa(int value);
	void setFamiliarSetThreshold(double threshold);
	
	void updateLC(double* contactDurations);
	
	void encounterCommunity(int *encLocalCommunity, int encID);
};
