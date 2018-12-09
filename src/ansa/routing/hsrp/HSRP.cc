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
* @author Jan Holusa
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#include "ansa/routing/hsrp/HSRP.h"

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
        containingModule = getContainingNode(this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this); //usable interfaces of tihs router
        hsrpMulticast = new L3Address(HSRP_MULTICAST_ADDRESS.c_str());
        socket = new UdpSocket(); //Udp socket used for sending messages
        socket->setOutputGate(gate("udpOut"));
        socket->setMulticastLoop(false);
        socket->bind(HSRP_UDP_PORT);
        this->parseConfig(par(CONFIG_PAR));
        hostname = std::string(containingModule->getName(), strlen(containingModule->getName()));
    }
}

/**
 * Omnet++ function for handeling incoming messages
 */
void HSRP::handleMessage(cMessage *msg)
{
    //get message from netwlayer
    if (msg->getArrivalGate()->isName("udpIn")){
        if (dynamic_cast<HSRPMessage*>(msg))
        {
            HSRPMessage *HSRPm = dynamic_cast<HSRPMessage*>(msg);
//            EV << "Recieved packet '" << HSRPm << "' from network layer, ";
            for (int i = 0; i < (int) virtualRouterTable.size(); i++){
                if (virtualRouterTable.at(i)->getGroup() == HSRPm->getGroup()) //&&
                {
//                    EV << "sending Advertisement to Virtual Router '" << virtualRouterTable.at(i) << "'" << endl;
                    send(msg, "hsrpOut", i);
                    return;
                }
            }
            EV << "unknown Virtual Router ID" << HSRPm->getGroup() << ", discard it." << endl;
            delete msg;
        }
    }
    //get message from HSRP
    else if (msg->getArrivalGate()->isName("hsrpIn")){
//        EV << "Recieved advertisement '" << msg << "' from Virtual Router, sending to network layer." << endl;
        send(msg, "udpOut");
    }
} //end handleMessage

void HSRP::addVirtualRouter(int interface, int vrid, const char* ifnam, std::string vip, int priority, bool preempt, int hellotime, int holdtime){
    int gateSize = virtualRouterTable.size() + 1;
    this->setGateSize("hsrpIn",gateSize);
    this->setGateSize("hsrpOut", gateSize);

    // name
    std::stringstream tmp;
    tmp << "VR_" << ifnam << "_" << vrid;
    std::string name = tmp.str();

    // create
    cModuleType *moduleType = cModuleType::get("ansa.routing.hsrp.HSRPVirtualRouter");
    cModule *module = moduleType->create(name.c_str(), this);

    // set up parameters
    module->par("configData") = par("configData");
    module->par("group") = vrid;
    module->par("interface") = interface;
    module->par("virtualIP") = vip;
    module->par("priority") = priority;
    module->par("preempt") = preempt;
    module->par("hellotime") = hellotime;
    module->par("holdtime") = holdtime;
    module->par("interfaceTableModule") = ift->getFullPath();
    cModule *containingModule = findContainingNode(this);
    module->par("arp") = containingModule->getSubmodule("networkLayer")->getSubmodule("ipv4")->getSubmodule("arp")->getFullPath();

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
    //Config element is empty
    if (!config)
        return;

    //Go through all interfaces and look for HSRP section
    cXMLElementList msa = config->getChildrenByTagName("Interface");
    for (cXMLElementList::iterator i = msa.begin(); i != msa.end(); ++i) {
        cXMLElement* m = *i;
        std::string ifname;
        ifname = m->getAttribute("name");

        int iid;
        //is interface configured with IP?
        if (ift->getInterfaceByName(ifname.c_str()) != nullptr){
            //get interface id
            iid = ift->getInterfaceByName(ifname.c_str())->getInterfaceId();
        }else{
            continue;
        }

        //Get through each group
        cXMLElementList gr = m->getElementsByTagName("Group");
        for (cXMLElementList::iterator j = gr.begin(); j != gr.end(); ++j) {
            cXMLElement* group = *j;

            //get GID
            int gid;
            std::stringstream strGID;
            if (!group->getAttribute("id")) {
                EV << "Config XML file missing tag or attribute - Group id" << endl;
                gid = 0; //def val
            } else
            {
                strGID << group->getAttribute("id");
                strGID >> gid;
                EV_DEBUG << "Setting GID:" <<gid<< endl;
            }

            //get Priority
            std::stringstream strValue2;
            int priority;
            if (!group->getAttribute("priority")) {
                EV << "Config XML file missing tag or attribute - Priority" << endl;
                priority = 100;//def val
            } else
            {
                strValue2 << group->getAttribute("priority");
                strValue2 >> priority;
                EV_DEBUG << "Setting priority:" <<priority<< endl;
            }

            //get Preempt flag
            bool preempt;
            if (!group->getAttribute("preempt")) {
                EV << "Config XML file missing tag or attribute - Preempt" << endl;
                preempt = false; //def val
            } else
            {
                if (!strcmp("true",group->getAttribute("preempt"))){
                    preempt = true;
                }else
                {
                    preempt = false;
                }
                EV_DEBUG << "Setting preemption:" <<preempt<< endl;
            }

            //get Hellotime
            int hellotime;
            std::stringstream strHellotime;
            if (!group->getAttribute("hellotime")) {
                EV << "Config XML file missing tag or attribute - Hellotime interval." << endl;
                hellotime = 3; //def val
            } else
            {
                strHellotime << group->getAttribute("hellotime");
                strHellotime >> hellotime;
                if ((hellotime<1) || (hellotime >254)){
                    std::cerr<<hostname<<" Group "<<gid<<": Wrong value of HSRP Hellotime timer. Must be <1-254>. Setting default 3 seconds."<<endl;
                    fflush(stderr);
                    hellotime = 3;
                }
                EV_DEBUG << "Setting Hellotime:" <<hellotime<< endl;
            }

            //get Holdtime
            int holdtime;
            std::stringstream strHoldtime;
            if (!group->getAttribute("holdtime")) {
                EV << "Config XML file missing tag or attribute - Holdtime interval." << endl;
                holdtime = 10; //def val
            } else
            {
                strHoldtime << group->getAttribute("holdtime");
                strHoldtime >> holdtime;
                if ((holdtime< hellotime+1) || (holdtime > 255)){
                    std::cerr<<hostname<<" Group "<<gid<<": Wrong value of HSRP Holdtime timer. Must be <(hellotime+1)-254>. Setting default 10 seconds."<<endl;
                    fflush(stderr);
                    holdtime = 10;
                }
                EV_DEBUG << "Setting Holdtime:" <<holdtime<< endl;
            }

            //get virtual IP
            std::string virtip;
            if (!group->getAttribute("ip")) {
                EV << "Config XML file missing tag or attribute - Ip" << endl;
                virtip = "0.0.0.0"; //sign that ip is not set
            } else
            {
                virtip = group->getAttribute("ip");
                if (is_unique(virtip, iid)){
                    EV_DEBUG << "Setting virtip:" <<virtip<< endl;
                }else{
                    std::cerr<<hostname<<" Group "<<gid<<": Wrong HSRP group IP in config file."<<endl;
                    fflush(stderr);
                    continue;
                }
            }


            checkAndJoinMulticast(iid);
            addVirtualRouter(iid , gid, ifname.c_str(), virtip, priority, preempt, hellotime, holdtime);
        }// end each group
    }//end each interface
}//end parseConfig

//interface join multicast group if it is not joined yet
void HSRP::checkAndJoinMulticast(int InterfaceId){
    bool contain=false;
    for (int i = 0; i < (int) multicastInterfaces.size(); i++){
        if (multicastInterfaces[i] == InterfaceId){
            contain = true;
        }
    }

    if (!contain){
        socket->joinMulticastGroup(*hsrpMulticast,InterfaceId);
        multicastInterfaces.push_back(InterfaceId);
    }
}


bool HSRP::is_unique(std::string virtip, int iid){
    //check local virtual IPs
    for (int i = 0; i < (int) virtualRouterTable.size(); i++){
        if (virtip.compare(virtualRouterTable.at(i)->par("virtualIP").stringValue()) == 0){
            EV<<hostname<<" % Address "<<virtip<<" in group "<<(int)virtualRouterTable.at(i)->par("group")<<"."<<endl;
            return false;
        }
    }

    //check physical IP of actual interface
    std::string InterfaceIP = ift->getInterfaceById(iid)->ipv4Data()->getIPAddress().str(false);
    if (InterfaceIP.compare(virtip) == 0){
        EV<<hostname<<" % Address cannot equal interface IP address."<<endl;
        return false;
    }

    //TODO: check overlapping ip with another interface
    return true;
}

void HSRP::updateDisplayString()
{
    if (!hasGUI())
        return;

    char buf[80];
    if (virtualRouterTable.size() == 1)
        sprintf(buf, "%d group", virtualRouterTable.size());
    else if (virtualRouterTable.size() > 1)
        sprintf(buf, "%d groups", virtualRouterTable.size());

    getDisplayString().setTagArg("t", 0, buf);
}


HSRP::~HSRP() {
//    printf("destrukce HSRP\n");
}

} /* namespace inet */
