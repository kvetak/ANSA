#include "ansa/routing/ospfv3/process/OSPFv3Instance.h"
#include "ansa/routing/ospfv3/interface/OSPFv3Interface.h"


namespace inet{

OSPFv3Instance::OSPFv3Instance(int instanceId, OSPFv3Process* parentProcess, int addressFamily)
{
    this->instanceID = instanceId;
    this->containingProcess=parentProcess;
    this->addressFamily = addressFamily;
    this->containingModule=findContainingNode(this->containingProcess);
    this->ift = check_and_cast<IInterfaceTable *>(containingModule->getSubmodule("interfaceTable"));

}

OSPFv3Instance::~OSPFv3Instance()
{
}

bool OSPFv3Instance::hasArea(IPv4Address areaId)
{
    std::map<IPv4Address, OSPFv3Area*>::iterator areaIt = this->areasById.find(areaId);
    if(areaIt == this->areasById.end())
        return false;

    return true;
}//hasArea

void OSPFv3Instance::addArea(OSPFv3Area* newArea)
{
    this->areas.push_back(newArea);
    this->areasById[newArea->getAreaID()]=newArea;
}//addArea

OSPFv3Area* OSPFv3Instance::getAreaById(IPv4Address areaId)
{
    std::map<IPv4Address, OSPFv3Area*>::iterator areaIt = this->areasById.find(areaId);
    if(areaIt == this->areasById.end())
        return nullptr;

    return areaIt->second;
}//getAreaById

void OSPFv3Instance::processPacket(OSPFv3Packet* packet)
{
    IPv6ControlInfo* ctlInfo = check_and_cast<IPv6ControlInfo* >(packet->getControlInfo());
    //OSPFv3Packet* packet = check_and_cast<OSPFv3Packet*>(msg);
    EV_INFO << "Process " << this->containingProcess->getProcessID() << " received packet: (" << packet->getClassName() << ")" << packet->getName() << "\n";

    if(packet->getVersion()!=3) {
        delete packet;
        return;
    }

    IPv4Address areaId = packet->getAreaID();
    int intfId = ctlInfo->getInterfaceId();
    OSPFv3Area* area = this->getAreaById(packet->getAreaID());

    if(area!=nullptr) {
        OSPFv3Interface *intf = area->getInterfaceById(intfId);

        if (intf == nullptr) {
            EV_DEBUG <<"Interface is null in instance::processPacket\n";
            //it must be the backbone area and...
            if (areaId == IPv4Address::UNSPECIFIED_ADDRESS) {
                if (this->getAreaCount() > 1) {
                    // it must be a virtual link and the source router's router ID must be the endpoint of this virtual link and...
                    intf = area->findVirtualLink(packet->getRouterID());

                    if (intf != nullptr) {
                        OSPFv3Area *virtualLinkTransitArea = this->getAreaById(intf->getTransitAreaID());

                        if (virtualLinkTransitArea != nullptr) {
//                            // the receiving interface must attach to the virtual link's configured transit area
                            OSPFv3Interface *virtualLinkInterface = virtualLinkTransitArea->getInterfaceById(ctlInfo->getInterfaceId());
//
                            if (virtualLinkInterface == nullptr) {
                                intf = nullptr;
                            }
                        }
                        else {
                            intf = nullptr;
                        }
                    }
                }
            }
        }
        if (intf != nullptr) {
            IPv6Address destinationAddress = ctlInfo->getDestAddr();
            IPv6Address allDRouters = IPv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST;

            OSPFv3Interface::OSPFv3InterfaceFAState interfaceState = intf->getState();

            // if destination address is ALL_D_ROUTERS the receiving interface must be in DesignatedRouter or Backup state
            if (
                    ((destinationAddress == allDRouters) &&
                            (
                                    (interfaceState == OSPFv3Interface::INTERFACE_STATE_DESIGNATED) ||
                                    (interfaceState == OSPFv3Interface::INTERFACE_STATE_BACKUP)
                            )
                    ) ||
                    (destinationAddress != allDRouters)
            )
            {
                // packet authentication
                OSPFv3PacketType packetType = static_cast<OSPFv3PacketType>(packet->getType());
                OSPFv3Neighbor* neighbor = nullptr;

                // all packets except HelloPackets are sent only along adjacencies, so a Neighbor must exist
                if (packetType != OSPFv3PacketType::HELLO_PACKET)
                    neighbor = intf->getNeighborById(packet->getRouterID());

                switch (packetType) {
                case OSPFv3PacketType::HELLO_PACKET:
                    intf->processHelloPacket(packet);
                    break;

                case OSPFv3PacketType::DATABASE_DESCRIPTION:
                    if (neighbor != nullptr) {
                        EV_DEBUG << "Instance is sending packet to interface\n";
                        intf->processDDPacket(packet);
                    }
                    break;

                case OSPFv3PacketType::LSR:
                    if (neighbor != nullptr) {
                        intf->processLSR(packet, neighbor);
                    }
                    break;

                case OSPFv3PacketType::LSU:
                    if (neighbor != nullptr) {
                        intf->processLSU(packet, neighbor);
                    }
                    break;

                case OSPFv3PacketType::LS_ACK:
                    if (neighbor != nullptr) {
                        intf->processLSAck(packet, neighbor);
                    }
                    break;

                default:
                    break;
                }
            }
        }
    }
}//processPacket

void OSPFv3Instance::removeFromAllRetransmissionLists(LSAKeyType lsaKey)
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        areas.at(i)->removeFromAllRetransmissionLists(lsaKey);
    }
}

void OSPFv3Instance::init()
{
    for(auto it = this->areas.begin(); it!=this->areas.end(); it++)
        (*it)->init();

    WATCH_PTRVECTOR(this->areas);
}//init

void OSPFv3Instance::debugDump()
{
    EV_DEBUG << "Instance " << this->getInstanceID() << "\n";
    for(auto it=this->areas.begin();it!=this->areas.end(); it++) {
        EV_DEBUG << "\tArea id " << (*it)->getAreaID() << " has these interfaces:\n";
        (*it)->debugDump();
    }
}//debugDump

IPv4Address OSPFv3Instance::getNewInterAreaPrefixLinkStateID()
{
    IPv4Address currIP = this->interAreaPrefixLsID;
    int newIP = currIP.getInt()+1;
    this->interAreaPrefixLsID = IPv4Address(newIP);
    return currIP;
}

void OSPFv3Instance::subtractInterAreaPrefixLinkStateID()
{
    IPv4Address currIP = this->interAreaPrefixLsID;
    int newIP = currIP.getInt()-1;
    this->interAreaPrefixLsID = IPv4Address(newIP);
}

std::string OSPFv3Instance::detailedInfo() const
{
    std::stringstream out;
    int processID = this->getProcess()->getProcessID();
    IPv4Address routerID = this->getProcess()->getRouterID();
    out << "OSPFv3 " << processID << " address-family ";

    if(this->addressFamily == IPV4INSTANCE)
        out << "IPv4 (router-id " << routerID << ")\n\n";
    else
        out << "IPv6 (router-id " << routerID << ")\n\n";

    out << "Neighbor ID\tPri\tState\t\tDead Time\tInterface ID\tInterface\n";
    for(auto it=this->areas.begin(); it!=this->areas.end(); it++){
        int intfCount = (*it)->getInterfaceCount();
        for(int i=0; i<intfCount; i++) {
            OSPFv3Interface* intf = (*it)->getInterface(i);
            int neiCount = intf->getNeighborCount();
            for(int n=0; n<neiCount; n++) {
                OSPFv3Neighbor* neighbor = intf->getNeighbor(n);
                out << neighbor->getNeighborID() << "\t";
                out << neighbor->getNeighborPriority() << "\t";
                switch(neighbor->getState()){
                    case OSPFv3Neighbor::DOWN_STATE:
                        out << "DOWN\t\t";
                        break;

                    case OSPFv3Neighbor::ATTEMPT_STATE:
                        out << "ATTEMPT\t\t";
                        break;

                    case OSPFv3Neighbor::INIT_STATE:
                        out << "INIT\t\t";
                        break;

                    case OSPFv3Neighbor::TWOWAY_STATE:
                        if(intf->getDesignatedID() == IPv4Address::UNSPECIFIED_ADDRESS)
                            out << "2WAY\t\t";
                        else
                            out << "2WAY/DROTHER\t";
                        break;

                    case OSPFv3Neighbor::EXCHANGE_START_STATE:
                        out << "EXSTART\t\t";
                        break;

                    case OSPFv3Neighbor::EXCHANGE_STATE:
                        out << "EXCHANGE\t\t";
                        break;

                    case OSPFv3Neighbor::LOADING_STATE:
                        out << "LOADING\t\t";
                        break;

                    case OSPFv3Neighbor::FULL_STATE:
                        if(neighbor->getNeighborID() == intf->getDesignatedID())
                            out << "FULL/DR\t\t";
                        else if(neighbor->getNeighborID() == intf->getBackupID())
                            out << "FULL/BDR\t\t";
                        else
                            out << "FULL/DROTHER\t";
                        break;
                }

                int dead = intf->getDeadInterval() - ((int)simTime().dbl() - neighbor->getLastHelloTime());
                if(dead < 0)
                    dead = 0;
                out << dead << "\t\t";//"00:00:40\t\t";
                out << neighbor->getNeighborInterfaceID() << "\t\t";
                out << intf->getIntName() << "\n";
            }
        }
    }

    return out.str();
}
}//namespace inet
