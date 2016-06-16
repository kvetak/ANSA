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
        containingModule = getContainingNode(this);
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this); //usable interfaces of tihs router
        glbpMulticastAddress = new L3Address(par("glbpMulticastAddress"));
        socket = new UDPSocket(); //UDP socket used for sending messages
        socket->setOutputGate(gate("udpOut"));
        socket->setReuseAddress(true);
        socket->bind((int) par("glbpUdpPort"));
        this->parseConfig(par(CONFIG_PAR));
        hostname = std::string(containingModule->getName(), strlen(containingModule->getName()));

    }
}

void GLBP::handleMessage(cMessage *msg)
{
    //get message from netwlayer
    if (msg->getArrivalGate()->isName("udpIn")){
        if (dynamic_cast<GLBPMessage*>(msg))
        {
            GLBPMessage *GLBPHm = dynamic_cast<GLBPMessage*>(msg);
            for (int i = 0; i < (int) virtualRouterTable.size(); i++){

                if (virtualRouterTable.at(i)->getGroup() == GLBPHm->getGroup())
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

void GLBP::addVirtualRouter(int interface, int vrid, const char* ifnam, std::string vip, int priority, bool preempt, int redirect, int timeout, int hellotime, int holdtime){
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
    module->par("group") = vrid;
    module->par("interface") = interface;
    module->par("virtualIP") = vip;
    module->par("priority") = priority;
    module->par("preempt") = preempt;
    module->par("redirect") = redirect;
    module->par("timeout") = timeout;
    module->par("hellotime") = hellotime;
    module->par("holdtime") = holdtime;
    module->par("interfaceTableModule") = ift->getFullPath();
    cModule *containingModule = findContainingNode(this);
    module->par("arp") = containingModule->getSubmodule("networkLayer")->getSubmodule("ipv4")->getSubmodule("arp")->getFullPath();

//    std::cout<< "full path:"<<ift->getFullPath()<<std::endl;
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

    //Go through all interfaces and look for GLBP section
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

            //get GID - OBLIGATORY!
            int gid;
            std::stringstream strGID;
            if (!group->getAttribute("id")) {
                EV << "Config XML file missing obligatory tag or attribute - Group id" << endl;
                std::cerr<<hostname<<" Group ??"<<": Wrong GLBP group number or missing in config file."<<endl;
                fflush(stderr);
                continue;
            } else
            {
                strGID << group->getAttribute("id");
                strGID >> gid;
                if ((gid < 0) || (gid > 1023)){
                    std::cerr<<hostname<<" : Wrong value of GLBP Group number. Must be <0-1023>. Skiping configuration of this section!!!"<<endl;
                    fflush(stderr);
                    continue;
                }
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
                if ((priority < 1) || (priority > 255)){
                    std::cerr<<hostname<<" Group "<<gid<<" : Wrong value of GLBP priority value. Must be <1-255>. Setting default priority 100."<<endl;
                    fflush(stderr);
                    priority = 100;
                }
                EV_DEBUG << "Setting priority:" <<priority<< endl;
            }

            //get Preempt flag
            bool preempt;
            if (!group->getAttribute("preempt")) {
                EV << "Config XML file missing tag or attribute - Preempt" << endl;
                preempt = false; //def val
            } else
            {
                if (strcmp("true",group->getAttribute("preempt"))){
                    preempt = true;
                }else
                {
                    preempt = false;
                }
                EV_DEBUG << "Setting preemption:" <<preempt<< endl;
            }

            //get virtual IP
            std::string virtip;
            if (!group->getAttribute("ip")) {
                EV << "Config XML file missing tag or attribute - Ip" << endl;
                virtip = "0.0.0.0"; //sign that ip is not set
            } else
            {
                virtip = group->getAttribute("ip");
                EV_DEBUG << "Setting virtip:" <<virtip<< endl;
            }

            //get Redirect Timer
            std::stringstream strRedirect;
            int redirect;
            if (!group->getAttribute("redirect")) {
                EV << "Config XML file missing tag or attribute - Redirect" << endl;
                redirect = 600;//def val = 10minutes
            } else
            {
                strRedirect << group->getAttribute("redirect");
                strRedirect >> redirect;
                if ((redirect < 0) || (redirect > 3600)){
                    std::cerr<<hostname<<" Group "<<gid<<": Wrong value of GLBP redirect timer. Must be <0-3600>. Setting default 600 seconds."<<endl;
                    fflush(stderr);
                    redirect = 600;
                }
                EV_DEBUG << "Setting redirect timer:" <<redirect<< endl;
            }

            //get Timeout
            std::stringstream strTimeout;
            int timeout;
            if (!group->getAttribute("timeout")) {
                EV << "Config XML file missing tag or attribute - Timeout" << endl;
                timeout = 14400;//def val = 4hours
            } else
            {
                strTimeout << group->getAttribute("timeout");
                strTimeout >> timeout;
                if ((timeout < 600+redirect) || (timeout > 64800)){
                    std::cerr<<hostname<<" Group "<<gid<<": Wrong value of GLBP timeout timer. Must be <(redirect+600)-64800>. Setting default 14400 seconds."<<endl;
                    fflush(stderr);
                    timeout = 14400;
                }
                EV_DEBUG << "Setting timeout timer:" <<timeout<< endl;
            }

            //get Hellotime
            std::stringstream strHellotime;
            int hellotime;
            if (!group->getAttribute("hellotime")) {
                EV << "Config XML file missing tag or attribute - Hellotime" << endl;
                hellotime = 3;//def val
            } else
            {
                strHellotime << group->getAttribute("hellotime");
                strHellotime >> hellotime;
                if ((hellotime < 1) || (hellotime > 60)){
                    std::cerr<<hostname<<" Group "<<gid<<": Wrong value of GLBP hellotimer. Must be <1-60>. Setting default 3 seconds."<<endl;
                    fflush(stderr);
                    hellotime = 3;
                }
                EV_DEBUG << "Setting hellotime:" <<hellotime<< endl;
            }

            //get Holdtime
            std::stringstream strHoldtime;
            int holdtime;
            if (!group->getAttribute("holdtime")) {
                EV << "Config XML file missing tag or attribute - Holdtime" << endl;
                holdtime = 10;//def val
            } else
            {
                strHoldtime << group->getAttribute("holdtime");
                strHoldtime >> holdtime;
                if ((holdtime<hellotime+1) || (holdtime > 160)){
                    std::cerr<<hostname<<" Group "<<gid<<": Wrong value of GLBP holdtime. Must be <(hellotome+1)-160>. Setting hellotime*3 ("<<hellotime*3<<") seconds."<<endl;
                    fflush(stderr);
                    holdtime = hellotime*3;
                }
                EV_DEBUG << "Setting holdtime:" <<holdtime<< endl;
            }

            //TODO Weight
            //TODO Load-Balancing algorithm

            checkAndJoinMulticast(iid);
            addVirtualRouter(iid , gid, ifname.c_str(), virtip, priority, preempt, redirect, timeout, hellotime, holdtime);
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
    if (!hasGUI())
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
