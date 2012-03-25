//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// Author: Veronika Rybova

cplusplus {{
#include "IPAddress.h"
}}

//struct IPAddress;
class noncobject IPAddress;

enum PIMPacketType 
{
   Hello = 0;
   Register = 1;
   RegisterStop = 2;
   JoinPrune = 3;
   Bootstrap = 4;
   Assert = 5;
   Graft = 6;
   GraftAck = 7;
   CandidateRPAdvertisement = 8;
   StateRefresh = 9;
}

// Hello record in format TLV
struct HelloEntry
{
	short Type;
	short Value;	   
}

// Record for Join/Prune message
// in structure it is not possible to use dynamic arrays, iit is needed to use class
class MulticastGroup
{
	IPAddress 	groupAddress;
	IPAddress 	joinedSourceAddress[]; 
	IPAddress 	prunedSourceAddress[]; 
}



///////////////////////////////////////////////////////////////////////////////////////////////
// Header
packet PIMPacket 
{
   short 		version = 2;
   short 		type enum(PIMPacketType);							
}

// Hello message
// DM, SM
packet PIMHello extends PIMPacket
{
	short 			type enum(PIMPacketType) = Hello;
	HelloEntry		helloContent[];	   
}

// Join/Prune message
// DM, SM
packet PIMJoinPrune extends PIMPacket
{
	short 			type enum(PIMPacketType) = JoinPrune;
	IPAddress		upstreamNeighborAddress;	
	int				holdTime;
	//int				numGroups;
	MulticastGroup	multicastGroups[];   
}

// Assert message
// DM, SM
packet PIMAssert extends PIMPacket
{
	short 			type enum(PIMPacketType) = Assert;
	IPAddress		groupAddress;
	IPAddress		sourceAddress;	
	bool			R;
	int				metric;   
	int				metricPreference;   
}

// Graft message
// DM
packet PIMGraft extends PIMJoinPrune
{
	short 			type enum(PIMPacketType) = Graft;	
	int				holdTime = 0; 
}

// GraftAck message
// DM
packet PIMGraftAck extends PIMJoinPrune
{
	short 			type enum(PIMPacketType) = GraftAck;	
	int				holdTime = 0; 
}

// State Refresh message
// DM
packet PIMStateRefresh extends PIMPacket
{
	short 			type enum(PIMPacketType) = StateRefresh;	
	IPAddress		groupAddress;
	IPAddress		sourceAddress;	
	IPAddress		originatorAddress;	
	int				metric;   
	int				metricPreference;  
	short			maskLen;
	short			ttl;
	bool			P;
	int				interval; 
}
// Register message
// SM
packet PIMRegister extends PIMPacket
{
	short 			type enum(PIMPacketType) = Register;	
	bool			B;
	bool			N;
	// TODO: multicast data
}

// Register-Stop message
// SM
packet PIMRegisterStop extends PIMPacket
{
	short 			type enum(PIMPacketType) = RegisterStop;	
	IPAddress		groupAddress;
	IPAddress		sourceAddress;
}