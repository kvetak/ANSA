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


cplusplus {{
#include "IPAddress.h"
}};

//struct IPAddress;
class noncobject IPAddress;
//class pimSplitter extends void;

enum PIMTimerType
{
    // global timers
   HelloTimer = 1;
   TriggeredHelloDelay = 2;
   
   // timers for each interface and each neighbor (N,I)
   NeighborLivenessTimer = 3;
   
   // timers for each interface and each source-group pair (S,G,I)
   AssertTimer = 4;
   PruneTimer = 5;
   PrunePendingTimer = 6;
   
   // timers for each source-group pair (S,G)
   GraftRetryTimer = 7;
   UpstreamOverrideTimer = 8;
   PruneLimitTimer = 9;
   SourceActiveTimer = 10;
   StateRefreshTimer = 11;
}


// Represents an PIM Timer
packet PIMTimer extends cMessage
{
      char 				timerKind enum(PIMTimerType);
}

// Represents an PIM Neighbor Livness Timer
packet PIMnlt extends PIMTimer
{
    timerKind = NeighborLivenessTimer;
    int 				ntId;          	// Identification of entry in Neigbor table
}

// represents PIM Prune Timer
packet PIMpt extends PIMTimer
{
    timerKind = PruneTimer;
    IPAddress 			source;          	// Source of multicast
    IPAddress			group;				// Multicast group address
    int					intId;				// ID of interface
}

// represents PIM Graft Retry Timer
packet PIMgrt extends PIMTimer
{
    timerKind = GraftRetryTimer;
    IPAddress 			source;          	// Source of multicast
    IPAddress			group;				// Multicast group address
}

// represents PIM Source Active Timer
packet PIMsat extends PIMTimer
{
    timerKind = SourceActiveTimer;
    IPAddress 			source;          	// Source of multicast
    IPAddress			group;				// Multicast group address
}

// represents PIM State Refresh Timer
packet PIMsrt extends PIMTimer
{
    timerKind = StateRefreshTimer;
    IPAddress 			source;          	// Source of multicast
    IPAddress			group;				// Multicast group address
}
