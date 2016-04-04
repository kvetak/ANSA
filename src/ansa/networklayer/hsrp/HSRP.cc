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

#include "HSRP.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"


namespace inet {

Define_Module(HSRP);


HSRP::HSRP() {
}

/**
 * Startup initializacion of HSRP
 */
void HSRP::initialize(int stage)
{

    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this); //usable interfaces of tihs router
        this->parseConfig(par(CONFIG_PAR));
    }
}

/**
 * Omnet++ function for handeling incoming messages
 */
void HSRP::handleMessage(cMessage *msg)
{

} //end handleMessage

void HSRP::addVirtualRouter(int interface, int vrid, const char* ifnam, std::string vip, int priority){
    int gateSize = virtualRouterTable.size() + 1;
    this->setGateSize("hsrpIn",gateSize);
    this->setGateSize("hsrpOut", gateSize);

    // name
    std::stringstream tmp;
    tmp << "VR_" << ifnam << "_" << vrid;
    std::string name = tmp.str();

    // create
    cModuleType *moduleType = cModuleType::get("ansa.networklayer.hsrp.HSRPVirtualRouter");
    cModule *module = moduleType->create(name.c_str(), this);

    // set up parameters
    module->par("deviceId") = par("deviceId");
    module->par("configData") = par("configData");
    module->par("vrid") = vrid;
    module->par("interface") = interface;
    module->par("virtualIP") = vip;
    module->par("priority") = priority;
    module->par("interfaceTableModule") = ift->getFullPath();
    cModule *containingModule = findContainingNode(this);
    module->par("arp") = containingModule->getSubmodule("networkLayer")->getSubmodule("ipv4")->getSubmodule("arp")->getFullPath();

    std::cout<< "full path:"<<ift->getFullPath()<<std::endl;
    module->finalizeParameters();

    // set up gate
    this->gate("hsrpOut", virtualRouterTable.size())->connectTo(module->gate("udpIn"));
    module->gate("udpOut")->connectTo(this->gate("hsrpIn", virtualRouterTable.size()));
    module->buildInside();

    // load
    module->scheduleStart(simTime());

    virtualRouterTable.push_back(dynamic_cast<HSRPVirtualRouter *>(module));

    updateDisplayString();
}

void HSRP::parseConfig(cXMLElement *config){
    //naparsuje config - >>> a rovnou zaklada HSRPVirtRoutery!! TODO

    //Config element is empty
    if (!config)
        return;

    //Go through all interfaces and look for HSRP section
    cXMLElementList msa = config->getChildrenByTagName("Interface");
    for (cXMLElementList::iterator i = msa.begin(); i != msa.end(); ++i) {
        cXMLElement* m = *i;
        std::string ifname;
        ifname = m->getAttribute("name");

        //Get through each group
        cXMLElementList gr = m->getElementsByTagName("Group");
        for (cXMLElementList::iterator j = gr.begin(); j != gr.end(); ++j) {
            cXMLElement* group = *j;

            //get GID
            int gid;
            std::stringstream strValue;
            if (!group->getAttribute("id")) {
                EV << "Config XML file missing tag or attribute - Group id" << endl;
                continue;
            } else
            {
                strValue << group->getAttribute("id");
                strValue >> gid;
                EV_DEBUG << "Setting GID:" <<gid<< endl;
            }

            //get Priority
            std::stringstream strValue2;
            int priority;
            if (!group->getAttribute("priority")) {
                EV << "Config XML file missing tag or attribute - Priority" << endl;
                priority = 100;
//                continue;
            } else
            {
                strValue2 << group->getAttribute("priority");
                strValue2 >> priority;
                EV_DEBUG << "Setting priority:" <<priority<< endl;
            }

            //get virtual IP
            std::string virtip;
            if (!group->getAttribute("ip")) {
                EV << "Config XML file missing tag or attribute - Ip" << endl;
                virtip = "";
//                continue;
            } else
            {
                virtip = group->getAttribute("ip");
                EV_DEBUG << "Setting virtip:" <<virtip<< endl;
//                virtualIP = new IPv4Address(virtip.c_str()); //TODO chybi!
            }
            printf("DevConf>> vrid:%s, GID: %d!\n", virtip.c_str(),  gid);
            fflush(stdout);

            int iid = ift->getInterfaceByName(ifname.c_str())->getInterfaceId();
            addVirtualRouter(iid , gid, ifname.c_str(), virtip, priority);
        }// end each group
    }//end each interface
}//end parseConfig

void HSRP::updateDisplayString()
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

HSRP::~HSRP() {
    printf("destrukce HSRP\n");
}

} /* namespace inet */
