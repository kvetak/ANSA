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

#ifndef ANSAOSPFINTERFACE6_H_
#define ANSAOSPFINTERFACE6_H_

#include "IPv6Address.h"

#include "ansaOspfCommon6.h"
#include "ansaOspfNeighbor6.h"
#include "ansaOspfTimer6_m.h"
#include "ansaOspfPacket6_m.h"


namespace AnsaOspf6 {

class InterfaceState;
class Area;

class Interface {
public:
   enum InterfaceEventType {
      InterfaceUp          = 0,
      HelloTimer           = 1,
      WaitTimer            = 2,
      AcknowledgementTimer = 3,
      BackupSeen           = 4,
      NeighborChange       = 5,
      LoopIndication       = 6,
      UnloopIndication     = 7,
      InterfaceDown        = 8
   };

   enum OspfInterfaceType {
      UnknownType       = 0,
      PointToPoint      = 1,
      Broadcast         = 2,
      NBMA              = 3,
      PointToMultiPoint = 4,
      Virtual           = 5
   };

   enum InterfaceStateType {
      DownState                = 0,
      LoopbackState            = 1,
      WaitingState             = 2,
      PointToPointState        = 3,
      NotDesignatedRouterState = 4,
      BackupState              = 5,
      DesignatedRouterState    = 6
   };

private:
   OspfInterfaceType                                                   interfaceType;
   InterfaceState*                                                     state;
   InterfaceState*                                                     previousState;
   IfaceID                                                             ifIndex;
   std::string                                                         ifName;
   InstanceID                                                          instanceID;
   unsigned short                                                      mtu;
   IPv6Address                                                         interfaceAddress;
   std::map<IPv6AddressPrefix, bool, IPv6AddressPrefix_Less>           advertiseAddressPrefixes;
   std::vector<IPv6AddressPrefix>                                      interfaceAddressPrefixes;
   std::map<LinkStateID, LinkLsa*>                                     linkLSAsByID;
   std::vector<LinkLsa*>                                               linkLSAs;
   AreaID                                                              areaID;
   AreaID                                                              transitAreaID;
   short                                                               helloInterval;
   short                                                               pollInterval;
   short                                                               routerDeadInterval;
   short                                                               interfaceTransmissionDelay;
   unsigned char                                                       routerPriority;
   OspfTimer6*                                                         helloTimer;
   OspfTimer6*                                                         waitTimer;
   OspfTimer6*                                                         acknowledgementTimer;
   std::map<RouterID, Neighbor*>                                       neighboringRoutersByID;
   std::map<IPv6Address, Neighbor*, IPv6Address_Less>                  neighboringRoutersByAddress;
   std::vector<Neighbor*>                                              neighboringRouters;
   std::map<IPv6Address, std::list<OspfLsaHeader6>, IPv6Address_Less>  delayedAcknowledgements;
   DesignatedRouterID                                                  designatedRouter;
   DesignatedRouterID                                                  backupDesignatedRouter;
   Metric                                                              interfaceOutputCost;
   short                                                               retransmissionInterval;
   short                                                               acknowledgementDelay;

   Area*                                                               parentArea;

   bool                                                                isGoingDown;

private:
   friend class InterfaceState;
   void ChangeState(InterfaceState* newState, InterfaceState* currentState);

public:
   Interface(OspfInterfaceType ifType = UnknownType);
   virtual ~Interface(void);

   void                ProcessEvent                        (InterfaceEventType event);
   void                Reset                               (void);
   void                SendHelloPacket                     (IPv6Address destination, short ttl = 1);
   void                SendLSAcknowledgement               (OspfLsaHeader6* lsaHeader, IPv6Address destination);
   Neighbor*           GetNeighborByID                     (RouterID neighborID);
   Neighbor*           GetNeighborByAddress                (IPv6Address address);
   void                AddNeighbor                         (Neighbor* neighbor);
   InterfaceStateType  GetState                            (void) const;
   static const char*  GetStateString                      (InterfaceStateType stateType);
   static const char*  GetTypeString                       (OspfInterfaceType type);
   bool                HasAnyNeighborInStates              (int states) const;
   void                RemoveFromAllRetransmissionLists    (LsaKeyType6 lsaKey);
   bool                IsOnAnyRetransmissionList           (LsaKeyType6 lsaKey) const;
   bool                FloodLSA                            (OspfLsa6* lsa, Interface* intf = NULL, Neighbor* neighbor = NULL);
   void                AddDelayedAcknowledgement           (OspfLsaHeader6& lsaHeader);
   void                SendDelayedAcknowledgements         (void);
   void                AgeTransmittedLSALists              (void);

   OspfLinkStateUpdatePacket6*  CreateUpdatePacket          (OspfLsa6* lsa);

   void                    SetType                         (OspfInterfaceType ifType)  { interfaceType = ifType; }
   OspfInterfaceType       GetType                         (void) const                { return interfaceType; }
   void                    SetIfIndex                      (IfaceID id);
   unsigned char           GetIfIndex                      (void) const                { return ifIndex; }
   void                    SetIfName                       (const char *name)          { ifName = name; };
   const char *            GetIfName                       (void) const                { return ifName.c_str(); }
   void                    SetInstanceID                   (InstanceID id)             { instanceID = id; }
   InstanceID              GetInstanceID                   (void) const                { return instanceID; }
   void                    SetMtu                          (unsigned short ifMTU)      { mtu = ifMTU; }
   unsigned short          GetMtu                          (void) const                { return mtu; }
   void                    SetAreaID                       (AreaID id)                 { areaID = id; }
   AreaID                  GetAreaID                       (void) const                { return areaID; }
   void                    SetTransitAreaID                (AreaID id)                 { transitAreaID = id; }
   AreaID                  GetTransitAreaID                (void) const                { return transitAreaID; }
   void                    SetOutputCost                   (Metric cost)               { interfaceOutputCost = cost; }
   Metric                  GetOutputCost                   (void) const                { return interfaceOutputCost; }
   void                    SetRetransmissionInterval       (short interval)            { retransmissionInterval = interval; }
   short                   GetRetransmissionInterval       (void) const                { return retransmissionInterval; }
   void                    SetTransmissionDelay            (short delay)               { interfaceTransmissionDelay = delay; }
   short                   GetTransmissionDelay            (void) const                { return interfaceTransmissionDelay; }
   void                    SetAcknowledgementDelay         (short delay)               { acknowledgementDelay = delay; }
   short                   GetAcknowledgementDelay         (void) const                { return acknowledgementDelay; }
   void                    SetRouterPriority               (unsigned char priority)    { routerPriority = priority; }
   unsigned char           GetRouterPriority               (void) const                { return routerPriority; }
   void                    SetHelloInterval                (short interval)            { helloInterval = interval; }
   short                   GetHelloInterval                (void) const                { return helloInterval; }
   void                    SetPollInterval                 (short interval)            { pollInterval = interval; }
   short                   GetPollInterval                 (void) const                { return pollInterval; }
   void                    SetRouterDeadInterval           (short interval)            { routerDeadInterval = interval; }
   short                   GetRouterDeadInterval           (void) const                { return routerDeadInterval; }

   void                    SetInterfaceAddress             (IPv6Address addr)          { interfaceAddress = addr; }
   IPv6Address             GetInterfaceAddress             (void) const                { return interfaceAddress; }
   void                    AddAddressPrefix                (IPv6AddressPrefix prefix, bool advertise)      { interfaceAddressPrefixes.push_back(prefix); advertiseAddressPrefixes[prefix] = advertise; }
   unsigned int            GetAddressPrefixCount           (void) const                                    { return interfaceAddressPrefixes.size(); }
   IPv6AddressPrefix       GetAddressPrefix                (unsigned int index) const                      { return interfaceAddressPrefixes[index]; }
   unsigned long           GetLinkLSACount                 (void) const               { return linkLSAs.size(); }
   LinkLsa*                GetLinkLSA                      (unsigned long i)          { return linkLSAs[i]; }
   const LinkLsa*          GetLinkLSA                      (unsigned long i) const    { return linkLSAs[i]; }

   OspfTimer6*             GetHelloTimer                   (void)                      { return helloTimer; }
   OspfTimer6*             GetWaitTimer                    (void)                      { return waitTimer; }
   OspfTimer6*             GetAcknowledgementTimer         (void)                      { return acknowledgementTimer; }
   DesignatedRouterID      GetDesignatedRouter             (void) const                { return designatedRouter; }
   DesignatedRouterID      GetBackupDesignatedRouter       (void) const                { return backupDesignatedRouter; }
   unsigned long           GetNeighborCount                (void) const                { return neighboringRouters.size(); }
   Neighbor*               GetNeighbor                     (unsigned long i)           { return neighboringRouters[i]; }
   const Neighbor*         GetNeighbor                     (unsigned long i) const     { return neighboringRouters[i]; }

   void                    SetArea                         (Area* area)                { parentArea = area; }
   Area*                   GetArea                         (void)                      { return parentArea; }
   const Area*             GetArea                         (void) const                { return parentArea; }

   bool                    IsGoingDown                     (void)                      { return isGoingDown; }
   void                    SetIsGoingDown                  (bool b)                    { isGoingDown = b; }
};

}

#endif /* ANSAOSPFINTERFACE6_H_ */
