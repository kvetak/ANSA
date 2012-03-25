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

enum PIMTimerType
{
    // global timers
   HelloTimer = 1;
   TriggeredHelloDelay = 2;
   
   // timers for each interface and each neighbor (N,I)
   NeighborLivnessTimer = 3;
   
   // timers for each interface and each source-group pair (S,G,I)
   AssertTimer = 4;
   PruneTimer = 5;
   PrunePendingTimer = 6;
   
   // timers for each source-group pair (S,G)
   GrafRetryTimer = 7;
   UpstreamOverrideTimer = 8;
   PruneLimitTimer = 9;
   SourceActiveTimer = 10;
   StateRefreshTimer = 11;
}


// Represents an PIM Timer
packet PIMTimer extends cMessage
{
      char 		timerKind enum(PIMTimerType);
}
