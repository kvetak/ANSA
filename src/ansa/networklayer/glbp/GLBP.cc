/*
 * GLBP.cc
 *
 *  Created on: 18.4. 2016
 *      Author: Jan Holusa
 */
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

#include "GLBP.h"

namespace inet {

Define_Module(GLBP);

GLBP::GLBP() {
}

void GLBP::initialize(int stage)
{

    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this); //usable interfaces of tihs router
        glbpMulticastAddress = new L3Address(par("glbpMulticastAddress"));
        socket = new UDPSocket(); //UDP socket used for sending messages
        socket->setOutputGate(gate("udpOut"));
        socket->setReuseAddress(true);
        socket->bind((int) par("glbpUdpPort"));
        this->parseConfig(par(CONFIG_PAR));
    }
}

void GLBP::handleMessage(cMessage *msg)
{
    //get message from netwlayer
    if (msg->getArrivalGate()->isName("udpIn")){
        if (dynamic_cast<GLBPMessage*>(msg))
        {
            GLBPMessage *GLBPHm = dynamic_cast<GLBPMessage*>(msg);
            //TODO to same pro GLBPResponseRequest message
            for (int i = 0; i < (int) virtualRouterTable.size(); i++){

                if (virtualRouterTable.at(i)->getGroup() == GLBPHm->getGroup()) //&&
//                        ((IPv4ControlInfo *)msg->getControlInfo())->getInterfaceId() ==
//                                virtualRouterTable.at(i)->getInterface()->getInterfaceId()
//                   )
                {
                    send(msg, "glbpOut", i);
                    return;
                }
            }
            EV << "unknown Virtual Router ID" << GLBPHm->getGroup() << ", discard it." << endl;
            delete msg;
        }
    }
    //get message from GLBP
    else if (msg->getArrivalGate()->isName("glbpIn")){
//        EV << "Recieved advertisement '" << msg << "' from Virtual Router, sending to network layer." << endl;
        send(msg, "udpOut");
    }
} //end handleMessage

void GLBP::addVirtualRouter(int interface, int vrid, const char* ifnam, std::string vip, int priority, bool preempt){
    int gateSize = virtualRouterTable.size() + 1;
    this->setGateSize("glbpIn",gateSize);
    this->setGateSize("glbpOut", gateSize);

    // name
    std::stringstream tmp;
    tmp << "VR_" << ifnam << "_" << vrid;
    std::string name = tmp.str();

    // create
    cModuleType *moduleType = cModuleType::get("ansa.networklayer.glbp.GLBPVirtualRouter");
    cModule *module = moduleType->create(name.c_str(), this);

    // set up parameters
    module->par("deviceId") = par("deviceId");
//    module->par("configData") = par("configData");
    module->par("vrid") = vrid;
    module->par("interface") = interface;
    module->par("virtualIP") = vip;
    module->par("priority") = priority;
    module->par("preempt") = preempt;
    //TODO timery! hello, hold, redirect, timeout
    module->par("interfaceTableModule") = ift->getFullPath();
    cModule *containingModule = findContainingNode(this);
    module->par("arp") = containingModule->getSubmodule("networkLayer")->getSubmodule("ipv4")->getSubmodule("arp")->getFullPath();

    std::cout<< "full path:"<<ift->getFullPath()<<std::endl;
    module->finalizeParameters();

    // set up gate
    this->gate("glbpOut", virtualRouterTable.size())->connectTo(module->gate("udpIn"));
    module->gate("udpOut")->connectTo(this->gate("glbpIn", virtualRouterTable.size()));
    module->buildInside();

    // load
    module->scheduleStart(simTime());

    virtualRouterTable.push_back(dynamic_cast<GLBPVirtualRouter *>(module));

    updateDisplayString();
}

void GLBP::parseConfig(cXMLElement *config){
    //naparsuje config - >>> a rovnou zaklada GLBPVirtRoutery

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

            //get GID - OBLIGATORY!
            int gid;
            std::stringstream strGID;
            if (!group->getAttribute("id")) {
                EV << "Config XML file missing tag or attribute - Group id" << endl;
                std::cerr<<par("deviceId").stringValue()<<" Group ??"<<": Wrong GLBP group number or missing in config file."<<endl;
                fflush(stderr);
                continue;
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
                if (strcmp("false",group->getAttribute("preempt"))){
                    preempt = false;
                }else
                {
                    preempt = true;
                }
                EV_DEBUG << "Setting preemption:" <<preempt<< endl;
            }

            //get interface id
            int iid = ift->getInterfaceByName(ifname.c_str())->getInterfaceId();

            //get virtual IP
            std::string virtip;
            if (!group->getAttribute("ip")) {
                EV << "Config XML file missing tag or attribute - Ip" << endl;
                virtip = "0.0.0.0"; //sign that ip is not set
            } else
            {
                virtip = group->getAttribute("ip");
//                if (is_unique(virtip, iid)){
                    EV_DEBUG << "Setting virtip:" <<virtip<< endl;
//                }else{
//                    std::cerr<<par("deviceId").stringValue()<<" Group "<<gid<<": Wrong HSRP group IP in config file."<<endl;
//                    fflush(stderr);
//                    continue;
//                }
            }


            checkAndJoinMulticast(iid);
            addVirtualRouter(iid , gid, ifname.c_str(), virtip, priority, preempt);
        }// end each group
    }//end each interface
}//end parseConfig

//interface join multicast group if it is not joined yet
void GLBP::checkAndJoinMulticast(int InterfaceId){
    bool contain=false;
    for (int i = 0; i < (int) multicastInterfaces.size(); i++){
        if (multicastInterfaces[i] == InterfaceId){
            contain = true;
        }
    }

    if (!contain){
        socket->joinMulticastGroup(*glbpMulticastAddress,InterfaceId);
        multicastInterfaces.push_back(InterfaceId);
    }
}

void GLBP::updateDisplayString()
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

GLBP::~GLBP() {
}

} /* namespace inet */
