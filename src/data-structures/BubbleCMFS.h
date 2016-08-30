#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef KCLIQUE_H
	#define KCLIQUE_H
	#include "KClique.h"
#endif

#ifndef LOUVAIN_H
	#define LOUVAIN_H
	#include "Louvain.h"
#endif

#ifndef COMMUNITYDETECTMODEL_H
	#define COMMUNITYDETECTMODEL_H
	#include "CommunityDetectModel.h"
#endif

#ifndef GOD_H
	#define GOD_H
	#include "../core/God.h"
#endif

#define DEFAULT_FAMILIAR_SET_THRESHOLD 388800		//extract
#define DEFAULT_KAPPA 3								//extract


class BubbleCMFS
{
protected:
	
	//CommunityDetectModel *community;

	int nodeID;
	int numNodes;
	int kappa;						//extract
	double familiarSetThreshold;	//extract
	double prevUpdate;
	//double *cumulativeContactDurations;
	bool *currConnectedNodes;
	//bool *myLocalCommunity;
	//int *myLocalCommunity;
	bool **myFamiliarSets;			//extract

public:
	CommunityDetectModel *community;

	double *cumulativeContactDurations;

	BubbleCMFS(int myID, int totalNodes, God *G, Settings *S);
	~BubbleCMFS();
	void setKappa(int value);
	void setFamiliarSetThreshold(double threshold);
	void connectionOccured(double currTime, int encID);
	void disconnectionOccured(double currTime, int encID);
	void updateLCandFS(double currentTime);
	void updateBubble(int *encLocalCommunity, bool **encFamiliarSets, int encID, double currentTime);
	int *cloneLocalCommunity(double currentTime);
	bool **cloneFamiliarSets(double currentTime);
	int getMyCommunity(){return community->getMyCommunity();};
};
