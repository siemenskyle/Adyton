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
 *  Written by Nikolaos Papanikos.
 */

#ifndef FLOODING_H
	#define FLOODING_H
	#include "Flooding.h"
#endif

// +----------------+
// | Protocol steps |
// +----------------+----------------------------------------------------------+
// | Node (a) encounters node (b)                                              |
// |===========================================================================|
// | (a): [method: Contact()]                                                  |
// |      - Searches inside its buffer for all packets                       |
// | (a)--------------data packet 1 destined to b--------------------->(b)     |
// | (a)--------------data packet 2 destined to b--------------------->(b)     |
// |                             ...                                           |
// | (a)--------------data packet n destined to b--------------------->(b)     |
// |===========================================================================|
// | (b): [method: ReceptionData()]                                            |
// |     - Receives the transmitted packets one by one.                        |
// +---------------------------------------------------------------------------+


Flooding::Flooding(PacketPool* PP, MAC* mc, PacketBuffer* Bf, int NID, Statistics* St, Settings *S, God *G): Routing(PP, mc, Bf, NID, St, S, G)
{
	return;
}


Flooding::~Flooding()
{
	return;
}

void Flooding::NewContact(double CTime, int NID)
{
	Contact(CTime, NID);
	return;
}



void Flooding::Contact(double CTime, int NID)
{
	int i;
	int *pkts;
	/* Send all packets that have NID as destination */
	//pkts = Buf->getPackets(NID);

//get all the packets in the buffer
pkts=Buf->getAllPackets();
/*
printf("all the packets which are stored in the buffer,these are \n");
for(i = 1; i <= pkts[0]; i++)
	{
		printf("packet ID is: %d \n", pkts[i]);
	}
*/
//the default scheduling is FIFO
	for(i = 1; i <= pkts[0]; i++)
	{
		//Add packets to scheduler
		sch->addPacket(pkts[i],NULL);
	}
	free(pkts);
	//Apply scheduling
	int *outgoing=sch->getOutgoingPackets();
	//Apply congestion control - not needed for direct routing OR flooding routing
	//outgoing=CC->filterPackets(outgoing);
	//Send Packets
	if(outgoing)
	{
		for(int i=1;i<=outgoing[0];i++)
		{
			SendPacket(CTime,outgoing[i],NID,1);
			/* Remove the packet from the buffer */
			//Buf->removePkt(outgoing[i]);
		}
		free(outgoing);
	}
	return;
}


void Flooding::ContactRemoved(double CTime, int NID)
{
	return;
}


void Flooding::recv(double rTime, int pktID)
{
//printf("this packet's ID is : %d \n",pktID);
	int PacketID;
	Packet *p;
	Header *h;
	if((p = pktPool->GetPacket(pktID)) == NULL)
	{
		printf("Error: Packet %d doesn't exist in Packet Pool!\nAborting...\n",pktID);
		exit(1);
	}
	h = p->getHeader();
	if(h->IsDuplicate() == true)
	{
		PacketID = h->GetOriginal();
	}
	else
	{
		PacketID = pktID;
	}
	/* Sanity check */
	if(rTime < 0 || p->GetStartTime() < 0)
	{
		printf("%f: Node %d received new packet with ID %d and type %d from %d\n", rTime, this->NodeID, p->getID(), p->getType(), h->GetprevHop());
		exit(1);
	}
	/* Identify the type of the packet */
	switch(p->getType())
	{
		case DATA_PACKET:
		{
			ReceptionData(h, p, pktID, rTime, PacketID);
			break;
		}
		default:
		{
			printf("Error: Unknown packet type! Flooding is not using this type of packet!\nAborting...\n");
			exit(1);
		}
	}
	return;
}


void Flooding::ReceptionData(Header* hd, Packet* pkt, int PID, double CurrentTime, int RealID)
{
	#ifdef DIRECT_DEBUG
		printf("%f: Node %d received new data packet with ID %d from %d\n", CurrentTime, this->NodeID, RealID, hd->GetprevHop());
	#endif
	/* Check if I am the packet creator (packet comes from the application layer) */
	if((hd->GetSource() == this->NodeID) && (hd->GetNextHop() == -1))
	{
		/* Update buffer */
		Buf->addPkt(RealID, hd->GetDestination(),hd->GetSource(),CurrentTime, hd->GetHops(), hd->GetprevHop(), pkt->GetStartTime());
		Stat->pktGen(RealID, hd->GetSource(), hd->GetDestination(), CurrentTime);
		return;
	}
	/* Check if I am not the next hop */
	if(hd->GetNextHop() != this->NodeID)
	{
		/* Garbage collection */
		if(pkt->AccessPkt() == false)//if there is no duplicated packets in packets' buffer
		{
			pktPool->ErasePacket(PID);
		}

		return;
	}
	/* Check if I am the destination of the packet */
	if(hd->GetDestination() == this->NodeID)
	{
		
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
		//printf("[Error]: Node %d received a packet with ID %d from node %d that already exists in its buffer\n", this->NodeID, RealID, hd->GetprevHop());
		//exit(EXIT_FAILURE);
	}
	else
	{
		/* Update buffer */
		Buf->addPkt(RealID, hd->GetDestination(), CurrentTime,hd->GetSource(),hd->GetHops(), hd->GetprevHop(), pkt->GetStartTime());
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


void Flooding::SendPacket(double STime, int pktID, int nHop, int RepValue)
{
	int CurrentN;
	Packet *p;
	Packet *newPkt;
	if((p = pktPool->GetPacket(pktID)) == NULL)
	{
		printf("Error: Packet %d doesn't exist in Packet Pool!\nAborting...\n", pktID);
		exit(1);
	}
	/* Duplicate the packet */
	newPkt = p->Duplicate(Buf->GetHops(pktID));
	newPkt->getHeader()->SetNextHop(nHop);
	newPkt->getHeader()->SetprevHop(this->NodeID);
	newPkt->getHeader()->SetRep(RepValue);
	pktPool->AddPacket(newPkt);
	/* Inform the current neighbors about the new packet */
	//printf("the original pkt_ID: %d, the duplicated pkt_ID: %d \n", pktID, newPkt->getID());
	CurrentN = Mlayer->BroadcastPkt(STime, this->NodeID, newPkt->getSize(), newPkt->getID());
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
		//printf("Error: there are no neighbors\n");
		pktPool->ErasePacket(newPkt->getID());
	}
	return;
}
