//
// Marek Cerny, 2MSK
// FIT VUT 2011
//
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

#include "ansaMessageHandler6.h"
#include "ansaOspfInterface6.h"
#include "ansaOspfRouting6.h"

#include "ansaOspfTimer6_m.h"



Define_Module(AnsaOspfRouting6);


AnsaOspfRouting6::AnsaOspfRouting6(){
   ospfEnabled = false;
}

AnsaOspfRouting6::~AnsaOspfRouting6(void){
   if (ospfEnabled){
      std::map<AnsaOspf6::ProcessID, AnsaOspf6::Router *>::iterator it;
      for (it = routers.begin(); it != routers.end(); it++){
         delete it->second;
      }
      routers.clear();
   }
}

void AnsaOspfRouting6::initialize(int stage){

   // stage 0 - register for receiving notifications
   if (stage == 0){

      nb = NotificationBoardAccess().get();
      if (nb == NULL){
         throw cRuntimeError("NotificationBoard not found");
      }

      nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);


   // stage 3 - load config from XML and create routers/areas/interfaces
   }else if (stage == 3){

      ift = AnsaInterfaceTableAccess().get();
      if (ift == NULL){
         throw cRuntimeError("AnsaInterfaceTable not found");
      }

      const char *deviceType = par("deviceType");
      const char *deviceId = par("deviceId");
      const char *configFile = par("configFile");

      cXMLElement *device = xmlParser::GetDevice(deviceType, deviceId, configFile);
      if (device == NULL){
         ev << "OSPFv3 is not enabled on this device (" << deviceType << " id=" << deviceId << ")" << endl;
         return;
      }


      cXMLElement *process = xmlParser::GetOspfProcess6(NULL, device);
      if (process == NULL){
         ev << "OSPFv3 is not enabled on this device (" << deviceType << " id=" << deviceId << ")" << endl;
         return;
      }
      loadOspfRouting(process);


      iface2Routers.resize(ift->getNumInterfaces());
      for (int i = 0; i < ift->getNumInterfaces(); i++){
         iface2Routers[i] = NULL;
      }

      cXMLElement *iface = xmlParser::GetInterface(NULL, device);
      if (iface == NULL && ospfEnabled){
         ev << "OSPFv3 enabled but no interface configuration found? (" << deviceType << " id=" << deviceId << ")" << endl;
      }
      loadInterfaceConfig(iface);

      addWatches();


   // stage 4 - start OSPF processes
   }else if (stage == 4){

      if (ospfEnabled){

         InterfaceEntry *firstIface = ift->getFirstInterface();
         if (firstIface == NULL){
            return;
         }
         int firstIfaceId = firstIface->getInterfaceId();

         // for each interface in interface list
         for (int i = 0; i < iface2Routers.size(); i++){

            Router *router = iface2Routers[i];
            if (router == NULL){
               continue;
            }

            Interface *iface = router->GetNonVirtualInterface(i+firstIfaceId);
            if (iface == NULL){
               continue;
            }

            // trigger startup interface event
            iface->ProcessEvent(AnsaOspf6::Interface::InterfaceUp);
         }
      }
   }
}


void AnsaOspfRouting6::handleMessage(cMessage *msg){

   // this is regular (OSPF?) message
   if (!msg->isSelfMessage()){

      if (!ospfEnabled){
         ev << "OSPFv3 is not enabled on this device" << endl;
         delete msg;
         return;
      }

      // get control info so we can find out from which interface the message arrived
      check_and_cast<OspfPacket6*> (msg);
      IPv6ControlInfo *info6 = (IPv6ControlInfo *) msg->getControlInfo();

      // get router by interface ID
      AnsaOspf6::Router *ospfRouter = getOspfRouter(info6->getInterfaceId());

      // retransmit the message
      if (ospfRouter == NULL){
         ev << "OSPFv3 routing is not enabled on this interface (" << info6->getInterfaceId() << ")" << endl;
         delete msg;
      }else{
         ev << "OSPFv3 module received an OSPF packet! " << endl;
         ospfRouter->GetMessageHandler()->MessageReceived(msg);
      }

   // this is one of the timer-messages
   }else{

      // we need to find out which object originated the timer
      // and get it's parent Router object
      OspfTimer6* timer = check_and_cast<OspfTimer6 *> (msg);
      AnsaOspf6::Router *router = NULL;
      AnsaOspf6::Area *area = NULL;
      AnsaOspf6::Interface *interface = NULL;
      AnsaOspf6::Neighbor *neighbor = NULL;

      switch(timer->getTimerKind()){
         // those timers are send by Neighbor objects
         case NeighborInactivityTimer:
         case NeighborPollTimer:
         case NeighborDDRetransmissionTimer:
         case NeighborUpdateRetransmissionTimer:
         case NeighborRequestRetransmissionTimer:
            neighbor = (AnsaOspf6::Neighbor *) msg->getContextPointer();
            interface = neighbor->GetInterface();
            area = interface->GetArea();
            router = area->GetRouter();

         // those timers are send by Interface objects
         case InterfaceHelloTimer:
         case InterfaceWaitTimer:
         case InterfaceAcknowledgementTimer:
            interface = (AnsaOspf6::Interface *) msg->getContextPointer();
            area = interface->GetArea();
            router = area->GetRouter();

         // this timer is send directly by Router objects
         case DatabaseAgeTimer:
            router = (AnsaOspf6::Router *) msg->getContextPointer();

         default:
            break;
      }

      // retransmit the message
      if (router == NULL){
         ev << "OSPFv3 module received unknown timer self-message" << endl;
         delete msg;
      }else{
         ev << "OSPFv3 module received known timer self-message " << endl;
         router->GetMessageHandler()->MessageReceived(msg);
      }
   }
}



/*
 * Called by the NotificationBoard whenever a change of a category
 * occurs to which this client has subscribed.
 */
void AnsaOspfRouting6::receiveChangeNotification(int category, const cPolymorphic *details){

   if (!ospfEnabled){
      return;
   }

   if (simulation.getContextType()==CTX_INITIALIZE){
      return;  // ignore notifications during initialize
   }

   Enter_Method_Silent();
   printNotificationBanner(category, details);

   if (category==NF_INTERFACE_STATE_CHANGED){ // change state of notified interface

      InterfaceEntry *entry = check_and_cast<InterfaceEntry*>(details);

      AnsaOspf6::Router *ospfRouter = getOspfRouter(entry->getInterfaceId());
      if (ospfRouter == NULL){
         return;
      }

      AnsaOspf6::Interface *intf = ospfRouter->GetNonVirtualInterface(entry->getInterfaceId());

      if(intf != NULL){
         EV << "Changing state of interface in OSPFv3\n";
         if (entry->isDown()){
            intf->ProcessEvent(AnsaOspf6::Interface::InterfaceDown);

         }else{
            intf->ProcessEvent(AnsaOspf6::Interface::InterfaceUp);
         }
      }else{
         EV << "Not changing state of interface in OSPFv3 :(" << endl;
      }
   }
}


void AnsaOspfRouting6::addWatches(){

   if (ospfEnabled){

      WATCH_PTRMAP(routers);

      for (int i = 0; i < ift->getNumInterfaces(); i++){
         InterfaceEntry *ie = ift->getInterface(i);
         std::string ifaceName = ie->getName();
         std::stringstream ifaceOspfProcess;

         if (iface2Routers[i] == NULL){
            ifaceOspfProcess << "OSPFv3 not running";
         }else{
            ifaceOspfProcess << "OSPFv3 process #";
            ifaceOspfProcess << iface2Routers[i]->GetProcessID();
            ifaceOspfProcess << ", router-ID: ";
            ifaceOspfProcess << IPAddress(iface2Routers[i]->GetRouterID());
         }
         interfaces[ifaceName] = ifaceOspfProcess.str();
      }

      WATCH_MAP(interfaces);
   }
}

AnsaOspf6::Router * AnsaOspfRouting6::getOspfRouter(int interfaceId){
   InterfaceEntry *firstIface = ift->getFirstInterface();
   if (firstIface == NULL){
      return NULL;
   }

   int dest = interfaceId - firstIface->getInterfaceId();
   return iface2Routers[dest];
}


void AnsaOspfRouting6::loadOspfRouting(cXMLElement *process){

   while(process != NULL){

      int pid;
      if (!xmlParser::Str2Int(&pid, process->getAttribute("id")) || pid <= 0){
         throw cRuntimeError("Invalid OSPFv3 process ID");
      }

      cXMLElement *routerId = process->getFirstChildWithTag("RouterId");
      if (routerId == NULL){
         throw cRuntimeError("Router-ID for OSPFv3 process #%d not found", pid);
      }

      IPAddress rid = routerId->getNodeValue();

      std::map<AnsaOspf6::ProcessID, AnsaOspf6::Router *>::iterator it;
      it = routers.find(pid);

      if (it == routers.end()){
         routers[pid] = new AnsaOspf6::Router(pid, rid.getInt(), this);
         ospfEnabled = true;
      }else{
         routers[pid]->SetRouterID(rid.getInt());
      }

      process = xmlParser::GetOspfProcess6(process, NULL);
   }
}

void AnsaOspfRouting6::loadInterfaceConfig(cXMLElement *iface){

   int firstIfaceId = 0;
   if (ift->getFirstInterface() != NULL){
      firstIfaceId = ift->getFirstInterface()->getInterfaceId();
   }else{
      return;
   }

   while (iface != NULL){

      InterfaceEntry *ie = ift->getInterfaceByName(iface->getAttribute("name"));
      if (ie == NULL){
         throw cRuntimeError("Interface %s not found on this device", iface->getAttribute("name"));
      }

      // let's find out if there is OSPFv3 routing enabled on this device
      int processId = -1;
      int areaId = -1;
      AnsaOspf6::Router *router = NULL;
      AnsaOspf6::Area *area = NULL;
      cXMLElement *processNode = iface->getFirstChildWithTag("OspfProcess6");
      cXMLElement *areaNode = iface->getFirstChildWithTag("OspfArea6");

      if (processNode != NULL){
         if (!xmlParser::Str2Int(&processId, processNode->getNodeValue()) || processId <= 0 || processId > OSPF_MAX_PROCESSID){
            throw cRuntimeError("Invalid OSPFv3 process ID on interface %s", ie->getName());
         }

         std::map<AnsaOspf6::ProcessID, AnsaOspf6::Router *>::iterator it;
         it = routers.find(processId);
         if (it == routers.end()){
            throw cRuntimeError("Invalid OSPFv3 process ID on interface %s (process #%d not found)", ie->getName(), processId);
         }

         router = it->second;
         iface2Routers[ie->getInterfaceId() - firstIfaceId] = router;
      }

      if (areaNode != NULL){

         if (router == NULL){
            throw cRuntimeError("Can't load area without selecting OSPFv3 process on interface %s", ie->getName());
         }

         if (!xmlParser::Str2Int(&areaId, areaNode->getNodeValue()) || areaId < 0){
            throw cRuntimeError("Invalid OSPFv3 area ID on interface %s", ie->getName());
         }

         area = router->GetArea(areaId);
         if (area == NULL){
            area = new AnsaOspf6::Area(areaId);
            router->AddArea(area);
         }
      }

      if (router != NULL && area == NULL){
         throw cRuntimeError("Area ID not set on interface %s", ie->getName());
      }


      // everything is fine, OSPFv3 is running
      if (router != NULL && area != NULL){

         // create interface
         AnsaOspf6::Interface::OspfInterfaceType type = type = AnsaOspf6::Interface::UnknownType;
         if (ie->isPointToPoint()){
            type = AnsaOspf6::Interface::PointToPoint;
         }else if (ie->isBroadcast()){
            type = AnsaOspf6::Interface::Broadcast;
         }

         AnsaOspf6::Interface *intf = new AnsaOspf6::Interface(type);
         intf->SetIfIndex(ie->getInterfaceId());
         intf->SetIfName(ie->getName());
         intf->SetAreaID(area->GetAreaID());
         intf->SetMtu(ie->getMTU());
         area->AddInterface(intf);
         ie->setMulticast(true);

         // load address prefixes to area
         cXMLElement *addr = xmlParser::GetIPv6Address(NULL, iface);
         while (addr != NULL){

            // get address string
            string addrFull = addr->getNodeValue();
            IPv6Address ipv6;
            int prefixLen;

            // check if it's a valid IPv6 address string with prefix and get prefix
            if (!ipv6.tryParseAddrWithPrefix(addrFull.c_str(), prefixLen)){
               throw cRuntimeError("Unable to load IPv6 address prefix %s on interface %s", addrFull.c_str(), ie->getName());
            }

            ipv6 = addrFull.substr(0, addrFull.find_last_of('/')).c_str();

            AnsaOspf6::IPv6AddressPrefix prefix = {ipv6.getPrefix(prefixLen), prefixLen};
            area->AddAddressPrefix(prefix, true);
            intf->AddAddressPrefix(prefix, true);

            // get next IPv6 address
            addr = xmlParser::GetIPv6Address(addr, NULL);
         }


         // load optional parameters
         for (cXMLElement *param = iface->getFirstChild(); param; param = param->getNextSibling()){

            // instance ID
            if(strcmp(param->getTagName(), "OspfInstance6") == 0){

               int instanceId = 0;
               if (!xmlParser::Str2Int(&instanceId, param->getNodeValue()) || instanceId < 0 || instanceId > OSPF_MAX_INSTANCEID){
                  throw cRuntimeError("Invalid OSPFv3 instance ID on interface %s", ie->getName());
               }

               intf->SetInstanceID(instanceId);


            // network type
            }else if(strcmp(param->getTagName(), "OspfNetworkType6") == 0){

               const char *networkType = param->getNodeValue();
               if (strcmp(networkType, "point-to-point") == 0){
                  intf->SetType(AnsaOspf6::Interface::PointToPoint);
               }else if (strcmp(networkType, "broadcast") == 0){
                  intf->SetType(AnsaOspf6::Interface::Broadcast);
               }else if (strcmp(networkType, "non-broadcast") == 0){
                  intf->SetType(AnsaOspf6::Interface::NBMA);
               }else if (strcmp(networkType, "point-to-multipoint") == 0){
                  intf->SetType(AnsaOspf6::Interface::PointToMultiPoint);
               }else{
                  throw cRuntimeError("Invalid network type \"%s\" on interface %s", networkType, ie->getName());
               }


            // cost and priority
            }else if(strcmp(param->getTagName(), "OspfCost6") == 0){

               int cost = 0;
               if (!xmlParser::Str2Int(&cost, param->getNodeValue()) || cost < 1 || cost > OSPF_MAX_COST){
                  throw cRuntimeError("Invalid OSPFv3 cost on interface %s", ie->getName());
               }

               intf->SetOutputCost(cost);

            }else if(strcmp(param->getTagName(), "OspfPriority6") == 0){

               int priority = 0;
               if (!xmlParser::Str2Int(&priority, param->getNodeValue()) || priority < 0 || priority > OSPF_MAX_PRIORITY){
                  throw cRuntimeError("Invalid OSPFv3 priority on interface %s", ie->getName());
               }

               intf->SetRouterPriority(priority);


            // timers
            }else if(strcmp(param->getTagName(), "OspfHelloInterval6") == 0){

               int timer = 0;
               if (!xmlParser::Str2Int(&timer, param->getNodeValue()) || timer < 1){
                  throw cRuntimeError("Invalid OSPFv3 hello interval on interface %s", ie->getName());
               }

               intf->SetHelloInterval(timer);

            }else if(strcmp(param->getTagName(), "OspfRouterDeadInterval6") == 0){

               int timer = 0;
               if (!xmlParser::Str2Int(&timer, param->getNodeValue()) || timer < 1){
                  throw cRuntimeError("Invalid OSPFv3 dead interval on interface %s", ie->getName());
               }

               intf->SetRouterDeadInterval(timer);

            }else if(strcmp(param->getTagName(), "OspfRetransmitInterval6") == 0){

               int timer = 0;
               if (!xmlParser::Str2Int(&timer, param->getNodeValue()) || timer < 1){
                  throw cRuntimeError("Invalid OSPFv3 retransmit interval on interface %s", ie->getName());
               }

               intf->SetRetransmissionInterval(timer);

            }else if(strcmp(param->getTagName(), "OspfTransmitDelay6") == 0){

               int timer = 0;
               if (!xmlParser::Str2Int(&timer, param->getNodeValue()) || timer < 1){
                  throw cRuntimeError("Invalid OSPFv3 transmit delay on interface %s", ie->getName());
               }

               intf->SetTransmissionDelay(timer);
            }
         }
      }

      iface = xmlParser::GetInterface(iface, NULL);
   }
}
