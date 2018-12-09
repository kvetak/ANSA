//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
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
/**
* @file VRRPv2.cc
* @author Petr Vitek
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
* @brief
* @detail
*/

#include "ansa/routing/vrrp/VRRPv2.h"
#include "ansa/routing/vrrp/VRRPv2DeviceConfigurator.h"
#include "inet/networklayer/contract/ipv4/Ipv4ControlInfo.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

Define_Module(VRRPv2);

std::ostream& operator<<(std::ostream& os, const VRRPv2VirtualRouter& e)
{
    os << e.showBrief();
    return os;
};

VRRPv2::VRRPv2()
{
}

void VRRPv2::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT)
    {
        // get the hostname
        cModule *containingMod = findContainingNode(this);
        if (containingMod)
            hostname = containingMod->getFullName();
        else
            hostname = "";
    }

    if (stage == INITSTAGE_NETWORK_LAYER_3) {
        // read the VRRP groups configuration
        VRRPv2DeviceConfigurator *devConf = new VRRPv2DeviceConfigurator(par(CONFIG_PAR), getModuleFromPar<IInterfaceTable>(par(IFT_PAR), this) );
        devConf->loadVRRPv2Config(this);
        WATCH_PTRVECTOR(virtualRouterTable);
        updateDisplayString();
    }
}

void VRRPv2::addVirtualRouter(int interface, int vrid, const char* ifnam)
{
    int gateSize = virtualRouterTable.size() + 1;
    this->setGateSize(VRIN_GATE,gateSize);
    this->setGateSize(VROUT_GATE, gateSize);

    // name
    std::stringstream tmp;
    tmp << "VR_" << ifnam << "_" << vrid;
    std::string name = tmp.str();

    // create
    cModuleType *moduleType = cModuleType::get(VR_MOD);
    cModule *module = moduleType->create(name.c_str(), this);

    // set up parameters
    module->par(HOSTNAME_PAR) = this->getHostname();
    module->par(CONFIG_PAR) = par(CONFIG_PAR);
    module->par(IFT_PAR) = par(IFT_PAR);
    module->par(VRID_PAR) = vrid;
    module->par(INTERFACE_PAR) = interface;
    module->par("arp") = getContainingNode(this)->getSubmodule("networkLayer")->getSubmodule("ipv4")->getSubmodule("arp")->getFullPath();
    module->finalizeParameters();

    // set up gate
    this->gate(VROUT_GATE, virtualRouterTable.size())->connectTo(module->gate(IPIN_GATE));
    module->gate(IPOUT_GATE)->connectTo(this->gate(VRIN_GATE, virtualRouterTable.size()));
    module->buildInside();

    // load
    module->scheduleStart(simTime());

    virtualRouterTable.push_back(dynamic_cast<VRRPv2VirtualRouter *>(module));

    updateDisplayString();
}

void VRRPv2::handleMessage(cMessage *msg)
{
    if (msg->getArrivalGate()->isName("ipIn"))
    {
        if (dynamic_cast<VRRPv2Advertisement*>(msg))
        {
            VRRPv2Advertisement* advert = dynamic_cast<VRRPv2Advertisement*>(msg);

            EV << "Recieved packet '" << advert << "' from network layer, ";

            for (int i = 0; i < (int) virtualRouterTable.size(); i++)
                if (virtualRouterTable.at(i)->getVrid() == advert->getVrid() &&
                        ((Ipv4ControlInfo *) msg->getControlInfo())->getInterfaceId() ==
                                virtualRouterTable.at(i)->getInterface()->getInterfaceId()
                )
                {
                    EV << "sending Advertisement to Virtual Router '" << virtualRouterTable.at(i) << "'" << endl;;
                    send(msg, "vrOut", i);
                    return;
                }

            EV << "unknown Virtual Router ID" << advert->getVrid() << ", discard it." << endl;
            delete msg;
        }
    }
    /* Zprava od VRRP */
    else if (msg->getArrivalGate()->isName("vrIn"))
    {
        EV << "Recieved advertisement '" << msg << "' from Virtual Router, sending to network layer." << endl;
        send(msg, "ipOut");
    }
}

void VRRPv2::updateDisplayString()
{
    if (!getEnvir()->isGUI())
        return;

    char buf[80];
    if (virtualRouterTable.size() == 1)
        sprintf(buf, "%d group", virtualRouterTable.size());
    else if (virtualRouterTable.size() > 1)
        sprintf(buf, "%d groups", virtualRouterTable.size());

    getDisplayString().setTagArg("t", 0, buf);
}

}
