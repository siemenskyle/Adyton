#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef COMMUNITYDETECTMODEL_H
	#define COMMUNITYDETECTMODEL_H
	#include "CommunityDetectModel.h"
#endif

#ifndef GOD_H
	#define GOD_H
	#include "../core/God.h"
#endif

class God;

class Louvain : public CommunityDetectModel
{
protected:
	double* myContactDurations;
	God *god;

public:
	Louvain(int myID, int totalNodes, God *G);
	~Louvain();
	double* getContactDurations();
	void updateLC(double* contactDurations);
	void setCommunities(int* communities);
};
