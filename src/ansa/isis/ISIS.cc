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

void ISIS::initialize(int stage) {
    //interface init at stage 2
    if (stage == 3) {

        deviceType = par("deviceType");
        deviceId = par("deviceId");
        configFile = par("configFile");

        ift = AnsaInterfaceTableAccess().get();
        if (ift == NULL) {
            throw cRuntimeError("AnsaInterfaceTable not found");
        }

        cXMLElement *device = xmlParser::GetDevice(deviceType, deviceId,
                configFile);
        if (device == NULL) {
            EV << "deviceId " << deviceId
                      << ": ISIS is not enabled on this device\n";
            return;
        }

        //load IS-IS routing info from config file
        cXMLElement *isisRouting = xmlParser::GetIsisRouting(device);
        if (isisRouting == NULL) {
            EV << "deviceId " << deviceId
                      << ": ISIS is not enabled on this device\n";
            return;
        }
        //TODO: multiple NETs for migrating purposes (merging, splitting areas)
        cXMLElement *net = isisRouting->getFirstChildWithTag("NET");
        if (net == NULL) {
            EV << "deviceId " << deviceId
                      << ": Net address wasn't specified in IS-IS routing\n";
            return;
        }

        netAddr = net->getNodeValue();
        if (netAddr == NULL || strcmp("", netAddr) == 0) {
            EV << "deviceId " << deviceId
                      << ": Net address wasn't specified in IS-IS routing\n";
            return;
        } else {
            if (!parseNetAddr()) {
                EV << "deviceId " << deviceId
                          << ": Invalid net address format\n";
                return;
            } else {
                EV << "deviceId " << deviceId << ": Net address set to: "
                          << netAddr << "\n";
            }
        }

        //set router IS type {L1 | L2 | L1L2 (default)}
        cXMLElement *routertype = isisRouting->getFirstChildWithTag("IS-Type");
        if (routertype == NULL) {
            this->isType = L1L2_TYPE;
        } else {
            const char* routerTypeValue = routertype->getNodeValue();
            if (routerTypeValue == NULL) {
                this->isType = L1L2_TYPE;
            } else {
                if (strcmp(routerTypeValue, "level-1") == 0) {
                    this->isType = L1_TYPE;
                } else {
                    if (strcmp(routerTypeValue, "level-2") == 0) {
                        this->isType = L2_TYPE;
                    } else {
                        this->isType = L1L2_TYPE;
                    }
                }
            }
        }

        //set L1 hello interval in seconds
        cXMLElement *L1HelloInt = isisRouting->getFirstChildWithTag(
                "L1-Hello-Interval");
        if (L1HelloInt == NULL || L1HelloInt->getNodeValue() == NULL) {
            this->L1HelloInterval = ISIS_HELLO_INTERVAL;
        } else {
            this->L1HelloInterval = atoi(L1HelloInt->getNodeValue());
        }

        //set L1 hello multiplier
        cXMLElement *L1HelloMult = isisRouting->getFirstChildWithTag(
                "L1-Hello-Multiplier");
        if (L1HelloMult == NULL || L1HelloMult->getNodeValue() == NULL) {
            this->L1HelloMultiplier = ISIS_HELLO_MULTIPLIER;
        } else {
            this->L1HelloMultiplier = atoi(L1HelloMult->getNodeValue());
        }

        //set L2 hello interval in seconds
        cXMLElement *L2HelloInt = isisRouting->getFirstChildWithTag(
                "L2-Hello-Interval");
        if (L2HelloInt == NULL || L2HelloInt->getNodeValue() == NULL) {
            this->L2HelloInterval = ISIS_HELLO_INTERVAL;
        } else {
            this->L2HelloInterval = atoi(L2HelloInt->getNodeValue());
        }

        //set L2 hello multiplier
        cXMLElement *L2HelloMult = isisRouting->getFirstChildWithTag(
                "L2-Hello-Multiplier");
        if (L2HelloMult == NULL || L2HelloMult->getNodeValue() == NULL) {
            this->L2HelloMultiplier = ISIS_HELLO_MULTIPLIER;
        } else {
            this->L2HelloMultiplier = atoi(L2HelloMult->getNodeValue());
        }


        cXMLElement *interfaces = device->getFirstChildWithTag("Interfaces");
        if (interfaces == NULL) {
            EV
                      << "deviceId "
                      << deviceId
                      << ": <Interfaces></Interfaces> tag is missing in configuration file: \""
                      << configFile << "\"\n";
            return;
        }

        // add all interfaces to ISISIft vector containing additional information
        InterfaceEntry *entryIFT = new InterfaceEntry();
        for (int i = 0; i < ift->getNumInterfaces(); i++) {
            entryIFT = ift->getInterface(i);
            //EV << entryIFT->getNetworkLayerGateIndex() << " " << entryIFT->getName() << " " << entryIFT->getFullName() << "\n";
            insertIft(
                    entryIFT,
                    interfaces->getFirstChildWithAttribute("Interface", "name",
                            entryIFT->getName()));
        }

        //TODO passive-interface
        this->initHandshake();

    }
}


/**
 * Set initial parameters of network interfaces.
 * @param entry Pointer to interface record in interfaceTable
 * @param intElement XML element of current interface in XML config file
 */
void ISIS::insertIft(InterfaceEntry *entry, cXMLElement *intElement)
{

    if(intElement == NULL){
        return;
    }
    ISISinterface newIftEntry;
    newIftEntry.intID = entry->getInterfaceId();

    newIftEntry.gateIndex = entry->getNetworkLayerGateIndex();
    EV <<"deviceId: " << this->deviceId << "ISIS: adding interface, gateIndex: " <<newIftEntry.gateIndex <<endl;

    //set interface priority
    newIftEntry.priority = ISIS_DIS_PRIORITY;  //default value

    /* Interface is NOT enabled by default. If ANY IS-IS related property is configured on interface then it's enabled. */
    newIftEntry.ISISenabled = false;

        cXMLElement *priority = intElement->getFirstChildWithTag("IS-IS-Priority");
        if(priority != NULL && priority->getNodeValue() != NULL)
        {
            newIftEntry.priority = (unsigned char) atoi(priority->getNodeValue());
            newIftEntry.ISISenabled = true;
        }


    //set network type (point-to-point vs. broadcast)

    newIftEntry.network = true; //default value

    cXMLElement *network = intElement->getFirstChildWithTag("IS-IS-Network");
    if (network != NULL && network->getNodeValue() != NULL)
    {
        if (!strcmp(network->getNodeValue(), "point-to-point"))
        {
            newIftEntry.network = false;
            EV << "Interface network type is point-to-point " << network->getNodeValue() << endl;
        }
        else if (!strcmp(network->getNodeValue(), "broadcast"))
        {
            EV << "Interface network type is broadcast " << network->getNodeValue() << endl;
        }
        else
        {
            EV << "ERORR: Unrecognized interface's network type: " << network->getNodeValue() << endl;

        }
        newIftEntry.ISISenabled = true;

    }



    //set interface metric

    newIftEntry.metric = ISIS_METRIC;    //default value

        cXMLElement *metric = intElement->getFirstChildWithTag("IS-IS-Metric");
        if(metric != NULL && metric->getNodeValue() != NULL)
        {
            newIftEntry.metric = (unsigned char) atoi(metric->getNodeValue());
            newIftEntry.ISISenabled = true;
        }




    //set interface type according to global router configuration
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

                    newIftEntry.circuitType = L1L2_TYPE;

                    cXMLElement *circuitType = intElement->getFirstChildWithTag("IS-IS-Circuit-Type");
                    if(circuitType != NULL && circuitType->getNodeValue()!= NULL)
                    {
                        if(strcmp(circuitType->getNodeValue(),"L2") == 0)
                            newIftEntry.circuitType = L2_TYPE;
                        else
                        {
                            if(strcmp(circuitType->getNodeValue(),"L1") == 0)
                            newIftEntry.circuitType = L1_TYPE;
                        }
                        newIftEntry.ISISenabled = true;
                    }
                    else
                    {
                        newIftEntry.circuitType = L1L2_TYPE;
                    }

                break;
        }
    }

    //set L1 hello interval in seconds
    cXMLElement *L1HelloInt = intElement->getFirstChildWithTag(
            "L1-Hello-Interval");
    if (L1HelloInt == NULL || L1HelloInt->getNodeValue() == NULL) {
        newIftEntry.L1HelloInterval = this->L1HelloInterval;
    } else {
        newIftEntry.L1HelloInterval = atoi(L1HelloInt->getNodeValue());
    }

    //set L1 hello multiplier
    cXMLElement *L1HelloMult = intElement->getFirstChildWithTag(
            "L1-Hello-Multiplier");
    if (L1HelloMult == NULL || L1HelloMult->getNodeValue() == NULL) {
        newIftEntry.L1HelloMultiplier = this->L1HelloMultiplier;
    } else {
        newIftEntry.L1HelloMultiplier = atoi(L1HelloMult->getNodeValue());
    }

    //set L2 hello interval in seconds
    cXMLElement *L2HelloInt = intElement->getFirstChildWithTag(
            "L2-Hello-Interval");
    if (L2HelloInt == NULL || L2HelloInt->getNodeValue() == NULL) {
        newIftEntry.L2HelloInterval = this->L2HelloInterval;
    } else {
        newIftEntry.L2HelloInterval = atoi(L2HelloInt->getNodeValue());
    }

    //set L2 hello multiplier
    cXMLElement *L2HelloMult = intElement->getFirstChildWithTag(
            "L2-Hello-Multiplier");
    if (L2HelloMult == NULL || L2HelloMult->getNodeValue() == NULL) {
        newIftEntry.L2HelloMultiplier = this->L2HelloMultiplier;
    } else {
        newIftEntry.L2HelloMultiplier = atoi(L2HelloMult->getNodeValue());
    }

    //TODO  priority is not needed for point-to-point
       //set priority of current DIS = me at start
       newIftEntry.L1DISpriority = newIftEntry.priority;
       newIftEntry.L2DISpriority = newIftEntry.priority;

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
 * Initiate handshake and neighbour discovery.
 */
void ISIS::initHandshake()
{
    for (unsigned int k = 0; k < ISISIft.size(); k++)
    {
        //TODO
        //schedule Hello timer per level => check if L1L2 on broadcast => schedule two timers
        //on PTP is L1L2 Hello valid timer
        // set first hello timer after 1 second
        ISISTimer *timerMsg = new ISISTimer("Hello_timer");
        timerMsg->setTimerKind(HELLO_TIMER);
        timerMsg->setIsType(ISISIft.at(k).circuitType);
        timerMsg->setInterfaceIndex(k);
        scheduleAt(simTime() + 1.0, timerMsg);
    }
    //set initial hello counter
    this->helloCounter = 0;

    //give network time to converge before first LSP is sent out (15 seconds)
    ISISTimer *LSPtimer = new ISISTimer("Send first LSP timer");
    LSPtimer->setTimerKind(L1LSP_REFRESH);
    scheduleAt(simTime() + 15.0, LSPtimer);

    //10 sec after first LSPs sent should be flooded CNSP packets
    ISISTimer *CSNPtimer = new ISISTimer("Send first CSNP packets");
    CSNPtimer->setTimerKind(CSNP_TIMER);
    scheduleAt(simTime() + 25.0, CSNPtimer);
}


/**
 * Handle incoming messages: Method differs between self messages and external messages
 * and executes appropriate function.
 * @param msg incoming message
 */
void ISIS::handleMessage(cMessage* msg)
{

    if (msg->isSelfMessage())
    {
        ISISTimer *timer = check_and_cast<ISISTimer *>(msg);
        switch (timer->getTimerKind())
            {
            case (HELLO_TIMER):
                this->sendHelloMsg(timer);
                delete timer;
                break;
            case (NEIGHBOUR_DEAD):
                this->removeDeadNeighbour(timer);
                delete timer;
                break;
            case (L1LSP_REFRESH):
                this->sendMyL1LSPs();
                delete timer;
                break;
            case (L1LSP_DEAD):
                this->removeDeadLSP(timer);
                delete timer;
                break;
            case (CSNP_TIMER):
                this->sendL1CSNP();
                delete timer;
                break;
            default:
                break;
            }
    }
    else
    {

        //every (at least all Hello) message should be checked for matching system-ID length

        //process PTP_HELLO


        //shouldn't check this->isType, but rather circuitType on incoming interface
        ISISMessage *inMsg = check_and_cast<ISISMessage *>(msg);
        if(!this->isMessageOK(inMsg)){
            EV<< "ISIS: Warning: discarding message" <<endl;
            //TODO schedule event discarding message
            return;
        }

        //get arrival interface
        int gateIndex = inMsg->getArrivalGate()->getIndex();
        ISISinterface * tmpIntf = this->getIfaceByGateIndex(gateIndex);
        if(tmpIntf == NULL){
            EV << "ISIS: ERROR: Couldn't find interface by gate index when ISIS::handleMessage" << endl;
            return;
        }

        //get circuit type for arrival interface
        short circuitType = tmpIntf->circuitType;

        /* Usually we shouldn't need to check matching circuit type with arrived message,
         * and these messages should be filtered based on destination MAC address.
         * Since we use broadcast MAC address, IS(router) cannot determine if it's
         * for ALL L1 or ALL L2 systems. Therefore wee need to check manually.
         * If appropriate Level isn't enabled on interface, then the message is dicarded.
        */
        switch (inMsg->getType())
        {
            case (LAN_L1_HELLO):
                if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
                {
                    this->handleL1HelloMsg(inMsg);

                    //comment if printing of the adjacency table is too disturbing
                    this->printAdjTable();

                }else{
                    EV << "deviceId " << deviceId
                                          << ": ISIS: WARNING: Discarding LAN_L1_HELLO message on unsupported circuit type interface\n"<< inMsg->getId() << endl;
                }
                delete inMsg;
                break;
            case (LAN_L2_HELLO):
                if (circuitType == L2_TYPE || circuitType == L1L2_TYPE)
                {
                    this->handleL2HelloMsg(inMsg);
                    this->printAdjTable();

                }
                else
                {
                    EV << "deviceId "
                                    << deviceId
                                    << ": ISIS: WARNING: Discarding LAN_L2_HELLO message on unsupported circuit type interface"
                                    << inMsg->getId() << endl;
                }
                delete inMsg;
                break;
            case (PTP_HELLO):
                    //On PTP link process all Hellos
                if (circuitType != RESERVED_TYPE)
                {
                    this->handlePTPHelloMsg(inMsg);
                    this->printAdjTable();
                }
                else
                {
                    EV
                                    << "deviceId "
                                    << deviceId
                                    << ": ISIS: WARNING: Discarding PTP_HELLO message. Received RESERVED_TYPE circuit type"
                                    << inMsg->getId() << endl;
                }
                delete inMsg;
                break;
            case (L1_LSP):
                if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
                {
                    this->handleL1LSP(inMsg);
                    inMsg->dup();

                    //comment if printing of the link-state database is too disturbing
                    this->printLSPDB();
                }else{
                                    EV << "deviceId " << deviceId
                                                          << ": ISIS: WARNING: Discarding LAN_L1_HELLO message on unsupported circuit type interface\n"<< inMsg->getId() << endl;
                                }
                                delete inMsg;
                                break;
            case (L1_CSNP):
                if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
                {
                    this->handleL1CSNP(inMsg);
                }else{
                                    EV << "deviceId " << deviceId
                                                          << ": ISIS: WARNING: Discarding LAN_L1_HELLO message on unsupported circuit type interface\n"<< inMsg->getId() << endl;
                                }
                                delete inMsg;
                                break;
            case (L1_PSNP):
                if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
                {
                    this->handleL1PSNP(inMsg);
                    delete inMsg;
                }
                break;
            default:
                EV << "deviceId " << deviceId
                                                                          << ": ISIS: WARNING: Discarding unknown message type. Msg id: "<< inMsg->getId() << endl;
                delete inMsg;
                break;

        }
    }
}




/**
 * Create hello packet and flood them out of every interface. This method handle
 * L1 and also L2 hello packets. Destination MAC address is broadcast (ff:ff:ff:ff:ff:ff).
 * Packets contain IS_NEIGHBOURS_HELLO and AREA_ADDRESS TLVs.
 */
void ISIS::sendHelloMsg(ISISTimer* timer)
{
    if(ISISIft.at(timer->getInterfaceIndex()).network){
        EV << "ISIS: sendingBroadcastHello: "<< endl;
        this->sendBroadcastHelloMsg(timer->getInterfaceIndex(), timer->getIsType());

    }else{
        EV << "ISIS: sendingPTPHello: "<< endl;
        this->sendPTPHelloMsg(timer->getInterfaceIndex(), timer->getIsType());
    }



}
/*
 * Send hello message on specified broadcast interface.
 * @param k is interface index (index to ISISIft)
 */
void ISIS::sendBroadcastHelloMsg(int k, short circuitType){
    // create L1 and L2 hello packets

    ISISL1HelloPacket *helloL1 = new ISISL1HelloPacket("L1 Hello");
    ISISL2HelloPacket *helloL2 = new ISISL2HelloPacket("L2 Hello");


    //set circuit type field
    helloL1->setCircuitType(L1_TYPE);
    helloL2->setCircuitType(L2_TYPE);


    //set source id
    for (unsigned int i = 0; i < 6; i++)
    {
        helloL1->setSourceID(i, sysId[i]);
        helloL2->setSourceID(i, sysId[i]);
    }
    EV << endl;



    /* TODO
     * They should have separate Ethernet control info but OMNeT++ simulation
     doesn't recognize 01:80:c2:00:00:14 and 01:80:c2:00:00:15 as multicast OSI
     MAC addresses. Therefore destination MAC address is always set to broadcast
     ff:ff:ff:ff:ff:ff
     */

    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
    Ieee802Ctrl *ctrl2;

    // set DSAP & NSAP fields
    ctrl->setDsap(SAP_CLNS);
    ctrl->setSsap(SAP_CLNS);

    //set appropriate destination MAC addresses
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
    this->copyArrayContent((unsigned char *) areaId, myTLV.value, 3, 0, 0);
    helloL1->setTLVArraySize(1); //resize array
    helloL1->setTLV(0, myTLV);

    helloL2->setTLVArraySize(1); //resize array
    helloL2->setTLV(0, myTLV);


    //set NEIGHBOURS L1 TLV
    myTLV.type = IS_NEIGHBOURS_HELLO;
    myTLV.length = adjL1Table.size() * 6; //number of records * 6 (6 is size of system ID/MAC address)
    myTLV.value = new unsigned char[adjL1Table.size() * 6];

    for (unsigned int h = 0; h < adjL1Table.size(); h++)
    {
        this->copyArrayContent(adjL1Table.at(h).mac.getAddressBytes(), myTLV.value, 6, 0, h * 6);
    }
    helloL1->setTLVArraySize(2);
    helloL1->setTLV(1, myTLV);

    //L2 neighbours
    myTLV.type = IS_NEIGHBOURS_HELLO;
    myTLV.length = adjL2Table.size() * 6; //number of records * 6 (6 is size of system ID/MAC address)
    myTLV.value = new unsigned char[adjL2Table.size() * 6];
    //L2 neighbours
    for (unsigned int h = 0; h < adjL2Table.size(); h++)
    {
        this->copyArrayContent(adjL2Table.at(h).mac.getAddressBytes(), myTLV.value, 6, 0, h * 6);
    }

    helloL2->setTLVArraySize(2);
    helloL2->setTLV(1, myTLV);


    //PADDING TLV is omitted

    //TODO Authentication TLV

    //TODO add eventually another TLVs (eg. from RFC 1195)

    //    for(unsigned int k=0; k<ISISIft.size(); k++)
    //    {

    //don't send hello packets from passive interfaces
    if (!ISISIft.at(k).passive && ISISIft.at(k).ISISenabled)
    {
        // if this interface is DIS for LAN, hellos are sent 3-times faster (3.33sec instead of 10.0)
        // decision is made according to global hello counter (dirty hax - don't blame me pls, but i don't have time to code it nice way :)

        switch (ISISIft.at(k).circuitType)
            {
            case (L1_TYPE): {
                //copy packet with control info
                ISISL1HelloPacket *copy1 = helloL1->dup();
                Ieee802Ctrl *ctrlCopy1 = ctrl->dup();
                copy1->setControlInfo(ctrlCopy1);

                //set LAN ID field (designated L1 IS)
                for (unsigned int j = 0; j < 7; j++)
                {
                    copy1->setLanID(j, ISISIft.at(k).L1DIS[j]);
                }

                copy1->setPriority(ISISIft.at(k).priority);
                send(copy1, "ifOut", ISISIft.at(k).gateIndex);
                EV
                        << "'devideId " << deviceId << ": L1 Hello packet was sent from "
                                << ISISIft.at(k).entry->getName() << "\n";
                break;
            }
            case (L2_TYPE): { //copy packet with control info
                ISISL2HelloPacket *copy2 = helloL2->dup();
                Ieee802Ctrl *ctrlCopy2 = ctrl->dup();
                copy2->setControlInfo(ctrlCopy2);

                //set LAN ID field (designated L2 IS)
                for (unsigned int j = 0; j < 7; j++)
                {
                    copy2->setLanID(j, ISISIft.at(k).L2DIS[j]);
                }

                copy2->setPriority(ISISIft.at(k).priority);
                send(copy2, "ifOut", ISISIft.at(k).gateIndex);
                EV
                        << "deviceId " << deviceId << ": L2 Hello packet was sent from "
                                << ISISIft.at(k).entry->getName() << "\n";
                break;
            }
            case (L1L2_TYPE): {
                /*Newly this method should be called per circuit type so this "case" shouldn't be needed.
                * Before deletion, this behavior needs to be implemented on several places throughout the code.
                * Mainly in initHandshake and rescheduling new events.
                */
                //send both - L1 & L2 hellos
                ISISL1HelloPacket *copy = helloL1->dup();
                Ieee802Ctrl *ctrlCopy = ctrl->dup();
                copy->setControlInfo(ctrlCopy);
                //set LAN ID field (designated L1 IS)
                for (unsigned int j = 0; j < 7; j++)
                {
                    copy->setLanID(j, ISISIft.at(k).L1DIS[j]);
                }
                copy->setPriority(ISISIft.at(k).priority);
                send(copy, "ifOut", ISISIft.at(k).gateIndex);
                EV
                        << "'devideId " << deviceId << ": L1 Hello packet was sent from "
                                << ISISIft.at(k).entry->getName() << "\n";

                ISISL2HelloPacket *copy2 = helloL2->dup();
                Ieee802Ctrl *ctrlCopy2 = ctrl->dup();
                copy2 = helloL2->dup();
                ctrlCopy2 = ctrl->dup();
                copy2->setControlInfo(ctrlCopy2);
                //set LAN ID field (designated L2 IS)
                for (unsigned int j = 0; j < 7; j++)
                {
                    copy2->setLanID(j, ISISIft.at(k).L2DIS[j]);
                }
                copy2->setPriority(ISISIft.at(k).priority);
                send(copy2, "ifOut", ISISIft.at(k).gateIndex);
                EV
                        << "deviceId " << deviceId << ": L2 Hello packet was sent from "
                                << ISISIft.at(k).entry->getName() << "\n";
                break;
            }
            default:
                EV << "deviceId " << deviceId << ": Warning: Unrecognized circuit type" << endl;
                break;


            }

    }

    //TODO
    //schedule new hello timer
    ISISTimer *timer = new ISISTimer("Hello_timer");
    timer->setTimerKind(HELLO_TIMER);
    timer->setIsType(circuitType);
    timer->setInterfaceIndex(k);
    scheduleAt(simTime() + 3.33, timer);




    //increment hello counter
    this->helloCounter++;

    //cleanup
    delete helloL1;
    delete helloL2;
}



void ISIS::sendPTPHelloMsg(int k, short circuitType){
    //don't send hello packets from passive interfaces
    if(ISISIft.at(k).passive || !ISISIft.at(k).ISISenabled){
        return;
    }

    /* TODO
     * They should have separate Ethernet control info but OMNeT++ simulation
     doesn't recognize 01:80:c2:00:00:14 and 01:80:c2:00:00:15 as multicast OSI
     MAC addresses. Therefore destination MAC address is always set to broadcast
     ff:ff:ff:ff:ff:ff
     */

    //TODO change to appropriate layer-2 protocol
    Ieee802Ctrl *ctrlPtp = new Ieee802Ctrl();

    // set DSAP & NSAP fields
    ctrlPtp->setDsap(SAP_CLNS);
    ctrlPtp->setSsap(SAP_CLNS);

    //set appropriate destination MAC addresses
    MACAddress ma;
    ma.setAddress("ff:ff:ff:ff:ff:ff");

    ctrlPtp->setDest(ma);

    //type
    ISISPTPHelloPacket *ptpHello = new ISISPTPHelloPacket("PTP Hello");

    //assign Ethernet control info
    ptpHello->setControlInfo(ctrlPtp);
    //circuitType
    ptpHello->setCircuitType(ISISIft.at(k).circuitType);

    //sourceID
    //set source id
    for (unsigned int i = 0; i < 6; i++)
    {
        ptpHello->setSourceID(i, sysId[i]);
    }

    //holdTime
    //set holdTime
    ptpHello->setHoldTime(this->getHoldTime(k, ISISIft.at(k).circuitType));

    //pduLength

    //localCircuitID
    ptpHello->setLocalCircuitID(ISISIft.at(k).gateIndex);

    //TLV[]
    //set TLVs
    TLV_t myTLV;

    //set area address
    myTLV.type = AREA_ADDRESS;
    myTLV.length = 3;
    myTLV.value = new unsigned char[3];
    this->copyArrayContent((unsigned char *) areaId, myTLV.value, 3, 0, 0);

    unsigned int tlvSize = ptpHello->getTLVArraySize();
    ptpHello->setTLVArraySize(tlvSize + 1);
    ptpHello->setTLV(tlvSize, myTLV);


        //ptp adjacency state TLV #240

    myTLV.type = PTP_HELLO_STATE;
    myTLV.length = 1;
    myTLV.value = new unsigned char[myTLV.length];
    ISISadj* tempAdj;
    //if adjacency for this interface exists, then its state is either UP or INIT
    //we also assumes that on point-to-point link only one adjacency can exist
    if ((tempAdj = this->getAdjByGateIndex(ISISIft.at(k).gateIndex)) != NULL)
    {
        if (!tempAdj->state)
        {
            myTLV.value[0] = PTP_INIT;
            EV << "ISIS::sendPTPHello: sending state PTP_INIT "<< endl;
        }
        else
        {
            myTLV.value[0] = PTP_UP;
            EV << "ISIS::sendPTPHello: sending state PTP_UP "<< endl;
        }

    }
    else
    {
        //if adjacency doesn't exist yet, then it's for sure down
        myTLV.value[0] = PTP_DOWN;
        EV << "ISIS::sendPTPHello: sending state PTP_DOWN "<< endl;
    }
    tlvSize = ptpHello->getTLVArraySize();
    ptpHello->setTLVArraySize(tlvSize + 1);
    ptpHello->setTLV(tlvSize, myTLV);
    //TODO TLV #129 Protocols supported

    ISISPTPHelloPacket *copyPtp = ptpHello->dup();
    Ieee802Ctrl *copyCtrl = ctrlPtp->dup();
    copyPtp->setControlInfo(copyCtrl);
    this->send(copyPtp,"ifOut", ISISIft.at(k).gateIndex);

    ISISTimer *timer = new ISISTimer("Hello_timer");
    timer->setTimerKind(HELLO_TIMER);
    timer->setInterfaceIndex(k);
    timer->setIsType(circuitType);
    timer->setInterfaceIndex(k);
//    scheduleAt(simTime() + ISISIft.at(k). )
    this->schedule(timer);

    delete ptpHello;


}

void ISIS::schedule(ISISTimer *timer){
    //TODO add support for various timers type
    double timeAt = this->getHelloInterval(timer->getInterfaceIndex(), timer->getIsType());
    double randomTime = uniform(0, 0.25 * timeAt);
    this->scheduleAt(simTime() + timeAt - randomTime, timer);
    EV << "ISIS::schedule: timeAt: "<< timeAt << " randomTime: "<< randomTime << endl;
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
                EV<<"BEZ ATOI" << net.substr(0,2).c_str() <<endl;
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
    EV <<"ISIS: AreaID: "<< areaId[0] <<endl;
    EV <<"ISIS: AreaID: "<< areaId[1] <<endl;
    EV <<"ISIS: AreaID: "<< areaId[2] <<endl;
    sysId = systemId;
    EV <<"ISIS: SystemID: "<< sysId <<endl;
    NSEL = nsel;
    EV <<"ISIS: NSEL: "<< NSEL <<endl;

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
        //TODO schedule event duplicitSystemID
        return;
    }

    ISISL1HelloPacket *msg = check_and_cast<ISISL1HelloPacket *>(inMsg);


    //check if at least one areaId matches our areaId (don't do this for L2)
    bool areaOK = false;
    TLV_t* tmpTLV;
    for (int i = 0; (tmpTLV = this->getTLVByType(msg, AREA_ADDRESS, i)) != NULL; i++)
    {
        areaOK = areaOK || this->isAreaIDOK(tmpTLV);
    }

    if (!areaOK)
    {
        //TODO schedule event AreaIDMismatch
        EV << "ISIS: Warning: L1_LAN_HELLO doesn't contain Area address TLV." << endl;
        return;
    }




    //if remote system ID is contained in adjL1Table
    ISISadj *tmpAdj;
    if((tmpAdj = this->getAdj(inMsg)) != NULL)
    {

        //reset timer
        cancelEvent(tmpAdj->timer);

        //TODO use this->schedule()
        scheduleAt(simTime() + msg->getHoldTime(), tmpAdj->timer);

        //DONT update the interface, if the gateIndex has changed, then declare the adjacency as down or at least init and then change gateIndex
        //update interface
        //tmpAdj->gateIndex = msg->getArrivalGate()->getIndex();

        //check for DIS priority and eventually set new DIS if needed; do it only if exists adjacency with state "UP"
        if(tmpAdj->state)
            electL1DesignatedIS(msg);
        //find neighbours TLV
        if ((tmpTLV = this->getTLVByType(msg, IS_NEIGHBOURS_HELLO)) != NULL)
        {
            unsigned char *tmpRecord = new unsigned char[ISIS_SYSTEM_ID];
            //walk through all neighbour identifiers contained in TLV
            for (unsigned int r = 0; r < (tmpTLV->length / ISIS_SYSTEM_ID); r++)
            {
                //check if my system id is contained in neighbour's adjL1Table
                this->copyArrayContent(tmpTLV->value, tmpRecord, ISIS_SYSTEM_ID, r * ISIS_SYSTEM_ID, 0);

                int gateIndex = inMsg->getArrivalGate()->getIndex();
                ISISinterface *tmpIntf = this->getIfaceByGateIndex(gateIndex);
                MACAddress tmpMAC = tmpIntf->entry->getMacAddress();
                if (compareArrays(tmpMAC.getAddressBytes(), tmpRecord, 6))
                {
                    //store previous state
                    bool changed = tmpAdj->state;
                    tmpAdj->state = true;

                    //if state changed, flood new LSP
                    if (changed != tmpAdj->state /*&& simTime() > 35.0*/) //TODO use at least some kind of ISIS_INITIAL_TIMER
                        this->sendMyL1LSPs();
                    //generate event adjacencyStateChanged

                    break;
                }

            }
            delete tmpRecord;


        } else {
            //TODO Delete after debugging

            EV << "ISIS: Warning: Didn't find IS_NEIGHBOURS_HELLO TLV in LAN_L1_HELLO" << endl;
            return;
        }

    }

    //else create new record in adjL1Table
    else
    {
        //EV << "CREATING NEW ADJ RECORD\n";

        //find area ID TLV

        //create new neighbour record and set parameters
        ISISadj neighbour;
        neighbour.state = false; //set state to initial

        //set timeout of neighbour
        neighbour.timer = new ISISTimer("Neighbour_timeout");
        neighbour.timer->setTimerKind(NEIGHBOUR_DEAD);
        neighbour.timer->setIsType(L1_TYPE);
        neighbour.timer->setInterfaceIndex(this->getIfaceIndex(this->getIfaceByGateIndex(neighbour.gateIndex)));
        //set source system ID in neighbour record & in timer to identify it

        for (unsigned int the_game = 0; the_game < msg->getSourceIDArraySize(); the_game++)
        {
            neighbour.sysID[the_game] = msg->getSourceID(the_game);
            neighbour.timer->setSysID(the_game, msg->getSourceID(the_game));
        }

        //get source MAC address of received frame
        Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
        neighbour.mac = ctrl->getSrc();

        //set gate index, which is neighbour connected to
        neighbour.gateIndex = msg->getArrivalGate()->getIndex();

        scheduleAt(simTime() + msg->getHoldTime(), neighbour.timer);

        //insert neighbour into adjL1Table
        adjL1Table.push_back(neighbour);

        //EV << "deviceId " << deviceId << ": new adjacency\n";

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
        //TODO schedule event duplicitSystemID
        return;
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
                if(!(this->compareArrays(msg->getTLV(j).value, adjL2Table.at(i).areaID, ISIS_AREA_ID)))
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

void ISIS::handlePTPHelloMsg(ISISMessage *inMsg){
    //duplicate system ID check
    if (this->checkDuplicateSysID(inMsg))
    {
        //TODO schedule event duplicitSystemID
        return;
    }

    ISISPTPHelloPacket *msg = check_and_cast<ISISPTPHelloPacket *>(inMsg);

    //check if at least one areaId matches our areaId (don't do this for L2)
    TLV_t* tmpTLV;
    if (msg->getCircuitType() != L2_TYPE)
    {
        bool areaOK = false;
        tmpTLV = this->getTLVByType(msg, AREA_ADDRESS, 0);
        for (int i = 0; (tmpTLV) != NULL; i++)
        {
            areaOK = areaOK || this->isAreaIDOK(tmpTLV);
            tmpTLV = this->getTLVByType(msg, AREA_ADDRESS, i);
        }

        if (!areaOK)
        {
            //TODO schedule event AreaIDMismatch
            EV << "ISIS: Warning: PTP_HELLO doesn't contain Area address TLV." << endl;
            return;
        }
    }
    //if remote system ID is contained in adjL1Table
    ISISadj *tmpAdj;
    if (msg->getCircuitType() == L1_TYPE || msg->getCircuitType() == L1L2_TYPE)
    {

        if ((tmpAdj = this->getAdj(inMsg, L1_TYPE)) != NULL)
        {

            //reset timer
            cancelEvent(tmpAdj->timer);

            //TODO use this->schedule()
            scheduleAt(simTime() + msg->getHoldTime(), tmpAdj->timer);

            //find neighbours TLV
            if ((tmpTLV = this->getTLVByType(msg, PTP_HELLO_STATE)) != NULL)
            {
                if (tmpTLV->length == 1)
                {

                    if (tmpAdj->state) //UP
                    {

                        if (tmpTLV->value[0] == PTP_UP)
                        {
                            //OK do nothing
                        }
                        else if(tmpTLV->value[0] == PTP_DOWN)
                        {
                            //state init
                            //TODO
                            tmpAdj->state = false;

                        }else{
                            //state accept
                        }

                    }
                    else // INIT
                    {
                        if (tmpTLV->value[0] == PTP_UP || tmpTLV->value[0] == PTP_INIT)
                        {
                            //OK
                            tmpAdj->state = true;
//                            if (simTime() > 35.0){ //TODO use at least some kind of ISIS_INITIAL_TIMER
                            this->sendMyL1LSPs();
//                            }
                            //TODO
                            //schedule adjacencyStateChange(up);
                        }
                        else
                        {
                            //stay init
                        }
                    }
                }

            }
            else
            {
                //TODO Delete after debugging
                EV << "ISIS: Warning: Didn't find PTP_HELLO_STATE TLV in PTP_HELLO L2" << endl;
                return;
            }

        }

        //else create new record in adjL1Table
        else
        {
            //EV << "CREATING NEW ADJ RECORD\n";

            //find area ID TLV

            //create new neighbour record and set parameters
            ISISadj neighbour;
            neighbour.state = false; //set state to initial

            //set timeout of neighbour
            neighbour.timer = new ISISTimer("Neighbour_timeout");
            neighbour.timer->setTimerKind(NEIGHBOUR_DEAD);
            neighbour.timer->setIsType(L1_TYPE);
            neighbour.timer->setInterfaceIndex(this->getIfaceIndex(this->getIfaceByGateIndex(neighbour.gateIndex)));
            //set source system ID in neighbour record & in timer to identify it

            for (unsigned int the_game = 0; the_game < msg->getSourceIDArraySize(); the_game++)
            {
                neighbour.sysID[the_game] = msg->getSourceID(the_game);
                neighbour.timer->setSysID(the_game, msg->getSourceID(the_game));
            }

            //get source MAC address of received frame
            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
            neighbour.mac = ctrl->getSrc();

            //set gate index, which is neighbour connected to
            neighbour.gateIndex = msg->getArrivalGate()->getIndex();

            //TODO predelat na this->schedule()
            scheduleAt(simTime() + msg->getHoldTime(), neighbour.timer);

            //insert neighbour into adjL1Table
            adjL1Table.push_back(neighbour);

            //EV << "deviceId " << deviceId << ": new adjacency\n";

        }
    }

    if (msg->getCircuitType() == L2_TYPE || msg->getCircuitType() == L1L2_TYPE)
    {
        if ((tmpAdj = this->getAdj(inMsg, L2_TYPE)) != NULL)
        {
            //reset timer
            cancelEvent(tmpAdj->timer);

            //TODO use this->schedule()
            scheduleAt(simTime() + msg->getHoldTime(), tmpAdj->timer);

            //find neighbours TLV
            if ((tmpTLV = this->getTLVByType(msg, PTP_HELLO_STATE)) != NULL)
            {
                if (tmpTLV->length == 1)
                {
                    if (tmpAdj->state) //UP
                    {
                        if (tmpTLV->value[0] == PTP_UP)
                        {
                            //OK do nothing
                        }
                        else if (tmpTLV->value[0] == PTP_DOWN)
                        {
                            //state init
                            //TODO
                            tmpAdj->state = false;

                        }
                        else
                        {
                            //state accept
                        }

                    }
                    else // INIT
                    {
                        if (tmpTLV->value[0] == PTP_UP || tmpTLV->value[0] == PTP_INIT)
                        {
                            //OK
                            tmpAdj->state = true;
//                            if (simTime() > 35.0)
//                            { //TODO use at least some kind of ISIS_INITIAL_TIMER
                                this->sendMyL2LSPs();
//                            }
                            //TODO
                            //schedule adjacencyStateChange(up);
                        }
                        else
                        {
                            //stay init
                        }
                    }
                }

            }
            else
            {
                //TODO Delete after debugging
                EV << "ISIS: Warning: Didn't find PTP_HELLO_STATE TLV in PTP_HELLO" << endl;
                return;
            }

        }

        //else create new record in adjL2Table
        else
        {
            //EV << "CREATING NEW ADJ RECORD\n";

            //find area ID TLV

            //create new neighbour record and set parameters
            ISISadj neighbour;
            neighbour.state = false; //set state to initial

            //set timeout of neighbour
            neighbour.timer = new ISISTimer("Neighbour_timeout");
            neighbour.timer->setTimerKind(NEIGHBOUR_DEAD);
            neighbour.timer->setIsType(L2_TYPE);
            neighbour.timer->setInterfaceIndex(this->getIfaceIndex(this->getIfaceByGateIndex(neighbour.gateIndex)));

            //set source system ID in neighbour record & in timer to identify it
            for (unsigned int the_game = 0; the_game < msg->getSourceIDArraySize(); the_game++)
            {
                neighbour.sysID[the_game] = msg->getSourceID(the_game);
                neighbour.timer->setSysID(the_game, msg->getSourceID(the_game));
            }
            //set neighbours area ID
            tmpTLV = this->getTLVByType(msg, AREA_ADDRESS);

             this->copyArrayContent(tmpTLV->value, neighbour.areaID, tmpTLV->length, 0, 0);

             for(unsigned int z=0; z<tmpTLV->length; z++)
             {
                 neighbour.timer->setAreaID(z, tmpTLV->value[z]);
             }

            //get source MAC address of received frame
            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
            neighbour.mac = ctrl->getSrc();

            //set gate index, which is neighbour connected to
            neighbour.gateIndex = msg->getArrivalGate()->getIndex();

            //TODO predelat na this->schedule()
            scheduleAt(simTime() + msg->getHoldTime(), neighbour.timer);

            //insert neighbour into adjL2Table
            adjL2Table.push_back(neighbour);

            //EV << "deviceId " << deviceId << ": new adjacency\n";

        }

    }

}

bool ISIS::isAdjBySystemID(unsigned char *systemID, short ciruitType)
{
    if (ciruitType == L1_TYPE)
    {
        //walk through adjacency table and look for existing neighbours
        for (std::vector<ISISadj>::iterator it = this->adjL1Table.begin(); it != adjL1Table.end(); ++it)
        {
            if (compareArrays(systemID, (*it).sysID, ISIS_SYSTEM_ID))
            {
                return true;
            }

        }
    }else if(ciruitType == L2_TYPE)
    {
        //walk through adjacency table and look for existing neighbours
        for (std::vector<ISISadj>::iterator it = this->adjL2Table.begin(); it != adjL2Table.end(); ++it)
        {
            if (compareArrays(systemID, (*it).sysID, ISIS_SYSTEM_ID))
            {
                return true;
            }

        }
    }

    return false;
}
/**
 * This method check adjacency in table based on inMsg type and compares System-ID, MAC address and gateIndex.
 * If any of these parameters don't match returns NULL, otherwise relevant adjacency;
 * @param inMsg is incoming message that is being parsed
 */
ISISadj* ISIS::getAdj(ISISMessage *inMsg, short circuitType)
{
//    short circuitType;
    unsigned char * systemID = new unsigned char[ISIS_SYSTEM_ID];
    std::vector<ISISadj> *adjTable;

    if (inMsg->getType() == LAN_L1_HELLO)
    {
        ISISL1HelloPacket *msg = check_and_cast<ISISL1HelloPacket *>(inMsg);
        systemID = this->getSysID(msg);
                    circuitType = L1_TYPE;
        adjTable = &(this->adjL1Table);
    }
    else if (inMsg->getType() == LAN_L2_HELLO)
    {
        ISISL2HelloPacket *msg = check_and_cast<ISISL2HelloPacket *>(inMsg);
        systemID = this->getSysID(msg);
                    circuitType = L2_TYPE;
        adjTable = &(this->adjL1Table);
    }
    else if (inMsg->getType() == PTP_HELLO)
    {
        ISISPTPHelloPacket *msg = check_and_cast<ISISPTPHelloPacket *>(inMsg);
        systemID = this->getSysID(msg);
//        circuitType = msg->getCircuitType();

        adjTable = &(this->adjL1Table);
        if (circuitType == L1_TYPE)
        {
            adjTable = &(this->adjL1Table);
        }
        else if (circuitType == L2_TYPE)
        {
            adjTable = &(this->adjL2Table);
        }
    }


    for (std::vector<ISISadj>::iterator it = adjTable->begin(); it != adjTable->end(); ++it)
    {
        //System-ID match?
        if (compareArrays(systemID, (*it).sysID, ISIS_SYSTEM_ID))
        {
            int gateIndex = inMsg->getArrivalGate()->getIndex();
//            ISISinterface * tmpIntf = this->getIfaceByGateIndex(gateIndex);
            //TODO for truly point-to-point link there would not be MAC address
            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(inMsg->getControlInfo());
            MACAddress tmpMac = ctrl->getSrc();

            //MAC Address and gateIndex
            //we need to check source (tmpMac) and destination interface thru we received this hello

            if (tmpMac.compareTo((*it).mac) == 0 && gateIndex == (*it).gateIndex)
            {
                if(circuitType == L2_TYPE){
                    //check if at least one areaId matches our areaId (don't do this for L2)
                    bool areaOK = false;
                    TLV_t* tmpTLV;
                    tmpTLV = this->getTLVByType(inMsg, AREA_ADDRESS, 0);
                    for (int i = 0; tmpTLV  != NULL; i++)
                    {
                        areaOK = areaOK || this->isAreaIDOK(tmpTLV, (*it).areaID);
                        tmpTLV = this->getTLVByType(inMsg, AREA_ADDRESS, i);
                    }

                    if (!areaOK)
                    {
                        //TODO schedule event AreaIDMismatch
//                        EV << "ISIS: Warning: L1_LAN_HELLO doesn't contain Area address TLV." << endl;
                        break;
                    }
                }
                return &(*it);
            }
        }
    }
    return NULL;

}
/**
 * Don't use this method, but rather ISIS::getAdj(ISISMessage *) which can handle multiple adjacencies between two ISes.
 *
 */
ISISadj *ISIS::getAdjBySystemID(unsigned char *systemID, short circuitType, int gateIndex){

    /* For redundant links there could be more than one adjacency for the same System-ID.
     * We should return std::vector<ISISadj> with all adjacencies */

    if (circuitType == L1_TYPE)
    {

        for (std::vector<ISISadj>::iterator it = this->adjL1Table.begin(); it != this->adjL1Table.end(); ++it)
        {
            if (compareArrays(systemID, (*it).sysID, ISIS_SYSTEM_ID))
            {
                if (gateIndex > -1 && (*it).gateIndex != gateIndex)
                {
                    continue;
                }
                return &(*it);
            }

        }
    }
    else if (circuitType == L2_TYPE)
    {

        for (std::vector<ISISadj>::iterator it = this->adjL2Table.begin(); it != this->adjL2Table.end(); ++it)
        {
            if (compareArrays(systemID, (*it).sysID, ISIS_SYSTEM_ID))
            {
                if (gateIndex > -1 && (*it).gateIndex != gateIndex)
                {
                    continue;
                }
                return &(*it);
            }

        }
    }else if (circuitType == L1L2_TYPE){
        EV << "ISIS: ERROR: getAdjBySystemID for L1L2_TYPE is not implemented (yet)" <<endl;
        //TODO
        /* For point-to-point link there should be only ONE adjacency in both tables*/
        for (std::vector<ISISadj>::iterator it = this->adjL1Table.begin(); it != this->adjL1Table.end(); ++it)
        {
            if (compareArrays(systemID, (*it).sysID, ISIS_SYSTEM_ID))
            {
                return &(*it);
            }

        }

    }
    return NULL;
}

unsigned char * ISIS::getSysID(ISISMessage *msg)
{

    unsigned char *systemID = new unsigned char[ISIS_SYSTEM_ID];
    if(msg->getType() == LAN_L1_HELLO){
                    ISISL1HelloPacket *l1hello = check_and_cast<ISISL1HelloPacket *>(msg);
                    for (int i = 0; i < ISIS_SYSTEM_ID; i++)
                    {
                        systemID[i] = l1hello->getSourceID(i);
                    }
    }else if(msg->getType() == LAN_L2_HELLO){
                    ISISL2HelloPacket *l2hello = check_and_cast<ISISL2HelloPacket *>(msg);
                    for (int i = 0; i < ISIS_SYSTEM_ID; i++)
                    {
                        systemID[i] = l2hello->getSourceID(i);
                    }
    }else if(msg->getType() == PTP_HELLO){
                    ISISPTPHelloPacket *ptphello = check_and_cast<ISISPTPHelloPacket *>(msg);
                    for (int i = 0; i < ISIS_SYSTEM_ID; i++)
                    {
                        systemID[i] = ptphello->getSourceID(i);
                    }

    }


    return systemID;

}

unsigned char * ISIS::getSysID(ISISL1HelloPacket *msg){
    unsigned char *systemID = new unsigned char[ISIS_SYSTEM_ID];

    for (int i = 0; i < ISIS_SYSTEM_ID; i++)
    {
        systemID[i] = msg->getSourceID(i);
    }

    return systemID;
}

unsigned char * ISIS::getSysID(ISISL2HelloPacket *msg){
    unsigned char *systemID = new unsigned char[ISIS_SYSTEM_ID];

    for (int i = 0; i < ISIS_SYSTEM_ID; i++)
    {
        systemID[i] = msg->getSourceID(i);
    }

    return systemID;
}

unsigned char * ISIS::getSysID(ISISPTPHelloPacket *msg){
    unsigned char *systemID = new unsigned char[ISIS_SYSTEM_ID];

    for (int i = 0; i < ISIS_SYSTEM_ID; i++)
    {
        systemID[i] = msg->getSourceID(i);
    }

    return systemID;
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
        for(unsigned int i=0; i<adjL1Table.size();)
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
            }else{
                i++;
            }
        }


        //clear LSP containing dead neighbour
        for(unsigned int i=0; i<L1LSP.size(); )
        {
            bool found = true;
            for(unsigned int j=0; j<msg->getSysIDArraySize(); j++)
            {

                if(msg->getSysID(j) != L1LSP.at(i).LSPid[j])
                    found = false;


            }
            if(found)
               {
                   //mark with sequence number 0
                   L1LSP.at(i).seq = 0;
                   L1LSP.at(i).neighbours.clear();
                   //send empty LSP informing about expiration
                   this->sendSpecificL1LSP(L1LSP.at(i).LSPid);
                   //now completely delete
                   L1LSP.erase(L1LSP.begin() + i);
               }else{
                   i++;

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

    //compare LAN-ID from message aka LAN DIS with supposed DIS
    bool equal = true;
    unsigned char * msgLanID = new unsigned char [7];
    for(unsigned int k=0; k<msg->getLanIDArraySize(); k++)
    {
        msgLanID[k] = msg->getLanID(k);
        if(msg->getLanID(k) != ISISIft.at(i).L1DIS[k]){
            equal = false;

        }
    }

    bool last = this->amIL1DIS(i);
    unsigned char* lastDIS = new unsigned char[ISIS_SYSTEM_ID + 1];
    for (unsigned int k = 0; k < msg->getLanIDArraySize(); k++)
    {
        lastDIS[k] = ISISIft.at(i).L1DIS[k];
    }

    MACAddress localDIS, receivedDIS;
    ISISadj *tmpAdj;

    MACAddress tmpMAC = ISISIft.at(i).entry->getMacAddress();


    //first old/local DIS
    //if DIS == me then use my MAC for specified interface
    if (this->amIL1DIS(i))
    {
        localDIS = ISISIft.at(i).entry->getMacAddress();
    }
    //else find adjacency and from that use MAC
    else
    {
        if((tmpAdj = this->getAdjBySystemID(lastDIS, L1_TYPE, ISISIft.at(i).gateIndex)) != NULL){
            localDIS = tmpAdj->mac;
        }else{
            EV << "deviceId: " << deviceId << " ISIS: Warning: Didn't find adjacency for local MAC comparison in electL1DesignatedIS "<<endl;
            localDIS = MACAddress("000000000000");
        }
    }
    //find out MAC address for IS with LAN-ID from received message
    if ((tmpAdj = this->getAdjBySystemID(msgLanID, L1_TYPE, ISISIft.at(i).gateIndex)) != NULL)
    {
        receivedDIS = tmpAdj->mac;
    }
    else
    {
        EV
                << "deviceId: " << deviceId
                        << " ISIS: Warning: Didn't find adjacency for received MAC comparison in electL1DesignatedIS "
                        << endl;
        localDIS = MACAddress("000000000000");
    }


    //if announced DIS priority is higher then actual one or if they are equal and src MAC is higher than mine, then it's time to update DIS
    if ((!equal)
            && ((msg->getPriority() > ISISIft.at(i).L1DISpriority)
                    || (msg->getPriority() == ISISIft.at(i).L1DISpriority
                            && (receivedDIS.compareTo(localDIS) > 0) )))
    {

        for (unsigned int j = 0; j < msg->getLanIDArraySize(); j++)
        {
            //set new DIS
            ISISIft.at(i).L1DIS[j] = msg->getLanID(j);
        }
        //and set his priority
        ISISIft.at(i).L1DISpriority = msg->getPriority();

        //purge DIS LSP
        //clear LSP containing dead neighbour
        for (unsigned int it = 0; it < L1LSP.size();)
        {
            bool found = false;
            if (this->compareArrays(this->L1LSP.at(it).LSPid, lastDIS, ISIS_SYSTEM_ID + 1))
            {
                found = true;

            }

            if (found)
            {
                //mark with sequence number 0
                L1LSP.at(it).seq = 0;
                L1LSP.at(it).neighbours.clear();
                //send empty LSP informing about expiration
                this->sendSpecificL1LSP(L1LSP.at(it).LSPid);
                //now completely delete
                L1LSP.erase(L1LSP.begin() + it);

            }
            else
            {
                it++;

            }
        }

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
    scheduleAt(simTime() + 18.0, timer);//TODO

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
     * Value� Multiples of LSP summaries, each consisting of the remaining lifetime (2 bytes), LSP ID (ID length + 2 bytes),
     * LSP sequence number(4 bytes), and LSP checksum (2 bytes). But we ignore LSP checksum so length of each LSP record in CSNP is
     * 2 bytes smaller.
     */
    for(unsigned int i=0;i<L1LSP.size(); i++)
    {
        //get remaining lifetime //TODO change value to constant or something
        unsigned short remTime = 50 - ((unsigned short)(simTime().dbl()) - (unsigned short)(L1LSP.at(i).deadTimer->getCreationTime().dbl()));

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
             * TLV Value� Multiples of LSP summaries, each consisting of the remaining lifetime (2 bytes), LSP ID (ID length + 2 bytes),
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
     * Value� Multiples of LSP summaries, each consisting of the remaining lifetime (2 bytes), LSP ID (ID length + 2 bytes),
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
             * TLV Value Multiples of LSP summaries, each consisting of the remaining lifetime (2 bytes), LSP ID (ID length + 2 bytes),
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
 * @return True if neighbour's sysId is the same as mine, false otherwise.
 */
bool ISIS::checkDuplicateSysID(ISISMessage * msg)
{

    bool test = false;

    if (msg->getType() == LAN_L1_HELLO || msg->getType() == LAN_L2_HELLO)
    {

        // typecast for L1 hello; L1 and L2 hellos differ only in "type" field
        ISISL1HelloPacket *hello = check_and_cast<ISISL1HelloPacket *>(msg);

        //check for area address tlv and compare with my area id
        for (unsigned int j = 0; j < hello->getTLVArraySize(); j++)
        {
            if (hello->getTLV(j).type == AREA_ADDRESS
                    && this->compareArrays((unsigned char *) this->areaId, hello->getTLV(j).value,
                            hello->getTLV(j).length))
            {

                bool equal = true;

                //compare sys ID
                for (unsigned int i = 0; i < hello->getSourceIDArraySize(); i++)
                {
                    if (this->sysId[i] != hello->getSourceID(i))
                        equal = false;
                }

                test = equal;
                break;
            }
        }
    }
    else if (msg->getType() == PTP_HELLO)
    {
        test = true;
        ISISPTPHelloPacket *hello = check_and_cast<ISISPTPHelloPacket *>(msg);
        for (unsigned int i = 0; i < hello->getSourceIDArraySize(); i++)
        {
            if (this->sysId[i] != hello->getSourceID(i))
            {
                test = false;
                break;
            }
        }
    }

    if (test)
    {
        EV << this->deviceId << ": IS-IS WARNING: possible duplicate system ID ";
        for (unsigned g = 0; g < 6; g++)
        {
            EV << setfill('0') << setw(2) << dec << (unsigned int) sysId[g];
            if (g == 1 || g == 3)
                EV << ".";
        }
        EV << " detected" << endl;
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
            scheduleAt(simTime() + 50, L1LSP.at(j).deadTimer);   //should be 1200 secs.

            //set neighbours (pseudonode neighbours)
            std::vector<LSPneighbour> neighbours;

            //as we are using ethernet, which is multiaccess medium, we have to add pseudonodes as IS neighbours
            //at this point, network should be converged and assigned appropriate designated IS for each LAN

            for(unsigned int i=0; i<ISISIft.size(); i++)
            {
                if(ISISIft.at(i).ISISenabled && (ISISIft.at(i).circuitType == L1_TYPE || ISISIft.at(i).circuitType == L1L2_TYPE))
                {
                    LSPneighbour neighbour;
                    if (ISISIft.at(i).network)
                    {
                        this->copyArrayContent(ISISIft.at(i).L1DIS, neighbour.LANid, 7, 0, 0);
                    }
                    else
                    {
                        this->copyArrayContent(this->getAdjByGateIndex(ISISIft.at(i).gateIndex)->sysID, neighbour.LANid, ISIS_SYSTEM_ID, 0, 0);
                        neighbour.LANid[6] = 0;

                    }

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
        for(unsigned int x = 0; x < record.deadTimer->getLSPidArraySize(); x++)
        {
            record.deadTimer->setLSPid(x, myLSPid[x]);
        }
        scheduleAt(simTime() + 50.0, record.deadTimer);//TODO

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
                        if(ISISIft.at(i).network){
                            this->copyArrayContent(ISISIft.at(i).L1DIS, neighbour.LANid, 7, 0, 0);
                        }else{
                            this->copyArrayContent(this->adjL1Table.at(f).sysID, neighbour.LANid, ISIS_SYSTEM_ID, 0, 0);
                            neighbour.LANid[6]=0;

                        }

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
        //check if this interface is DIS for LAN AND it is broadcast interface
        if(this->compareArrays((unsigned char*) this->sysId, ISISIft.at(i).L1DIS, 6) && ISISIft.at(i).network)
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
                    scheduleAt(simTime() + 50, L1LSP.at(j).deadTimer);   //should be 1200 secs.

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
                scheduleAt(simTime() + 50.0, record.deadTimer);

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


/**
 * Since this method is used for point-to-point links only one adjacency is expected
 * @param gateIndex index to global interface table
 */

ISISadj* ISIS::getAdjByGateIndex(int gateIndex){

    for (std::vector<ISISadj>::iterator it = this->adjL1Table.begin(); it != this->adjL1Table.end(); ++it)
    {
        if ((*it).gateIndex == gateIndex)
        {
            return &(*it);
        }
    }

    for (std::vector<ISISadj>::iterator it = this->adjL2Table.begin(); it != this->adjL2Table.end(); ++it)
    {
        if ((*it).gateIndex == gateIndex)
        {
            return &(*it);
        }
    }

    return NULL;


}

/**
 *
 * @param gateIndex index to global interface table
 */

ISISinterface* ISIS::getIfaceByGateIndex(int gateIndex){

    for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
    {

        if ((*it).gateIndex == gateIndex)
        {
            return &(*it);
        }
    }
    return NULL;
}


/**
 * @param interfaceIndex is index to ISISIft vector
 */
unsigned short ISIS::getHoldTime(int interfaceIndex, short circuitType){

    int dis1, dis2;
    dis1 = dis2 = 1;
    ISISinterface tmpIntf = this->ISISIft.at(interfaceIndex);
    //TODO check whether this interface is DIS and if so, return only 1/3
    if (this->amIL1DIS(interfaceIndex))
    {
        dis1 = 3;
    }

    if (this->amIL2DIS(interfaceIndex))
    {
        dis2 = 3;
    }

    if (circuitType == L1_TYPE)
    {
        if (tmpIntf.L1HelloInterval == 0)
        {
            return 1;
        }
        else
        {
            return (tmpIntf.L1HelloMultiplier * tmpIntf.L1HelloInterval) / dis1;
        }
    }
    else if (circuitType == L2_TYPE)
    {
        if (tmpIntf.L2HelloInterval == 0)
        {
            return 1;
        }
        else
        {
            return (tmpIntf.L2HelloMultiplier * tmpIntf.L2HelloInterval) / dis2;
        }

    }
    else if (circuitType == L1L2_TYPE)
    {
        //return smaller of L1 and L2 hold timers
        return (this->getHoldTime(interfaceIndex, L1_TYPE) < this->getHoldTime(interfaceIndex, L2_TYPE) ? this->getHoldTime(
                interfaceIndex, L1_TYPE) :
                this->getHoldTime(interfaceIndex, L2_TYPE));

    }
    else
    {
        return 1;
        EV << "deviceId " << deviceId << ": Warning: Unrecognized circuitType used in ISIS::getHoldTime()\n";
    }
    return 1; //this is here just so OMNeT wouldn't bother me with "no return in non-void method" warning
}


double ISIS::getHelloInterval(int interfaceIndex, short circuitType){

    //check if interface is DIS for specified circuitType (only on broadcast interfaces)
    //for DIS interface return one third of actual value
    int dis1, dis2;
    dis1 = dis2 = 1;
    ISISinterface tmpIntf = this->ISISIft.at(interfaceIndex);
    //TODO check whether this interface is DIS and if so, return only 1/3
    if (this->amIL1DIS(interfaceIndex))
    {
        dis1 = 3;
    }

    if (this->amIL2DIS(interfaceIndex))
    {
        dis2 = 3;
    }

    if (circuitType == L1_TYPE)
        {
            if (tmpIntf.L1HelloInterval == 0)
            {
                return 1 / tmpIntf.L1HelloMultiplier;
            }
            else
            {
                return (tmpIntf.L1HelloInterval) / dis1;
            }
        }
        else if (circuitType == L2_TYPE)
        {
            if (tmpIntf.L2HelloInterval == 0)
            {
                return 1 / tmpIntf.L2HelloMultiplier;
            }
            else
            {
                return (tmpIntf.L2HelloInterval) / dis2;
            }

        }
        else if (circuitType == L1L2_TYPE)
        {
            EV << "deviceId " << deviceId << ": Warning: Are you sure you want to know Hello interval for L1L2_TYPE ?!? in ISIS::getHelloInterval()\n";
            //return smaller of L1 and L2 hold timers
            return (this->getHelloInterval(interfaceIndex, L1_TYPE) < this->getHelloInterval(interfaceIndex, L2_TYPE) ? this->getHelloInterval(
                    interfaceIndex, L1_TYPE) :
                    this->getHelloInterval(interfaceIndex, L2_TYPE));

        }
        else
        {
            return 1;
            EV << "deviceId " << deviceId << ": Warning: Unrecognized circuitType used in ISIS::getHelloInterval()\n";
        }

return 0.0;



}

bool ISIS::amIL1DIS(int interfaceIndex){
    //if this is not broadcast interface then no DIS is elected
    if(!this->ISISIft.at(interfaceIndex).network){
        return false;
    }

    return (compareArrays((unsigned char *) this->sysId, this->ISISIft.at(interfaceIndex).L1DIS, ISIS_SYSTEM_ID));

}

bool ISIS::amIL2DIS(int interfaceIndex){
    //if this is not broadcast interface then no DIS is elected
    if(!this->ISISIft.at(interfaceIndex).network){
        return false;
    }

    return (compareArrays((unsigned char *) this->sysId, this->ISISIft.at(interfaceIndex).L2DIS, ISIS_SYSTEM_ID));

}

TLV_t* ISIS::getTLVByType(ISISMessage *inMsg, enum TLVtypes tlvType, int offset){

    if (inMsg->getType() == LAN_L1_HELLO)
    {
        ISISL1HelloPacket *msg = check_and_cast<ISISL1HelloPacket *>(inMsg);
        return this->getTLVByType(msg, tlvType, offset);
    }
    else if (inMsg->getType() == LAN_L2_HELLO)
    {
        ISISL2HelloPacket *msg = check_and_cast<ISISL2HelloPacket *>(inMsg);
        return this->getTLVByType(msg, tlvType, offset);
    }
    else if (inMsg->getType() == PTP_HELLO)
    {
        ISISPTPHelloPacket *msg = check_and_cast<ISISPTPHelloPacket *>(inMsg);
        return this->getTLVByType(msg, tlvType, offset);
    }
    return NULL;
}


TLV_t* ISIS::getTLVByType(ISISL1HelloPacket *msg, enum TLVtypes tlvType, int offset){

    for(unsigned int i = 0; i < msg->getTLVArraySize(); i++){
        if(msg->getTLV(i).type == tlvType){
            if(offset == 0){
                return &(msg->getTLV(i));
            }else{
                offset--;
            }
        }
    }

    return NULL;
}

TLV_t* ISIS::getTLVByType(ISISL2HelloPacket *msg, enum TLVtypes tlvType, int offset){

    for(unsigned int i = 0; i < msg->getTLVArraySize(); i++){
        if(msg->getTLV(i).type == tlvType){
            if(offset == 0){
                return &(msg->getTLV(i));
            }else{
                offset--;
            }
        }
    }

    return NULL;
}

TLV_t* ISIS::getTLVByType(ISISPTPHelloPacket *msg, enum TLVtypes tlvType, int offset){

    for(unsigned int i = 0; i < msg->getTLVArraySize(); i++){
        if(msg->getTLV(i).type == tlvType){
            if(offset == 0){
                return &(msg->getTLV(i));
            }else{
                offset--;
            }
        }
    }

    return NULL;
}

bool ISIS::isMessageOK(ISISMessage *inMsg){

    if(inMsg->getIdLength() != ISIS_SYSTEM_ID && inMsg->getIdLength() != 0){
        return false;
    }

    if(inMsg->getMaxAreas() != ISIS_MAX_AREAS && inMsg->getMaxAreas() != 0){
            return false;
        }


    return true;
}

bool ISIS::isAreaIDOK(TLV_t *areaAddressTLV, unsigned char *compare){

    //TODO if length == 0
    if(compare == NULL){
        compare = (unsigned char *) this->areaId;
    }

    if(areaAddressTLV->length == 0){
        return false;
    }
    for(unsigned int i = 0; i < areaAddressTLV->length / ISIS_AREA_ID; i++){
        if(this->compareArrays(compare, &areaAddressTLV->value[i * ISIS_AREA_ID], ISIS_AREA_ID)){
            //if one address match return false
            return true;
        }

    }
    //not even single areaID match, so return false
    return false;
}

int ISIS::getIfaceIndex(ISISinterface *interface){



    for (unsigned int i = 0; i < this->ISISIft.size(); i++)
    {

        if (interface == &(ISISIft.at(i)))
        {
            return i;
        }
    }
    return 0;
}


