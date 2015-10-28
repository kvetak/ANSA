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
* @brief
* @detail
*/

#include "VRRPv2.h"

#include "VirtualForwarder.h"
#include "deviceConfigurator.h"
#include "VRRPv2DeviceConfigurator.h"

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
    if (stage == 0)
    {
        // get the hostname
        cModule *containingMod = findContainingNode(this);
        if (containingMod)
            hostname = containingMod->getFullName();
        else
            hostname = "";
    }

    if (stage != 3)
        return;

    WATCH_PTRVECTOR(virtualRouterTable);
    updateDisplayString();

    // read the VRRP groups configuration
    VRRPv2DeviceConfigurator *devConf = ModuleAccess<VRRPv2DeviceConfigurator>("VRRPv2DeviceConfigurator").get();
    devConf->loadVRRPv2Config(this);



}

void VRRPv2::addVirtualRouter(int interface, int vrid, const char* ifnam)
{
    int gateSize = virtualRouterTable.size() + 1;
    this->setGateSize("vrIn",gateSize);
    this->setGateSize("vrOut", gateSize);

    // name
    std::stringstream tmp;
    tmp << "VR_" << ifnam << "_" << vrid;
    std::string name = tmp.str();

    // create
    cModuleType *moduleType = cModuleType::get("inet.ansa.networklayer.vrrpv2.VRRPv2VirtualRouter");
    cModule *module = moduleType->create(name.c_str(), this);

    // set up parameters
    module->par("hostname") = this->getHostname();
    module->par("deviceId") = par("deviceId");
    module->par("configFile") = par("configFile");
    module->par("vrid") = vrid;
    module->par("interface") = interface;
    module->finalizeParameters();

    // set up gate
    this->gate("vrOut", virtualRouterTable.size())->connectTo(module->gate("ipIn"));
    module->gate("ipOut")->connectTo(this->gate("vrIn", virtualRouterTable.size()));
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
                        ((IPv4ControlInfo *) msg->getControlInfo())->getInterfaceId() ==
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
    if (!ev.isGUI())
        return;

    char buf[80];
    if (virtualRouterTable.size() == 1)
        sprintf(buf, "%d group", virtualRouterTable.size());
    else if (virtualRouterTable.size() > 1)
        sprintf(buf, "%d groups", virtualRouterTable.size());

    getDisplayString().setTagArg("t", 0, buf);
}
