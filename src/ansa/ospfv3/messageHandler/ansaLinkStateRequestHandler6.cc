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

#include "ansaOspfNeighbor6.h"
#include "ansaOspfRouter6.h"

#include "ansaLinkStateRequestHandler6.h"

AnsaOspf6::LinkStateRequestHandler::LinkStateRequestHandler(AnsaOspf6::Router* containingRouter) :
   AnsaOspf6::IMessageHandler(containingRouter) {
}

void AnsaOspf6::LinkStateRequestHandler::ProcessPacket(OspfPacket6* packet,
      AnsaOspf6::Interface* intf, AnsaOspf6::Neighbor* neighbor) {

   router->GetMessageHandler()->PrintEvent("Link State Request packet received", intf, neighbor);

   AnsaOspf6::Neighbor::NeighborStateType neighborState = neighbor->GetState();

   if ((neighborState == AnsaOspf6::Neighbor::ExchangeState) || (neighborState
         == AnsaOspf6::Neighbor::LoadingState) || (neighborState == AnsaOspf6::Neighbor::FullState)){
      OspfLinkStateRequestPacket6* lsRequestPacket = check_and_cast<OspfLinkStateRequestPacket6*> (packet);

      unsigned long requestCount = lsRequestPacket->getRequestsArraySize();
      bool error = false;
      std::vector<OspfLsa6*> lsas;

      EV<< "  Processing packet contents:\n";

      for (unsigned long i = 0; i < requestCount; i++){
         OspfLsaRequest6& request = lsRequestPacket->getRequests(i);
         AnsaOspf6::LsaKeyType6 lsaKey;
         char addressString[16];

         EV << "    LSARequest: type="
         << request.lsType
         << ", LSID="
         << request.linkStateID
         << ", advertisingRouter="
         << IPAddress(request.advertisingRouter) << endl;

         lsaKey.linkStateID = request.linkStateID;
         lsaKey.advertisingRouter = request.advertisingRouter;

         OspfLsa6* lsaInDatabase = router->FindLSA(static_cast<LsaType6> (request.lsType), lsaKey, intf->GetArea()->GetAreaID());

         if (lsaInDatabase != NULL){
            lsas.push_back(lsaInDatabase);
         }else{
            error = true;
            neighbor->ProcessEvent(AnsaOspf6::Neighbor::BadLinkStateRequest);
            break;
         }
      }

      if (!error){
         int updatesCount = lsas.size();
         int ttl          = (intf->GetType() == AnsaOspf6::Interface::Virtual) ? VIRTUAL_LINK_TTL : 1;
         AnsaOspf6::MessageHandler* messageHandler = router->GetMessageHandler();

         for (int j = 0; j < updatesCount; j++) {
            OspfLinkStateUpdatePacket6* updatePacket = intf->CreateUpdatePacket(lsas[j]);
            if (updatePacket != NULL) {
               if (intf->GetType() == AnsaOspf6::Interface::Broadcast) {
                  if ((intf->GetState() == AnsaOspf6::Interface::DesignatedRouterState) ||
                        (intf->GetState() == AnsaOspf6::Interface::BackupState) ||
                        (intf->GetDesignatedRouter() == AnsaOspf6::NullDesignatedRouterID))
                  {
                     messageHandler->SendPacket(updatePacket, AnsaOspf6::AllSPFRouters, intf->GetIfIndex(), ttl);
                  }else{
                     messageHandler->SendPacket(updatePacket, AnsaOspf6::AllDRouters, intf->GetIfIndex(), ttl);
                  }
               }else{
                  if (intf->GetType() == AnsaOspf6::Interface::PointToPoint) {
                     messageHandler->SendPacket(updatePacket, AnsaOspf6::AllSPFRouters, intf->GetIfIndex(), ttl);
                  }else{
                     messageHandler->SendPacket(updatePacket, neighbor->GetAddress(), intf->GetIfIndex(), ttl);
                  }
               }

            }
         }
      }
   }
}
