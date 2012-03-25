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

#ifndef ANSALINKSTATEUPDATEHANDLER6_H_
#define ANSALINKSTATEUPDATEHANDLER6_H_

#include "ansaOspfCommon6.h"
#include "ansaIMessageHandler6.h"

namespace AnsaOspf6 {

class LinkStateUpdateHandler: public IMessageHandler {
private:
   struct AcknowledgementFlags {
         bool floodedBackOut;
         bool lsaIsNewer;
         bool lsaIsDuplicate;
         bool impliedAcknowledgement;
         bool lsaReachedMaxAge;
         bool noLSAInstanceInDatabase;
         bool anyNeighborInExchangeOrLoadingState;
   };

private:
   void AcknowledgeLSA(OspfLsaHeader6& lsaHeader, Interface* intf, AcknowledgementFlags acknowledgementFlags, RouterID lsaSource);

public:
   LinkStateUpdateHandler(Router* containingRouter);

   void ProcessPacket(OspfPacket6* packet, Interface* intf, Neighbor* neighbor);
};
}

#endif /* ANSALINKSTATEUPDATEHANDLER6_H_ */
