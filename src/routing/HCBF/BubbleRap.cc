/*
 *  Adyton: A Network Simulator for Opportunistic Networks
 *  Copyright (C) 2015  Nikolaos Papanikos, Dimitrios-Georgios Akestoridis,
 *  and Evangelos Papapetrou
 *
 *  This file is part of Adyton.
 *
 *  Adyton is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Adyton is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Adyton.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Written by Dimitrios-Georgios Akestoridis.
 */


#ifndef BUBBLERAP_H
	#define BUBBLERAP_H
	#include "BubbleRap.h"
#endif

//#define BUBBLERAP_DEBUG

//familiarSetThreshold, kappa,multi-copy are all 0.0 or None. they are not stored in the "S". Besides, S->ProfileExists() would return false, which means that all the these three if-statement would be executed.
BubbleRap::BubbleRap(PacketPool* PP, MAC* mc, PacketBuffer* Bf, int NID, Statistics* St, Settings *S, God *G) : Routing(PP, mc, Bf, NID, St, S, G)
{
	string profileAttribute;
	InterCopyOn=false;
	IntraCopyOn=false;

	ranking = new CentralityApproximation(NID, Set->getNN());//getNN()): total nodes number
	labeling = new CommunityDetection(NID, Set->getNN());


//------------set extra attributes by the extra arguments in the profile.txt-------------//
	if(S->ProfileExists() && (profileAttribute = S->GetProfileAttribute("familiarSetThreshold")) != "none")
//familiarSetThresholdevolution of the communities detected at different times,default value of "famiiarSetThreshold" is 388800
	{
		labeling->setFamiliarSetThreshold(atof(profileAttribute.c_str()));
	}
//kappa: the number of time units that have elapsed since the last time the delivery predictabilities were aged. default value is 3
	if(S->ProfileExists() && (profileAttribute = S->GetProfileAttribute("kappa")) != "none")
	{
		labeling->setKappa(atof(profileAttribute.c_str()));
	}

	if(S->ProfileExists() && (profileAttribute = S->GetProfileAttribute("multi-copy")) != "none")
	{
		Set->setCopyMode(false);
		if(profileAttribute == "all")
		{
			IntraCopyOn=true;
			InterCopyOn=true;
		}
		else if(profileAttribute == "inter")
		{
			IntraCopyOn=false;
			InterCopyOn=true;
		}
		else
		{
			exit(1);
		}
	}
	//printf("familiar thershold is:  %f \n",labeling->familiarSetThreshold);
	return;
}

BubbleRap::~BubbleRap()
{
	delete ranking;
	delete labeling;
	return;
}

void BubbleRap::NewContact(double CTime, int NID)//CTime: current time
{
	
	ranking->connectionOccured(CTime, NID);//set the connection and update curr/prev TimeSlot (when it get into next time period(next 6 hours))
	labeling->connectionOccured(CTime, NID);//do the connection, then cumulate the duration, and check if exceed the threshold

	Contact(CTime, NID);

	return;
}


void BubbleRap::ContactRemoved(double CTime, int NID)
{

	ranking->disconnectionOccured(CTime, NID);
	labeling->disconnectionOccured(CTime, NID);
	bool* encComm=this->SimGod->getCommunityFromCertainNode(NID);
	labeling->BidirectionalCheck(encComm,NID);
	return;
}


void BubbleRap::Contact(double CTime, int NID)
{
	//First send all packets that have NID as destination, and then do the cleaning(buff)
	//SendDirectPackets(CTime,NID);
	
	//Get information about known delivered packets
	int *Information=DM->GetInfo();//For JustTTL: it just return NULL
	//Buf->checkAllPkts();
	if(Information != NULL)
	{
	
		//Create a vaccine information packet
		SendVaccine(CTime,NID,Information);
	}
	else
	{
		
		//Clean buffer using Deletion method (Delivered pkts)
		DM->CleanBuffer(this->Buf);//not for justTTL
		if(DM->ExchangeDirectSummary())// return false for the default
		{
			SendDirectSummary(CTime,NID);
		}
		else
		{
			SendDirectPackets(CTime,NID);
		}
	}
	return;
}

void BubbleRap::AfterDirectTransfers(double CTime, int NID)
{
	Packet *ReqPacket;
	Header *h;
	/* Create a new request packet (Local Community and Familiar Sets request) */
	ReqPacket = new ReqLCandFS(CTime, 0);
	h = new BasicHeader(this->NodeID, NID);
	ReqPacket->setHeader(h);
	pktPool->AddPacket(ReqPacket);
	/* Send the packet to the new contact */
	Mlayer->SendPkt(CTime, this->NodeID, NID, ReqPacket->getSize(), ReqPacket->getID());
	#ifdef BUBBLERAP_DEBUG
		printf("current time is : %f and the PKT_ID is: %d \n", CTime,ReqPacket->getID());
		printf("%d: Sending a request packet to node %d for its Familiar Set and its Local Community\n", this->NodeID, NID);
	#endif
}



void BubbleRap::recv(double rTime, int pktID)
{
	int PacketID;
	Packet *p;
	Header *h;

	if((p = pktPool->GetPacket(pktID)) == NULL)
	{
		printf("Error: Packet %d doesn't exist in Packet Pool! Aborting...\n", pktID);
		exit(1);
	}

	h = p->getHeader();//h->GetprevHop(): could get the node_id of the sender 

	if(h->IsDuplicate() == true)// the first transmission event is to add into the sender buffer with the original pktID, the second(current) transmission has a different pktID, so we need to get the original pktID
	{	
		#ifdef BUBBLERAP_DEBUG	
		printf("going to receive a duplicated packet\n");
		#endif
		PacketID = h->GetOriginal();
	}
	else
	{
 		PacketID = pktID;
	}


	/* Sanity check */
	if(rTime < 0 || p->GetStartTime() < 0)
	{
		printf("%f: Node %d received new packet with ID %d and type %d from node %d\n", rTime, this->NodeID, p->getID(), p->getType(), h->GetprevHop());
		exit(1);
	}



	/* Identify the type of the packet */
	switch(p->getType())
	{
		case DATA_PACKET://others
		{
			ReceptionData(h, p, pktID, rTime, PacketID);
			break;
		}
		case DIRECT_SUMMARY_PACKET:// all the case below do not need to worried about "IsDuplicate". cause they do not need "PacketID".
		{
			ReceptionDirectSummary(h,p,pktID,rTime);
			break;
		}
		case DIRECT_REQUEST_PACKET:
		{
			ReceptionDirectRequest(h,p,pktID,rTime);
			break;
		}
		case REQUEST_LC_FS://second third
		{
			ReceptionReqLCandFS(h, p, pktID, rTime);
			break;
		}
		case LC_FS://fourth fifth
		{
			ReceptionLCandFS(h, p, pktID, rTime);
			break;
		}
		case BUBBLE_SUMMARY://sixth seventh
		{
			ReceptionBubbleSummary(h, p, pktID, rTime);
			break;
		}
		case MARKED_REQUEST_PKTS:
		{
			ReceptionRequestVector(h, p, pktID, rTime);
			break;
		}
		case ANTIPACKET:
		{
			ReceptionAntipacket(h,p,pktID,rTime);
			break;
		}
		case ANTIPACKET_RESPONSE:
		{
			ReceptionAntipacketResponse(h,p,pktID,rTime);
			break;
		}
		case REQUEST_BUFFER_INFO:
		{
			ReceptionBufferReq(h, p, pktID, rTime);
			break;
		}
		case BUFFER_INFO:
		{
			ReceptionBufferRsp(h, p, pktID, rTime);
			break;
		}
		default:
		{
			printf("Error: Unknown packet type #%d\nBubble Rap is not using this type of packet... Aborting...\n", p->getType());
			exit(1);
		}
	}

	return;
}


void BubbleRap::ReceptionData(Header* hd, Packet* pkt, int PID, double CurrentTime, int RealID)//PID: the old packet_ID, may differ from "RealID" if Dupulicated  others
{
	#ifdef BUBBLERAP_DEBUG
		printf("%f: Node %d received new data packet with ID %d from node %d\n", CurrentTime, this->NodeID, RealID, hd->GetprevHop());
	#endif

	
	/* Check if I am the packet creator (packet comes from the application layer) */ 
	//it is the source.e.g. hd->GetSource()
	if((hd->GetSource() == this->NodeID) && (hd->GetNextHop() == -1))// a node sending a packet to it self. Now adding this packet into the buffer, wait to forwarding
	{
		Buf->addPkt(RealID, hd->GetDestination(),this->NodeID, CurrentTime, hd->GetHops(), hd->GetprevHop(), pkt->GetStartTime());
		Stat->pktGen(RealID, hd->GetSource(), hd->GetDestination(), CurrentTime);
		return;
	}


	/* Check if I am not the next hop */
	if(hd->GetNextHop() != this->NodeID)
	{
		/* Garbage collection */
		if(pkt->AccessPkt() == false)
		{
			pktPool->ErasePacket(PID);
		}

		return;
	}


	/* Check if I am the destination of the packet */
	if(hd->GetDestination() == this->NodeID)
	{
		#ifdef BUBBLERAP_DEBUG
		printf("this packet is finnally delivered \n");
		#endif
		//The above lines enable checking for duplicates 
		if(DM->NoDuplicatesSupport() && DM->isDelivered(RealID))
		{
			printf("Problem: Packet %d has been already delivered!\n",RealID);
			exit(1);
		}
		DM->setAsDelivered(RealID);
		/* Update statistics */
		Stat->pktRec(hd->GetHops(), CurrentTime - pkt->GetStartTime(), pkt, pkt->GetStartTime(), false);


		/* Garbage collection */
		if(pkt->AccessPkt() == false)
		{
			pktPool->ErasePacket(PID);
		}

		return;
	}


	/* I am the next hop */
	if(Buf->PacketExists(RealID))
	{
		printf("[Error]: Node %d received a packet with ID %d from node %d that already exists in its buffer\n", this->NodeID, RealID, hd->GetprevHop());
		exit(EXIT_FAILURE);
	}
	else
	{
		/* Update buffer */
		Buf->addPkt(RealID, hd->GetDestination(),hd->GetSource(),CurrentTime, hd->GetHops(), hd->GetprevHop(), pkt->GetStartTime());


		/* Update statistics */
		Stat->incTimesAsRelayNode(pkt->GetStartTime());
	}


	/* Garbage collection */
	if(pkt->AccessPkt() == false)
	{
		pktPool->ErasePacket(PID);
	}

	return;
}


void BubbleRap::ReceptionReqLCandFS(Header *hd, Packet *pkt, int PID, double CurrentTime)//local Community , familar setting. second third
{
	bool *myLocalCommunity;
	bool **myFamiliarSets;
	Packet *responsePacket;
	Header *responseHeader;


	#ifdef BUBBLERAP_DEBUG
		printf("%f: Node %d received a request packet for its Local Community and its Familiar Sets with ID %d from node %d\n", CurrentTime, this->NodeID, PID, hd->GetprevHop());
	#endif
		
	/* Create a response packet containing my Local Community and my Familiar Sets */
	myLocalCommunity = labeling->cloneLocalCommunity(CurrentTime);// the time parameter is used to updateLCandFS()
	myFamiliarSets = labeling->cloneFamiliarSets(CurrentTime);
	responsePacket = new LCandFS(CurrentTime, myLocalCommunity, myFamiliarSets, 0);
	responseHeader = new BasicHeader(this->NodeID, hd->GetprevHop());//? header contain current node ID and the previous node ID
	responsePacket->setHeader(responseHeader);
	((LCandFS *)responsePacket)->setmaxN(Set->getNN());
	pktPool->AddPacket(responsePacket);


	/* Send the packet with the response */
	Mlayer->SendPkt(CurrentTime, this->NodeID, hd->GetprevHop(), responsePacket->getSize(), responsePacket->getID());

	#ifdef BUBBLERAP_DEBUG
		printf("%f: Node %d sent its Local Community and its Familiar Sets with ID %d to node %d\n", CurrentTime, this->NodeID, responsePacket->getID(), hd->GetprevHop());
	#endif
	
	/* Delete the request packet to free memory */
	pktPool->ErasePacket(PID);

	return;
}


void BubbleRap::ReceptionLCandFS(Header *hd,Packet *pkt,int PID,double CurrentTime)//
{
	int i;
	int *allPackets;
	double myLocalRank;
	double myGlobalRank;
	bool *myLocalCommunity;
	bool *encLocalCommunity;//enc:received package
	bool **encFamiliarSets;
	struct PktDest *mySummaryVector;
	Packet *responsePacket;
	Header *responseHeader;


	#ifdef BUBBLERAP_DEBUG
		printf("%f: Node %d received a packet containing the Local Community and the Familiar Sets with ID %d from node %d\n", CurrentTime, this->NodeID, PID, hd->GetprevHop());
	#endif


	/* Get packet contents */
	encLocalCommunity = ((LCandFS *) pkt)->getLocalCommunity();
	encFamiliarSets = ((LCandFS *) pkt)->getFamiliarSets();
	

	/* Update my bubble */
	//check if the encountered node should be added into my community (have enought CommonMembers)
	labeling->updateBubble(encLocalCommunity, encFamiliarSets, hd->GetprevHop(), CurrentTime);


	/* Create my summary vector */// contain PIDs and DESTs of packets in buffer
// 	allPackets = Buf->getAllPackets();
	allPackets = Buf->getPacketsNotDestinedTo(hd->GetprevHop());//??? every nodes except the previous node. the sturcture of allPackets is like my neighbors[]
	mySummaryVector = (struct PktDest *) malloc(sizeof(struct PktDest) * allPackets[0]);//allPackets[0] contain the number of items in this array. just like my neighbors[];
	for(i = 1; i <= allPackets[0]; i++)
	{
		mySummaryVector[i-1].PID = allPackets[i];
		mySummaryVector[i-1].Dest = Buf->GetPktDestination(allPackets[i]);
	#ifdef BUBBLERAP_DEBUG
	printf("summaryVector pkt, PID: %d, Destination:%d \n",mySummaryVector[i-1].PID,mySummaryVector[i-1].Dest);
	#endif
		
	}


	/* Get my bubble information */
	myLocalCommunity = labeling->cloneLocalCommunity(CurrentTime);
	myLocalRank = ranking->getLocalRank(CurrentTime, myLocalCommunity);
	myGlobalRank = ranking->getGlobalRank(CurrentTime);//all the nodes, include those nodes in MyLocalCommunity
	
	/* Create a response packet containing bubble information and summary vector */
	responsePacket = new BubbleSummary(CurrentTime, allPackets[0], mySummaryVector, myLocalCommunity, myLocalRank, myGlobalRank, 0);
	responseHeader = new BasicHeader(this->NodeID, hd->GetprevHop());
	responsePacket->setHeader(responseHeader);
	pktPool->AddPacket(responsePacket);


	/* Send the packet with the response */
	Mlayer->SendPkt(CurrentTime, this->NodeID, hd->GetprevHop(), responsePacket->getSize(), responsePacket->getID());


	#ifdef BUBBLERAP_DEBUG
		printf("%f: Node %d sent its bubble information and its summary vector with ID %d to node %d\n", CurrentTime, this->NodeID, responsePacket->getID(), hd->GetprevHop());
	#endif
	
	free(allPackets);
	/* Delete the packet to free memory */
	pktPool->ErasePacket(PID);
	return;
}


void BubbleRap::ReceptionBubbleSummary(Header *hd, Packet *pkt, int PID, double CurrentTime)// sixth seventh
{
	int i=0;
	int reqPos=0;
	int encNumPkts=0;
	int *myRequestVector=NULL;
	bool *myRequestMarks=NULL;
	bool encBetterCarrier=true;
	bool IamBetterGlobally=false;
	bool *encLocalCommunity=NULL;
	bool *myLocalCommunity=NULL;
	double encLocalRank=0.0;
	double myLocalRank=0.0;
	double encGlobalRank=0.0;
	double myGlobalRank=0.0;
	struct PktDest *encSummaryVector=NULL;
	Packet *responsePacket=NULL;
	Header *responseHeader=NULL;

	bool *destLocalCommunity=NULL;
	int encNodeID=hd->GetprevHop();

	#ifdef BUBBLERAP_DEBUG
		printf("%f: Node %d received a packet containing bubble information and summary vector with ID %d from node %d\n", CurrentTime, this->NodeID, PID, hd->GetprevHop());
	#endif


	/* Get packet contents */
	encNumPkts = ((BubbleSummary *) pkt)->getNumPackets();
	encSummaryVector = ((BubbleSummary *) pkt)->getSummaryVector();//info of all packets in sender's buf
	encLocalCommunity = ((BubbleSummary *) pkt)->getLocalCommunity();
	encLocalRank = ((BubbleSummary *) pkt)->getLocalRank();
	encGlobalRank = ((BubbleSummary *) pkt)->getGlobalRank();
	#ifdef BUBBLERAP_DEBUG
		printf("lets check the community of node %d \n",encNodeID);
	#endif

	for(int i=0; i<13;i++)
	{
		if(encLocalCommunity[i]&& i!=encNodeID){
	#ifdef BUBBLERAP_DEBUG
		printf("node:%d is in this node's community\n",i);
	#endif
	}
	}

	/* Get my bubble information */
	myLocalCommunity = labeling->cloneLocalCommunity(CurrentTime);
	myLocalRank = ranking->getLocalRank(CurrentTime, myLocalCommunity);
	myGlobalRank = ranking->getGlobalRank(CurrentTime);

	/* Initialize the request vector */
	myRequestVector = (int *) malloc(sizeof(int));
	myRequestMarks = (bool *)malloc(sizeof(bool));
	myRequestVector[0] = 0;// contain the pid of packet[i] if it did not have an encBetterCarrier
	myRequestMarks[0]=true;//indicated if the current node has a higher global ranking than node[i]
	reqPos = 0;
	#ifdef BUBBLERAP_DEBUG
	printf("check the destiantions of pkts which are stored in node:%d \n", encNodeID);
	#endif
	for(i = 0; i < encNumPkts; i++)// Key part 
	{
	#ifdef BUBBLERAP_DEBUG
	printf("the dest_node is: %d \n", encSummaryVector[i].Dest);
	#endif
	}
	/* Bubble Rap Forwarding */
	for(i = 0; i < encNumPkts; i++)// Key part 
	{
		if(Buf->PacketExists(encSummaryVector[i].PID))
		{
	#ifdef BUBBLERAP_DEBUG
			printf("skipped for this packets for it is already in my buffer \n");
	#endif
			continue;
		}

		encBetterCarrier = true;
		IamBetterGlobally = false;


		

/*
		//My HCBF forwarding
		//I-node is going to become a relay for packets from the encountered-node
		int PktID=encSummaryVector[i].PID;
		destLocalCommunity=this->SimGod->getCommunityFromCertainNode(encSummaryVector[i].Dest);
		//set the enc-node as the MaxCBC and MaxNCF nodes
		PacketEntry* pktinfo=this->SimGod->getPacketInfo(encNodeID,PktID);
		if(pktinfo==NULL)
		{
			fprintf(stderr, "Error:this pktinfo is empty\n");
			return;
		}
		//set the MaxCBC/NCF node as the encNode
		if(pktinfo->CBC_node==0){//I think we dont need forward a packet to a node just better the encNode, but worse than the previous node
		//Buf->setMaxCBC(,encNodeID,PktID);
		pktinfo->CBC=this->SimGod->getCBC(destLocalCommunity,encLocalCommunity,CurrentTime);
		pktinfo->CBC_node=encNodeID;
		}
		if(pktinfo->NCF_node==0){
		//Buf->setMaxNCF(,encNodeID,PktID);
		pktinfo->NCF=this->SimGod->getNCF(destLocalCommunity,encNodeID);
		pktinfo->NCF_node=encNodeID;
		}  

		//if I-node and enc-node are in the same community
		if(encLocalCommunity[this->NodeID] && myGlobalRank>encGlobalRank)// current node my just be added into the community of encountered node, So I checked the encLocalCommunity instead of the myLocalCommunity
		{
			#ifdef BUBBLERAP_DEBUG
			printf("I-node and enc-node are in the same community\n");
			#endif
			if(myGlobalRank>pktinfo->UI ||pktinfo->UI==0){
			//Buf->setMaxUI(myGlobalRank,this->NodeID,PktID);//store meID as the MaxUI of encountered node
			pktinfo->UI=myGlobalRank;
			pktinfo->UI_node=this->NodeID;
			}
			//if(this->SimGod->getMaxUI(encNodeID)==0 || myGlobalRank>this->SimGod->getMaxUI(encNodeID))
		}

		// if the Destination and I-node are in the same community
		if(myLocalCommunity[encSummaryVector[i].Dest])
		{
			#ifdef BUBBLERAP_DEBUG
			printf("this packet's dest:%d is in my community\n",encSummaryVector[i].Dest);
			#endif
			if(myLocalRank>=pktinfo->LP ||pktinfo->LP==0){//may need to check the previous maxLP-node is belong to this community too.
			//Buf->setMaxLP(myLocalRank,this->NodeID,PktID);
			pktinfo->LP=myLocalRank;
			pktinfo->LP_node=this->NodeID;
			//printf("the LP_node is:%d \n",pktinfo->LP_node);
			}
			
		}
		
		else if(!encLocalCommunity[this->NodeID])//I-node is not in enc-node's community nor destination's
		{
			int myCBC=this->SimGod->getCBC(destLocalCommunity,myLocalCommunity,CurrentTime);
			#ifdef BUBBLERAP_DEBUG
			printf("myCBC:%d,pktinfo->CBC:%d\n",myCBC,pktinfo->CBC);
			#endif
			if(myCBC > pktinfo->CBC)//if we need to delete this MaxCBC at a certain time ???
			{
				#ifdef BUBBLERAP_DEBUG
				printf("CBC works~, and its value is :%d \n",myCBC);
				#endif
				//Buf->setMaxCBC(myCBC,this->NodeID,PktID);
				pktinfo->CBC=myCBC;
				pktinfo->CBC_node=this->NodeID;
			}
		}
		else//I-node and enc-node are in the same non-destination community. need to check why enc-node is not in the destination community
		{
			
			int myNCF=this->SimGod->getNCF(destLocalCommunity,this->NodeID);
			if(myNCF > pktinfo->NCF)// if we need to delete this MaxNCF at a certain time ???
			{
				#ifdef BUBBLERAP_DEBUG
				printf("I-node and enc-node are in the same non-destination community, so applying NCF\n");
				#endif
				pktinfo->NCF=myNCF;
				pktinfo->NCF_node=this->NodeID;
			}
			
			//this->SimGod->SetMaxNCF(encNodeID,this->NodeID,myNCF);
			//else if(myLocalRank > this->SimGod->getMaxUI(encNodeID))//it should apply UI ???
			//{
			//	pktinfo->LP=myLocalRank;
			//	pktinfo->LP_node=this->NodeID;
			//}
			//this->SimGod->SetMaxLP(encNodeID,this->NodeID,myLocalRank);
			
		}
	 		
		//if MaxUI node is in the dest community
		if(pktinfo->UI_node!=0)
		{
		bool* MaxUICommunity= this->SimGod->getCommunityFromCertainNode(pktinfo->UI_node);
			if(MaxUICommunity[encSummaryVector[i].Dest])
			{
				#ifdef BUBBLERAP_DEBUG
			printf("first MaxUI work~\n");
				#endif
			encBetterCarrier=false;
			IamBetterGlobally=true;
			}
		}
		//if MaxLP node is in the dest community	this->SimGod->getMaxLP_node(encNodeID) fff
		if(pktinfo->LP_node!=0 && encBetterCarrier)
		{
		bool* MaxLPCommunity= this->SimGod->getCommunityFromCertainNode(pktinfo->LP_node);
			if(MaxLPCommunity[encSummaryVector[i].Dest])
			{
				#ifdef BUBBLERAP_DEBUG
			printf("frist MaxLP work~\n");
				#endif
			encBetterCarrier=false;
			}
		}
		//if we have a new maxCBC/NCF node
		if(encBetterCarrier && pktinfo->CBC_node!=encNodeID)//if(pktinfo->CBC_node!=encNodeID)
		{
				#ifdef BUBBLERAP_DEBUG
		printf("maxCBC work~\n");
				#endif
		encBetterCarrier=false;
		}
		if(encBetterCarrier && pktinfo->NCF_node!=encNodeID)//if(pktinfo->NCF_node!=encNodeID)
		{
				#ifdef BUBBLERAP_DEBUG
		printf("maxNCF work~\n");
				#endif
		encBetterCarrier=false;
		}
		if(encBetterCarrier && pktinfo->UI>encGlobalRank)
		{
				#ifdef BUBBLERAP_DEBUG
		printf("last maxUI work~\n");
				#endif
		encBetterCarrier=false;
		IamBetterGlobally=true;
		}
		if(encBetterCarrier && pktinfo->LP>encLocalRank)
		{
				#ifdef BUBBLERAP_DEBUG
		printf("last maxLP work~\n");
				#endif
		encBetterCarrier=false;
		}
*/

		if(myLocalCommunity[encSummaryVector[i].Dest])//if I got connected to a node that is the destination of the packets from sender's buf. which means I may be the relay of that packets
		{
			#ifdef BUBBLERAP_DEBUG
			printf("destination is in my community \n");
			#endif
			if((encLocalCommunity[encSummaryVector[i].Dest]) && (encLocalRank >= myLocalRank))
			{
				encBetterCarrier = true;// I am not the better carrier node, whereas the encounter node (sender) is better
			}
			else if((encLocalCommunity[encSummaryVector[i].Dest]) && (encLocalRank < myLocalRank))
			{
				encBetterCarrier = false;// I am the better carrier node
			}
			else if(!encLocalCommunity[encSummaryVector[i].Dest])
			{
				encBetterCarrier = false;// I am the better carrier node
			}
		}
		else
		{
			#ifdef BUBBLERAP_DEBUG
			printf("destination is not in my community \n");
			#endif
			if(encLocalCommunity[encSummaryVector[i].Dest])
			{
				encBetterCarrier = true;
			}
			else if(encGlobalRank >= myGlobalRank)
			{
				encBetterCarrier = true;
			}
			else if(encGlobalRank < myGlobalRank)//since neither the EncounteredNode or INode have the DestNode in communities, so just check UI of these two nodes
			{
				encBetterCarrier = false;
				IamBetterGlobally = true;
			}
		}
/**/



		if(!encBetterCarrier)// I am the better Carrier, so the sender node should send the relative packets to me
		{
			#ifdef BUBBLERAP_DEBUG
			printf("I_Node:%d am the better Carrier, so the sender node should send the relative packet:%d to me, the destination of this packet is:%d \n",this->NodeID,encSummaryVector[i].PID,encSummaryVector[i].Dest);
			#endif
			/* Add the packet in the request vector */
			reqPos++;

			myRequestVector=(int *) realloc(myRequestVector, (reqPos + 1)*sizeof(int));
			myRequestVector[reqPos] = encSummaryVector[i].PID;
			myRequestVector[0] = reqPos;//?? the first item in myRequestVector is the amount of this whole vector
			
			myRequestMarks=(bool *) realloc(myRequestMarks, (reqPos + 1)*sizeof(bool));//???
			myRequestMarks[reqPos] = IamBetterGlobally;//indicate I am not in the Destination community, but only has greater UI
		}

		else{
		#ifdef BUBBLERAP_DEBUG
		printf("is not a betterRelay\n");
		#endif
		}
	}
	free(myLocalCommunity);

	/* Create a response packet containing the requested packets *///this type of packet is to inform the receiver node to treat me as a relay, and send me some relative packets
	responsePacket = new MarkedRequestPacket(CurrentTime, 0);
	responsePacket->setContents((void *) myRequestVector);
	((MarkedRequestPacket *)responsePacket)->setMarked(myRequestMarks);
	responseHeader = new BasicHeader(this->NodeID, hd->GetprevHop());
	responsePacket->setHeader(responseHeader);
	pktPool->AddPacket(responsePacket);

	/* Send the packet with the response */
	Mlayer->SendPkt(CurrentTime, this->NodeID, hd->GetprevHop(), responsePacket->getSize(), responsePacket->getID());


	#ifdef BUBBLERAP_DEBUG
		printf("%f: Node %d sent its request for packets with ID %d to node %d\n", CurrentTime, this->NodeID, responsePacket->getID(), hd->GetprevHop());//GetprevHop(): destination node???
	#endif
	
	/* Delete the packet to free memory */
	pktPool->ErasePacket(PID);
	return;
}


bool BubbleRap::winsGlobally(int PID, int* Requests, bool* mark)
{
	for(int i = 1; i <= Requests[0]; i++)
	{
		if(Requests[i] == PID)
		{
			return mark[i];
		}
	}
	//We should not reach this point
	printf("(BubbleRap error) %d: The packet %d never requested by node in contact!Exiting..\n",this->NodeID,PID);
	exit(1);
}


void BubbleRap::ReceptionRequestVector(Header *hd, Packet *pkt, int PID, double CurrentTime)
{
	int i=0;
	int *encRequestVector=NULL;
	bool *encBetterGlobally=NULL;


	/* Send the packets in the request vector */
	encRequestVector = (int *) pkt->getContents();
	encBetterGlobally = ((MarkedRequestPacket *)pkt)->getMarked();
	//printf("whats inside the vector[0]:%d \n",encRequestVector[0]);
	//printf("ReceptionRequestVector(): sending packets from my buffer as requestion, for that node is better \n");
	for(i = 1; i <= encRequestVector[0]; i++)
	{
		#ifdef BUBBLERAP_DEBUG
		printf("the receiver node is a better relay \n");
		#endif
		sch->addPacket(encRequestVector[i],NULL);
	}
	//Apply scheduling
	int *outgoing=sch->getOutgoingPackets();
	//Apply congestion control
	outgoing=CC->filterPackets(outgoing);
	//Send packets
	if(outgoing)//fff
	{
		for(int i=1;i<=outgoing[0];i++)
		{
			SendPacket(CurrentTime, outgoing[i], hd->GetprevHop(),1);
			/* Remove the packet from the buffer */
			if(Set->getCopyMode())
			{
				Buf->removePkt(outgoing[i]);
			}
			
			if(!IntraCopyOn && InterCopyOn)
			{
				if(!winsGlobally(outgoing[i],encRequestVector,encBetterGlobally))// if the mark of this PID is false
				{
					Buf->removePkt(outgoing[i]);
				}
			}
		}
		free(outgoing);
	}
	/* Delete the packet to free memory */
	pktPool->ErasePacket(PID);

	return;
}


void BubbleRap::SendPacket(double STime, int pktID,int nHop,int RepValue)// what is RepValue. it always 1
{
	int CurrentN;
	Packet *p;
	Packet *newPkt;

	
	if((p = pktPool->GetPacket(pktID)) == NULL)
	{
		printf("Error: Packet %d doesn't exist in Packet Pool! Aborting...\n", pktID);
		exit(1);
	}


	/* Duplicate the packet */
	newPkt = p->Duplicate(Buf->GetHops(pktID));// int hops ??? Duplicate
	newPkt->getHeader()->SetNextHop(nHop);
	newPkt->getHeader()->SetprevHop(this->NodeID);
	newPkt->getHeader()->SetRep(RepValue);
	pktPool->AddPacket(newPkt);
	//printf("node %d sending a packet which is pktID to node: %d directly\n",this->NodeID,nHop);

	/* Inform the current neighbors about the new packet */
	CurrentN = Mlayer->BroadcastPkt(STime, this->NodeID, newPkt->getSize(), newPkt->getID());//CurrentN: amount of current neighours
	

	/* Garbage collection */
	if(CurrentN > 0)
	{
		/* Set access attribute to safely delete the packet later */
		newPkt->SetRecipients(CurrentN);


		/* Update statistics */
		if(newPkt->getHeader()->GetDestination() == nHop)
		{
			Stat->incHandovers(Buf->GetPktCreationTime(pktID));
		}

		Stat->incForwards(pktID, Buf->GetPktCreationTime(pktID));
	}
	else
	{
		/* Cancel broadcast and delete packet (there are no neighbors) */
		pktPool->ErasePacket(newPkt->getID());
	}

	return;
}



bool *BubbleRap::getCommunity(double CurrentTime)
{
	return labeling->cloneLocalCommunity(CurrentTime);
}

int BubbleRap::getNCFHelper(bool *destCommunity)
{
	return ranking->getContactsBetweenNodeCommunities(destCommunity);

}

int BubbleRap::getContactsNum(bool *destCommunity)
{
	return ranking->getContactsBetweenNodeCommunities(destCommunity);

}
