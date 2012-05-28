// Author: Matej Hrncirik
// FIT VUT 2012
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

#include "ISIS.h"

Define_Module(ISIS);

ISIS::~ISIS()
{

}

/**
 * Initialization function called at the start of simulation. This method provides initial
 * parsing of XML config file and configuration of whole module including network interfaces.
 * NED address is validated and after it's loaded. Initial timers for hello and LSP packets
 * are also set.
 * @see insertIft(InterfaceEntry *entry, cXMLElement *intElement)
 * @see parseNetAddr()
 * @param stage defines stage(step) of global initialization
 */

void ISIS::initialize(int stage)
{
    //interface init at stage 2
    if(stage == 3)
    {


            deviceType = par("deviceType");
            deviceId = par("deviceId");
            configFile = par("configFile");

            ift = AnsaInterfaceTableAccess().get();
            if (ift == NULL){
                    throw cRuntimeError("AnsaInterfaceTable not found");
            }


            cXMLElement *device = xmlParser::GetDevice(deviceType, deviceId, configFile);
            if (device == NULL)
            {
                EV << "deviceId " << deviceId << ": ISIS is not enabled on this device\n";
                return;
            }

            //load IS-IS routing info from config file
            cXMLElement *isisRouting = xmlParser::GetIsisRouting(device);
            if(isisRouting == NULL)
            {
                EV << "deviceId " << deviceId << ": ISIS is not enabled on this device\n";
                return;
            }

            cXMLElement *net = isisRouting->getFirstChildWithTag("NET");
            if(net == NULL)
            {
                EV << "deviceId " << deviceId << ": Net address wasn't specified in IS-IS routing\n";
                return;
            }

            netAddr = net->getNodeValue();
            if( netAddr == NULL || strcmp("",netAddr) == 0)
            {
                EV << "deviceId " << deviceId << ": Net address wasn't specified in IS-IS routing\n";
                return;
            }
            else
            {
                if(!parseNetAddr())
                {
                    EV << "deviceId " << deviceId << ": Invalid net address format\n";
                    return;
                }
                else
                {
                    EV << "deviceId " << deviceId << ": Net address set to: " << netAddr << "\n";
                }
            }

            //set router IS type {L1 | L2 | L1L2 (default)}
            cXMLElement *routertype = isisRouting->getFirstChildWithTag("IS-Type");
            if(routertype == NULL)
            {
                this->isType = L1L2_TYPE;
            }
            else
            {
                const char* routerTypeValue = routertype->getNodeValue();
                if(routerTypeValue == NULL)
                {
                    this->isType = L1L2_TYPE;
                }
                else
                {
                    if(strcmp(routerTypeValue,"level-1") == 0)
                    {
                        this->isType = L1_TYPE;
                    }
                    else
                    {
                        if(strcmp(routerTypeValue,"level-2") == 0)
                        {
                            this->isType = L2_TYPE;
                        }
                        else
                        {
                            this->isType = L1L2_TYPE;
                        }
                    }
                }
            }

           cXMLElement *interfaces = device->getFirstChildWithTag("Interfaces");
           if(interfaces == NULL)
           {
               EV << "deviceId " << deviceId << ": <Interfaces></Interfaces> tag is missing in configuration file: \""
               << configFile << "\"\n";
               return;
           }

           // add all interfaces to ISISIft vector containing additional information
           InterfaceEntry *entryIFT = new InterfaceEntry();
           for (int i = 0; i < ift->getNumInterfaces(); i++)
           {
             entryIFT = ift->getInterface(i);
             //EV << entryIFT->getNetworkLayerGateIndex() << " " << entryIFT->getName() << " " << entryIFT->getFullName() << "\n";
             insertIft(entryIFT, interfaces->getFirstChildWithAttribute("Interface", "name", entryIFT->getName()));
           }

           //TODO passive-interface


           // set first hello timer after 1 second
            ISISTimer *timerMsg = new ISISTimer("Hello_timer");
            timerMsg->setTimerKind(HELLO_TIMER);
            scheduleAt(simTime() + 1.0, timerMsg);

            //set initial hello counter
            this->helloCounter = 0;

            //give network time to converge before first LSP is sent out (15 seconds)
            ISISTimer *LSPtimer =  new ISISTimer("Send first LSP timer");
            LSPtimer->setTimerKind(L1LSP_REFRESH);
            scheduleAt(simTime() + 15.0, LSPtimer);

            //10 sec after first LSPs sent should be flooded CNSP packets
            ISISTimer *CSNPtimer =  new ISISTimer("Send first CSNP packets");
            CSNPtimer->setTimerKind(CSNP_TIMER);
            scheduleAt(simTime() + 25.0, CSNPtimer);
        }
}


/**
 * Handle incoming messages: Method differs between self messages and external messages
 * and executes appropriate function.
 * @param msg incomming message
 */
void ISIS::handleMessage(cMessage* msg)
{

    if(msg->isSelfMessage())
    {
        ISISTimer *timer = check_and_cast <ISISTimer *> (msg);
        switch(timer->getTimerKind())
        {
            case(HELLO_TIMER):
                    this->sendHelloMsg();
                    delete timer;
                    break;
            case(NEIGHBOUR_DEAD):
                    this->removeDeadNeighbour(timer);
                    delete timer;
                    break;
            case(L1LSP_REFRESH):
                    this->sendMyL1LSPs();
                    delete timer;
                    break;
            case(L1LSP_DEAD):
                    this->removeDeadLSP(timer);
                    delete timer;
                    break;
            case(CSNP_TIMER):
                    this->sendL1CSNP();
                    delete timer;
                    break;
            default:
                    break;
        }
    }

    else
    {

        ISISMessage *inMsg = check_and_cast<ISISMessage *>(msg);

        switch(inMsg->getType())
        {
        case(LAN_L1_HELLO):
                if(this->isType == L1_TYPE || this->isType == L1L2_TYPE)
                {
                    this->handleL1HelloMsg(inMsg);

                    //comment if printing of the adjacency table is too disturbing
                    this->printAdjTable();
                    delete inMsg;
                }
                break;
        case(LAN_L2_HELLO):
                if(this->isType == L2_TYPE || this->isType == L1L2_TYPE)
                {
                    this->handleL2HelloMsg(inMsg);
                    this->printAdjTable();
                    delete inMsg;
                }
                break;
        case(L1_LSP):
                if(this->isType == L1_TYPE || this->isType == L1L2_TYPE)
                {
                    this->handleL1LSP(inMsg);

                    //comment if printing of the link-state database is too disturbing
                    this->printLSPDB();
                    delete inMsg;
                }
                break;
        case(L1_CSNP):
                if(this->isType == L1_TYPE || this->isType == L1L2_TYPE)
                {
                    this->handleL1CSNP(inMsg);
                    delete inMsg;
                }
                break;
        case(L1_PSNP):
                if(this->isType == L1_TYPE || this->isType == L1L2_TYPE)
                {
                    this->handleL1PSNP(inMsg);
                    delete inMsg;
                }
                break;
        default:
                delete inMsg;
                break;

        }
    }
}

/**
 * Set initial parameters of network interfaces.
 * @param entry Pointer to interface record in interfaceTable
 * @param intElement XML element of current interface in XML config file
 */
void ISIS::insertIft(InterfaceEntry *entry, cXMLElement *intElement)
{


    ISISinterface newIftEntry;
    newIftEntry.intID = entry->getInterfaceId();

    newIftEntry.gateIndex = entry->getNetworkLayerGateIndex();

    //set interface priority
    newIftEntry.priority = 64;  //default value

    if(intElement != NULL)
    {
        cXMLElement *priority = intElement->getFirstChildWithTag("IS-IS-Priority");
        if(priority != NULL && priority->getNodeValue() != NULL)
        {
            newIftEntry.priority = (unsigned char) atoi(priority->getNodeValue());
        }
    }



    //set interface metric

    newIftEntry.metric = 10;    //default value
    if(intElement != NULL)
    {
        cXMLElement *metric = intElement->getFirstChildWithTag("IS-IS-Metric");
        if(metric != NULL && metric->getNodeValue() != NULL)
        {
            newIftEntry.metric = (unsigned char) atoi(metric->getNodeValue());
        }
    }

    // activate IS-IS on this ift - default
    // future possible modification - ISIS on Ift will be activated only when enabled on interface in XML config (this is the way how it works in Cisco IOS)
    newIftEntry.ISISenabled = true;

    //set priority of current DIS = me at start
    newIftEntry.L1DISpriority = newIftEntry.priority;
    newIftEntry.L2DISpriority = newIftEntry.priority;

    //seet interface type according to global router configuration
    switch(this->isType)
    {
        case(L1_TYPE):
                 newIftEntry.circuitType = L1_TYPE;
                 break;
        case(L2_TYPE):
                 newIftEntry.circuitType = L2_TYPE;
                 break;
        //if router is type is equal L1L2, then interface configuration sets the type
        default:
        {
                if(intElement == NULL)
                {
                    newIftEntry.circuitType = L1L2_TYPE;
                }
                else
                {
                    cXMLElement *circuitType = intElement->getFirstChildWithTag("IsisCircuitType");
                    if(circuitType != NULL && circuitType->getNodeValue()!= NULL)
                    {
                        if(strcmp(circuitType->getNodeValue(),"L2") == 0)
                            newIftEntry.circuitType = L2_TYPE;
                        else
                        {
                            if(strcmp(circuitType->getNodeValue(),"L1") == 0)
                            newIftEntry.circuitType = L1_TYPE;
                        }
                    }
                    else
                    {
                        newIftEntry.circuitType = L1L2_TYPE;
                    }
                }
                break;
        }
    }

    //set initial designated IS as himself
    this->copyArrayContent((unsigned char*)this->sysId, newIftEntry.L1DIS, 6, 0, 0);
    //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
    newIftEntry.L1DIS[6] = entry->getInterfaceId() - 99;

    //do the same for L2 DIS
    this->copyArrayContent((unsigned char*)this->sysId, newIftEntry.L2DIS, 6, 0, 0);
    newIftEntry.L2DIS[6] = entry->getInterfaceId() - 99;

    newIftEntry.passive = false;
    newIftEntry.entry = entry;
    ISISIft.push_back(newIftEntry);
}


/**
 * Create hello packet and flood them out of every interface. This method handle
 * L1 and also L2 hello packets. Destination MAC address is broadcast (ff:ff:ff:ff:ff:ff).
 * Packets contain IS_NEIGHBOURS_HELLO and AREA_ADDRESS TLVs.
 */
void ISIS::sendHelloMsg()
{

    // create L1 and L2 hello packets

    ISISL1HelloPacket *helloL1 = new ISISL1HelloPacket("L1 Hello");
    ISISL2HelloPacket *helloL2 = new ISISL2HelloPacket("L2 Hello");

    //set circuit type field
    helloL1->setCircuitType(L1_TYPE);
    helloL2->setCircuitType(L2_TYPE);

    //set source id
    for(unsigned int i=0; i<6; i++)
    {
        helloL1->setSourceID(i,sysId[i]);
        helloL2->setSourceID(i,sysId[i]);
    }
    EV << endl;

    /* They should have separated Ethernet control info but OMNeT++ simulation
       doesn't recognize 01:80:c2:00:00:14 and 01:80:c2:00:00:15 as multicast OSI
       MAC addresses. Therefore destination MAC address is always set to broadcast
       ff:ff:ff:ff:ff:ff
    */

    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
    Ieee802Ctrl *ctrl2;

    // set DSAP & NSAP fields
    ctrl->setDsap(SAP_CLNS);
    ctrl->setSsap(SAP_CLNS);
    //set apprpriate destination MAC addresses

    MACAddress ma;
    ma.setAddress("ff:ff:ff:ff:ff:ff");

    ctrl->setDest(ma);


    ctrl2 = ctrl->dup();
    //assign Ethernet control info
    helloL1->setControlInfo(ctrl);
    helloL2->setControlInfo(ctrl2);

    //set TLVs
    TLV_t myTLV;
    //helloL1->setTLVArraySize(0);

    //set area address
    myTLV.type = AREA_ADDRESS;
    myTLV.length = 3;
    myTLV.value = new unsigned char[3];
    this->copyArrayContent((unsigned char *)areaId, myTLV.value, 3, 0, 0);
    helloL1->setTLVArraySize(1); //resize array
    helloL1->setTLV(0, myTLV);
    helloL2->setTLVArraySize(1); //resize array
    helloL2->setTLV(0, myTLV);

    //set NEIGHBOURS L1 TLV
    myTLV.type = IS_NEIGHBOURS_HELLO;
    myTLV.length = adjL1Table.size() * 6;   //number of records * 6 (6 is size of system ID/MAC address)
    myTLV.value = new unsigned char[adjL1Table.size() * 6];

    for(unsigned int h = 0; h<adjL1Table.size(); h++)
    {
        this->copyArrayContent(adjL1Table.at(h).mac.getAddressBytes(), myTLV.value, 6, 0, h*6);
    }
    helloL1->setTLVArraySize(2);
    helloL1->setTLV(1, myTLV);

    //L2 neighbours
    myTLV.type = IS_NEIGHBOURS_HELLO;
    myTLV.length = adjL2Table.size() * 6;   //number of records * 6 (6 is size of system ID/MAC address)
    myTLV.value = new unsigned char[adjL2Table.size() * 6];
    //L2 neighbours
    for(unsigned int h = 0; h<adjL2Table.size(); h++)
   {
       this->copyArrayContent(adjL2Table.at(h).mac.getAddressBytes(), myTLV.value, 6, 0, h*6);
   }

    helloL2->setTLVArraySize(2);
    helloL2->setTLV(1, myTLV);


    //PADDING TLV is omitted

    //TODO Authentication TLV

    //TODO add eventually another TLVs (eg. from RFC 1195)


    for(unsigned int k=0; k<ISISIft.size(); k++)
    {

        //don't send hello packets from passive interafaces
        if(!ISISIft.at(k).passive && ISISIft.at(k).ISISenabled)
        {
            // if this interface is DIS for LAN, hellos are sent 3-times faster (3.33sec instead of 10.0)
            // decision is made according to global hello counter (dirty hax - don't blame me pls, but i don't have time to code it nice way :)
            if(this->compareArrays((unsigned char*)this->sysId, ISISIft.at(k).L1DIS, 6) || (!(this->compareArrays((unsigned char*)this->sysId, ISISIft.at(k).L1DIS, 6)) && (this->helloCounter % 3 == 0)))
            {

                switch(ISISIft.at(k).circuitType)
                {
                    case(L1_TYPE):
                        {
                            //copy packet with control info
                            ISISL1HelloPacket *copy1 = helloL1->dup();
                            Ieee802Ctrl *ctrlCopy1 = ctrl->dup();
                            copy1->setControlInfo(ctrlCopy1);

                            //set LAN ID field (designated L1 IS)
                            for(unsigned int j=0; j<7; j++)
                            {
                                copy1->setLanID(j, ISISIft.at(k).L1DIS[j]);
                            }
                            copy1->setPriority(ISISIft.at(k).priority);
                            send(copy1, "ifOut", ISISIft.at(k).gateIndex);
                            EV << "'devideId " << deviceId << ": L1 Hello packet was sent from " <<
                            ISISIft.at(k).entry->getName() << "\n";
                            break;
                        }
                    case(L2_TYPE):
                        {    //copy packet with control info
                             ISISL2HelloPacket *copy2 = helloL2->dup();
                             Ieee802Ctrl *ctrlCopy2 = ctrl->dup();
                             copy2->setControlInfo(ctrlCopy2);

                             //set LAN ID field (designated L2 IS)
                             for(unsigned int j=0; j<7; j++)
                             {
                                 copy2->setLanID(j, ISISIft.at(k).L2DIS[j]);
                             }
                             copy2->setPriority(ISISIft.at(k).priority);
                             send(copy2, "ifOut", ISISIft.at(k).gateIndex);
                             EV << "deviceId " << deviceId << ": L2 Hello packet was sent from " <<
                             ISISIft.at(k).entry->getName() << "\n";
                             break;
                        }
                    default:
                        {
                             //send both - L1 & L2 hellos
                             ISISL1HelloPacket *copy = helloL1->dup();
                             Ieee802Ctrl *ctrlCopy = ctrl->dup();
                             copy->setControlInfo(ctrlCopy);
                             //set LAN ID field (designated L1 IS)
                             for(unsigned int j=0; j<7; j++)
                             {
                                 copy->setLanID(j, ISISIft.at(k).L1DIS[j]);
                             }
                             copy->setPriority(ISISIft.at(k).priority);
                             send(copy, "ifOut", ISISIft.at(k).gateIndex);
                             EV << "'devideId " << deviceId << ": L1 Hello packet was sent from " <<
                             ISISIft.at(k).entry->getName() << "\n";

                             ISISL2HelloPacket *copy2 = helloL2->dup();
                             Ieee802Ctrl *ctrlCopy2 = ctrl->dup();
                             copy2 = helloL2->dup();
                             ctrlCopy2 = ctrl->dup();
                             copy2->setControlInfo(ctrlCopy2);
                             //set LAN ID field (designated L2 IS)
                             for(unsigned int j=0; j<7; j++)
                             {
                                 copy2->setLanID(j, ISISIft.at(k).L2DIS[j]);
                             }
                             copy2->setPriority(ISISIft.at(k).priority);
                             send(copy2, "ifOut", ISISIft.at(k).gateIndex);
                             EV << "deviceId " << deviceId << ": L2 Hello packet was sent from " <<
                             ISISIft.at(k).entry->getName() << "\n";
                             break;
                        }
                }
            }
        }
    }


    //schedule new hello timer
    ISISTimer *timer = new ISISTimer("Hello_timer");
    timer->setTimerKind(HELLO_TIMER);
    scheduleAt(simTime() + 3.33, timer);

    //increment hello counter
    this->helloCounter++;

    //cleanup
    delete helloL1;
    delete helloL2;

}

/**
 * Parse NET address stored in this->netAddr into areaId, sysId and NSEL.
 * Method is used in initialization.
 * @see initialize(int stage)
 * @return Return true if NET address loaded from XML file is valid. Otherwise return false.
 */
bool ISIS::parseNetAddr()
{
    std::string net = netAddr;
    unsigned int dots = 0;
    size_t found;

    //net address (in this module - not according to standard O:-) MUST have this format:
    //49.0001.1921.6801.2003.00

    found=net.find_first_of(".");
    if(found != 2 || net.length()!=25)
    {
        return false;
    }


    unsigned char *area = new unsigned char[3];
    unsigned char *systemId = new unsigned char[6];
    unsigned char *nsel = new unsigned char[1];

    while (found!=string::npos)
    {

        switch(found){
            case 2:
                dots++;
                area[0] = (unsigned char)(atoi(net.substr(0,2).c_str()));
                break;
            case 7:
                dots++;
                area[1] = (unsigned char)(atoi(net.substr(3,2).c_str()));
                area[2] = (unsigned char)(atoi(net.substr(5,2).c_str()));
                break;
            case 12:
                dots++;
                systemId[0] = (unsigned char)(atoi(net.substr(8,2).c_str()));
                systemId[1] = (unsigned char)(atoi(net.substr(10,2).c_str()));
                break;
            case 17:
                dots++;
                systemId[2] = (unsigned char)(atoi(net.substr(13,2).c_str()));
                systemId[3] = (unsigned char)(atoi(net.substr(15,2).c_str()));
                break;
            case 22:
                dots++;
                systemId[4] = (unsigned char)(atoi(net.substr(18,2).c_str()));
                systemId[5] = (unsigned char)(atoi(net.substr(20,2).c_str()));
                break;
            default:
                return false;
                break;

        }

        found=net.find_first_of(".",found+1);
    }

    if(dots!=5)
    {
         return false;
    }

    nsel[0] = (unsigned char)(atoi(net.substr(23,2).c_str()));

    //49.0001.1921.6801.2003.00

    areaId = area;
    sysId = systemId;
    NSEL = nsel;

    return true;
}

/**
 * Handle L1 hello messages. Insert new neighbours into L1 adjacency table (this->adjL1Table) and
 * update status of existing neighbours.
 * @param inMsg incoming L1 hello packet
 */
void ISIS::handleL1HelloMsg(ISISMessage *inMsg)
{

    //duplicate system ID check
    if(this->checkDuplicateSysID(inMsg))
    {
        EV << this->deviceId << ": IS-IS WARNING: possible duplicate system ID ";
        for(unsigned g=0; g<6; g++)
        {
            EV << setfill('0') << setw(2) << dec << (unsigned int)sysId[g];
            if(g == 1 || g == 3)
                EV << ".";
        }
        EV<<" detected"<< endl;
    }

    ISISL1HelloPacket *msg = check_and_cast<ISISL1HelloPacket *>(inMsg);
    bool idMatch = false;

    unsigned int i;
    //walk through adjacency table and look for existing neighbours
    for(i=0;i<adjL1Table.size();i++)
    {

        bool found = true;
        //try to find match in system ID
        for(unsigned int j=0; j<msg->getSourceIDArraySize();j++)
        {
            if(msg->getSourceID(j) != adjL1Table.at(i).sysID[j])
                found = false;
        }

        //if we found match
        if(found)
        {
            idMatch = true;
            break;
        }
     }

    //if remote system ID is contained in adjL1Table
    if(idMatch)
    {
        //reset timer
        cancelEvent(adjL1Table.at(i).timer);
        scheduleAt(simTime() + msg->getHoldTime(), adjL1Table.at(i).timer);

        //update interface
        adjL1Table.at(i).gateIndex = msg->getArrivalGate()->getIndex();

        //check for DIS priority and eventually set new DI if needed; do it only if exists adjacency with state "UP"
        if(adjL1Table.at(i).state)
            electL1DesignatedIS(msg);

        //find neighbours TLV
        for(unsigned int j = 0; j< msg->getTLVArraySize(); j++)
        {
            //we found it!
            if(msg->getTLV(j).type == IS_NEIGHBOURS_HELLO)
            {
                unsigned char *tmpRecord = new unsigned char [6];
                //walk through all neighbour identifiers contained in TLV
                for(unsigned int r = 0; r < (msg->getTLV(j).length / 6); r++)
                {
                    //check if my system id is contained in neighbour's adjL1Table
                    this->copyArrayContent(msg->getTLV(j).value, tmpRecord, 6, r*6, 0);

                    //check if TLV contains one of my own MAC addresses
                    for(unsigned int o = 0; o<ISISIft.size(); o++)
                    {
                        MACAddress tmpMAC = ISISIft.at(o).entry->getMacAddress();
                        if(compareArrays(tmpMAC.getAddressBytes(), tmpRecord, 6))
                        {
                            //store previous state
                            bool changed = adjL1Table.at(i).state;
                            adjL1Table.at(i).state = true;

                            //if state changed, flood new LSP
                            if(changed!= adjL1Table.at(i).state && simTime()>35.0)
                                this->sendMyL1LSPs();

                            break;
                        }
                    }
                }
                delete tmpRecord;
                break;  //break finding cycle
            }
        }
    }

    //else create new record in adjL1Table
    else
    {
        //EV << "CREATING NEW ADJ RECORD\n";

        //find area ID TLV
        unsigned int k;
        for(k = 0; k<msg->getTLVArraySize(); k++)
        {
            //we found area ID TLV
            if(msg->getTLV(k).type == AREA_ADDRESS)
            {

                //if packet is L1 hello and area addresses match, then add new neighbour record to adjL1Table
                if(this->compareArrays((unsigned char *)areaId, msg->getTLV(k).value, 3))
                {
                    //create new neighbour record and set parameters
                    ISISadj neighbour;
                    neighbour.state = false;                    //set state to initial

                    //set timeout of neighbour
                    neighbour.timer = new ISISTimer("Neighbour_timeout");
                    neighbour.timer->setTimerKind(NEIGHBOUR_DEAD);
                    neighbour.timer->setIsType(L1_TYPE);
                    //set source system ID in neighbour record & in timer to identify it

                    for(unsigned int the_game = 0; the_game<msg->getSourceIDArraySize(); the_game++)
                    {
                        neighbour.sysID[the_game] = msg->getSourceID(the_game);
                        neighbour.timer->setSysID(the_game, msg->getSourceID(the_game));
                    }

                    //get source MAC address of received frame
                    Ieee802Ctrl *ctrl = check_and_cast <Ieee802Ctrl *> (msg->getControlInfo());
                    neighbour.mac = ctrl->getSrc();

                    //set gate index, which is neighbour connected to
                    neighbour.gateIndex = msg->getArrivalGate()->getIndex();

                    scheduleAt(simTime() + msg->getHoldTime(), neighbour.timer);

                    //insert neighbour into adjL1Table
                    adjL1Table.push_back(neighbour);

                    //EV << "deviceId " << deviceId << ": new adjacency\n";
                }

                break;  //end cycle
            }
        }
   }
}

/**
 * Handle L2 hello messages. Insert new neighbours into L2 adjacency table (this->adjL2Table) and
 * update status of existing neighbours.
 * @param inMsg incoming L2 hello packet
 */
void ISIS::handleL2HelloMsg(ISISMessage *inMsg)
{

    //duplicate system ID check
    if(this->checkDuplicateSysID(inMsg))
    {
        EV << this->deviceId << ": ISIS: possible duplicate system ID ";
        for(unsigned g=0; g<6; g++)
        {
            EV << setfill('0') << setw(2) << dec << (unsigned int)sysId[g];
            if(g == 1 || g == 3)
                EV << ".";
        }
        EV<<" detected"<< endl;
    }


    ISISL2HelloPacket *msg = check_and_cast<ISISL2HelloPacket *>(inMsg);
    bool idMatch = false;

    //check for DIS priority and eventually set new DI if needed

    //TODO enable L2 election when it's implemented ^_^
    //electL2DesignatedIS(msg);


    unsigned int i;
    //walk through adjacency table and look for existing L2 neighbours
    for(i=0;i<adjL2Table.size();i++)
    {

        bool found = true;
        //try to find match in system ID
        for(unsigned int j=0; j<msg->getSourceIDArraySize();j++)
        {
            if(msg->getSourceID(j) != adjL2Table.at(i).sysID[j])
                found = false;
        }

        //area ID must also match
        for(unsigned int j=0; j<msg->getTLVArraySize();j++)
        {
            if(msg->getTLV(j).type == AREA_ADDRESS)
            {
                if(!(this->compareArrays(msg->getTLV(j).value, adjL2Table.at(i).areaID, 3)))
                {
                    found = false;
                }
            }
        }

        //if we found match
        if(found)
        {
            idMatch = true;
            break;
        }
     }

    //if remote system ID is contained in adjL2Table
    if(idMatch)
    {

        //reset timer
        cancelEvent(adjL2Table.at(i).timer);
        scheduleAt(simTime() + msg->getHoldTime(), adjL2Table.at(i).timer);

        //update interface
        adjL2Table.at(i).gateIndex = msg->getArrivalGate()->getIndex();

        //find neighbours TLV
        for(unsigned int j = 0; j< msg->getTLVArraySize(); j++)
        {
            //we found it!
            if(msg->getTLV(j).type == IS_NEIGHBOURS_HELLO)
            {

                unsigned char *tmpRecord = new unsigned char [6];
                //walk through all neighbour identifiers contained in TLV

                //EV<< "TLV Length " << msg->getTLV(j).length <<endl;
                for(unsigned int r = 0; r < (msg->getTLV(j).length / 6); r++)
                {
                    //check if my system id is contained in neighbour's adjL1Table
                    this->copyArrayContent(msg->getTLV(j).value, tmpRecord, 6, r*6, 0);

                    EV << "MAC: ";
                    for(unsigned int hh = 0; hh < 6; hh++)
                    {
                        EV << setfill('0') << setw(2) << hex << (unsigned int)tmpRecord[hh] << ":";
                    }
                    EV <<endl;

                    //check if TLV contains one of my own MAC addresses
                    for(unsigned int o = 0; o<ISISIft.size(); o++)
                    {
                        MACAddress tmpMAC = ISISIft.at(o).entry->getMacAddress();
                        if(compareArrays(tmpMAC.getAddressBytes(), tmpRecord, 6))
                        {
                            adjL2Table.at(i).state = true;
                            break;
                        }
                    }
                }
                delete tmpRecord;
                break;  //break finding cycle
            }
        }
    }

    //else create new record in adjL1Table
    else
    {
        //EV << "CREATING NEW ADJ RECORD\n";

        //find area ID TLV
        unsigned int k;
        for(k = 0; k<msg->getTLVArraySize(); k++)
        {
            //we found area ID TLV
            //area IDs have to be different!!
            if(msg->getTLV(k).type == AREA_ADDRESS)
            {

                //create new neighbour record and set parameters
                ISISadj neighbour;
                neighbour.state = false;                    //set state to initial

                //set timeout of neighbour
                neighbour.timer = new ISISTimer("Neighbour_timeout");
                neighbour.timer->setTimerKind(NEIGHBOUR_DEAD);
                neighbour.timer->setIsType(L2_TYPE);
                //set source system ID in neighbour record & in timer to identify it

                //set neighbours system ID
                for(unsigned int the_game = 0; the_game<msg->getSourceIDArraySize(); the_game++)
                {
                    neighbour.sysID[the_game] = msg->getSourceID(the_game);
                    neighbour.timer->setSysID(the_game, msg->getSourceID(the_game));
                }

                //set neighbours area ID
                this->copyArrayContent(msg->getTLV(k).value, neighbour.areaID, msg->getTLV(k).length, 0, 0);

                for(unsigned int z=0; z<msg->getTLV(k).length; z++)
                {
                    neighbour.timer->setAreaID(z, msg->getTLV(k).value[z]);
                }

                //get source MAC address of received frame
                Ieee802Ctrl *ctrl = check_and_cast <Ieee802Ctrl *> (msg->getControlInfo());
                neighbour.mac = ctrl->getSrc();

                //set gate index, which is neighbour connected to
                neighbour.gateIndex = msg->getArrivalGate()->getIndex();

                scheduleAt(simTime() + msg->getHoldTime(), neighbour.timer);

                //insert neighbour into adjL1Table
                adjL2Table.push_back(neighbour);

                //EV << "deviceId " << deviceId << ": new adjacency\n";
                 break;  //end cycle
            }
        }
   }
}


/**
 * Print L1 and L2 adjacency tables to EV.
 * This function is currently called every time hello packet is received and processed.
 * @see handleMessage(cMessage* msg)
 */
void ISIS::printAdjTable()
{
    EV << "L1 adjacency table of IS ";

    //print area id
    for(unsigned int i=0; i<3; i++)
    {
        EV << setfill('0') << setw(2) << dec << (unsigned int)areaId[i];
        if(i % 2 == 0)
            EV << ".";

    }

    //print system id
    for(unsigned int i=0; i<6; i++)
    {
        EV << setfill('0') << setw(2) << dec << (unsigned int)sysId[i];
        if(i % 2 == 1)
            EV << ".";
    }

    //print NSEL
    EV << setfill('0') << setw(2) << dec << (unsigned int) NSEL[0] << "\tNo. of records in Table: " << adjL1Table.size() << endl;

    //print neighbour records
    for(unsigned int j=0; j<adjL1Table.size(); j++)
    {
        EV << "\t";
        //print neighbour system id
        for(unsigned int i=0; i<6; i++)
        {
            EV <<  setfill('0') << setw(2) << dec << (unsigned int)adjL1Table.at(j).sysID[i];
            if(i == 1 || i ==3)
                        EV << ".";
        }
        EV << "\t";


        //print neighbour MAC address
        for(unsigned int i=0; i<6; i++)
        {
            EV <<  setfill('0') << setw(2) << hex << (unsigned int)adjL1Table.at(j).mac.getAddressByte(i);
            if(i <5)
                 EV << ":";
        }
        EV << "\t";


        if(!adjL1Table.at(j).state)
            EV << "Init\n";
        else
            EV << "Up\n";
    }

    EV << "--------------------------------------------------------------------\n";

    EV << "L2 adjacency table of IS ";

        //print area id
        for(unsigned int i=0; i<3; i++)
        {
            EV << setfill('0') << setw(2) << dec << (unsigned int)areaId[i];
            if(i % 2 == 0)
                EV << ".";

        }

        //print system id
        for(unsigned int i=0; i<6; i++)
        {
            EV << setfill('0') << setw(2) << dec << (unsigned int)sysId[i];
            if(i % 2 == 1)
                EV << ".";
        }

        //print NSEL
        EV << setfill('0') << setw(2) << dec << (unsigned int) NSEL[0] << "\tNo. of records in Table: " << adjL2Table.size() << endl;

        //print neighbour records
        for(unsigned int j=0; j<adjL2Table.size(); j++)
        {
            EV << "\t";
            //print neighbour area id and system id
            for(unsigned int i=0; i<3; i++)
            {
                EV << setfill('0') << setw(2) << dec << (unsigned int)adjL2Table.at(j).areaID[i];
                if(i % 2 == 0)
                    EV << ".";

            }


            for(unsigned int i=0; i<6; i++)
            {
                EV <<  setfill('0') << setw(2) << dec << (unsigned int)adjL2Table.at(j).sysID[i];
                if(i == 1 || i ==3)
                            EV << ".";
            }
            EV << "\t";


            //print neighbour MAC address
            for(unsigned int i=0; i<6; i++)
            {
                EV <<  setfill('0') << setw(2) << hex << (unsigned int)adjL2Table.at(j).mac.getAddressByte(i);
                if(i <5)
                     EV << ":";
            }
            EV << "\t";


            if(!adjL2Table.at(j).state)
                EV << "Init\n";
            else
                EV << "Up\n";
        }
}


/**
 * Print content of L1 LSP database to EV. L2 print isn't implemented yet, because
 * handling of L2 LSP packets isn't implemented neither.
 */
void ISIS::printLSPDB()
{
    EV << "L1 LSP database of IS ";

    //print area id
    for(unsigned int i=0; i<3; i++)
    {
        EV << setfill('0') << setw(2) << dec << (unsigned int)areaId[i];
        if(i % 2 == 0)
            EV << ".";

    }

    //print system id
    for(unsigned int i=0; i<6; i++)
    {
        EV << setfill('0') << setw(2) << dec << (unsigned int)sysId[i];
        if(i % 2 == 1)
            EV << ".";
    }

    //print NSEL
    EV << setfill('0') << setw(2) << dec << (unsigned int) NSEL[0] << "\tNo. of records in database: " << L1LSP.size() << endl;


    for(unsigned int i=0;i<L1LSP.size();i++)
    {
        EV << "\t";
        //print LSP ID
        for(unsigned int j=0; j<8; j++)
        {
            EV <<  setfill('0') << setw(2) << dec << (unsigned int)L1LSP.at(i).LSPid[j];
            if(j == 1 || j == 3 || j == 5)
                        EV << ".";
            if(j==6)
                EV<<"-";
        }
        EV << "\t0x";

        //print sequence number
        EV << setfill('0') << setw(8) << hex << L1LSP.at(i).seq << endl;



        //print neighbours
        for(unsigned int k=0; k<L1LSP.at(i).neighbours.size(); k++)
        {
            EV <<"\t\t";
            for(unsigned int l=0; l<7; l++)
            {
                EV <<  setfill('0') << setw(2) << dec << (unsigned int)L1LSP.at(i).neighbours.at(k).LANid[l];
                if(l % 2 == 1)
                    EV << ".";
            }

            EV << "\tmetric: " << setfill('0') << setw(2) << dec << (unsigned int)L1LSP.at(i).neighbours.at(k).metrics.defaultMetric << endl;


        }

    }




    //TODO print L2 LSP DB
}


/**
 * Compare two unsigned char arrays and return result of comparison.
 * @param first First unsigned char array to be compared
 * @param second First unsigned char array to be compared
 * @param size Size of two arrays
 * @return True if arrays match, false otherwise.
 */
bool ISIS::compareArrays(unsigned char * first, unsigned char * second, unsigned int size)
{
    bool result = true;
    for(unsigned int i=0; i<size;i++)
    {
        if(first[i] != second[i])
            result = false;
    }

    return result;
}

/**
 * Copy content of one unsigned char array to another.
 * @param src Source array to be copied from.
 * @param dst Destination array to be copied to.
 * @param size Size in bytes co be copied.
 * @param startSrc Position in first array, where copying has to be started (usually 0).
 * @param startDst Position in second, where data are saved.
 */
void ISIS::copyArrayContent(unsigned char * src, unsigned char * dst, unsigned int size, unsigned int startSrc, unsigned int startDst)
{
    for(unsigned int i=0; i<size;i++)
    {
        dst[i+startDst] = src[i+startSrc];
    }
}

/**
 * Remove dead neighbour, when timer expires. Remove neighbour from adjacency table (L1 or L2)
 * and start new DIS election process.
 * @param msg Timer associated with dead neighbour.
 */
void ISIS::removeDeadNeighbour(ISISTimer *msg)
{

    //L1 neighbour dead
    // * remove from adjL1Table
    // * reset DIS on Ift
    if(msg->getIsType() == L1_TYPE)
    {
        for(unsigned int i=0; i<adjL1Table.size(); i++)
        {
            bool found = true;
            for(unsigned int j = 0; j<msg->getSysIDArraySize(); j++)
            {
                if(msg->getSysID(j) != adjL1Table.at(i).sysID[j])
                    found = false;
            }

            if(found)
            {
                adjL1Table.erase(adjL1Table.begin()+i);
            }
        }

        //clear LSP containing dead neighbour
        for(unsigned int i=0; i<L1LSP.size(); i++)
        {
            for(unsigned int j=0; j<msg->getSysIDArraySize(); j++)
            {
                bool found = true;
                if(msg->getSysID(j) != L1LSP.at(i).LSPid[j])
                    found = false;

                if(found)
                {
                    //mark with sequence number 0
                    L1LSP.at(i).seq = 0;
                    L1LSP.at(i).neighbours.clear();
                    //send empty LSP informing about expiration
                    this->sendSpecificL1LSP(L1LSP.at(i).LSPid);
                    //now completely delete
                    L1LSP.erase(L1LSP.begin() + i);
                }
            }
        }



        /* Start new designated IS election
           Better way to do this is to start election only on interface, to which was (now dead) neighbour connected to.
           Now, I don't have such association table available. So I reset DIS on every Ift. After receiving next hello
           packet, election will start.
           //TODO fix this dumb solution
        */
        this->resetDIS(L1_TYPE);

        //give network the time to converge again (I mean set the right designated routers)
        ISISTimer *LSPtimer =  new ISISTimer("Update L1 LSP DB");
        LSPtimer->setTimerKind(L1LSP_REFRESH);
        scheduleAt(simTime() + 25.0, LSPtimer); //25 seconds should be enough


    }

    //else it's L2 dead neighbour
    // * remove from adjL2Table
    else
    {
        for(unsigned int i=0; i<adjL2Table.size(); i++)
        {
            bool found = true;
            for(unsigned int j = 0; j<msg->getSysIDArraySize(); j++)
            {
                if(msg->getSysID(j) != adjL2Table.at(i).sysID[j])
                    found = false;
            }


            //area ID must also match
            for(unsigned int j=0; j<msg->getAreaIDArraySize();j++)
            {
                if(msg->getAreaID(j) != adjL2Table.at(i).areaID[j])
                {
                    found = false;
                }
            }

            if(found)
            {
                adjL2Table.erase(adjL2Table.begin()+i);
            }
        }

        //same as in L1 reset
        this->resetDIS(L2_TYPE);

    }

}


/**
 * Election of L1 Designated IS. Priorities are compared when received new
 * hello packet. Higher priority wins.
 * @see handleL1HelloMsg(ISISMessage *inMsg)
 * @param msg Received hello packet containing neighbours priority.
 */
void ISIS::electL1DesignatedIS(ISISL1HelloPacket *msg)
{

    Ieee802Ctrl *ctrl = check_and_cast <Ieee802Ctrl *> (msg->getControlInfo());

    unsigned int i;
    for(i=0; i<ISISIft.size(); i++)
    {


        if(ISISIft.at(i).gateIndex == msg->getArrivalGate()->getIndex())
        {
            //break the cycle, we have the right position stored at "i"
            break;
        }
    }


    bool equal = true;
    for(unsigned int k=0; k<msg->getLanIDArraySize(); k++)
    {
        if(msg->getLanID(k) != ISISIft.at(i).L1DIS[k])
            equal = false;
    }

    //if announced DIS priority is higher then actual one or if they are equal and src MAC is higher than mine, then it's time to update DIS
    if((!equal) &&
      ((msg->getPriority() > ISISIft.at(i).L1DISpriority) ||
      (msg->getPriority() == ISISIft.at(i).L1DISpriority && (ctrl->getSrc().compareTo(ISISIft.at(i).entry->getMacAddress()) == 1))))
    {
        for(unsigned int j=0; j< msg->getLanIDArraySize(); j++)
        {
            //set new DIS
            ISISIft.at(i).L1DIS[j] = msg->getLanID(j);
        }
        //and set his priority
        ISISIft.at(i).L1DISpriority = msg->getPriority();
    }
}




/**
 * Not implemented yet
 * @see electL1DesignatedIS(ISISL1HelloPacket *msg)
 */
void ISIS::electL2DesignatedIS(ISISL2HelloPacket *msg)
{

    //TODO implement
}

/**
 * Reset DIS on all interfaces. New election occurs after reset.
 * @see electL1DesignatedIS(ISISL1HelloPacket *msg)
 * @param IStype Defines IS type - L1 or L2
 */
void ISIS::resetDIS(short IStype)
{


    if(IStype == L1_TYPE || IStype == L1L2_TYPE)
    {
        for(unsigned int j = 0; j< ISISIft.size(); j++)
        {
            //set myself as DIS
            ISISIft.at(j).L1DISpriority = ISISIft.at(j).priority;
            //set initial designated IS as himself
            this->copyArrayContent((unsigned char*)this->sysId, ISISIft.at(j).L1DIS, 6, 0, 0);
            //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
            ISISIft.at(j).L1DIS[7] = ISISIft.at(j).entry->getInterfaceId() - 99 ;


        }
    }

    if(IStype == L2_TYPE || IStype == L1L2_TYPE)
    {
        for(unsigned int j = 0; j< ISISIft.size(); j++)
        {
            //set myself as DIS
            ISISIft.at(j).L2DISpriority = ISISIft.at(j).priority;
            //set initial designated IS as himself
            this->copyArrayContent((unsigned char*)this->sysId, ISISIft.at(j).L2DIS, 6, 0, 0);
            //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
            ISISIft.at(j).L2DIS[7] = ISISIft.at(j).entry->getInterfaceId() - 99;
        }
    }
}

/**
 * Flood L1 LSP packets containing whole link-state database to neighbours.
 * Set new timer (900s) after sending.
 * @see updateMyLSP()
 */
void ISIS::sendMyL1LSPs()
{
    //update my own LSPs
    this->updateMyLSP();

    ISISLSPL1Packet *LSP = new ISISLSPL1Packet("L1 LSP");

    //add Ethernet controll info
    Ieee802Ctrl *ctrl = new Ieee802Ctrl();

    // set DSAP & NSAP fields
    ctrl->setDsap(SAP_CLNS);
    ctrl->setSsap(SAP_CLNS);

    //set destination broadcast address
    //It should be multicast 01-80-C2-00-00-14 MAC address, but it doesn't work in OMNeT
    MACAddress ma;
    ma.setAddress("ff:ff:ff:ff:ff:ff");
    ctrl->setDest(ma);

    //set area address TLV
    TLV_t myTLV;
    myTLV.type = AREA_ADDRESS;
    myTLV.length = 3;
    myTLV.value = new unsigned char[3];
    this->copyArrayContent((unsigned char*)this->areaId, myTLV.value, 3, 0, 0);

    LSP->setTLVArraySize(1);
    LSP->setTLV(0, myTLV);

    /* - TODO Auth TLV
       - eventually implement ES neighbours TLV, but I don't think it's necessary
       - next TLVs from RFC 1195 if IP should be supported
    */

    for(unsigned int a=0; a<ISISIft.size(); a++)
    {
        if(!ISISIft.at(a).passive && ISISIft.at(a).ISISenabled && (ISISIft.at(a).circuitType == L1_TYPE || ISISIft.at(a).circuitType == L1L2_TYPE))
        {
            //flood my LSP on links
            for(unsigned int i=0; i<L1LSP.size(); i++)
            {
                //if I find LSP of my own, send it out
                if(this->compareArrays(L1LSP.at(i).LSPid, (unsigned char*)this->sysId, 6))
                {
                    ISISLSPL1Packet *LSPcopy = LSP->dup();
                    Ieee802Ctrl *ctrlCopy = ctrl->dup();
                    LSPcopy->setControlInfo(ctrlCopy);

                    //set LSP ID field
                    for(unsigned int j=0; j<LSPcopy->getLspIDArraySize(); j++)
                    {
                        LSPcopy->setLspID(j, L1LSP.at(i).LSPid[j]);
                    }

                    //set sequence number
                    LSPcopy->setSeqNumber(L1LSP.at(i).seq);


                    myTLV.type = IS_NEIGHBOURS_LSP;
                    myTLV.length = 1 + L1LSP.at(i).neighbours.size()*11;
                    myTLV.value = new unsigned char [myTLV.length];
                    myTLV.value[0] = 0;                                     //reserved byte

                    //set neighbours
                    for(unsigned int k=0; k<L1LSP.at(i).neighbours.size(); k++)
                    {
                        myTLV.value[(k*11)+1] = L1LSP.at(i).neighbours.at(k).metrics.defaultMetric;
                        myTLV.value[(k*11)+2] = L1LSP.at(i).neighbours.at(k).metrics.delayMetric;
                        myTLV.value[(k*11)+3] = L1LSP.at(i).neighbours.at(k).metrics.expenseMetric;
                        myTLV.value[(k*11)+4] = L1LSP.at(i).neighbours.at(k).metrics.errortMetric;
                        this->copyArrayContent(L1LSP.at(i).neighbours.at(k).LANid, myTLV.value, 7, 0, (k*11)+5); //set system ID
                    }

                    //assign TLV
                    LSPcopy->setTLVArraySize(2);
                    LSPcopy->setTLV(1, myTLV);

                    send(LSPcopy, "ifOut", ISISIft.at(a).gateIndex);
                }
            }
        }
    }


    //schedule refresh timer (after 900s)
    ISISTimer *timer = new ISISTimer("LSP_Refresh");
    timer->setTimerKind(L1LSP_REFRESH);
    scheduleAt(simTime() + 900.0, timer);

    delete LSP;
}

/**
 * Not implemented yet
 * @see sendMyL1LSPs()
 */
void ISIS::sendMyL2LSPs()
{
    //TODO
}

/**
 * Handle received L1 LSP packet.
 * Create new LSP records in L1 link-state database (this->L1LSP) for each
 * additional LSP contained in TLV of LSP packet. Update existing LSP records
 * (update sequence number and content). Flood packet further to all other
 * neighbours if seq number of LSP > mine LSP seq number in link-state db.
 * If packet LSP seq num < mine LSP seq number, flood mine version of LSP
 * to all neighbours. If packet LSP seq num == 0, reflood packet further
 * and delete dead LSP.
 * @param inMsg received L1 LSP packet
 */
void ISIS::handleL1LSP(ISISMessage * inMsg)
{
    ISISLSPL1Packet *msg = check_and_cast<ISISLSPL1Packet *>(inMsg);

    //check if area IDs match
    for(unsigned int i = 0; i<msg->getTLVArraySize(); i++)
    {
        if(msg->getTLV(i).type == AREA_ADDRESS && this->compareArrays((unsigned char *) this->areaId, msg->getTLV(i).value, msg->getTLV(i).length))
        {

            //area address is OK
            //try to find LSDP ID in L1 LSP DB
            bool match = false;
            unsigned int j;
            for(j=0; j<L1LSP.size(); j++)
            {
                bool found = true;
                //compare LSP IDs
                for(unsigned int k=0; k<msg->getLspIDArraySize(); k++)
                {
                    if(msg->getLspID(k) != L1LSP.at(j).LSPid[k])
                        found = false;
                }

                if(found)
                {
                    match = true;
                    break;

                }
            }

            //update record
            if(match)
            {

                //update record only if we receiver LSP with higher sequence number
                if(msg->getSeqNumber() > L1LSP.at(j).seq)
                {

                    //update timer
                    cancelEvent(L1LSP.at(j).deadTimer);
                    scheduleAt(simTime() + msg->getRemLifeTime(), L1LSP.at(j).deadTimer);   //should be 1200 secs.

                    //update sequence number
                    L1LSP.at(j).seq = msg->getSeqNumber();

                    //update neighbour records
                    std::vector<LSPneighbour> neighbours;

                    //find IS_NEIGHBOURS_LSP  TLV
                    for(unsigned int a = 0; a<msg->getTLVArraySize(); a++)
                    {
                        if(msg->getTLV(a).type == IS_NEIGHBOURS_LSP)
                        {

                            unsigned int size = (msg->getTLV(a).length - 1)/11;
                            for(unsigned int b=0; b<size; b++)
                            {
                                //neighbour record
                                LSPneighbour neighbour;

                                //set metrics
                                neighbour.metrics.defaultMetric = msg->getTLV(a).value[(b*11)+1];
                                neighbour.metrics.delayMetric = msg->getTLV(a).value[(b*11)+2];
                                neighbour.metrics.expenseMetric = msg->getTLV(a).value[(b*11)+3];
                                neighbour.metrics.errortMetric = msg->getTLV(a).value[(b*11)+4];


                                //copy LAN id
                                this->copyArrayContent(msg->getTLV(a).value, neighbour.LANid, 7, (b*11)+5, 0 );

                                //store to neighbours vector
                                neighbours.push_back(neighbour);
                            }

                            L1LSP.at(j).neighbours = neighbours;

                            break;
                        }
                    }

                    //flood msg further to other neighbours
                    this->floodFurtherL1LSP(msg);
                }
                else
                {
                    //if seqNumber is zero, purge that LSP and flood empty LSP ID with seqNumber 0 to other neighbour, so they can purge it too
                    if(msg->getSeqNumber() == 0)
                    {
                        //delete LSP
                        L1LSP.erase(L1LSP.begin() + j);
                        //send further message informing about LSP death
                        this->floodFurtherL1LSP(msg);
                    }
                    else
                    {
                        //we have received older version of LSP than we have in L1LSP DB
                        //our task is to transmit our newer version of LSP to all neighbours
                        if(msg->getSeqNumber() < L1LSP.at(j).seq)
                        {
                            this->sendSpecificL1LSP(L1LSP.at(j).LSPid);
                        }
                    }
                }
            }
            else    //create new LSP record
            {

                //don't create already dead LSP
                if(msg->getSeqNumber() > 0)
                {

                    //set timer
                    LSPrecord record;
                    record.deadTimer = new ISISTimer("L1 LSP dead");
                    record.deadTimer->setTimerKind(L1LSP_DEAD);

                    //set timer LSP ID and record LSP ID
                    for(unsigned int a=0; a<msg->getLspIDArraySize(); a++)
                    {
                        record.LSPid[a] = msg->getLspID(a);
                        record.deadTimer->setLSPid(a, msg->getLspID(a));
                    }
                    scheduleAt(simTime() + msg->getRemLifeTime(), record.deadTimer);

                    //set sequence number
                    record.seq = msg->getSeqNumber();

                    //find IS_NEIGHBOURS_LSP  TLV
                    for(unsigned int a = 0; a<msg->getTLVArraySize(); a++)
                    {
                        if(msg->getTLV(a).type == IS_NEIGHBOURS_LSP)
                        {

                            unsigned int size = (msg->getTLV(a).length - 1)/11;
                            for(unsigned int b=0; b<size; b++)
                            {
                                //neighbour record
                                LSPneighbour neighbour;

                                //set metrics
                                neighbour.metrics.defaultMetric = msg->getTLV(a).value[(b*11)+1];
                                neighbour.metrics.delayMetric = msg->getTLV(a).value[(b*11)+2];
                                neighbour.metrics.expenseMetric = msg->getTLV(a).value[(b*11)+3];
                                neighbour.metrics.errortMetric = msg->getTLV(a).value[(b*11)+4];


                                //copy LAN id
                                this->copyArrayContent(msg->getTLV(a).value, neighbour.LANid, 7, (b*11)+5, 0 );

                                //store to neighbours vector
                                record.neighbours.push_back(neighbour);
                            }

                            L1LSP.push_back(record);

                            break;
                        }
                    }

                    this->floodFurtherL1LSP(msg);
                }
            }
        }
    }
}

/**
 * Not implemented yet
 * @see handleL1LSP(ISISMessage * inMsg)
 */
void ISIS::handleL2LSP(ISISMessage * msg)
{
    //TODO
}

/**
 * Send CSNP packet to each LAN, where this router represents DIS.
 * Repeat every 10 seconds.
 */
void ISIS::sendL1CSNP()
{
    //update my own LSPs

    ISISCSNPL1Packet *packet = new ISISCSNPL1Packet("L1 CSNP");

    //add Ethernet controll info
    Ieee802Ctrl *ctrl = new Ieee802Ctrl();

    // set DSAP & NSAP fields
    ctrl->setDsap(SAP_CLNS);
    ctrl->setSsap(SAP_CLNS);

    //set destination broadcast address
    //It should be multicast 01-80-C2-00-00-14 MAC address, but it doesn't work in OMNeT
    MACAddress ma;
    ma.setAddress("ff:ff:ff:ff:ff:ff");
    ctrl->setDest(ma);

    //set system ID field which consists of my system id + zero circuit id inc this case
    for(unsigned int i=0; i<packet->getSourceIDArraySize()-1; i++)
    {
        packet->setSourceID(i, this->sysId[i]);
    }
    packet->setSourceID(6, 0);

    //set start LSP ID to zeros and end LSP ID to max value
    for(unsigned int i=0; i<packet->getStartLspIDArraySize(); i++)
    {
        packet->setStartLspID(i,0);
        packet->setEndLspID(i,255);
    }

    //set area address TLV
    TLV_t myTLV;
    myTLV.type = LSP_ENTRIES;
    myTLV.length = this->L1LSP.size()*14;
    myTLV.value = new unsigned char[myTLV.length];
    /*
     * Value— Multiples of LSP summaries, each consisting of the remaining lifetime (2 bytes), LSP ID (ID length + 2 bytes),
     * LSP sequence number(4 bytes), and LSP checksum (2 bytes). But we ignore LSP checksum so length of each LSP record in CSNP is
     * 2 bytes smaller.
     */
    for(unsigned int i=0;i<L1LSP.size(); i++)
    {
        //get remaining lifetime
        unsigned short remTime = 1200 - ((unsigned short)(simTime().dbl()) - (unsigned short)(L1LSP.at(i).deadTimer->getCreationTime().dbl()));

        //convert unsigned short to unsigned char array and insert to TLV
        myTLV.value[(i*14)] = ((remTime >> 8) & 0xFF);
        myTLV.value[(i*14)+1] = (remTime & 0xFF);

        //copy LSP ID to TLV
        this->copyArrayContent(L1LSP.at(i).LSPid, myTLV.value, 8, 0, (i*14)+2);

        //convert unsigned long seq number to unsigned char array[4] and insert into TLV
        for(unsigned int j=0; j<4; j++)
        {
            myTLV.value[(i*14)+10+j] = (L1LSP.at(i).seq >> (24-(8*j))) & 0xFF;
        }
    }

    packet->setTLVArraySize(1);
    packet->setTLV(0, myTLV);


    /* - TODO Auth TLV
       - eventually implement ES neighbours TLV, but I don't think it's necessary
       - next TLVs from RFC 1195 if IP should be supported in future
    */

    //walk through all itnerfaces
    for(unsigned int a=0; a<ISISIft.size(); a++)
    {
        //if interface status meets condition
        if(!ISISIft.at(a).passive && ISISIft.at(a).ISISenabled && (ISISIft.at(a).circuitType == L1_TYPE || ISISIft.at(a).circuitType == L1L2_TYPE))
        {
            //check if this Ift represents DIS on associated LAN
            unsigned char myLANid[7];
            this->copyArrayContent((unsigned char *)this->sysId, myLANid, 7, 0, 0);
            myLANid[6] = ISISIft.at(a).gateIndex + 1;

            //if they match, send CSNP packet to that LAN
            if(this->compareArrays(myLANid, ISISIft.at(a).L1DIS, 7))
            {
                ISISCSNPL1Packet *packetCopy = packet->dup();
                Ieee802Ctrl *ctrlCopy = ctrl->dup();
                packetCopy->setControlInfo(ctrlCopy);
                send(packetCopy, "ifOut", ISISIft.at(a).gateIndex);
            }
        }
    }

    //set new CNSP timer (10s)
    ISISTimer *CSNPtimer =  new ISISTimer("Send CSNP packets");
    CSNPtimer->setTimerKind(CSNP_TIMER);
    scheduleAt(simTime() + 10.0, CSNPtimer);

    delete packet;
}

/**
 * Not implemented yet
 */
void ISIS::sendL2CSNP()
{
    //TODO
}

/**
 * Handle received L1 CSNP packet. Check for existing LSPs in LSP table.
 * Those with lower sequence number number or missing are asked for using
 * PSNP packets.
 * @param inMsg recived L1 CSNP packet
 */
void ISIS::handleL1CSNP(ISISMessage * inMsg)
{
    ISISCSNPL1Packet *msg = check_and_cast<ISISCSNPL1Packet *>(inMsg);

    for(unsigned int i=0; i<msg->getTLVArraySize(); i++)
    {
        //find LSP_ENTRIES TLV
        if(msg->getTLV(i).type == LSP_ENTRIES)
        {

            /*
             * TLV Value— Multiples of LSP summaries, each consisting of the remaining lifetime (2 bytes), LSP ID (ID length + 2 bytes),
             * LSP sequence number(4 bytes), and LSP checksum (2 bytes). But we ignore LSP checksum so length of each LSP record in CSNP is
             * 2 bytes smaller, which give us 14 bytes of data per LSP entry.
             */

            //list of LSP to be asked for using PSNP packets
            std::vector<unsigned char *> LSPlist;

            //parse each LSP entry separatly
            for(unsigned int j=0; j<msg->getTLV(i).length / 14; j++)
            {
                unsigned char *lspEntry  = new unsigned char[14];
                this->copyArrayContent(msg->getTLV(i).value, lspEntry, 14, j*14, 0);

                //ignore time remaining for now
                //copy and compare for existing LSP id in L1LSP
                unsigned char lspid[8];
                this->copyArrayContent(lspEntry, lspid, 8, 2, 0);

                bool found = false;
                unsigned int k;
                for(k=0; k<L1LSP.size(); k++)
                {
                    if(this->compareArrays(lspid, L1LSP.at(k).LSPid, 8))
                    {
                        found = true;
                        break;

                    }
                }

                //LSP ID exists
                if(found)
                {
                    //compare sequence numbers
                    //we must built sequence number from received unsigned char array using bitshifting
                    unsigned long seqNum = (( lspEntry[10] << 24)
                            + (lspEntry[11] << 16)
                            + (lspEntry[12] << 8)
                            + (lspEntry[13] ));


                    //request update if DIS is holding newer version of LSP
                    if(seqNum > L1LSP.at(k).seq)
                    {
                         LSPlist.push_back(lspEntry);
                    }
                    else
                    {
                        //In case I received LSP with older seq number than I have stored in my db,
                        //flood my newer version
                        if(seqNum < L1LSP.at(k).seq)
                            this->sendSpecificL1LSP(lspid);
                    }
                }

                //LSP entry not found
                else
                {
                    LSPlist.push_back(lspEntry);
                }
            }


            //send PSNP packets if I have missing/outdated LSP entry/entries
            if(LSPlist.size() > 0)
            {
                this->sendL1PSNP(&LSPlist, msg->getArrivalGate()->getIndex());
            }

            break;
        }
    }
}

/**
 * Not implemented yet
 */
void ISIS::handleL2CSNP(ISISMessage * msg)
{
    //TODO
}

/**
 * Send PSNP packet after being found, that some LSP entry/entries are outdated or
 * missing. Packet consists of LSP list to be asked for.
 * @see handleL1CSNP(ISISMessage * inMsg)
 * @param gateIndex index of gate which is received CSNP packet came from (we will send PSNP packet out from this ift)
 * @param LSPlist list of LSP entries to be asked for
 */
void ISIS::sendL1PSNP(std::vector<unsigned char *> * LSPlist, int gateIndex)
{
    ISISPSNPL1Packet *packet = new ISISPSNPL1Packet("L1 PSNP");

    //add Ethernet controll info
    Ieee802Ctrl *ctrl = new Ieee802Ctrl();

    // set DSAP & NSAP fields
    ctrl->setDsap(SAP_CLNS);
    ctrl->setSsap(SAP_CLNS);

    //set destination broadcast address
    //It should be multicast 01-80-C2-00-00-14 MAC address, but it doesn't work in OMNeT
    MACAddress ma;
    ma.setAddress("ff:ff:ff:ff:ff:ff");
    ctrl->setDest(ma);

    packet->setControlInfo(ctrl);

    //set system ID field which consists of my system id + zero circuit id in this case
    for(unsigned int i=0; i<packet->getSourceIDArraySize()-1; i++)
    {
        packet->setSourceID(i, this->sysId[i]);
    }
    packet->setSourceID(6, 0);

    //set area address TLV
    TLV_t myTLV;
    myTLV.type = LSP_ENTRIES;
    myTLV.length = LSPlist->size()*14;
    myTLV.value = new unsigned char[myTLV.length];
    /*
     * Value— Multiples of LSP summaries, each consisting of the remaining lifetime (2 bytes), LSP ID (ID length + 2 bytes),
     * LSP sequence number(4 bytes), and LSP checksum (2 bytes). But we ignore LSP checksum so length of each LSP record in CSNP is
     * 2 bytes smaller.
     */
    for(unsigned int i=0;i<LSPlist->size(); i++)
    {
        this->copyArrayContent(LSPlist->at(i), myTLV.value, 14, 0, i*14);
    }

    packet->setTLVArraySize(1);
    packet->setTLV(0, myTLV);


    /* - TODO Auth TLV
       - eventually implement ES neighbours TLV, but I don't think it's necessary
       - next TLVs from RFC 1195 if IP should be supported in future
    */

    //send packet to same interface as CSNP packet came in
    //DIS should receive this packet and handle it
    send(packet, "ifOut", gateIndex);
}

/**
 * Not implemented yet
 */
void ISIS::sendL2PSNP()
{
    //TODO
}

/**
 * Handle PSNP received from another IS. Content of packet informs about LSPs which have to be sent.
 * @param inMsg received PSNP packet
 * @see sendL1PSNP(std::vector<unsigned char *> * LSPlist, int gateIndex)
 * @see handleL1CSNP(ISISMessage * inMsg)
 */
void ISIS::handleL1PSNP(ISISMessage * inMsg)
{
    ISISPSNPL1Packet *msg = check_and_cast<ISISPSNPL1Packet *>(inMsg);


    //PSNP packets should process only DIS, therefore we need to check this

    //find interface which packet came from
    for(unsigned int i=0; i<ISISIft.size(); i++)
    {
        //we found dat interface
        if(ISISIft.at(i).gateIndex == msg->getArrivalGate()->getIndex())
        {
            unsigned char LanID[7];

            //set LAN ID which consists of system ID + pseudonode ID
            this->copyArrayContent((unsigned char *)this->sysId, LanID, 6, 0, 0);   //set system ID
            LanID[6] = ISISIft.at(i).gateIndex + 1;    //set pseudonode ID

            //compare Ift's DIS with LAN ID
            //if they don't match, I'm not DIS on dat LAN and no processing of PSNP packet is necessary
            if(!(this->compareArrays(LanID, ISISIft.at(i).L1DIS, 7)))
                return;
        }
    }

    for(unsigned int i=0; i<msg->getTLVArraySize(); i++)
    {
        //find LSP_ENTRIES TLV
        if(msg->getTLV(i).type == LSP_ENTRIES)
        {

            /*
             * TLV Value— Multiples of LSP summaries, each consisting of the remaining lifetime (2 bytes), LSP ID (ID length + 2 bytes),
             * LSP sequence number(4 bytes), and LSP checksum (2 bytes). But we ignore LSP checksum so length of each LSP record in CSNP is
             * 2 bytes smaller, which give us 14 bytes of data per LSP entry.
             */

            //parse each LSP entry separately
            for(unsigned int j=0; j<msg->getTLV(i).length / 14; j++)
            {
                unsigned char lspEntry[14];
                this->copyArrayContent(msg->getTLV(i).value, lspEntry, 14, j*14, 0);

                //ignore time remaining for now
                //copy and compare for existing LSP id in L1LSP
                unsigned char lspid[8];
                this->copyArrayContent(lspEntry, lspid, 8, 2, 0);

                //send LSP packet containing relevant data
                this->sendSpecificL1LSP(lspid);
            }
        }

        break;
    }
}

/**
 * Not implemented yet
 */
void ISIS::handleL2PSNP(ISISMessage * msg)
{
    //TODO
}

/**
 * Check sysId from received hello packet for duplicity.
 * @return True if neighbour's sysId is the same sa mine, false otherwise.
 */
bool ISIS::checkDuplicateSysID(ISISMessage * msg)
{

    bool test = false;

    if(msg->getType() == LAN_L1_HELLO || msg->getType() == LAN_L2_HELLO)
    {

        // typecast for L1 hello; L1 and L2 hellos differ only in "type" field
        ISISL1HelloPacket *hello = check_and_cast<ISISL1HelloPacket *>(msg);

        //check for area address tlv and compare with my area id
        for(unsigned int j=0; j<hello->getTLVArraySize(); j++)
        {
            if(hello->getTLV(j).type == AREA_ADDRESS && this->compareArrays((unsigned char *) this->areaId, hello->getTLV(j).value, hello->getTLV(j).length))
            {

                bool equal = true;

                //compare sys ID
                for(unsigned int i=0; i<hello->getSourceIDArraySize(); i++)
                {
                    if(this->sysId[i] != hello->getSourceID(i))
                        equal = false;
                }

                test = equal;
                break;

            }
        }

    }

    return test;
}

/**
 * Remove LSP which wasn't refreshed for 900 sec. Flood LSP packet with
 * sequence ID = 0 indicating dead LSP.
 * @param timer Timer associated with dead LSP
 */
void ISIS::removeDeadLSP(ISISTimer *timer)
{
    for(unsigned int i=0; i<L1LSP.size(); i++)
    {
        //find dead LSP ID
        bool found = true;
        for(unsigned int j=0; j<timer->getLSPidArraySize(); j++)
        {
            if(timer->getLSPid(j) != L1LSP.at(i).LSPid[j])
                found = false;
        }

        //we found it!
        if(found)
        {

            //mark with sequence number 0
            L1LSP.at(i).seq = 0;
            L1LSP.at(i).neighbours.clear();
            //send empty LSP informing about expiration
            this->sendSpecificL1LSP(L1LSP.at(i).LSPid);
            //now completely delete
            L1LSP.erase(L1LSP.begin() + i);
            break;
        }
    }
}

/**
 * Create or update my own LSP.
 * @see ISIS::sendMyL1LSPs()
 */
void ISIS::updateMyLSP()
{
    unsigned char myLSPid[8];

    //at first, we generate (or update existing) non-psudonode LSP with pesudonode ID (myLSPid[6]) equal to 0 according to ISO10859
    //myLSPid[7] is always 0 because omnet doesn't support packet fragmentation (or I don't want to solve that :D)
    this->copyArrayContent((unsigned char*)this->sysId, myLSPid, 6, 0, 0);
    myLSPid[6] = 0; myLSPid[7] = 0;


    bool found = false;

    //we try to find existing LSP with this ID
    for(unsigned int j=0; j<L1LSP.size(); j++)
    {
        //if we have found matching LSP ID
        if(this->compareArrays(myLSPid, L1LSP.at(j).LSPid, 8))
        {
            //update it

            //increment sequence number
            L1LSP.at(j).seq++;

            //reset dead timer
            cancelEvent(L1LSP.at(j).deadTimer);
            scheduleAt(simTime() + 1200, L1LSP.at(j).deadTimer);   //should be 1200 secs.

            //set neighbours (pseudonode neighbours)
            std::vector<LSPneighbour> neighbours;

            //as we are using ethernet, which is multiaccess medium, we have to add pseudonodes as IS neighbours
            //at this point, network should be converged and assigned appropriate designated IS for each LAN

            for(unsigned int i=0; i<ISISIft.size(); i++)
            {
                if(ISISIft.at(i).ISISenabled && (ISISIft.at(i).circuitType == L1_TYPE || ISISIft.at(i).circuitType == L1L2_TYPE))
                {
                    LSPneighbour neighbour;
                    this->copyArrayContent(ISISIft.at(i).L1DIS, neighbour.LANid, 7, 0, 0);

                    //find interface which is DIS connected to and set metrics
                    neighbour.metrics.defaultMetric = ISISIft.at(i).metric;     //default = 10
                    neighbour.metrics.delayMetric = 128;    //disabled;
                    neighbour.metrics.expenseMetric = 128;  //disabled
                    neighbour.metrics.errortMetric = 128;   //disabled

                    neighbours.push_back(neighbour);
                }
            }

            //replace old neighbours
            L1LSP.at(j).neighbours = neighbours;


            found = true;
            break;
        }
    }

    //if LSP record doesn't exist, create it
    if(!found)
    {
        LSPrecord record;

        //set LSP ID
        this->copyArrayContent(myLSPid, record.LSPid, 8, 0, 0);

        //set dead timer
        record.deadTimer = new ISISTimer("L1 LSP dead");
        record.deadTimer->setTimerKind(L1LSP_DEAD);
        for(unsigned int x=0; x<record.deadTimer->getLSPidArraySize(); x++)
        {
            record.deadTimer->setLSPid(x, myLSPid[x]);
        }
        scheduleAt(simTime() + 1200.0, record.deadTimer);

        //initial sequence number = 1
        record.seq = 1;

        //set pseudonode neighbours
        std::vector<LSPneighbour> neighbours;
        for(unsigned int i=0; i<ISISIft.size(); i++)
        {
            if(ISISIft.at(i).ISISenabled && (ISISIft.at(i).circuitType == L1_TYPE || ISISIft.at(i).circuitType == L1L2_TYPE))
            {
                for(unsigned int f=0; f< this->adjL1Table.size(); f++)
                {
                    if(ISISIft.at(i).gateIndex == adjL1Table.at(f).gateIndex && adjL1Table.at(f).state)
                    {
                        LSPneighbour neighbour;
                        this->copyArrayContent(ISISIft.at(i).L1DIS, neighbour.LANid, 7, 0, 0);

                        //find interface which is DIS connected to and set metrics
                        neighbour.metrics.defaultMetric = ISISIft.at(i).metric;     //default = 10
                        neighbour.metrics.delayMetric = 128;    //disabled;
                        neighbour.metrics.expenseMetric = 128;  //disabled
                        neighbour.metrics.errortMetric = 128;   //disabled

                        neighbours.push_back(neighbour);
                        break;
                    }
                }
            }
        }

        if(neighbours.size() > 0)
        {
            record.neighbours = neighbours;
            L1LSP.push_back(record);
        }
     }


    //end of non-pseudonode LSP
    //####################################################################################################
    //start of pseudonode LSP

    for(unsigned int i=0; i<ISISIft.size(); i++)
    {
        //check if this interface is DIS for LAN
        if(this->compareArrays((unsigned char*) this->sysId, ISISIft.at(i).L1DIS, 6))
        {
            //check if LSP ID exists; update neighbours if record exists; create new record otherwise
            //I have already my system id contained in myLSPid; we have to set only pseudonode ID byte

            found = false;
            myLSPid[6] = ISISIft.at(i).L1DIS[6];
            for(unsigned int j=0; j< L1LSP.size(); j++)
            {
                //we found the match
                if(this->compareArrays(myLSPid, L1LSP.at(j).LSPid, 8))
                {

                    //update sequence number
                    L1LSP.at(j).seq++;

                    //reset dead timer
                    cancelEvent(L1LSP.at(j).deadTimer);
                    scheduleAt(simTime() + 1200, L1LSP.at(j).deadTimer);   //should be 1200 secs.

                    //set neighbours (NON-pseudonode neighbours)
                    std::vector<LSPneighbour> neighbours;

                    //set every adjacent IS as neighbour
                    for(unsigned int k=0; k<adjL1Table.size(); k++)
                    {
                        //consider only directly connected neighbours in "UP" state
                        if(adjL1Table.at(k).state && adjL1Table.at(k).gateIndex == ISISIft.at(i).gateIndex)
                        {
                            LSPneighbour neighbour;
                            this->copyArrayContent(adjL1Table.at(k).sysID, neighbour.LANid, 6, 0, 0);
                            neighbour.LANid[6] = 0;
                            neighbour.metrics.defaultMetric = 0;     //metric to every neighbour in pseudonode LSP is always zero!!!
                            neighbour.metrics.delayMetric = 128;    //disabled;
                            neighbour.metrics.expenseMetric = 128;  //disabled
                            neighbour.metrics.errortMetric = 128;   //disabled

                            neighbours.push_back(neighbour);
                        }
                    }

                    //add also mine non-pseudonode interface as neighbour
                    LSPneighbour neighbour;
                    this->copyArrayContent((unsigned char*)this->sysId, neighbour.LANid, 6, 0, 0);
                    neighbour.LANid[6] = 0;
                    neighbour.metrics.defaultMetric = 0;     //metric to every neighbour in pseudonode LSP is always zero!!!
                    neighbour.metrics.delayMetric = 128;    //disabled;
                    neighbour.metrics.expenseMetric = 128;  //disabled
                    neighbour.metrics.errortMetric = 128;   //disabled

                    neighbours.push_back(neighbour);

                    //replace old neighbours
                    L1LSP.at(j).neighbours = neighbours;

                    found = true;
                    break;
                }
            }


            //create new pseudonode LSP record
            if(!found)
            {
                LSPrecord record;

                //set LSP ID
                this->copyArrayContent(myLSPid, record.LSPid, 8, 0, 0);

                //set dead timer
                record.deadTimer = new ISISTimer("L1 LSP dead");
                record.deadTimer->setTimerKind(L1LSP_DEAD);
                for(unsigned int x=0; x<record.deadTimer->getLSPidArraySize(); x++)
                {
                    record.deadTimer->setLSPid(x, myLSPid[x]);
                }
                scheduleAt(simTime() + 1200.0, record.deadTimer);

                //initial sequence number = 1
                record.seq = 1;

                //set neighbours (NON-pseudonode neighbours)
                std::vector<LSPneighbour> neighbours;


                //set every adjacent IS as neighbour
                for(unsigned int k=0; k<adjL1Table.size(); k++)
                {
                    //consider only directly connected neighbours in "UP" state
                    if(adjL1Table.at(k).state && adjL1Table.at(k).gateIndex == ISISIft.at(i).gateIndex)
                    {
                        LSPneighbour neighbour;
                        this->copyArrayContent(adjL1Table.at(k).sysID, neighbour.LANid, 6, 0, 0);
                        neighbour.LANid[6] = 0;
                        neighbour.metrics.defaultMetric = 0;     //metric to every neighbour in pseudonode LSP is always zero!!!
                        neighbour.metrics.delayMetric = 128;    //disabled;
                        neighbour.metrics.expenseMetric = 128;  //disabled
                        neighbour.metrics.errortMetric = 128;   //disabled

                        neighbours.push_back(neighbour);
                    }
                }

                if(neighbours.size() > 0)
                {
                    //add also mine non-pseudonode interface as neighbour
                    LSPneighbour neighbour;
                    this->copyArrayContent((unsigned char*)this->sysId, neighbour.LANid, 6, 0, 0);
                    neighbour.LANid[6] = 0;
                    neighbour.metrics.defaultMetric = 0;     //metric to every neighbour in pseudonode LSP is always zero!!!
                    neighbour.metrics.delayMetric = 128;    //disabled;
                    neighbour.metrics.expenseMetric = 128;  //disabled
                    neighbour.metrics.errortMetric = 128;   //disabled

                    neighbours.push_back(neighbour);

                    record.neighbours = neighbours;

                    //replace old neighbours
                    L1LSP.push_back(record);
                }
            }
        }
    }
}

/**
 * Send received LSP packet to all other neighbours (flooding).
 * @param msg received packet that has to be resent
 */
void ISIS::floodFurtherL1LSP(ISISLSPL1Packet *msg)
{
    //duplicate packet

    Ieee802Ctrl *ctrl = check_and_cast <Ieee802Ctrl *> (msg->getControlInfo());


    //resend unchanged message to all L1 IS-IS enabled interfaces except of that, which message came from
    for(unsigned int i=0; i<ISISIft.size(); i++)
    {
        if(!ISISIft.at(i).passive &&
            ISISIft.at(i).ISISenabled &&
           (ISISIft.at(i).circuitType == L1_TYPE || ISISIft.at(i).circuitType == L1L2_TYPE) &&
            ISISIft.at(i).gateIndex != msg->getArrivalGate()->getIndex())
        {
            //send it
            ISISLSPL1Packet *myMsg = msg->dup();
            Ieee802Ctrl *ctrlCopy = ctrl->dup();
            myMsg->setControlInfo(ctrlCopy);

            send(myMsg, "ifOut", ISISIft.at(i).gateIndex);
        }
    }
}

/**
 * Send only specific LSP to all attached neighbours.
 * @param LSPid ID of LSP which should be flooded using LSP packets
 */
void ISIS::sendSpecificL1LSP(unsigned char *LSPid)
{
    ISISLSPL1Packet *LSP = new ISISLSPL1Packet("L1 LSP");
    //add Ethernet controll info
    Ieee802Ctrl *ctrl = new Ieee802Ctrl();

    // set DSAP & NSAP fields
    ctrl->setDsap(SAP_CLNS);
    ctrl->setSsap(SAP_CLNS);

    //set destination broadcast address
    //It should be multicast 01-80-C2-00-00-14 MAC address, but it doesn't work in OMNeT
    MACAddress ma;
    ma.setAddress("ff:ff:ff:ff:ff:ff");
    ctrl->setDest(ma);

    //set LSP ID packet field
    for(unsigned int j=0; j<LSP->getLspIDArraySize(); j++)
    {
        LSP->setLspID(j, LSPid[j]);
    }


    //set area address TLV
    TLV_t myTLV;
    myTLV.type = AREA_ADDRESS;
    myTLV.length = 3;
    myTLV.value = new unsigned char[3];
    this->copyArrayContent((unsigned char*)this->areaId, myTLV.value, 3, 0, 0);

    LSP->setTLVArraySize(1);
    LSP->setTLV(0, myTLV);

    //find LSP
    bool found = false;
    unsigned int j;
    for(j=0; j<L1LSP.size(); j++)
    {
        if(this->compareArrays(LSPid, L1LSP.at(j).LSPid, 8))
        {
            found = true;
            break;
        }
    }

    if(found)
    {

        //set sequence number
        LSP->setSeqNumber(L1LSP.at(j).seq);

        //cout << "seq: " << LSP->getSeqNumber() << endl;

        //set neighbours TLV
        myTLV.type = IS_NEIGHBOURS_LSP;
        myTLV.length = 1 + L1LSP.at(j).neighbours.size()*11;
        myTLV.value = new unsigned char [myTLV.length];
        myTLV.value[0] = 0;                                     //reserved byte

        //set neighbours
        for(unsigned int k=0; k<L1LSP.at(j).neighbours.size(); k++)
        {
            myTLV.value[(k*11)+1] = L1LSP.at(j).neighbours.at(k).metrics.defaultMetric;
            myTLV.value[(k*11)+2] = L1LSP.at(j).neighbours.at(k).metrics.delayMetric;
            myTLV.value[(k*11)+3] = L1LSP.at(j).neighbours.at(k).metrics.expenseMetric;
            myTLV.value[(k*11)+4] = L1LSP.at(j).neighbours.at(k).metrics.errortMetric;
            this->copyArrayContent(L1LSP.at(j).neighbours.at(k).LANid, myTLV.value, 7, 0, (k*11)+5); //set LAN ID
        }

        LSP->setTLVArraySize(2);
        LSP->setTLV(1, myTLV);

        /* - TODO Auth TLV
           - eventually implement ES neighbours TLV, but I don't think it's necessary
           - next TLVs from RFC 1195 if IP should be supported
        */

        for(unsigned int a=0; a<ISISIft.size(); a++)
        {
            if(!ISISIft.at(a).passive && ISISIft.at(a).ISISenabled && (ISISIft.at(a).circuitType == L1_TYPE || ISISIft.at(a).circuitType == L1L2_TYPE))
            {
                ISISLSPL1Packet *LSPcopy = LSP->dup();
                Ieee802Ctrl *ctrlCopy = ctrl->dup();
                LSPcopy->setControlInfo(ctrlCopy);
                send(LSPcopy, "ifOut", ISISIft.at(a).gateIndex);
            }
        }
    }
    //cleanup
    delete LSP;
}

