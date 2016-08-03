// Copyright (C) 2012 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file ISIS.cc
 * @author Matej Hrncirik, Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 7.3.2012
 * @brief Base class for the IS-IS module.
 * @detail Base class for the IS-IS module.
 * @todo TODO B3 Multicast MAC adresses 01:80:c2:00:00:14 and :15 works so replace the ff:ff:...
 *       TODO A1 SEVERE call trillDIS() -> such method would appoint forwarder and handle other TRILL-DIS related duties
 *       TODO B3 Add SimTime isisStarted; and compute initial wait period (for LSP generating and SPF calculation} from such variable
 */

#include "ISIS.h"
#include "TRILL.h"

#include "deviceConfigurator.h"
#include "TRILLInterfaceData.h"

Define_Module(ISIS);

ISIS::ISIS()
{
    this->L1LSPDb = new std::vector<LSPRecord*>;
    this->L2LSPDb = new std::vector<LSPRecord*>;

    this->L1SRMBQueue = new std::vector<std::vector<FlagRecord *> *>;
    this->L2SRMBQueue = new std::vector<std::vector<FlagRecord *> *>;

    this->L1SRMPTPQueue = new std::vector<std::vector<FlagRecord *> *>;
    this->L2SRMPTPQueue = new std::vector<std::vector<FlagRecord *> *>;

    this->L1SSNBQueue = new std::vector<std::vector<FlagRecord *> *>;
    this->L2SSNBQueue = new std::vector<std::vector<FlagRecord *> *>;

    this->L1SSNPTPQueue = new std::vector<std::vector<FlagRecord *> *>;
    this->L2SSNPTPQueue = new std::vector<std::vector<FlagRecord *> *>;

    this->attIS = new ISISNeighbours_t;

    att = false;
}

/*
 * This method deallocate dynamically created objects and free their memory.
 */
ISIS::~ISIS()
{
    if (this->L1LSPDb != NULL)
    {
        std::vector<LSPRecord *>::iterator it = this->L1LSPDb->begin();
        for (; it != this->L1LSPDb->end(); ++it)
        {
            //delete (*it)->LSP;
            //delete (*it)->deadTimer;
            delete (*it);
        }
        this->L1LSPDb->clear();
        delete this->L1LSPDb;
    }

    if (this->L2LSPDb != NULL)
    {
        std::vector<LSPRecord *>::iterator it = this->L2LSPDb->begin();
        for (; it != this->L2LSPDb->end(); ++it)
        {
            //delete (*it)->LSP;
            //delete (*it)->deadTimer;
            delete (*it);
        }
        this->L2LSPDb->clear();
        delete this->L2LSPDb;
    }

    /* delete SRM */
    if (this->L1SRMBQueue != NULL)
    {
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L1SRMBQueue->begin();
        for (; qIt != this->L1SRMBQueue->end(); ++qIt)
        {
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for (; it != (*qIt)->end(); ++it)
            {
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L1SRMBQueue->clear();
        delete this->L1SRMBQueue;
    }

    if (this->L1SRMPTPQueue != NULL)
    {
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L1SRMPTPQueue->begin();
        for (; qIt != this->L1SRMPTPQueue->end(); ++qIt)
        {
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for (; it != (*qIt)->end(); ++it)
            {
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L1SRMPTPQueue->clear();
        delete this->L1SRMPTPQueue;
    }

    if (this->L2SRMBQueue != NULL)
    {
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L2SRMBQueue->begin();
        for (; qIt != this->L2SRMBQueue->end(); ++qIt)
        {
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for (; it != (*qIt)->end(); ++it)
            {
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L2SRMBQueue->clear();
        delete this->L2SRMBQueue;
    }

    if (this->L2SRMPTPQueue != NULL)
    {
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L2SRMPTPQueue->begin();
        for (; qIt != this->L2SRMPTPQueue->end(); ++qIt)
        {
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for (; it != (*qIt)->end(); ++it)
            {
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L2SRMPTPQueue->clear();
        delete this->L2SRMPTPQueue;
    }
    /* end of delete SRM */

    /* delete SSN */
    if (this->L1SSNBQueue != NULL)
    {
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L1SSNBQueue->begin();
        for (; qIt != this->L1SSNBQueue->end(); ++qIt)
        {
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for (; it != (*qIt)->end(); ++it)
            {

                delete (*it);

            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L1SSNBQueue->clear();
        delete this->L1SSNBQueue;
    }

    if (this->L1SSNPTPQueue != NULL)
    {
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L1SSNPTPQueue->begin();
        for (; qIt != this->L1SSNPTPQueue->end(); ++qIt)
        {
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for (; it != (*qIt)->end(); ++it)
            {
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L1SSNPTPQueue->clear();
        delete this->L1SSNPTPQueue;
    }

    if (this->L2SSNBQueue != NULL)
    {
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L2SSNBQueue->begin();
        for (; qIt != this->L2SSNBQueue->end(); ++qIt)
        {
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for (; it != (*qIt)->end(); ++it)
            {
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L2SSNBQueue->clear();
        delete this->L2SSNBQueue;
    }

    if (this->L2SSNPTPQueue != NULL)
    {
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L2SSNPTPQueue->begin();
        for (; qIt != this->L2SSNPTPQueue->end(); ++qIt)
        {
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for (; it != (*qIt)->end(); ++it)
            {
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L2SSNPTPQueue->clear();
        delete this->L2SSNPTPQueue;
    }
    /* end of delete SSN */

}

/**
 * Handles notifications received through notification board.
 * @param is fired notification
 * @param detail is user defined pointer
 */
void ISIS::receiveChangeNotification(int category, const cObject *details)
{
    // TODO B1:
//    return;
    // ignore notifications during initialization
    if (simulation.getContextType() == CTX_INITIALIZE){
        return;
    }
    Enter_Method_Silent();
    if(category == NF_ISIS_ADJ_CHANGED){
        /*TODO B3 Make new timer Type and schedule it when you receive ADJ_CHANGED for short period of time (TBD)
         * and increment some internal counter. When the counter hit certain threshold, stop pushing the timer and wait for
         * timeout.
         * Achievement of this is that we bridge the short period of time when all adjacencies are coming up, so we don't
         * have to generate LSP, calculate SPF and such for every ajd go UP.
         * OR maybe set the timer only once so we meet the criterium of minimum time between LSP generation and SPF calculation as specified.
         *
         *
         */
//        ISISadj *adj = check_and_cast<ISISadj*>(details);
        ISISTimer *timer = check_and_cast<ISISTimer*>(details);

        generateLSP(timer);

        if(timer->getIsType() == L1_TYPE){
            cancelEvent(this->spfL1Timer);
            scheduleAt(simTime(), this->spfL1Timer);
        }else{
            cancelEvent(this->spfL2Timer);
            scheduleAt(simTime(), this->spfL2Timer);

        }

    }
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
    if(stage == 0){
         nb = NotificationBoardAccess().get();
//         nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
         nb->subscribe(this, NF_ISIS_ADJ_CHANGED);
    }

    if (stage == 1)
    {
        deviceType = string((const char *) par("deviceType"));
        deviceId = string((const char *) par("deviceId"));
        configFile = string((const char *) par("configFile"));
        if (deviceType == "Router")
        {
            this->mode = L3_ISIS_MODE;
        }
        else if (deviceType == "RBridge")
        {
            this->mode = L2_ISIS_MODE;
        }
        else
        {
            throw cRuntimeError("Unknown device type for IS-IS module");
        }

    }

    if (stage == 3)
    {
        DeviceConfigurator *devConf = ModuleAccess<DeviceConfigurator>("deviceConfigurator").get();
        devConf->loadISISConfig(this, this->mode);



        //TODO passive-interface
        //create SRMQueue for each interface (even though it would be used only for broadcast interfaces)
        for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
        {
            this->L1SRMBQueue->push_back(new std::vector<FlagRecord *>);
        }

        for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
        {
            this->L1SRMPTPQueue->push_back(new std::vector<FlagRecord *>);
        }

        //SSNflags
        for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
        {
            this->L1SSNBQueue->push_back(new std::vector<FlagRecord *>);
        }

        for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
        {
            this->L1SSNPTPQueue->push_back(new std::vector<FlagRecord *>);
        }

        //create SRMQueue for each interface (even though it would be used only for broadcast interfaces)
        for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
        {
            this->L2SRMBQueue->push_back(new std::vector<FlagRecord *>);
        }

        for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
        {
            this->L2SRMPTPQueue->push_back(new std::vector<FlagRecord *>);
        }

        //SSNflags
        for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
        {
            this->L2SSNBQueue->push_back(new std::vector<FlagRecord *>);
        }

        for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
        {
            this->L2SSNPTPQueue->push_back(new std::vector<FlagRecord *>);
        }

//        //TODO A3
//        this->L1SPFFullInterval = ISIS_SPF_FULL_INTERVAL;
//        this->L2SPFFullInterval = ISIS_SPF_FULL_INTERVAL;

    }
    else if (stage == 4)
    {
//        this->initISIS();
        ISISTimer *timerMsg = new ISISTimer("ISIS Start", ISIS_START_TIMER);
        timerMsg->setTimerKind(ISIS_START_TIMER);
        this->schedule(timerMsg);

    }

}

/**
 * Set initial parameters of network interfaces.
 * @param entry Pointer to interface record in interfaceTable
 * @param intElement XML element of current interface in XML config file
 */
void ISIS::insertIft(InterfaceEntry *entry, cXMLElement *intElement)
{

    if (intElement == NULL)
    {
        return;
    }
    ISISinterface newIftEntry;
    newIftEntry.intID = entry->getInterfaceId();

    newIftEntry.gateIndex = entry->getNetworkLayerGateIndex();
    EV<< "deviceId: " << this->deviceId << "ISIS: adding interface, gateIndex: " << newIftEntry.gateIndex << endl;

    //set interface priority
    newIftEntry.priority = ISIS_DIS_PRIORITY; //default value

    /* Interface is NOT enabled by default. If ANY IS-IS related property is configured on interface then it's enabled. */
    newIftEntry.ISISenabled = false;

    cXMLElement *priority = intElement->getFirstChildWithTag("ISIS-Priority");
    if (priority != NULL && priority->getNodeValue() != NULL)
    {
        newIftEntry.priority = (unsigned char) atoi(priority->getNodeValue());
        newIftEntry.ISISenabled = true;
    }

    //set network type (point-to-point vs. broadcast)

    newIftEntry.network = true; //default value

    cXMLElement *network = intElement->getFirstChildWithTag("ISIS-Network");
    if (network != NULL && network->getNodeValue() != NULL)
    {
        if (!strcmp(network->getNodeValue(), "point-to-point"))
        {
            newIftEntry.network = false;
            EV<< "Interface network type is point-to-point " << network->getNodeValue() << endl;
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

    newIftEntry.metric = ISIS_METRIC; //default value

    cXMLElement *metric = intElement->getFirstChildWithTag("ISIS-Metric");
    if (metric != NULL && metric->getNodeValue() != NULL)
    {
        newIftEntry.metric = (unsigned char) atoi(metric->getNodeValue());
        newIftEntry.ISISenabled = true;
    }

    //set interface type according to global router configuration
    switch (this->isType)
        {
        case (L1_TYPE):
            newIftEntry.circuitType = L1_TYPE;
            break;
        case (L2_TYPE):
            newIftEntry.circuitType = L2_TYPE;
            break;
            //if router is type is equal L1L2, then interface configuration sets the type
        default: {

            newIftEntry.circuitType = L1L2_TYPE;

            cXMLElement *circuitType = intElement->getFirstChildWithTag("ISIS-Circuit-Type");
            if (circuitType != NULL && circuitType->getNodeValue() != NULL)
            {
                if (strcmp(circuitType->getNodeValue(), "L2") == 0)
                {
                    newIftEntry.circuitType = L2_TYPE;
                }
                else
                {
                    if (strcmp(circuitType->getNodeValue(), "L1") == 0)
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
    cXMLElement *L1HelloInt = intElement->getFirstChildWithTag("ISIS-L1-Hello-Interval");
    if (L1HelloInt == NULL || L1HelloInt->getNodeValue() == NULL)
    {
        newIftEntry.L1HelloInterval = this->L1HelloInterval;
    }
    else
    {
        newIftEntry.L1HelloInterval = atoi(L1HelloInt->getNodeValue());
    }

    //set L1 hello multiplier
    cXMLElement *L1HelloMult = intElement->getFirstChildWithTag("ISIS-L1-Hello-Multiplier");
    if (L1HelloMult == NULL || L1HelloMult->getNodeValue() == NULL)
    {
        newIftEntry.L1HelloMultiplier = this->L1HelloMultiplier;
    }
    else
    {
        newIftEntry.L1HelloMultiplier = atoi(L1HelloMult->getNodeValue());
    }

    //set L2 hello interval in seconds
    cXMLElement *L2HelloInt = intElement->getFirstChildWithTag("ISIS-L2-Hello-Interval");
    if (L2HelloInt == NULL || L2HelloInt->getNodeValue() == NULL)
    {
        newIftEntry.L2HelloInterval = this->L2HelloInterval;
    }
    else
    {
        newIftEntry.L2HelloInterval = atoi(L2HelloInt->getNodeValue());
    }

    //set L2 hello multiplier
    cXMLElement *L2HelloMult = intElement->getFirstChildWithTag("ISIS-L2-Hello-Multiplier");
    if (L2HelloMult == NULL || L2HelloMult->getNodeValue() == NULL)
    {
        newIftEntry.L2HelloMultiplier = this->L2HelloMultiplier;
    }
    else
    {
        newIftEntry.L2HelloMultiplier = atoi(L2HelloMult->getNodeValue());
    }

    //set lspInterval
    cXMLElement *cxlspInt = intElement->getFirstChildWithTag("ISIS-LSP-Interval");
    if (cxlspInt == NULL || cxlspInt->getNodeValue() == NULL)
    {
        newIftEntry.lspInterval = ISIS_LSP_INTERVAL;
    }
    else
    {
        newIftEntry.lspInterval = atoi(cxlspInt->getNodeValue());
    }

    //set L1CsnpInterval
    cXMLElement *cxL1CsnpInt = intElement->getFirstChildWithTag("ISIS-L1-CSNP-Interval");
    if (cxL1CsnpInt == NULL || cxL1CsnpInt->getNodeValue() == NULL)
    {
        newIftEntry.L1CsnpInterval = ISIS_CSNP_INTERVAL;
    }
    else
    {
        newIftEntry.L1CsnpInterval = atoi(cxL1CsnpInt->getNodeValue());
    }

    //set L2CsnpInterval
    cXMLElement *cxL2CsnpInt = intElement->getFirstChildWithTag("ISIS-L2-CSNP-Interval");
    if (cxL2CsnpInt == NULL || cxL2CsnpInt->getNodeValue() == NULL)
    {
        newIftEntry.L2CsnpInterval = ISIS_CSNP_INTERVAL;
    }
    else
    {
        newIftEntry.L2CsnpInterval = atoi(cxL2CsnpInt->getNodeValue());
    }

    //set L1PsnpInterval
    cXMLElement *cxL1PsnpInt = intElement->getFirstChildWithTag("ISIS-L1-PSNP-Interval");
    if (cxL1PsnpInt == NULL || cxL1PsnpInt->getNodeValue() == NULL)
    {
        newIftEntry.L1PsnpInterval = ISIS_CSNP_INTERVAL;
    }
    else
    {
        newIftEntry.L1PsnpInterval = atoi(cxL1PsnpInt->getNodeValue());
    }

    //set L2PsnpInterval
    cXMLElement *cxL2PsnpInt = intElement->getFirstChildWithTag("ISIS-L2-PSNP-Interval");
    if (cxL2PsnpInt == NULL || cxL2PsnpInt->getNodeValue() == NULL)
    {
        newIftEntry.L2PsnpInterval = ISIS_CSNP_INTERVAL;
    }
    else
    {
        newIftEntry.L2PsnpInterval = atoi(cxL2PsnpInt->getNodeValue());
    }

    // priority is not needed for point-to-point, but it won't hurt
    // set priority of current DIS = me at start
    newIftEntry.L1DISpriority = newIftEntry.priority;
    newIftEntry.L2DISpriority = newIftEntry.priority;

    //set initial designated IS as himself
    this->copyArrayContent((unsigned char*) this->sysId, newIftEntry.L1DIS, ISIS_SYSTEM_ID, 0, 0);
    //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
    //newIftEntry.L1DIS[6] = entry->getInterfaceId() - 99;
    newIftEntry.L1DIS[ISIS_SYSTEM_ID] = newIftEntry.gateIndex + 1;
    //do the same for L2 DIS
    this->copyArrayContent((unsigned char*) this->sysId, newIftEntry.L2DIS, ISIS_SYSTEM_ID, 0, 0);
    //newIftEntry.L2DIS[6] = entry->getInterfaceId() - 99;
    newIftEntry.L2DIS[ISIS_SYSTEM_ID] = newIftEntry.gateIndex + 1;

    newIftEntry.passive = false;
    newIftEntry.entry = entry;
    this->ISISIft.push_back(newIftEntry);
}

/**
 * Initiate scheduling timers.
 */
void ISIS::initISIS()
{
    if (this->mode == ISIS::L3_ISIS_MODE)
    {
        this->initHello();
    }
    else
    {
        this->initTRILLHello();
    }
    this->initGenerate();
    this->initRefresh(); //this could be called after at least one adjcency becomes UP
    this->initPeriodicSend();
    this->initCsnp(); //this could be called within initRefresh();
    this->initPsnp(); //see above
    this->initSPF();

}

/*
 * Initiate scheduling Hello timers.
 */
void ISIS::initHello()
{
    ISISTimer *timerMsg;
    ISISinterface *iface;
    EV<< "ISIS: initHello()" << endl;
    for (unsigned int k = 0; k < this->ISISIft.size(); k++)
    {
        //don't schedule Hello message on Loopback interfaces
        if (this->ISISIft.at(k).entry->isLoopback())
        {
            continue;
        }

        //schedule Hello timer per level => check if L1L2 on broadcast => schedule two timers
        //on PTP is L1L2 Hello valid timer
        iface = &(this->ISISIft.at(k));

        //don't schedule sending hello PDU on passive or not ISIS-enabled interface
        if (!iface->ISISenabled || iface->passive)
        {
            continue;
        }

        if (iface->network && iface->circuitType == L1L2_TYPE)
        {
            timerMsg = new ISISTimer("Hello_L1_timer");
            timerMsg->setTimerKind(HELLO_TIMER);
            timerMsg->setIsType(L1_TYPE);
            timerMsg->setInterfaceIndex(k);
            timerMsg->setGateIndex(iface->gateIndex);
            this->schedule(timerMsg);

            timerMsg = new ISISTimer("Hello_L2_timer");
            timerMsg->setTimerKind(HELLO_TIMER);
            timerMsg->setIsType(L2_TYPE);
            timerMsg->setInterfaceIndex(k);
            timerMsg->setGateIndex(iface->gateIndex);
            this->schedule(timerMsg);

        }
        else
        {
            timerMsg = new ISISTimer("Hello_timer");
            timerMsg->setTimerKind(HELLO_TIMER);
            timerMsg->setIsType(iface->circuitType);
            timerMsg->setInterfaceIndex(k);
            timerMsg->setGateIndex(iface->gateIndex);
            this->schedule(timerMsg);

        }
    }
}

/*
 * Initiate scheduling TRILL Hello timers.
 */
void ISIS::initTRILLHello()
{
    ISISTimer *timerMsg;
    ISISinterface *iface;
    EV<< "ISIS: initTRILLHello()" << endl;
    for (unsigned int k = 0; k < this->ISISIft.size(); k++)
    {
        //don't schedule Hello message on Loopback interfaces
        if (this->ISISIft.at(k).entry->isLoopback())
        {
            continue;
        }

        //schedule Hello timer per level => check if L1L2 on broadcast => schedule two timers
        //on PTP is L1L2 Hello valid timer
        iface = &(this->ISISIft.at(k));

        //don't schedule sending hello PDU on passive or not ISIS-enabled interface
        if (!iface->ISISenabled || iface->passive)
        {
            continue;
        }

        timerMsg = new ISISTimer("TRILL_HELLO_timer");
        timerMsg->setTimerKind(TRILL_HELLO_TIMER);
        timerMsg->setIsType(iface->circuitType);
        timerMsg->setInterfaceIndex(k);
        timerMsg->setGateIndex(iface->gateIndex);
        scheduleAt(simTime(), timerMsg);
//        this->schedule(timerMsg);

    }
}

/*
 * Initial schedule of timer for generating LSPs
 */
void ISIS::initGenerate()
{
    ISISTimer *timerMsg;

    if (this->isType == L1L2_TYPE)
    {
        timerMsg = new ISISTimer("Generate LSPs timer");
        timerMsg->setTimerKind(GENERATE_LSP_TIMER);
        timerMsg->setIsType(L1_TYPE);
        this->genL1LspTimer = timerMsg;
        this->schedule(timerMsg);

        timerMsg = new ISISTimer("Generate LSPs timer");
        timerMsg->setTimerKind(GENERATE_LSP_TIMER);
        timerMsg->setIsType(L2_TYPE);
        this->genL2LspTimer = timerMsg;
        this->schedule(timerMsg);

    }
    else
    {
        timerMsg = new ISISTimer("Generate LSPs timer");
        timerMsg->setTimerKind(GENERATE_LSP_TIMER);
        timerMsg->setIsType(this->isType);
        if(this->isType == L1_TYPE){
            this->genL1LspTimer = timerMsg;
        }else {
            this->genL2LspTimer = timerMsg;
        }
        this->schedule(timerMsg);

    }
    EV<< "ISIS: initGenerate()" << endl;

}
    /*
     * Initial schedule of timer for refreshing LSPs
     */
void ISIS::initRefresh()
{
    ISISTimer *timerMsg;

    timerMsg = new ISISTimer("Refresh LSPs timer");
    timerMsg->setTimerKind(LSP_REFRESH_TIMER);
    timerMsg->setIsType(this->isType);
    this->schedule(timerMsg);
    EV<< "ISIS: initRefresh()" << endl;

}

    /*
     * Initial schedule of timer for periodic sending LSPs.
     */
void ISIS::initPeriodicSend()
{
    ISISTimer *timerMsg;

    if (this->isType == L1L2_TYPE)
    {
        timerMsg = new ISISTimer("Periodic send");
        timerMsg->setTimerKind(PERIODIC_SEND_TIMER);
        timerMsg->setIsType(L1_TYPE);
        this->periodicL1Timer = timerMsg;
        this->schedule(timerMsg);

        timerMsg = new ISISTimer("Periodic send");
        timerMsg->setTimerKind(PERIODIC_SEND_TIMER);
        timerMsg->setIsType(L2_TYPE);
        this->periodicL2Timer = timerMsg;
        this->schedule(timerMsg);
    }
    else
    {
        timerMsg = new ISISTimer("Periodic send");
        timerMsg->setTimerKind(PERIODIC_SEND_TIMER);
        timerMsg->setIsType(this->isType);
        if(this->isType == L1_TYPE){
            this->periodicL1Timer = timerMsg;
        }else {
            this->periodicL2Timer = timerMsg;
        }
        this->schedule(timerMsg);
    }
}

/*
 * Initial schedule of timer for sending CSNP.
 */
void ISIS::initCsnp()
{

    ISISTimer *timerMsg;
    ISISinterface *iface;
    for (unsigned int k = 0; k < this->ISISIft.size(); k++)
    {
        iface = &(this->ISISIft.at(k));
        //don't schedule Hello message on Loopback interfaces
        if (this->ISISIft.at(k).entry->isLoopback())
        {
            continue;
        }

        //don't schedule sending CSNP PDU on passive or not ISIS-enabled interface
        if (!iface->ISISenabled || iface->passive)
        {
            continue;
        }

        if (iface->network && iface->circuitType == L1L2_TYPE)
        {
            timerMsg = new ISISTimer("CSNP L1");
            timerMsg->setTimerKind(CSNP_TIMER);
            timerMsg->setIsType(L1_TYPE);
            timerMsg->setInterfaceIndex(k);
            timerMsg->setGateIndex(iface->gateIndex);
            this->schedule(timerMsg);

            timerMsg = new ISISTimer("CSNP L2");
            timerMsg->setTimerKind(CSNP_TIMER);
            timerMsg->setIsType(L2_TYPE);
            timerMsg->setInterfaceIndex(k);
            timerMsg->setGateIndex(iface->gateIndex);
            this->schedule(timerMsg);

        }
        else
        {
            timerMsg = new ISISTimer("CSNP");
            timerMsg->setTimerKind(CSNP_TIMER);
            timerMsg->setIsType(iface->circuitType);
            timerMsg->setInterfaceIndex(k);
            timerMsg->setGateIndex(iface->gateIndex);
            this->schedule(timerMsg);

        }
    }
}

/*
 * Initial schedule of timer for sending PSNP.
 */
void ISIS::initPsnp()
{

    ISISTimer *timerMsg;
    ISISinterface *iface;
    for (unsigned int k = 0; k < this->ISISIft.size(); k++)
    {
        iface = &(this->ISISIft.at(k));

        //don't schedule Hello message on Loopback interfaces
        if (this->ISISIft.at(k).entry->isLoopback())
        {
            continue;
        }

        //don't schedule sending PSNP on passive or not ISIS-enabled interface
        if (!iface->ISISenabled || iface->passive)
        {
            continue;
        }

        if (iface->network && iface->circuitType == L1L2_TYPE)
        {
            timerMsg = new ISISTimer("PSNP L1");
            timerMsg->setTimerKind(PSNP_TIMER);
            timerMsg->setIsType(L1_TYPE);
            timerMsg->setInterfaceIndex(k);
            timerMsg->setGateIndex(iface->gateIndex);
            this->schedule(timerMsg);

            timerMsg = new ISISTimer("PSNP L2");
            timerMsg->setTimerKind(PSNP_TIMER);
            timerMsg->setIsType(L2_TYPE);
            timerMsg->setInterfaceIndex(k);
            timerMsg->setGateIndex(iface->gateIndex);
            this->schedule(timerMsg);

        }
        else
        {
            timerMsg = new ISISTimer("PSNP");
            timerMsg->setTimerKind(PSNP_TIMER);
            timerMsg->setIsType(iface->circuitType);
            timerMsg->setInterfaceIndex(k);
            timerMsg->setGateIndex(iface->gateIndex);
            this->schedule(timerMsg);

        }
    }
}

/*
 * Initial schedule of timer for computing shortest paths.
 */
void ISIS::initSPF()
{
    ISISTimer *timerMsg;

    if (this->isType == L1L2_TYPE)
    {
        timerMsg = new ISISTimer("L1 SPF Full");
        timerMsg->setTimerKind(SPF_FULL_TIMER);
        timerMsg->setIsType(L1_TYPE);
        this->spfL1Timer = timerMsg;
        this->schedule(timerMsg);

        timerMsg = new ISISTimer("L2 SPF Full");
        timerMsg->setTimerKind(SPF_FULL_TIMER);
        timerMsg->setIsType(L2_TYPE);
        this->spfL2Timer = timerMsg;
        this->schedule(timerMsg);
    }
    else
    {
        timerMsg = new ISISTimer("SPF Full");
        timerMsg->setTimerKind(SPF_FULL_TIMER);
        timerMsg->setIsType(this->isType);
        if(this->isType == L1_TYPE){
            this->spfL1Timer = timerMsg;
        }else {
            this->spfL2Timer = timerMsg;
        }
        this->schedule(timerMsg);
    }
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
            case (ISIS_START_TIMER):
                this->initISIS();
                delete timer;
                break;

            case (HELLO_TIMER):
                this->sendHelloMsg(timer);
                break;

            case (TRILL_HELLO_TIMER):
                this->sendTRILLHelloMsg(timer);
                break;

            case (NEIGHBOUR_DEAD_TIMER):
                this->removeDeadNeighbour(timer);
                delete timer;
                break;

            case (GENERATE_LSP_TIMER):
                this->generateLSP(timer);
                break;

            case (LSP_REFRESH_TIMER):
                this->refreshLSP(timer);
                break;

            case (LSP_DEAD_TIMER):
                this->purgeLSP(this->getLspID(timer), timer->getIsType());
                // don't delete timer, it's re-used for LSP_DELETE_TIMER
                //delete timer;
                break;

            case (LSP_DELETE_TIMER):
                this->deleteLSP(timer);
                this->drop(timer);
                delete timer;
                break;

            case (CSNP_TIMER):
                if (timer->getIsType() == L1L2_TYPE)
                {
                    EV<< "ISIS: Warning: Discarding CSNP_TIMER for L1L2." << endl;
                    delete timer;
                }
                else
                {
                    this->sendCsnp(timer);
                }
                break;

                case (PSNP_TIMER):
                if (timer->getIsType() == L1L2_TYPE)
                {
                    EV << "ISIS: Warning: Discarding PSNP_TIMER for L1L2." << endl;
                    delete timer;
                }
                else
                {
                    this->sendPsnp(timer);
                }
                break;

                case (PERIODIC_SEND_TIMER):
                this->periodicSend(timer, timer->getIsType());
                break;

                case (SPF_FULL_TIMER):
                this->fullSPF(timer);
                break;

                default:
                EV << "ISIS: Warning: Received unsupported Timer type in handleMessage" << endl;
                delete timer;
                break;
            }
        }
        else
        {

            //TODO externalDomain check
            //TODO every (at least all Hello) message should be checked for matching system-ID length

            ISISMessage *inMsg = check_and_cast<ISISMessage *>(msg);
            if (!this->isMessageOK(inMsg))
            {
                EV << "ISIS: Warning: discarding message" << endl;
                //TODO schedule event discarding message
                delete msg;
                return;
            }

            //get arrival interface
            int gateIndex = inMsg->getArrivalGate()->getIndex();
            ISISinterface * tmpIntf = this->getIfaceByGateIndex(gateIndex);
            if (tmpIntf == NULL)
            {
                EV << "ISIS: ERROR: Couldn't find interface by gate index when ISIS::handleMessage" << endl;
                delete msg;
                return;
            }

            //get circuit type for arrival interface
            short circuitType = tmpIntf->circuitType;

            /* Usually we shouldn't need to check matching circuit type with arrived message
             * and these messages should be filtered based on destination MAC address.
             * Since we use broadcast MAC address, IS(router) cannot determine if it's
             * for ALL L1 or ALL L2 systems. Therefore we need to check manually.
             * If appropriate Level isn't enabled on interface, then the message is discarded.
             */

            switch (inMsg->getType())
            {
                case (LAN_L1_HELLO):
                if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
                {
                    this->handleL1HelloMsg(inMsg);

                    //comment if printing of the adjacency table is too disturbing
                    this->printAdjTable();

                }
                else
                {
                    EV
                    << "deviceId "
                    << deviceId
                    << ": ISIS: WARNING: Discarding LAN_L1_HELLO message on unsupported circuit type interface "
                    << inMsg->getId() << endl;
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
                    EV
                    << "deviceId "
                    << deviceId
                    << ": ISIS: WARNING: Discarding LAN_L2_HELLO message on unsupported circuit type interface "
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
                    << ": ISIS: WARNING: Discarding PTP_HELLO message. Received RESERVED_TYPE circuit type "
                    << inMsg->getId() << endl;
                }
                delete inMsg;
                break;

                case (TRILL_HELLO):

                if (circuitType != RESERVED_TYPE)
                {
                    this->handleTRILLHelloMsg(inMsg);
                    this->printAdjTable();
                }
                else
                {
                    EV
                    << "deviceId "
                    << deviceId
                    << ": ISIS: WARNING: Discarding PTP_HELLO message. Received RESERVED_TYPE circuit type "
                    << inMsg->getId() << endl;
                }
                delete inMsg;
                break;

                case (L1_LSP):
                if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
                {

                    this->handleLsp(check_and_cast<ISISLSPPacket *>(msg));

                    //comment if printing of the link-state database is too disturbing
                    this->printLSPDB();
                }
                else
                {
                    EV
                    << "deviceId "
                    << deviceId
                    << ": ISIS: WARNING: Discarding L1_LSP message on unsupported circuit type interface "
                    << inMsg->getId() << endl;
                }
                //delete inMsg; //TODO don't delete inMsg in new version

                break;

                case (L2_LSP):
                if (circuitType == L2_TYPE || circuitType == L1L2_TYPE)
                {

                    this->handleLsp(check_and_cast<ISISLSPPacket *>(msg));

                    //comment if printing of the link-state database is too disturbing
                    this->printLSPDB();
                }
                else
                {
                    EV
                    << "deviceId "
                    << deviceId
                    << ": ISIS: WARNING: Discarding L1_LSP message on unsupported circuit type interface "
                    << inMsg->getId() << endl;
                }
                //delete inMsg; //TODO don't delete inMsg in new version

                break;

                case (L1_CSNP):
                if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
                {
                    //this->handleL1CSNP(inMsg);
                    this->handleCsnp(check_and_cast<ISISCSNPPacket *>(msg));
                    this->printLSPDB();
                }
                else
                {
                    EV
                    << "deviceId "
                    << deviceId
                    << ": ISIS: WARNING: Discarding L1_CSNP message on unsupported circuit type interface\n"
                    << inMsg->getId() << endl;
                }
                //delete inMsg;
                break;

                case (L2_CSNP):
                if (circuitType == L2_TYPE || circuitType == L1L2_TYPE)
                {
                    //this->handleL1CSNP(inMsg);
                    this->handleCsnp(check_and_cast<ISISCSNPPacket *>(msg));
                    this->printLSPDB();
                }
                else
                {
                    EV
                    << "deviceId "
                    << deviceId
                    << ": ISIS: WARNING: Discarding L2_CSNP message on unsupported circuit type interface\n"
                    << inMsg->getId() << endl;
                }
                //delete inMsg;
                break;

                case (L1_PSNP):
                if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
                {
                    this->handlePsnp(check_and_cast<ISISPSNPPacket *>(msg));
                    this->printLSPDB();
                    //delete inMsg;
                }
                break;

                case (L2_PSNP):
                if (circuitType == L2_TYPE || circuitType == L1L2_TYPE)
                {
                    this->handlePsnp(check_and_cast<ISISPSNPPacket *>(msg));
                    this->printLSPDB();
                    //delete inMsg;
                }
                break;

                default:
                EV
                << "deviceId " << deviceId << ": ISIS: WARNING: Discarding unknown message type. Msg id: "
                << inMsg->getId() << endl;
                delete inMsg;
                break;

            }
        }
    }

/**
 * Create hello packet and send it out to specified interface. This method handle
 * LAN hello and PTP hello packets. Destination MAC address is broadcast (ff:ff:ff:ff:ff:ff).
 * @param timer is timer that triggered this action
 */
void ISIS::sendHelloMsg(ISISTimer* timer)
{
    if (this->ISISIft.at(timer->getInterfaceIndex()).network)
    {
//        EV<< "ISIS: sendingBroadcastHello: " << endl;
        this->sendBroadcastHelloMsg(timer->getInterfaceIndex(), timer->getGateIndex(), timer->getIsType());

    }
    else
    {
//        EV << "ISIS: sendingPTPHello: " << endl;
        this->sendPTPHelloMsg(timer->getInterfaceIndex(), timer->getGateIndex(), timer->getIsType());
    }
    //re-schedule timer
    this->schedule(timer);

}

/*
 * Send hello message on specified broadcast interface.
 * Packets contain IS_NEIGHBOURS_HELLO and AREA_ADDRESS TLVs.
 * @param k is interface index (index to ISISIft)
 * @param circuitType is circuit type of specified interface.
 */
void ISIS::sendBroadcastHelloMsg(int interfaceIndex, int gateIndex, short circuitType)
{

    /*
     * TODO B1
     * Hellos are scheduled per interface per level, so only one Hello Packet
     * should be created and then sent.
     * Appropriate level should be based on circuitType from timer and not from
     * interface's circuitType.
     */
//    unsigned int tlvSize;
    unsigned char * disID;
    ISISinterface *iface = &(this->ISISIft.at(interfaceIndex));

    // create L1 and L2 hello packets

    /*ISISL1HelloPacket *helloL1 = new ISISL1HelloPacket("L1 Hello");
     ISISL2HelloPacket *helloL2 = new ISISL2HelloPacket("L2 Hello");
     */
    ISISLANHelloPacket *hello = new ISISLANHelloPacket("Hello");
    //set appropriate destination MAC addresses
//    MACAddress ma;

    if (circuitType == L1_TYPE)
    {
        hello->setType(LAN_L1_HELLO);
        disID = iface->L1DIS;
//        ma.setAddress(ISIS_ALL_L1_IS);
    }
    else if (circuitType == L2_TYPE)
    {
        hello->setType(LAN_L2_HELLO);
        disID = iface->L2DIS;
//        ma.setAddress(ISIS_ALL_L2_IS);
    }
    else
    {
        EV<< "ISIS: Warning: Sending LAN Hello" << endl;
        return;
    }

//    hello->setCircuitType(circuitType);

    //set source id
    for (unsigned int i = 0; i < 6; i++)
    {
        hello->setSourceID(i, sysId[i]);
    }


//    Ieee802Ctrl *ctrl = new Ieee802Ctrl();

    // set DSAP & NSAP fields
//    ctrl->setDsap(SAP_CLNS);
//    ctrl->setSsap(SAP_CLNS);
//
//    ctrl->setDest(ma);


    //assign Ethernet control info
//    hello->setControlInfo(ctrl);

    //set TLVs
//    TLV_t myTLV;
    //helloL1->setTLVArraySize(0);

    //set area address
    this->addTLV(hello, AREA_ADDRESS, circuitType);

    //set NEIGHBOURS
    this->addTLV(hello, IS_NEIGHBOURS_HELLO, circuitType, gateIndex);


    //TODO PADDING TLV is omitted
    //TODO Authentication TLV
    //TODO add eventually another TLVs (eg. from RFC 1195)
    //don't send hello packets from passive interfaces
    if (!iface->passive && iface->ISISenabled)
    {
        // if this interface is DIS for LAN, hellos are sent 3-times faster (3.33sec instead of 10.0)
        // decision is made according to global hello counter (dirty hax - don't blame me pls, but i don't have time to code it nice way :)

        //set LAN ID field (DIS-ID)
        for (unsigned int j = 0; j < 7; j++)
        {
            hello->setLanID(j, disID[j]);
        }

        hello->setPriority(iface->priority);
        send(hello, "lowerLayerOut", iface->gateIndex);
//        EV<< "'devideId :" << deviceId << " ISIS: L1 Hello packet was sent from " << iface->entry->getName() << "\n";
        EV<< "ISIS::sendLANHello: Source-ID: ";
        for (unsigned int i = 0; i < 6; i++)
            {
                EV << setfill('0') << setw(2) << dec << (unsigned int) this->sysId[i];
                if (i % 2 == 1)
                    EV << ".";
            }
        EV<< " DIS: ";
        for (unsigned int i = 0; i < 7; i++)
                    {
                        EV << setfill('0') << setw(2) << dec << (unsigned int) disID[i];
                        if (i % 2 == 1)
                            EV << ".";
                    }
        EV<< endl;

    }
}

/*
 * Sends hello message to specified PtP interface.
 * Packets contain IS_NEIGHBOURS_HELLO and AREA_ADDRESS TLVs.
 * @param gateIndex is interface index (index to ISISIft)
 * @param circuitType is circuit type of specified interface.
 */
void ISIS::sendPTPHelloMsg(int interfaceIndex, int gateIndex, short circuitType)
{

//    unsigned int tlvSize;
    ISISinterface *iface = &(this->ISISIft.at(interfaceIndex));
    //don't send hello packets from passive interfaces
    if (iface->passive || !iface->ISISenabled)
    {
        return;
    }


    //TODO change to appropriate layer-2 protocol
//    Ieee802Ctrl *ctrlPtp = new Ieee802Ctrl();

    // set DSAP & NSAP fields
//    ctrlPtp->setDsap(SAP_CLNS);
//    ctrlPtp->setSsap(SAP_CLNS);

    //set appropriate destination MAC addresses
//    MACAddress ma;
//
//    if (iface->circuitType == L1_TYPE)
//    {
//        ma.setAddress(ISIS_ALL_L1_IS);
//    }
//    else
//    {
//        ma.setAddress(ISIS_ALL_L2_IS);
//    }
//
//
//    ctrlPtp->setDest(ma);

    //type
    ISISPTPHelloPacket *ptpHello = new ISISPTPHelloPacket("PTP Hello");

    //assign Ethernet control info
//    ptpHello->setControlInfo(ctrlPtp);
    //circuitType
    ptpHello->setCircuitType(iface->circuitType);

    //sourceID
    //set source id
    for (unsigned int i = 0; i < 6; i++)
    {
        ptpHello->setSourceID(i, sysId[i]);
    }

    //holdTime
    //set holdTime
    ptpHello->setHoldTime(this->getHoldTime(interfaceIndex, iface->circuitType));

    //pduLength

    //localCircuitID
    ptpHello->setLocalCircuitID(iface->gateIndex);

    //TLV[]
    //set TLVs
//    TLV_t myTLV;

    //set area address
    this->addTLV(ptpHello, AREA_ADDRESS, circuitType);

    //ptp adjacency state TLV #240
    this->addTLV(ptpHello, PTP_HELLO_STATE, circuitType, gateIndex);

     //TODO TLV #129 Protocols supported

    this->send(ptpHello, "lowerLayerOut", iface->gateIndex);

}

/******************/
/*   TRILL Hellos */
/******************/
/**
 * Generates TRILL hellos and places them in TRILLInterfaceData
 * @param interfaceId is interface id in interfacetable
 * @param circuitType is circuit type of interface with @param interfaceId
 */
void ISIS::genTRILLHello(int interfaceId, ISISCircuitType circuitType)
{

    const unsigned char *disId;
    InterfaceEntry *ie = ift->getInterfaceById(interfaceId);
    ISISInterfaceData *d = ie->isisData();
    ISISinterface *iface = this->getIfaceByGateIndex(ie->getNetworkLayerGateIndex());

    if (d->isHelloValid())
    { //TODO A1 remove true
        //nothing has changed since last Hello generating
        return;
    }
    //clear previously generated Hellos
    d->clearHello();

    std::vector<TLV_t *>* tlvTable = new std::vector<TLV_t *>;

    /* TODO MUST appear in every TRILL-Hello
     * 1. VLAN ID of the Designated VLAN
     * 2. A copy of the Outer.VLAN ID with which the Hello was tagged on sending.
     * 3. 16-bit port-ID (interfaceId)
     * 4. Nickname of the sending RBridge
     * 5. Two flags
     *    detected VLAN mapping
     *    believe it is appointed forwarder for the VLAN and port on which the TRILL-Hello was sent.
     *
     * ** MAY appear **
     * 1. set of VLANs for which end-station is enabled on the port
     * 2. Several flags
     *    sender's port is configured as access port
     *    sender's port is configured as trunk port
     *    bypass pseudonode
     * 3. If the sender is DRB, the RBridges (excluding etself) that it appoints as forwarder for that link and the VLANs
     *    for which it appoints them. Not all the appointment information need to be included in all Hellos.
     *    Its absence means continue as previously configured.
     * 4. TRILL neighbor list (TLV_TRILL_NEIGHBOR)
     */

    //TLV AREA ADDRESS (I'm not ever sure it MUST be present)
    this->addTLV(tlvTable, AREA_ADDRESS, circuitType);

    //TLV TLV_MT_PORT_CAP
    this->addTLV(tlvTable, TLV_MT_PORT_CAP, circuitType, ie);

    //TLV TLV_TRILL_NEIGHBOR
    this->addTLV(tlvTable, TLV_TRILL_NEIGHBOR, circuitType, ie);

    TLV_t *tmpTlv;
//    unsigned tlvSize;
    unsigned int availableSpace = ISIS_TRILL_MAX_HELLO_SIZE;

    for (unsigned int fragment = 0; !tlvTable->empty(); fragment++)
    {
        TRILLHelloPacket *hello = new TRILLHelloPacket("TRILL Hello");
        //set source id
        for (unsigned int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            hello->setSourceID(i, sysId[i]);
        }

        //DIS (TRILL currently supports only L1)
        disId = d->getL1Dis();
        for (unsigned int j = 0; j < ISIS_LAN_ID; j++)
        {
            hello->setLanID(j, disId[j]);

        }
        //TODO A1 change to define
        hello->setMaxAreas(1);
        hello->setPriority(d->getPriority());
        hello->setHoldTime(this->getHoldTime(this->getIfaceIndex(iface), circuitType));

        for (; !tlvTable->empty();)
        {
            tmpTlv = tlvTable->front();

            if (tmpTlv->length > ISIS_TRILL_MAX_HELLO_SIZE)
            {
                //tlv won't fit into single message (this shouldn't happen)
                EV<< "ISIS: Warning: When generating TRILL Hello, TLV won't fit into single message (this shouldn't happen)" << endl;
                tlvTable->erase(tlvTable->begin());
            }
            else if(tmpTlv->length > availableSpace)
            {
                //tlv won't fit into this LSP, so break cycle and create new LSP
                //                tmpLSPDb->push_back(LSP);
                EV << "ISIS: This TLV is full." << endl;
                break;//ends inner cycle
            }
            else
            {

                this->addTLV(hello, tmpTlv);
                //update availableSpace
                //2Bytes are Type and Length fields
                availableSpace = availableSpace - (2 + tmpTlv->length);

                //clean up
//                    delete tmpTlv->value;
//                    delete tmpTlv;
                tlvTable->erase(tlvTable->begin());
            }
        }
        d->addHello(hello);

    }

    d->setHelloValid(true);

//    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
//    // set DSAP & NSAP fields (not sure about this)
//    ctrl->setDsap(SAP_CLNS);
//    ctrl->setSsap(SAP_CLNS);
//
//    ctrl->setDest(MACAddress(ALL_IS_IS_RBRIDGES));
//    hello.setControlInfo(ctrl);

}

/**
 * Create hello packet and send it out to specified interface. This method handle
 * LAN hello and PTP hello packets. Destination MAC address is broadcast (ff:ff:ff:ff:ff:ff).
 * @param timer is timer that triggered this action
 */
void ISIS::sendTRILLHelloMsg(ISISTimer* timer)
{
    if (this->ISISIft.at(timer->getInterfaceIndex()).network)
    {
        EV<< "ISIS: sendingTRILLBroadcastHello: " << endl;
        this->sendTRILLBroadcastHelloMsg(timer->getInterfaceIndex(), timer->getGateIndex(), timer->getIsType());

    }
    else
    {
        EV << "ISIS: sendingPTPHello: " << endl;
        this->sendTRILLPTPHelloMsg(timer->getInterfaceIndex(), timer->getGateIndex(), timer->getIsType());
    }
    //re-schedule timer
    this->schedule(timer);

}

/*
 * Send hello message on specified broadcast interface.
 * Packets contain IS_NEIGHBOURS_HELLO and AREA_ADDRESS TLVs.
 * @param k is interface index (index to ISISIft)
 * @param circuitType is circuit type of specified interface.
 */
//TODO the last parameter is not necessary
void ISIS::sendTRILLBroadcastHelloMsg(int interfaceIndex, int gateIndex, short circuitType)
{
    if (circuitType != L1_TYPE)
    {
        throw cRuntimeError("sendTRILLBroadcastHelloMsg: Wrong circuitType for TRILL (only L1_TYPE is supported");
    }

    ISISinterface *iface = &(this->ISISIft.at(interfaceIndex));
    if (iface->passive || !iface->ISISenabled)
    {
        return;
    }

    InterfaceEntry *ie = this->ISISIft.at(interfaceIndex).entry;
    this->genTRILLHello(ie->getInterfaceId(), (ISISCircuitType) circuitType);
    ISISInterfaceData *d = ie->isisData();
    std::vector<ISISMessage *> hellos = d->getHellos();
    for (std::vector<ISISMessage *>::iterator it = hellos.begin(); it != hellos.end(); ++it)
    {

        TRILLHelloPacket *trillHello = ((TRILLHelloPacket *) (*it))->dup();
//        Ieee802Ctrl *ctrl = new Ieee802Ctrl();
//        // set DSAP & NSAP fields
//        ctrl->setDsap(SAP_CLNS);
//        ctrl->setSsap(SAP_CLNS);

        //set appropriate destination MAC addresses
//        MACAddress ma;
//        ma.setAddress(ALL_IS_IS_RBRIDGES);
//        ctrl->setDest(ma);

        //assign Ethernet control info
//        trillHello->setControlInfo(ctrl);
        this->send(trillHello, "lowerLayerOut", gateIndex);

    }


}

/* TODO transform it to TRILL versions
 * Sends hello message to specified PtP interface.
 * Packets contain IS_NEIGHBOURS_HELLO and AREA_ADDRESS TLVs.
 * @param gateIndex is interface index (index to ISISIft)
 * @param circuitType is circuit type of specified interface.
 */
void ISIS::sendTRILLPTPHelloMsg(int interfaceIndex, int gateIndex, short circuitType)
{

//    unsigned int tlvSize;
    ISISinterface *iface = &(this->ISISIft.at(interfaceIndex));
    //don't send hello packets from passive interfaces
    if (iface->passive || !iface->ISISenabled)
    {
        return;
    }

    //TODO change to appropriate layer-2 protocol
    Ieee802Ctrl *ctrlPtp = new Ieee802Ctrl();

    // set DSAP & NSAP fields
    ctrlPtp->setDsap(SAP_CLNS);
    ctrlPtp->setSsap(SAP_CLNS);

    //set appropriate destination MAC addresses
    MACAddress ma;
    /*
     * I assume that since there is only one level, destination is All ISIS Systems.
     */
    ma.setAddress(ALL_IS_IS_RBRIDGES);

    ctrlPtp->setDest(ma);

    //type
    ISISPTPHelloPacket *ptpHello = new ISISPTPHelloPacket("PTP Hello");

    //assign Ethernet control info
    ptpHello->setControlInfo(ctrlPtp);
    //circuitType
    ptpHello->setCircuitType(iface->circuitType);

    //sourceID
    //set source id
    for (unsigned int i = 0; i < 6; i++)
    {
        ptpHello->setSourceID(i, sysId[i]);
    }

    //holdTime
    //set holdTime
    ptpHello->setHoldTime(this->getHoldTime(interfaceIndex, iface->circuitType));

    //pduLength

    //localCircuitID
    ptpHello->setLocalCircuitID(iface->gateIndex);

    //TLV[]
    //set TLVs
//    TLV_t myTLV;

    this->addTLV(ptpHello, AREA_ADDRESS, circuitType);

    this->addTLV(ptpHello, PTP_HELLO_STATE, circuitType, gateIndex);

     //TODO TLV #129 Protocols supported

    this->send(ptpHello, "lowerLayerOut", iface->gateIndex);

}

/*
 * Schedule specified timer according to it's type.
 * @param timer is timer to be scheduled
 * @param timee additional time information
 */
void ISIS::schedule(ISISTimer *timer, double timee)
{

    double timeAt;
    double randomTime;

    switch (timer->getTimerKind())
        {
        case (ISIS_START_TIMER): {
            this->scheduleAt(0, timer);
            break;
        }
        case (TRILL_HELLO_TIMER): {
            //no brak here is on purpose
        }

        case (HELLO_TIMER): {
            timeAt = this->getHelloInterval(timer->getInterfaceIndex(), timer->getIsType());
            randomTime = uniform(0, 0.25 * timeAt);
            this->scheduleAt(simTime() + timeAt - randomTime, timer);
//            EV<< "ISIS::schedule: timeAt: " << timeAt << " randomTime: " << randomTime << endl;
            break;
        }
        case (NEIGHBOUR_DEAD_TIMER): {
            if (timee < 0)
            {
                EV<< "ISIS: Warning: You forgot provide additional time to schedule for NEIGHBOUR_DEAD_TIMER" << endl;
                timee = 0;
            }
            timeAt = timee;
            randomTime = uniform(0, 0.25 * timeAt);
//            randomTime = 0;
            this->scheduleAt(simTime() + timeAt - randomTime, timer);
            break;
        }
        case (PERIODIC_SEND_TIMER): {
            cancelEvent(timer);
            if (timer->getIsType() == L1_TYPE)
            {
                timeAt = (double)this->L1LspSendInterval;
            }
            else if (timer->getIsType() == L2_TYPE)
            {
                timeAt = (double)this->L2LspSendInterval;
            }
            else
            {
                EV<< "ISIS: Warning: Unsupported IS-Type in schedule for PERIODIC_SEND_TIMER" << endl;
            }

//            randomTime = uniform(0, 0.25 * timeAt);
            randomTime = 0;
            this->scheduleAt(simTime() + timeAt - randomTime, timer);
            break;
        }
        case (LSP_REFRESH_TIMER):
        {
            timeAt = this->lspRefreshInterval;
            randomTime = uniform(0, 0.25 * timeAt);
//            randomTime = 0;
            this->scheduleAt(simTime() + timeAt - randomTime, timer);
            break;
        }
        case (LSP_DEAD_TIMER):
        {
            if (timee < 0)
            {
                EV << "ISIS: Warning: You forgot provide additional time to schedule for LSP_DEAD_TIMER" << endl;
                timee = 0;
            }
            timeAt = timee;
            //randomTime = uniform(0, 0.25 * timeAt);
            this->scheduleAt(simTime() + timeAt, timer);

            /*dirty hack */
            this->runSPF(0, timer);

            break;
        }
        case (LSP_DELETE_TIMER):
        {
            timeAt = this->lspMaxLifetime * 2;
            this->scheduleAt(simTime() + timeAt, timer);
            break;
        }
        case (CSNP_TIMER):
        {
            if (timer->getIsType() == L1_TYPE)
            {
                timeAt = this->ISISIft.at(timer->getInterfaceIndex()).L1CsnpInterval;
            }
            else if (timer->getIsType() == L2_TYPE)
            {
                timeAt = this->ISISIft.at(timer->getInterfaceIndex()).L2CsnpInterval;
            }
            else
            {
                EV << "ISIS: Warning: Unsupported IS-Type in schedule for CSNP_TIMER" << endl;
            }

            randomTime = uniform(0, 0.25 * timeAt);
//            randomTime = 0;
            this->scheduleAt(simTime() + timeAt - randomTime, timer);
            break;
        }
        case (PSNP_TIMER):
        {
            if (timer->getIsType() == L1_TYPE)
            {
                timeAt = this->ISISIft.at(timer->getInterfaceIndex()).L1PsnpInterval;
            }
            else if (timer->getIsType() == L2_TYPE)
            {
                timeAt = this->ISISIft.at(timer->getInterfaceIndex()).L2PsnpInterval;
            }
            else
            {
                EV << "ISIS: Warning: Unsupported IS-Type in schedule for PSNP_TIMER" << endl;
            }

            randomTime = uniform(0, 0.25 * timeAt);
//            randomTime = 0;
            this->scheduleAt(simTime() + timeAt - randomTime, timer);
            break;
        }
        case (L1_CSNP):
        {
            timeAt = this->ISISIft.at(timer->getInterfaceIndex()).L1CsnpInterval;
            randomTime = uniform(0, 0.25 * timeAt);
//            randomTime = 0;
            this->scheduleAt(simTime() + timeAt - randomTime, timer);

            break;
        }
        case (L1_PSNP):
        {
            timeAt = this->ISISIft.at(timer->getInterfaceIndex()).L1PsnpInterval;
            randomTime = uniform(0, 0.25 * timeAt);
            randomTime = 0;
            this->scheduleAt(simTime() + timeAt - randomTime, timer);

            break;
        }
        case (GENERATE_LSP_TIMER):
        {
            if (timer->getIsType() == L1_TYPE)
            {
                if ((simTime().dbl() + this->L1LspGenInterval) * 1000 < this->L1LspInitWait)
                {
                    timeAt = this->L1LspInitWait / 1000;
                }
                else
                {
                    timeAt = this->L1LspGenInterval;
                }
            }
            else if (timer->getIsType() == L2_TYPE)
            {
                if ((simTime().dbl() + this->L2LspGenInterval) * 1000 < this->L2LspInitWait)
                {
                    timeAt = this->L2LspInitWait / 1000;
                }
                else
                {
                    timeAt = this->L2LspGenInterval;
                }
            }
            else
            {
                EV << "ISIS: ERROR: Unsupported IS-Type in PERIODIC_SEND_TIMER timer" << endl;
            }
            randomTime = uniform(0, 0.25 * timeAt);
//            randomTime = 0;
            this->scheduleAt(simTime() + timeAt - randomTime, timer);

            break;
        }
        case (SPF_FULL_TIMER):
        {
            if (timer->getIsType() == L1_TYPE)
            {
                timeAt = this->L1SPFFullInterval;
            }
            else if (timer->getIsType() == L2_TYPE)
            {
                timeAt = this->L2SPFFullInterval;
            }
            else
            {
                EV << "ISIS: Error Unsupported IS-Type in SPF_FULL timer" << endl;
            }
            randomTime = uniform(0, 0.25 * timeAt);
//            randomTime = 0;
            this->scheduleAt(simTime() + timeAt - randomTime, timer);
            break;
        }
        default:
        EV << "ISIS: ERROR: Unsupported timer type in schedule" << endl;
        break;
    }

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

    found = net.find_first_of(".");
    if (found != 2 || net.length() != 25)
    {
        return false;
    }

    unsigned char *area = new unsigned char[ISIS_AREA_ID];
    unsigned char *systemId = new unsigned char[ISIS_SYSTEM_ID];
    unsigned char *nsel = new unsigned char[1];

    while (found != string::npos)
    {

        switch (found)
            {
            case 2:
                dots++;
                area[0] = (unsigned char) (atoi(net.substr(0, 2).c_str()));
                cout << "BEZ ATOI" << net.substr(0, 2).c_str() << endl;
                break;
            case 7:
                dots++;
                area[1] = (unsigned char) (atoi(net.substr(3, 2).c_str()));
                area[2] = (unsigned char) (atoi(net.substr(5, 2).c_str()));
                break;
            case 12:
                dots++;
                systemId[0] = (unsigned char) (strtol(net.substr(8, 2).c_str(), NULL, 16));
                systemId[1] = (unsigned char) (strtol(net.substr(10, 2).c_str(), NULL, 16));
                break;
            case 17:
                dots++;
                systemId[2] = (unsigned char) (strtol(net.substr(13, 2).c_str(), NULL, 16));
                systemId[3] = (unsigned char) (strtol(net.substr(15, 2).c_str(), NULL, 16));
                break;
            case 22:
                dots++;
                systemId[4] = (unsigned char) (strtol(net.substr(18, 2).c_str(), NULL, 16));
                systemId[5] = (unsigned char) (strtol(net.substr(20, 2).c_str(), NULL, 16));
                break;
            default:
                return false;
                break;

            }

        found = net.find_first_of(".", found + 1);
    }

    if (dots != 5)
    {
        return false;
    }

    nsel[0] = (unsigned char) (atoi(net.substr(23, 2).c_str()));

    //49.0001.1921.6801.2003.00

    areaId = area;
//    cout << "ISIS: AreaID: " << areaId[0] << endl;
//    cout << "ISIS: AreaID: " << areaId[1] << endl;
//    cout << "ISIS: AreaID: " << areaId[2] << endl;
    sysId = systemId;
//    cout << "ISIS: SystemID: " << sysId << endl;
    NSEL = nsel;
//    cout << "ISIS: NSEL: " << NSEL << endl;

    this->nickname = this->sysId[ISIS_SYSTEM_ID - 1] + this->sysId[ISIS_SYSTEM_ID - 2] * 0xFF;


    return true;
}

/**
 * Handle L1 hello messages. Insert new neighbours into L1 adjacency table (this->adjL1Table) and
 * update status of existing neighbours.
 * @param inMsg incoming L1 hello packet
 */
void ISIS::handleL1HelloMsg(ISISMessage *inMsg)
{

    TLV_t* tmpTLV;
    int gateIndex = inMsg->getArrivalGate()->getIndex();
    ISISinterface *tmpIntf = this->getIfaceByGateIndex(gateIndex);

    //duplicate system ID check
    if (this->checkDuplicateSysID(inMsg))
    {
        //TODO schedule event duplicitSystemID
        return;
    }

    ISISLANHelloPacket *msg = check_and_cast<ISISLANHelloPacket *>(inMsg);

    /* 8.4.2.2 */
    //check if at least one areaId matches our areaId (don't do this for L2)
    bool areaOK = false;

    for (int i = 0; (tmpTLV = this->getTLVByType(msg, AREA_ADDRESS, i)) != NULL; i++)
    {
        areaOK = areaOK || this->isAreaIDOK(tmpTLV);
    }

    if (!areaOK)
    {
        //TODO schedule event AreaIDMismatch
        EV<< "ISIS: Warning: L1_LAN_HELLO doesn't contain Area address TLV." << endl;
        return;
    }

        //if remote system ID is contained in adjL1Table
    ISISadj *tmpAdj;
    if ((tmpAdj = this->getAdj(inMsg)) != NULL)
    {

        //reset timer
        cancelEvent(tmpAdj->timer);

        //TODO use this->schedule()
        //scheduleAt(simTime() + msg->getHoldTime(), tmpAdj->timer);
        this->schedule(tmpAdj->timer, msg->getHoldTime());

        //TODO DONT update the interface, if the gateIndex has changed, then declare the adjacency as down or at least init and then change gateIndex
        // I think this is covered in getAdj() -> there is performed check to ensure the adjacency match even gateIndex
        //update interface
        //tmpAdj->gateIndex = msg->getArrivalGate()->getIndex();

        //find neighbours TLV
        if ((tmpTLV = this->getTLVByType(msg, IS_NEIGHBOURS_HELLO)) != NULL)
        {
            //TODO check length of IS_NEIGHBOURS_HELLO value
            unsigned char *tmpRecord = new unsigned char[MAC_ADDRESS_SIZE]; //size of MAC address
            //walk through all neighbour identifiers contained in TLV
            for (unsigned int r = 0; r < (tmpTLV->length / 6); r++)
            {
                //check if my system id is contained in neighbour's adjL1Table
                this->copyArrayContent(tmpTLV->value, tmpRecord, 6, r * 6, 0);

                MACAddress tmpMAC = tmpIntf->entry->getMacAddress();
                unsigned char *tmpMACAddress = new unsigned char[6];
                tmpMAC.getAddressBytes(tmpMACAddress);

                if (compareArrays(tmpMACAddress, tmpRecord, 6))
                {
                    //store previous state
                    ISISAdjState changed = tmpAdj->state;
                    tmpAdj->state = ISIS_ADJ_REPORT;

                    //if state changed, flood new LSP
                    if (changed != tmpAdj->state)
                    {
                        //this->sendMyL1LSPs();
                        //TODO generate event adjacencyStateChanged
                        nb->fireChangeNotification(NF_ISIS_ADJ_CHANGED, this->genL1LspTimer);
                    }
                    break;
                }

            }
            delete[] tmpRecord;

        }
        else
        {
            //TODO Delete after debugging

            EV<< "ISIS: Warning: Didn't find IS_NEIGHBOURS_HELLO TLV in LAN_L1_HELLO" << endl;
            return;
        }

            //check for DIS priority and eventually set new DIS if needed; do it only if exists adjacency with state "UP"
        if (tmpAdj->state == ISIS_ADJ_REPORT)
        {
            electDIS(msg);
        }

    }

    //else create new record in adjL1Table
    else
    {
        //EV << "CREATING NEW ADJ RECORD\n";

        //find area ID TLV

        //create new neighbour record and set parameters
        ISISadj neighbour;
        neighbour.state = ISIS_ADJ_DETECT; //set state to initial

        //set timeout of neighbour
        neighbour.timer = new ISISTimer("Neighbour_timeout");
        neighbour.timer->setTimerKind(NEIGHBOUR_DEAD_TIMER);
        neighbour.timer->setIsType(L1_TYPE);
        neighbour.timer->setInterfaceIndex(this->getIfaceIndex(tmpIntf));
        neighbour.timer->setGateIndex(gateIndex);
        //set source system ID in neighbour record & in timer to identify it
        //set also lspId for easier purging LSPs
        for (unsigned int the_game = 0; the_game < msg->getSourceIDArraySize(); the_game++)
        {
            neighbour.sysID[the_game] = msg->getSourceID(the_game);
            neighbour.timer->setSysID(the_game, msg->getSourceID(the_game));
            neighbour.timer->setLSPid(the_game, msg->getSourceID(the_game));
        }
        neighbour.timer->setLSPid(ISIS_SYSTEM_ID, 0);
        neighbour.timer->setLSPid(ISIS_SYSTEM_ID + 1, 0);

        //TODO should be from message's TLV Area Addresses
        for (unsigned int i = 0; i < ISIS_AREA_ID; i++)
        {
            neighbour.areaID[i] = this->areaId[i];
            neighbour.timer->setAreaID(i, this->areaId[i]);
        }

        //get source MAC address of received frame
        Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
        neighbour.mac = ctrl->getSrc();

        //set gate index, which is neighbour connected to
        neighbour.gateIndex = gateIndex;

        //set network type
        neighbour.network = tmpIntf->network;

        neighbour.priority = msg->getPriority();

        this->schedule(neighbour.timer, msg->getHoldTime());
        //scheduleAt(simTime() + msg->getHoldTime(), neighbour.timer);

        //insert neighbour into adjL1Table
        adjL1Table.push_back(neighbour);
        std::sort(this->adjL1Table.begin(), this->adjL1Table.end());

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

    TLV_t* tmpTLV;
    int gateIndex = inMsg->getArrivalGate()->getIndex();
    ISISinterface *iface = this->getIfaceByGateIndex(gateIndex);
    ISISLANHelloPacket *msg = check_and_cast<ISISLANHelloPacket *>(inMsg);

    short circuitType = L2_TYPE;

    //duplicate system ID check
    if (this->checkDuplicateSysID(inMsg))
    {
        //TODO schedule event duplicitSystemID
        return;
    }

    //if remote system ID is contained in adjL2Table
    ISISadj *tmpAdj;
    if ((tmpAdj = this->getAdj(inMsg)) != NULL)
    {

        //cancel timer
        cancelEvent(tmpAdj->timer);
        //re-schedule timer
        this->schedule(tmpAdj->timer, msg->getHoldTime());

        //check for DIS priority and eventually set new DIS if needed; do it only if exists adjacency with state "UP"
        if (tmpAdj->state == ISIS_ADJ_REPORT)
            electDIS(msg);

        //find neighbours TLV
        if ((tmpTLV = this->getTLVByType(msg, IS_NEIGHBOURS_HELLO)) != NULL)
        {
            unsigned char *tmpRecord = new unsigned char[ISIS_SYSTEM_ID];
            //walk through all neighbour identifiers contained in TLV

            for (unsigned int r = 0; r < (tmpTLV->length / ISIS_SYSTEM_ID); r++)
            {
                //check if my system id is contained in neighbour's adjL1Table
                this->copyArrayContent(tmpTLV->value, tmpRecord, ISIS_SYSTEM_ID, r * ISIS_SYSTEM_ID, 0);

                MACAddress tmpMAC = iface->entry->getMacAddress();
                unsigned char *tmpMACAddress = new unsigned char[6];
                tmpMAC.getAddressBytes(tmpMACAddress);

                if (compareArrays(tmpMACAddress, tmpRecord, 6))
                {
                    //store previous state
                    bool changed = tmpAdj->state;
                    tmpAdj->state = ISIS_ADJ_REPORT;

                    //if state changed, flood new LSP
                    if (changed != tmpAdj->state)
                    {
                        //this->sendMyL1LSPs();
                        //TODO B1 generate event adjacencyStateChanged (i suppose it's done)
                        nb->fireChangeNotification(NF_ISIS_ADJ_CHANGED, this->genL1LspTimer);
                        //TODO support multiple area addresses
                        if (!compareArrays((unsigned char *) this->areaId, tmpAdj->areaID, ISIS_AREA_ID)
                                && this->isType == L1L2_TYPE)
                        {
//                            this->att = true;
                            this->updateAtt(true);
                        }
                    }
                    break;
                }

            }
            delete tmpRecord;
        }
        else
        {
            //TODO Delete after debugging

            EV<< "ISIS: Warning: Didn't find IS_NEIGHBOURS_HELLO TLV in LAN_L2_HELLO" << endl;
            return;
        }

    }

    //else create new record in adjL2Table
    else
    {
        //EV << "CREATING NEW ADJ RECORD\n";

        //find area ID TLV

        if ((tmpTLV = this->getTLVByType(msg, AREA_ADDRESS)) != NULL)
        {
            //TODO add loop thru tmpTLV to add support for multiple Area Addresses

            //create new neighbour record and set parameters
            ISISadj neighbour;
            neighbour.state = ISIS_ADJ_DETECT;//set state to initial

            //set timeout of neighbour
            neighbour.timer = new ISISTimer("Neighbour_timeout");
            neighbour.timer->setTimerKind(NEIGHBOUR_DEAD_TIMER);
            neighbour.timer->setInterfaceIndex(this->getIfaceIndex(iface));
            neighbour.timer->setGateIndex(gateIndex);
            neighbour.timer->setIsType(circuitType);
            //set source system ID in neighbour record & in timer to identify it

            //set neighbours system ID
            //set also lspId for easier purging LSPs
            for (unsigned int the_game = 0; the_game < msg->getSourceIDArraySize(); the_game++)
            {
                neighbour.sysID[the_game] = msg->getSourceID(the_game);
                neighbour.timer->setSysID(the_game, msg->getSourceID(the_game));
                neighbour.timer->setLSPid(the_game, msg->getSourceID(the_game));
            }
            neighbour.timer->setLSPid(ISIS_SYSTEM_ID, 0);
            neighbour.timer->setLSPid(ISIS_SYSTEM_ID + 1, 0);

            //TODO check that area address length match with ISIS_AREA_ID
            // tmpTLV->value[0] == ISIS_AREA_ID
            //set neighbours area ID
            this->copyArrayContent(tmpTLV->value, neighbour.areaID, ISIS_AREA_ID, 1, 0);

            for (unsigned int z = 0; z < ISIS_AREA_ID; z++)
            {
                neighbour.timer->setAreaID(z, tmpTLV->value[z]);
            }

            //get source MAC address of received frame
            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
            neighbour.mac = ctrl->getSrc();

            //set gate index, which is neighbour connected to
            neighbour.gateIndex = gateIndex;

            //set network type
            neighbour.network = iface->network;

            this->schedule(neighbour.timer, msg->getHoldTime());
            //scheduleAt(simTime() + msg->getHoldTime(), neighbour.timer);

            //insert neighbour into adjL1Table
            adjL2Table.push_back(neighbour);
            std::sort(this->adjL2Table.begin(), this->adjL2Table.end());

            //EV << "deviceId " << deviceId << ": new adjacency\n";

        }

    }
}
/**
 * Handle TRILL Hello messages.
 * I necessary creates new adjacency.
 * Handles only L1.
 * @param inMsg is incoming TRILL Hello message
*/
void ISIS::handleTRILLHelloMsg(ISISMessage *inMsg)
{

    //RFC 6327 7.2 Receiving TRILL Hellos
    TLV_t* tmpTLV;

    //duplicate system ID check
    if (this->checkDuplicateSysID(inMsg))
    {
        //TODO schedule event duplicitSystemID
        return;
    }

    ISISLANHelloPacket *msg = check_and_cast<ISISLANHelloPacket *>(inMsg);

    /* 8.4.2.2 */
    //check if at least one areaId matches our areaId (don't do this for L2)
    bool areaOK = false;

    for (int i = 0; (tmpTLV = this->getTLVByType(msg, AREA_ADDRESS, i)) != NULL; i++)
    {
        areaOK = areaOK || this->isAreaIDOK(tmpTLV);
    }

    if (!areaOK)
    {
        //TODO schedule event AreaIDMismatch
        EV<< "ISIS: Warning: TRILL_HELLO doesn't match Area address." << endl;
        return;
    }

    if (msg->getMaxAreas() != 1)
    {
        EV<< "ISIS: Warning: Received TRILL Hello with wrong MaximumAreaAddress field." << endl;
        return;
    }

    tmpTLV = this->getTLVByType(msg, TLV_MT_PORT_CAP);

    if (tmpTLV == NULL)
    {
        EV<< "ISIS: Warning: Received TRILL Hello without TLV TLV_MT_PORT_CAP." << endl;
        return;
    }
    unsigned char *subTLV;
    if ((subTLV = this->getSubTLVByType(tmpTLV, TLV_MT_PORT_CAP_VLAN_FLAG)) == NULL)
    {
        EV<< "ISIS: Warning: Received TRILL Hello without subTLV TLV_MT_PORT_CAP_VLAN_FLAG." << endl;
        return;
    }

    int tmpDesigVlanId, tmpOuterVlanId, tmpPortId;
//    int senderNickname;
//    bool af, ac, vm, by, tr;
    //parsing TLV_MT_PORT_CAP
//    tmpPortId = subTLV[2] + subTLV[3] * 0xFF;
//    senderNickname = subTLV[4] + subTLV[5] * 0xFF;
    tmpDesigVlanId = subTLV[6] + (subTLV[7] & 0x0F) * 0xFF;
//    af = (subTLV[7] >> 7) & 0x01;
//    ac = (subTLV[7] >> 6) & 0x01;
//    vm = (subTLV[7] >> 5) & 0x01;
//    by = (subTLV[7] >> 4) & 0x01;
//    tmpOuterVlanId = subTLV[8] + (subTLV[8] & 0x0F) * 0xFF;
//    tr = (subTLV[9] >> 7) & 0x01;







    int gateIndex = inMsg->getArrivalGate()->getIndex();
    ISISinterface *tmpIntf = this->getIfaceByGateIndex(gateIndex);
    MACAddress tmpMAC = tmpIntf->entry->getMacAddress();
    unsigned char *tmpMACChars = new unsigned char[MAC_ADDRESS_SIZE];
    tmpMAC.getAddressBytes(tmpMACChars);
    //if sender is DIS
    if(memcmp(this->getSysID(msg), tmpIntf->L1DIS, ISIS_SYSTEM_ID) == 0){
        TRILLInterfaceData *trillD = tmpIntf->entry->trillData();
        trillD->setDesigVlan(tmpDesigVlanId);
        if ((subTLV = this->getSubTLVByType(tmpTLV, TLV_MT_PORT_CAP_APP_FWD)) != NULL)
        {
            //THIS SHOULD BE HANDLED IN electDIS. based on result act accordingly
            //handle Appointed Forwarders subTLV
            //TODO A2 appointed Fwd should be touples <nickname, vlanIdRange> or simply <nickname, vlanId>



            trillD->clearAppointedForwarder();
            for(int subLen = 0; subLen < subTLV[1];){
                int appointeeNickname = subTLV[subLen + 2] + subTLV[subLen + 1 + 2] * 0xFF;
                int startVlan = subTLV[subLen + 2 + 2] + (subTLV[subLen + 3 + 2] & 0x0F) * 0xFF;
                int endVlan = subTLV[subLen + 4 + 2] + (subTLV[subLen + 5 + 2] & 0x0F) * 0xFF;
                subLen += 6;
                for(int appVlan = startVlan; appVlan <= endVlan; appVlan++){
                    trillD->addAppointedForwarder(appVlan, appointeeNickname);
                }
            }



        }
    }

    //check for presence of TLV Protocols Supported and if so it must list TRILL NLPID (0xC0)
    //if such TLV is not present then continue as it would list TRILL NLPID
    //(that's what we will do for now)

    //TODO We're gonna need a bigger boat erm I mean more complex Adjacency State Machinery. Preferably new method to handle it.
    //TODO RFC6327 3.3. event A0 (received Hello with SNPA matching SNPA of port this Hello was received on.
    //                   event A2 received on non desig VLAN -> ignore TLV TRILL_NEIGHBOR
    //                         A4, A5, A6, A7, A8
    ISISadj *tmpAdj;
    if ((tmpAdj = this->getAdj(inMsg)) != NULL)
    {
        //reset timer
        cancelEvent(tmpAdj->timer);

        this->schedule(tmpAdj->timer, msg->getHoldTime());

        //TODO A3? updating interface and gate index (see handleL1HelloMsg())

//        if (tmpAdj->state == ISIS_ADJ_REPORT)
//        {
//            electDIS(msg);
//        }
        //TODO handle MT
        for (int offset = 0; (tmpTLV = this->getTLVByType(msg, TLV_TRILL_NEIGHBOR, offset)) != NULL; offset++)
        {
            unsigned char *tmpRecord = new unsigned char[MAC_ADDRESS_SIZE];
//            bool s, l;
//            s = tmpTLV->value[0] >> 7 & 0x01;
//            l = tmpTLV->value[0] >> 6 & 0x01;

            for (unsigned int r = 0; r < ((tmpTLV->length - 1) / 9); r++)
            {
                //reported SNPA (MAC Address)
                memcpy(tmpRecord, &tmpTLV->value[1 + (r * 9) + 3], MAC_ADDRESS_SIZE);
                bool failedMTU = tmpTLV->value[1 + r * 9] >> 7 && 0x01;
                if (memcmp(tmpMACChars, tmpRecord, MAC_ADDRESS_SIZE) == 0)
                {
                    //RFC 6327 3.3. A1 (sender explicitly list this IS's SNPA)
                    //store previous state
                    ISISAdjState changed = tmpAdj->state;
                    switch (tmpAdj->state)
                        {
                        case ISIS_ADJ_DOWN: //not sure if this could occur (but the chosen behavior is correct)
                        case ISIS_ADJ_DETECT:
                        case ISIS_ADJ_2WAY:
                            tmpAdj->state = ISIS_ADJ_2WAY;
                            break;
                        case ISIS_ADJ_REPORT:
                            break;
                        }

                    if (!failedMTU)
                    {
                        //RFC 6327 3.3 A6
                        switch (tmpAdj->state)
                            {
                            case ISIS_ADJ_DOWN: //not sure if this could occur (but the chosen behavior is correct)
                            case ISIS_ADJ_DETECT:
                                EV<< "ISIS: ERROR: This Adjacency state shoudn't occur" << endl;
                                break;
                                case ISIS_ADJ_2WAY:
                                case ISIS_ADJ_REPORT:
                                tmpAdj->state = ISIS_ADJ_REPORT;
                                break;
                            }

                        }

                    if (changed != tmpAdj->state)
                    {
                        //TODO generate event adjacencyStateChanged
                        nb->fireChangeNotification(NF_ISIS_ADJ_CHANGED, this->genL1LspTimer);
                        int gateIndex = msg->getArrivalGate()->getIndex();
                //        int gateIndex = timer->getInterfaceIndex();
                        InterfaceEntry *entry = this->ift->getInterfaceByNetworkLayerGateIndex(gateIndex);
                        ISISInterfaceData *isisData = entry->isisData();
                        isisData->setHelloValid(false);

                    }

                }
                else
                {
                    //reported neighbor's SNPA is not this port's SNPA
                    //TODO dunno if I shoudn't add this reported neighbor anyway
                }
            }

        }

        //TODO A!
        if (tmpAdj->state >= ISIS_ADJ_2WAY)
        {
            electDIS(msg);
        }

    }
    //else create new record in adjL1Table
    else
    {

        ISISadj neighbour;
        neighbour.state = ISIS_ADJ_DETECT;

        //set timeout timer of neighbour
        neighbour.timer = new ISISTimer("Neighbour_timeout");
        neighbour.timer->setTimerKind(NEIGHBOUR_DEAD_TIMER);
        neighbour.timer->setIsType(L1_TYPE);
        neighbour.timer->setInterfaceIndex(this->getIfaceIndex(tmpIntf));
        neighbour.timer->setGateIndex(gateIndex);
        //set source system ID in neighbour record & in timer to identify it
        //set also lspId for easier purging LSPs
        for (unsigned int the_game = 0; the_game < msg->getSourceIDArraySize(); the_game++)
        {
            neighbour.sysID[the_game] = msg->getSourceID(the_game);
            neighbour.timer->setSysID(the_game, msg->getSourceID(the_game));
            neighbour.timer->setLSPid(the_game, msg->getSourceID(the_game));
        }
        neighbour.timer->setLSPid(ISIS_SYSTEM_ID, 0);
        neighbour.timer->setLSPid(ISIS_SYSTEM_ID + 1, 0);

        //TODO should be from message's TLV Area Addresses
        for (unsigned int i = 0; i < ISIS_AREA_ID; i++)
        {
            neighbour.areaID[i] = this->areaId[i];
            neighbour.timer->setAreaID(i, this->areaId[i]);
        }

        //get source MAC address of received frame
        Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
        neighbour.mac = ctrl->getSrc();

        //set gate index, which is neighbour connected to
        neighbour.gateIndex = gateIndex;

        //set network type
        neighbour.network = tmpIntf->network;

        neighbour.priority = msg->getPriority();

        //TODO B check that tmpDesigVlanId is DESIRED and not currently used designated VLAN
        neighbour.desiredDesVLAN = tmpDesigVlanId;
        this->schedule(neighbour.timer, msg->getHoldTime());
        //scheduleAt(simTime() + msg->getHoldTime(), neighbour.timer);

        //insert neighbour into adjL1Table
        adjL1Table.push_back(neighbour);
        std::sort(this->adjL1Table.begin(), this->adjL1Table.end());

//        nb->fireChangeNotification(NF_ISIS_ADJ_CHANGED, this->genL1LspTimer);
        int gateIndex = msg->getArrivalGate()->getIndex();
//        int gateIndex = timer->getInterfaceIndex();
        InterfaceEntry *entry = this->ift->getInterfaceByNetworkLayerGateIndex(gateIndex);
        ISISInterfaceData *isisData = entry->isisData();
        isisData->setHelloValid(false);

    }

}
/**
 * Handle PTP hello messages.
 * If necessary creates new adjacency.
 * Update status of existing neighbours.
 * @param inMsg incoming PtP hello packet
 */
void ISIS::handlePTPHelloMsg(ISISMessage *inMsg)
{

    int gateIndex = inMsg->getArrivalGate()->getIndex();
    ISISinterface *iface = this->getIfaceByGateIndex(gateIndex);

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
            EV<< "ISIS: Warning: PTP_HELLO doesn't contain Area address TLV." << endl;
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

                    if (tmpAdj->state == ISIS_ADJ_REPORT) //UP
                    {

                        if (tmpTLV->value[0] == PTP_UP)
                        {
                            //OK do nothing
                        }
                        else if (tmpTLV->value[0] == PTP_DOWN)
                        {
                            //state init
                            //TODO B2
                            tmpAdj->state = ISIS_ADJ_DETECT;

                        }
                        else
                        {
                            //state accept
                        }

                    }
                    else if (tmpAdj->state == ISIS_ADJ_DETECT) // INIT
                    {
                        if (tmpTLV->value[0] == PTP_UP || tmpTLV->value[0] == PTP_INIT)
                        {
                            //OK
                            tmpAdj->state = ISIS_ADJ_REPORT;
//                            if (simTime() > 35.0){ //TODO use at least some kind of ISIS_INITIAL_TIMER
                            //this->sendMyL1LSPs();
//                            }
                            //TODO B2
                            //schedule adjacencyStateChange(up);
                            nb->fireChangeNotification(NF_ISIS_ADJ_CHANGED, this->genL1LspTimer);

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
                EV<< "ISIS: Warning: Didn't find PTP_HELLO_STATE TLV in PTP_HELLO L2" << endl;
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
            neighbour.state = ISIS_ADJ_DETECT;//set state to initial

            //set timeout of neighbour
            neighbour.timer = new ISISTimer("Neighbour_timeout");
            neighbour.timer->setTimerKind(NEIGHBOUR_DEAD_TIMER);
            neighbour.timer->setIsType(L1_TYPE);
            neighbour.timer->setInterfaceIndex(this->getIfaceIndex(iface));
            neighbour.timer->setGateIndex(gateIndex);
            //set source system ID in neighbour record & in timer to identify it

            for (unsigned int the_game = 0; the_game < msg->getSourceIDArraySize(); the_game++)
            {
                neighbour.sysID[the_game] = msg->getSourceID(the_game);
                neighbour.timer->setSysID(the_game, msg->getSourceID(the_game));
            }

            //TODO should be from message's TLV Area Addresses
            for (unsigned int i = 0; i < ISIS_AREA_ID; i++)
            {
                neighbour.areaID[i] = this->areaId[i];
            }

            //get source MAC address of received frame
            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
            neighbour.mac = ctrl->getSrc();

            //set gate index, which is neighbour connected to
            neighbour.gateIndex = msg->getArrivalGate()->getIndex();

            //set network type
            neighbour.network = this->getIfaceByGateIndex(neighbour.gateIndex)->network;

            this->schedule(neighbour.timer, msg->getHoldTime());
            //scheduleAt(simTime() + msg->getHoldTime(), neighbour.timer);

            //insert neighbour into adjL1Table
            adjL1Table.push_back(neighbour);
            std::sort(this->adjL1Table.begin(), this->adjL1Table.end());

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
                    if (tmpAdj->state == ISIS_ADJ_REPORT) //UP
                    {
                        if (tmpTLV->value[0] == PTP_UP)
                        {
                            //OK do nothing
                        }
                        else if (tmpTLV->value[0] == PTP_DOWN)
                        {
                            //state init
                            //TODO B2
                            tmpAdj->state = ISIS_ADJ_DETECT;

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
                            tmpAdj->state = ISIS_ADJ_REPORT;
//                            if (simTime() > 35.0)
//                            { //TODO use at least some kind of ISIS_INITIAL_TIMER
                            //this->sendMyL2LSPs();
//                            }
                            //TODO B2
                            //schedule adjacencyStateChange(up);
                            nb->fireChangeNotification(NF_ISIS_ADJ_CHANGED, this->genL2LspTimer);
                            //TODO support multiple area addresses
                            if (!compareArrays((unsigned char *) this->areaId, tmpAdj->areaID, ISIS_AREA_ID)
                                    && this->isType == L1L2_TYPE)
                            {
                                this->updateAtt(true);
                            }
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
                EV<< "ISIS: Warning: Didn't find PTP_HELLO_STATE TLV in PTP_HELLO" << endl;
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
            neighbour.state = ISIS_ADJ_DETECT;//set state to initial

            //set timeout of neighbour
            neighbour.timer = new ISISTimer("Neighbour_timeout");
            neighbour.timer->setTimerKind(NEIGHBOUR_DEAD_TIMER);
            neighbour.timer->setIsType(L2_TYPE);
            neighbour.timer->setInterfaceIndex(this->getIfaceIndex(iface));
            neighbour.timer->setGateIndex(gateIndex);

            //set source system ID in neighbour record & in timer to identify it
            for (unsigned int the_game = 0; the_game < msg->getSourceIDArraySize(); the_game++)
            {
                neighbour.sysID[the_game] = msg->getSourceID(the_game);
                neighbour.timer->setSysID(the_game, msg->getSourceID(the_game));
            }
            //set neighbours area ID
            tmpTLV = this->getTLVByType(msg, AREA_ADDRESS);
            //TODO compare tmpTLV->value[0] and ISIS_AREA_ID
            this->copyArrayContent(tmpTLV->value, neighbour.areaID, tmpTLV->value[0], 1, 0);

            for (unsigned int z = 0; z < tmpTLV->value[0]; z++)
            {
                neighbour.timer->setAreaID(z, tmpTLV->value[z + 1]);
            }

            //get source MAC address of received frame
            Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
            neighbour.mac = ctrl->getSrc();

            //set gate index, which is neighbour connected to
            neighbour.gateIndex = msg->getArrivalGate()->getIndex();

            //set network type
            neighbour.network = this->getIfaceByGateIndex(neighbour.gateIndex)->network;

            this->schedule(neighbour.timer, msg->getHoldTime());
            //scheduleAt(simTime() + msg->getHoldTime(), neighbour.timer);

            //insert neighbour into adjL2Table
            adjL2Table.push_back(neighbour);
            std::sort(this->adjL2Table.begin(), this->adjL2Table.end());

            //EV << "deviceId " << deviceId << ": new adjacency\n";

        }
    }
}

void ISIS::updateAtt(bool action)
{
    //TODO IS should regenerate zero-th LSP when attached flag changes
    if (action)
    {
        if (!this->att)
        {
            this->att = true;
            this->setClosestAtt();
        }

    }
    else if (action != this->att)
    {
        for (AdjTab_t::iterator it = this->adjL2Table.begin(); it != this->adjL2Table.end(); ++it)
        {
            if (!compareArrays((unsigned char *) this->areaId, (*it).areaID, ISIS_AREA_ID && this->isType == L1L2_TYPE))
            {
                //at least one L2 adjacency with another area exists so don't clear Attached flag
                this->setClosestAtt();
                return;
            }
        }

        this->att = false;
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
    }
    else if (ciruitType == L2_TYPE)
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
        ISISLANHelloPacket *msg = check_and_cast<ISISLANHelloPacket *>(inMsg);
        systemID = this->getSysID(msg);
        circuitType = L1_TYPE;
        adjTable = &(this->adjL1Table);
    }
    else if (inMsg->getType() == LAN_L2_HELLO)
    {
        ISISLANHelloPacket *msg = check_and_cast<ISISLANHelloPacket *>(inMsg);
        systemID = this->getSysID(msg);
        circuitType = L2_TYPE;
        adjTable = &(this->adjL2Table);
    }
    else if (inMsg->getType() == TRILL_HELLO)
    {
        TRILLHelloPacket *msg = check_and_cast<TRILLHelloPacket *>(inMsg);
        systemID = this->getSysID(msg);
        circuitType = L1_TYPE;
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

    int gateIndex = inMsg->getArrivalGate()->getIndex();
    //            ISISinterface * tmpIntf = this->getIfaceByGateIndex(gateIndex);
    //TODO for truly point-to-point link there would not be MAC address
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(inMsg->getControlInfo());
    MACAddress tmpMac = ctrl->getSrc();

    for (std::vector<ISISadj>::iterator it = adjTable->begin(); it != adjTable->end(); ++it)
    {
        //System-ID match?
        if (compareArrays(systemID, (*it).sysID, ISIS_SYSTEM_ID))
        {

            //MAC Address and gateIndex
            //we need to check source (tmpMac) and destination interface thru we received this hello

            if (tmpMac.compareTo((*it).mac) == 0 && gateIndex == (*it).gateIndex)
            {
                if (circuitType != L2_TYPE)
                { //TODO shoudn't it be "!="
                  //check if at least one areaId matches our areaId (don't do this for L2)
                    bool areaOK = false;
                    TLV_t* tmpTLV;
                    tmpTLV = this->getTLVByType(inMsg, AREA_ADDRESS, 0);
                    for (int i = 0; tmpTLV != NULL; i++)
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
                delete[] systemID;
                return &(*it);
            }
        }
    }
    delete[] systemID;
    return NULL;

}
/**
 * Don't use this method, but rather ISIS::getAdj(ISISMessage *) which can handle multiple adjacencies between two ISes.
 *
 * @param systemID is system-ID for which we want to find adjacency
 * @param specify level
 * @param gateIndex specify interface
 */
ISISadj *ISIS::getAdjBySystemID(unsigned char *systemID, short circuitType, int gateIndex)
{

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
    }
    else if (circuitType == L1L2_TYPE)
    {
        EV<< "ISIS: ERROR: getAdjBySystemID for L1L2_TYPE is not implemented (yet)" << endl;
        //TODO B3
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

ISISadj *ISIS::getAdjByMAC(const MACAddress &address, short circuitType, int gateIndex){

    std::vector<ISISadj> *adjTab = this->getAdjTab(circuitType);

    for(std::vector<ISISadj>::iterator it = adjTab->begin(); it != adjTab->end(); ++it){
        if((*it).mac.compareTo(address) == 0){
            if(gateIndex == -1 || gateIndex == (*it).gateIndex){
                return &(*it);
            }
        }
    }

    return NULL;
}

/*
 * Extract System-ID from message.
 * @param msg incomming msg
 * @return newly allocated system-id
 */
unsigned char * ISIS::getSysID(ISISMessage *msg)
{

    unsigned char *systemID = new unsigned char[ISIS_SYSTEM_ID];
    if (msg->getType() == LAN_L1_HELLO)
    {
        ISISLANHelloPacket *l1hello = check_and_cast<ISISLANHelloPacket *>(msg);
        for (int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            systemID[i] = l1hello->getSourceID(i);
        }
    }
    else if (msg->getType() == LAN_L2_HELLO)
    {
        ISISLANHelloPacket *l2hello = check_and_cast<ISISLANHelloPacket *>(msg);
        for (int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            systemID[i] = l2hello->getSourceID(i);
        }
    }
    else if (msg->getType() == TRILL_HELLO)
    {
        TRILLHelloPacket *trillHello = check_and_cast<TRILLHelloPacket *>(msg);
        for (int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            systemID[i] = trillHello->getSourceID(i);
        }
    }
    else if (msg->getType() == PTP_HELLO)
    {
        ISISPTPHelloPacket *ptphello = check_and_cast<ISISPTPHelloPacket *>(msg);
        for (int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            systemID[i] = ptphello->getSourceID(i);
        }

    }
    else if (msg->getType() == L1_PSNP)
    {
        ISISPSNPPacket *psnp = check_and_cast<ISISPSNPPacket *>(msg);
        for (int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            systemID[i] = psnp->getSourceID(i);
        }

    }
    else if (msg->getType() == L2_PSNP)
    {
        ISISPSNPPacket *psnp = check_and_cast<ISISPSNPPacket *>(msg);
        for (int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            systemID[i] = psnp->getSourceID(i);
        }

    }
    else if (msg->getType() == L1_CSNP)
    {
        ISISCSNPPacket *csnp = check_and_cast<ISISCSNPPacket *>(msg);
        for (int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            systemID[i] = csnp->getSourceID(i);
        }

    }
    else if (msg->getType() == L2_CSNP)
    {
        ISISCSNPPacket *csnp = check_and_cast<ISISCSNPPacket *>(msg);
        for (int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            systemID[i] = csnp->getSourceID(i);
        }

    }
    else if (msg->getType() == L1_LSP || msg->getType() == L2_LSP)
    {
        ISISLSPPacket *lsp = check_and_cast<ISISLSPPacket *>(msg);
        for (int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            systemID[i] = lsp->getLspID(i);
        }

    }
    else
    {
        EV<< "ISIS: ERROR: getSysID for this message type is not implemented (yet?): " << msg->getType() << endl;
    }

    return systemID;

}

/*
 * Extract System-ID from timer.
 * @param msg incomming msg
 * @return newly allocated system-id
 */
unsigned char* ISIS::getSysID(ISISTimer *timer)
{
    unsigned char *systemID = new unsigned char[ISIS_SYSTEM_ID];

    for (int i = 0; i < ISIS_SYSTEM_ID; i++)
    {
        systemID[i] = timer->getSysID(i);
    }

    return systemID;
}

/*
 * Extract LSP-ID from timer.
 * @param timer incomming timer
 * @return newly allocated system-id
 */
unsigned char* ISIS::getLspID(ISISTimer *timer)
{
    unsigned char *lspID = new unsigned char[ISIS_SYSTEM_ID + 2];

    for (int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
    {
        lspID[i] = timer->getLSPid(i);
    }

    return lspID;
}


/*
 * Extract LSP-ID from message.
 * @param msg incomming msg
 * @return newly allocated system-id
 */
unsigned char* ISIS::getLspID(ISISLSPPacket *msg)
{

    unsigned char *lspId = new unsigned char[8]; //TODO change back to ISIS_SYSTEM_ID + 2

    for (int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
    {
        lspId[i] = msg->getLspID(i);
    }

    return lspId;

}

void ISIS::setLspID(ISISLSPPacket *msg, unsigned char * lspId)
{

    for (int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
    {
        msg->setLspID(i, lspId[i]);
    }

}

unsigned char* ISIS::getLanID(ISISLANHelloPacket *msg)
{

    unsigned char *lanID = new unsigned char(ISIS_SYSTEM_ID +  2);

    for (int i = 0; i < ISIS_SYSTEM_ID + 1; ++i)
    {
        lanID[i] = msg->getLanID(i); //XXX Invalid write? How?
    }

    return lanID;
}

/**
 * Print L1 and L2 adjacency tables to EV.
 * This function is currently called every time hello packet is received and processed.
 * @see handleMessage(cMessage* msg)
 */
void ISIS::printAdjTable()
{
    std::sort(this->adjL1Table.begin(), this->adjL1Table.end());
    EV<< "L1 adjacency table of IS ";

    //print area id
    for (unsigned int i = 0; i < 3; i++)
    {
        EV<< setfill('0') << setw(2) << dec << (unsigned int) areaId[i];
        if (i % 2 == 0)
        EV << ".";

    }

        //print system id
    for (unsigned int i = 0; i < 6; i++)
    {
        EV<< setfill('0') << setw(2) << hex << (unsigned int) sysId[i];
        if (i % 2 == 1)
        EV << ".";
    }

        //print NSEL
    EV<< setfill('0') << setw(2) << dec << (unsigned int) NSEL[0] << "\tNo. of records in Table: "
    << adjL1Table.size() << endl;

    //print neighbour records
    for (unsigned int j = 0; j < adjL1Table.size(); j++)
    {
        EV<< "\t";
        //print neighbour system id
        for (unsigned int i = 0; i < 6; i++)
        {
            EV << setfill('0') << setw(2) << hex << (unsigned int) adjL1Table.at(j).sysID[i];
            if (i == 1 || i == 3)
            EV << ".";
        }
        EV << "\t";

        //print neighbour MAC address
        for (unsigned int i = 0; i < 6; i++)
        {
            EV << setfill('0') << setw(2) << hex << (unsigned int) adjL1Table.at(j).mac.getAddressByte(i);
            if (i < 5)
            EV << ":";
        }
        EV << "\t";

        if (adjL1Table.at(j).state == ISIS_ADJ_DETECT)
        EV << "Init (Detect)\n";
        else if (adjL1Table.at(j).state == ISIS_ADJ_2WAY)
        EV << "Up (2Way)\n";
        else if (adjL1Table.at(j).state == ISIS_ADJ_REPORT)
        EV << "Up (Report)\n";
    }

    EV<< "--------------------------------------------------------------------\n";

    EV<< "L2 adjacency table of IS ";

    //print area id
    for (unsigned int i = 0; i < 3; i++)
    {
        EV<< setfill('0') << setw(2) << dec << (unsigned int) areaId[i];
        if (i % 2 == 0)
        EV << ".";

    }

        //print system id
    for (unsigned int i = 0; i < 6; i++)
    {
        EV<< setfill('0') << setw(2) << hex << (unsigned int) sysId[i];
        if (i % 2 == 1)
        EV << ".";
    }

        //print NSEL
    EV<< setfill('0') << setw(2) << dec << (unsigned int) NSEL[0] << "\tNo. of records in Table: "
    << adjL2Table.size() << endl;

    //print neighbour records
    for (unsigned int j = 0; j < adjL2Table.size(); j++)
    {
        EV<< "\t";
        //print neighbour area id and system id
        for (unsigned int i = 0; i < 3; i++)
        {
            EV << setfill('0') << setw(2) << dec << (unsigned int) adjL2Table.at(j).areaID[i];
            if (i % 2 == 0)
            EV << ".";

        }

        for (unsigned int i = 0; i < 6; i++)
        {
            EV << setfill('0') << setw(2) << dec << (unsigned int) adjL2Table.at(j).sysID[i];
            if (i == 1 || i == 3)
            EV << ".";
        }
        EV << "\t";

        //print neighbour MAC address
        for (unsigned int i = 0; i < 6; i++)
        {
            EV << setfill('0') << setw(2) << hex << (unsigned int) adjL2Table.at(j).mac.getAddressByte(i);
            if (i < 5)
            EV << ":";
        }
        EV << "\t";

        if (adjL2Table.at(j).state == ISIS_ADJ_DETECT)
        EV << "Init (Detect)\n";
        else if (adjL2Table.at(j).state == ISIS_ADJ_2WAY)
        EV << "Up (2Way)\n";
        else if (adjL2Table.at(j).state == ISIS_ADJ_REPORT)
        EV << "Up (Report)\n";
    }
}

void ISIS::setClnsTable(CLNSTable *clnsTable)
{
    this->clnsTable = clnsTable;
}

void ISIS::setNb(NotificationBoard *nb)
{
    this->nb = nb;
}
/* Sets and then parses NET address.
 * @param netAddr specify NET address
 */
void ISIS::setNetAddr(string netAddr)
{
    this->netAddr = netAddr;

    if (!this->parseNetAddr())
    {
        throw cRuntimeError("Unable to parse NET address.");
    }
}

void ISIS::setMode(ISIS_MODE mode)
{
    this->mode = mode;
}

void ISIS::subscribeNb(void)
{
    nb->subscribe(this, NF_INTERFACE_STATE_CHANGED);
    nb->subscribe(this, NF_CLNS_ROUTE_DELETED);
    nb->subscribe(this, NF_ISIS_ADJ_CHANGED);

}

void ISIS::setIft(IInterfaceTable *ift)
{
    this->ift = ift;
}

void ISIS::setTrill(TRILL *trill)
{
    if (trill == NULL)
    {
        throw cRuntimeError("Got NULL pointer instead of TRILL reference");
    }
    this->trill = trill;
}

/**
 * Print content of L1 LSP database to EV.
 */
void ISIS::printLSPDB()
{
    short circuitType = L1_TYPE;
    std::vector<LSPRecord *> *lspDb = this->getLSPDb(circuitType);
    std::sort(lspDb->begin(), lspDb->end(),cmpLSPRecord());
    EV<< "L1 LSP database of IS ";

    //print area id
    for (unsigned int i = 0; i < 3; i++)
    {
        EV<< setfill('0') << setw(2) << dec << (unsigned int) areaId[i];
        if (i % 2 == 0)
        EV << ".";

    }

        //print system id
    for (unsigned int i = 0; i < 6; i++)
    {
        EV<< setfill('0') << setw(2) << dec << (unsigned int) sysId[i];
        if (i % 2 == 1)
        EV << ".";
    }

        //print NSEL
    EV<< setfill('0') << setw(2) << dec << (unsigned int) NSEL[0] << "\tNo. of records in database: "
    << lspDb->size() << endl;
//    unsigned char *lspId;
    std::vector<LSPRecord *>::iterator it = lspDb->begin();
    for (; it != lspDb->end(); ++it)
    {
        EV<< "\t";
        //print LSP ID
        for (unsigned int j = 0; j < 8; j++)
        {
            EV << setfill('0') << setw(2) << dec << (unsigned int) (*it)->LSP->getLspID(j);
            if (j == 1 || j == 3 || j == 5)
            EV << ".";
            if (j == 6)
            EV << "-";
        }
        EV << "\t0x";

        //print sequence number
        EV << setfill('0') << setw(8) << hex << (*it)->LSP->getSeqNumber();
        EV << "\t" << setfill('0') << setw(5) << dec << (*it)->LSP->getRemLifeTime() << endl;
        //EV <<"SeqNum: " << (*it)->LSP->getSeqNumber()<<endl;

        TLV_t *tmpTlv;

        //print neighbours
        for (unsigned int k = 0; (tmpTlv = this->getTLVByType((*it)->LSP, IS_NEIGHBOURS_LSP, k)) != NULL; k++)
        {
            for (unsigned int m = 1; m + 11 <= tmpTlv->length; m += 11)
            {
                EV << "\t\t";
                for (unsigned int l = 0; l < 7; l++)
                {
                    //1 = virtual flag, m = current neighbour record, 4 is offset in current neigh. record(start LAN-ID)
                    EV << setfill('0') << setw(2) << dec << (unsigned int) tmpTlv->value[m + 4 + l];
                    if (l % 2 == 1)
                    EV << ".";
                }

                EV << "\tmetric: " << setfill('0') << setw(2) << dec << (unsigned int) tmpTlv->value[m + 0] << endl;
            }

        }

    }

    if (this->attIS != NULL)
    {
        EV<< "Closest L1L2 IS:" << endl;
        for (ISISNeighbours_t::iterator attIt = this->attIS->begin(); attIt != this->attIS->end(); ++attIt)
        {
            for (unsigned int l = 0; l < 7; l++)
            {
                //1 = virtual flag, m = current neighbour record, 4 is offset in current neigh. record(start LAN-ID)
                EV << setfill('0') << setw(2) << dec << (unsigned int) (*attIt)->id[l];
                if (l % 2 == 1)
                EV << ".";
            }
            EV << endl;
        }
    }

    circuitType = L2_TYPE;
    lspDb = this->getLSPDb(circuitType);
    EV<< "L2 LSP database of IS ";

    //print area id
    for (unsigned int i = 0; i < 3; i++)
    {
        EV<< setfill('0') << setw(2) << dec << (unsigned int) areaId[i];
        if (i % 2 == 0)
        EV << ".";

    }

        //print system id
    for (unsigned int i = 0; i < 6; i++)
    {
        EV<< setfill('0') << setw(2) << dec << (unsigned int) sysId[i];
        if (i % 2 == 1)
        EV << ".";
    }

        //print NSEL
    EV<< setfill('0') << setw(2) << dec << (unsigned int) NSEL[0] << "\tNo. of records in database: "
    << lspDb->size() << endl;
//        unsigned char *lspId;
    it = lspDb->begin();
    for (; it != lspDb->end(); ++it)
    {
        EV<< "\t";
        //print LSP ID
        for (unsigned int j = 0; j < 8; j++)
        {
            EV << setfill('0') << setw(2) << dec << (unsigned int) (*it)->LSP->getLspID(j);
            if (j == 1 || j == 3 || j == 5)
            EV << ".";
            if (j == 6)
            EV << "-";
        }
        EV << "\t0x";

        //print sequence number
        EV << setfill('0') << setw(8) << hex << (*it)->LSP->getSeqNumber();
        EV << "\t" << setfill('0') << setw(5) << dec << (*it)->LSP->getRemLifeTime() << endl;
        //EV <<"SeqNum: " << (*it)->LSP->getSeqNumber()<<endl;

        TLV_t *tmpTlv;

        //print neighbours
        for (unsigned int k = 0; (tmpTlv = this->getTLVByType((*it)->LSP, IS_NEIGHBOURS_LSP, k)) != NULL; k++)
        {
            for (unsigned int m = 1; m + 11 <= tmpTlv->length; m += 11)
            {
                EV << "\t\t";
                for (unsigned int l = 0; l < 7; l++)
                {
                    //1 = virtual flag, m = current neighbour record, 4 is offset in current neigh. record(start LAN-ID)
                    EV << setfill('0') << setw(2) << dec << (unsigned int) tmpTlv->value[m + 4 + l];
                    if (l % 2 == 1)
                    EV << ".";
                }

                EV << "\tmetric: " << setfill('0') << setw(2) << dec << (unsigned int) tmpTlv->value[m + 0] << endl;
            }

        }

    }

    //TODO print L2 LSP DB
}
/*
 * Print contents of LSP to std::cout
 * @param lsp is LSP packet
 * @param from is description where from this print has been called.
 */
void ISIS::printLSP(ISISLSPPacket *lsp, char *from)
{
    std::cout << "PrintLSP called from: " << from << endl;
    unsigned char * lspId = this->getLspID(lsp);
    std::cout << "Printing LSP: ";
    this->printLspId(lspId);
    std::cout << "Print LSP->test:";
    std::cout << "seqNum: " << lsp->getSeqNumber() << endl;
    std::cout << "Length of TLVarray: " << lsp->getTLVArraySize() << endl;
    std::cout << "TLV: " << endl;
    for (unsigned int i = 0; i < lsp->getTLVArraySize(); i++)
    {
        std::cout << "Type: " << (unsigned short) lsp->getTLV(i).type << endl;
        std::cout << "Length: " << (unsigned short) lsp->getTLV(i).length << endl;
    }

    TLV_t *tmpTlv;
    for (unsigned int k = 0; (tmpTlv = this->getTLVByType(lsp, IS_NEIGHBOURS_LSP, k)) != NULL; k++)
    {
        std::cout << "Start printing TLV of length: " << (unsigned int) tmpTlv->length << endl;
        for (unsigned int m = 0; (m + 11) < tmpTlv->length; m += 11)
        {
            std::cout << "LAN-ID:";
            for (unsigned int l = 0; l < 7; l++)
            {
                //1 = virtual flag, m = current neighbour record, 4 is offset in current neigh. record(start LAN-ID)
                std::cout << (unsigned short) tmpTlv->value[1 + m + 4 + l];
                if (l % 2 == 1)
                    std::cout << ".";
            }

            std::cout << "\t metric: " << (unsigned short) tmpTlv->value[1 + m + 0] << endl;
        }
    }
}

void ISIS::printLspId(unsigned char *lspId)
{

    this->printSysId(lspId);
    std::cout << setfill('0') << setw(2) << dec << (unsigned int) lspId[ISIS_SYSTEM_ID];
    std::cout << "-";
    std::cout << setfill('0') << setw(2) << dec << (unsigned int) lspId[ISIS_SYSTEM_ID + 1] << endl;
}

void ISIS::printSysId(unsigned char *sysId)
{

    //print system id
    for (unsigned int i = 0; i < 6; i++)
    {
        std::cout << setfill('0') << setw(2) << dec << (unsigned int) sysId[i];
        if (i % 2 == 1)
            std::cout << ".";
    }
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
    for (unsigned int i = 0; i < size; ++i)
    {
        if (first[i] != second[i])
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
void ISIS::copyArrayContent(unsigned char * src, unsigned char * dst, unsigned int size, unsigned int startSrc,
        unsigned int startDst)
{
    for (unsigned int i = 0; i < size; i++)
    {
        dst[i + startDst] = src[i + startSrc];
    }
}

/**
 * Remove dead neighbour, when timer expires. Remove neighbour from adjacency table (L1 or L2)
 * and start new DIS election process.
 * @param msg Timer associated with dead neighbour.
 */
void ISIS::removeDeadNeighbour(ISISTimer *msg)
{
    //TODO timer should also have an interface index and only adjacency with matching interface should be deleted
    //we can have multiple adjacencies with neighbour and only one link failed
    /* Check it and rewrite it */
    EV<< "ISIS: Warning: RemoveDeadNeighbour: If everything is working correctly, this method shoudn't be called"
    << endl;

    //L1 neighbour dead
    // * remove from adjL1Table
    // * reset DIS on Ift

    if (msg->getIsType() == L1_TYPE)
    {
        for (unsigned int i = 0; i < adjL1Table.size();)
        {
            if (msg->getGateIndex() == adjL1Table.at(i).gateIndex)
            {
                bool found = true;
                for (unsigned int j = 0; j < msg->getSysIDArraySize() && found; j++)
                {
                    if (msg->getSysID(j) != adjL1Table.at(i).sysID[j])
                    {
                        found = false;
                    }
                }

                if (found)
                {
                    adjL1Table.erase(adjL1Table.begin() + i);
                }
                else
                {
                    i++;
                }
            }
            else
            {
                i++;
            }
        }
        std::sort(this->adjL1Table.begin(), this->adjL1Table.end());

        unsigned char *lspId = this->getLspID(msg);
        this->purgeRemainLSP(lspId, msg->getIsType());

        if (this->ISISIft.at(msg->getInterfaceIndex()).network)
        {
            //lspId has two bytes more than we need (that does no harm)
            this->resetDIS(lspId, msg->getInterfaceIndex(), msg->getIsType());
        }
        delete lspId;

    }

    //else it's L2 dead neighbour
    // * remove from adjL2Table
    else
    {
        for (unsigned int i = 0; i < adjL2Table.size(); i++)
        {
            if (msg->getGateIndex() == adjL1Table.at(i).gateIndex)
            {
                bool found = true;
                for (unsigned int j = 0; j < msg->getSysIDArraySize(); j++)
                {
                    if (msg->getSysID(j) != adjL2Table.at(i).sysID[j])
                    found = false;
                }

                //XXX WHY must area ID also match??

                //area ID must also match
                for (unsigned int j = 0; j < msg->getAreaIDArraySize(); j++)
                {
                    if (msg->getAreaID(j) != adjL2Table.at(i).areaID[j])
                    {
                        found = false;
                    }
                }

                if (found)
                {
                    adjL2Table.erase(adjL2Table.begin() + i);
                }
            }
        }
        std::sort(this->adjL2Table.begin(), this->adjL2Table.end());

        unsigned char *lspId = this->getLspID(msg);
        this->purgeRemainLSP(lspId, msg->getIsType());

        if (this->ISISIft.at(msg->getInterfaceIndex()).network)
        {
            //lspId has two bytes more than we need (that does no harm)
            this->resetDIS(lspId, msg->getInterfaceIndex(), msg->getIsType());
        }

        delete lspId;

        //update att flag, since we removing neighbour, try to clear it -> sending false as action
        this->updateAtt(false);
    }

}
/**
 * Performs DIS election
 * @param msg is received LAN Hello PDU
 */
void ISIS::electDIS(ISISLANHelloPacket *msg)
{

    short circuitType;
    unsigned char *disID;
    unsigned char disPriority;
    int gateIndex;
    int interfaceIndex;
    ISISinterface* iface;

    gateIndex = msg->getArrivalGate()->getIndex();

    iface = this->getIfaceByGateIndex(gateIndex);
    interfaceIndex = this->getIfaceIndex(iface);
    //set circuiType according to LAN Hello Packet type
    if (msg->getType() == LAN_L1_HELLO || msg->getType() == TRILL_HELLO)
    {
        circuitType = L1_TYPE;
        disID = iface->L1DIS;
        disPriority = iface->L1DISpriority;
    }
    else if (msg->getType() == LAN_L2_HELLO)
    {
        circuitType = L2_TYPE;
        disID = iface->L2DIS;
        disPriority = iface->L2DISpriority;
    }

    unsigned char *msgLanID;
    msgLanID = this->getLanID(msg);

    if (this->compareArrays(msgLanID, disID, ISIS_SYSTEM_ID + 1))
    {
        //LAN DIS in received message is the same as the one that THIS IS have
        delete[] msgLanID;
        return;
    }

    unsigned char* lastDIS = new unsigned char[ISIS_SYSTEM_ID + 1];
    this->copyArrayContent(disID, lastDIS, ISIS_SYSTEM_ID + 1, 0, 0);

    MACAddress localDIS, receivedDIS;
    ISISadj *tmpAdj;

    //first old/local DIS
    //if DIS == me then use my MAC for specified interface
    if (this->amIL1DIS(interfaceIndex))
    {
        localDIS = iface->entry->getMacAddress();
    }
    //else find adjacency and from that use MAC
    else
    {
        if ((tmpAdj = this->getAdjBySystemID(lastDIS, circuitType, gateIndex)) != NULL)
        {
            localDIS = tmpAdj->mac;
        }
        else
        {
            EV<< "deviceId: " << deviceId
            << " ISIS: Warning: Didn't find adjacency for local MAC comparison in electL1DesignatedIS "
            << endl;
            localDIS = MACAddress("000000000000");
        }
    }

            //find out MAC address for IS with LAN-ID from received message
    if ((tmpAdj = this->getAdjBySystemID(msgLanID, circuitType, gateIndex)) != NULL)
    {
        receivedDIS = tmpAdj->mac;
    }
    else
    {
        EV<< "deviceId: " << deviceId
        << " ISIS: Warning: Didn't find adjacency for received MAC comparison in electL1DesignatedIS "
        << endl;
        receivedDIS = MACAddress("000000000000");
    }

        //if announced DIS priority is higher then actual one or if they are equal and src MAC is higher than mine, then it's time to update DIS
    if (((msg->getPriority() > disPriority)
            || (msg->getPriority() == disPriority && (receivedDIS.compareTo(localDIS) > 0))))
    {
        unsigned char * disLspID = new unsigned char[ISIS_SYSTEM_ID + 2];
        this->copyArrayContent(lastDIS, disLspID, ISIS_SYSTEM_ID + 1, 0, 0);
        disLspID[ISIS_SYSTEM_ID + 1] = 0; //set fragment-ID
        //purge lastDIS's LSPs
        this->purgeRemainLSP(disLspID, circuitType);

        //set new DIS (LAN-ID)
        this->copyArrayContent(msgLanID, disID, ISIS_SYSTEM_ID + 1, 0, 0);
        //and set his priority
        disPriority = msg->getPriority();

        if(mode == L2_ISIS_MODE){
            //TODO B1 SEVERE call trillDIS() -> such method would appoint forwarder and handled other TRILL-DIS related duties
//            TRILLInterfaceData *trillD = ift->getInterfaceByNetworkLayerGateIndex(gateIndex)->trillData();
//            trillD->clearAppointedFWDs();
            //when appointed forwarder observes that DRB on link has changet, it NO LONGER considers itselft as app fwd


        }
        delete[] disLspID;
    }

    //clean
    delete[] msgLanID;
    delete[] lastDIS;
}





/**
 * Reset DIS on all interfaces. New election occurs after reset.
 * @see electL1DesignatedIS(ISISL1HelloPacket *msg)
 * @param IStype Defines IS type - L1 or L2
 */
void ISIS::resetDIS(unsigned char* systemID, int interfaceIndex, short circuitType)
{

    ISISinterface *iface = &(this->ISISIft.at(interfaceIndex));
    if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
    {
        //if the systemID was DIS, then set DIS = me. if dead neighbour wasn't DIS do nothing.
        if (this->compareArrays(systemID, iface->L1DIS, ISIS_SYSTEM_ID + 1))
        {
            //set myself as DIS
            iface->L1DISpriority = iface->priority;
            //set initial designated IS as himself
            this->copyArrayContent((unsigned char*) this->sysId, iface->L1DIS, ISIS_SYSTEM_ID, 0, 0);
            //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
            iface->L1DIS[ISIS_SYSTEM_ID] = interfaceIndex + 1;
        }
    }

    if (circuitType == L2_TYPE || circuitType == L1L2_TYPE)
    {
        //if the systemID was DIS, then set DIS = me. if dead neighbour wasn't DIS do nothing.
        if (this->compareArrays(systemID, iface->L2DIS, ISIS_SYSTEM_ID + 1))
        {
            //set myself as DIS
            iface->L2DISpriority = iface->priority;
            //set initial designated IS as himself
            this->copyArrayContent((unsigned char*) this->sysId, iface->L2DIS, ISIS_SYSTEM_ID, 0, 0);
            //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
            iface->L2DIS[ISIS_SYSTEM_ID] = interfaceIndex + 1;
        }
    }
}





short ISIS::getLevel(ISISMessage *msg)
{
    switch (msg->getType())
        {
        case (LAN_L1_HELLO):
        case (L1_LSP):
        case (L1_CSNP):
        case (L1_PSNP):
            return L1_TYPE;
            break;

        case (LAN_L2_HELLO):
        case (L2_LSP):
        case (L2_CSNP):
        case (L2_PSNP):
            return L2_TYPE;
            break;

        default:
            EV<< "deviceId " << deviceId << ": ISIS: WARNING: Unsupported Message Type in getLevel" << endl;

            break;

        }
        return L1_TYPE;
    }

/*
 * Handles L1 LSP according to ISO 10589 7.3.15.1
 * @param lsp is received LSP
 */
void ISIS::handleLsp(ISISLSPPacket *lsp)
{

    /*
     * Verify that we have adjacency UP for Source-ID in lsp
     * if the lsp is received on broadcast circuit compare senders MAC address with MAC address in adjacency
     * We don't need to check Area address, it was done by verifying adjacency
     * if remainingLifetime == 0 -> purgeLSP
     * if lsp->getLspId() == this->sysID //if it's my LSP
     *  and i don't have it anymore in my lspDb, then init network wide purge ... purgeLSP()
     *  if it's my LSP and I have it in DB perform 7.3.16.1 -> they have old version so send new version to that link
     *  source is somebody else:
     * received lsp is newer or i don't have it in DB => installLSP
     *   set SRMflags and clear it for incomming interface
     *   if received on PTP set SSNflag for that interface and clear other
     * received lsp is equal(same lsp-ID, seqNumber, remLifetime both zero or both non-zero)
     *   clear SRM flag for C
     *   if C is PTP set SSN
     * received is older than in DB
     *   set SRM flag for C
     *   clear SSN flag for C
     *

     * Process PATT.. flag (IS Type, overload, etc.)

     *
     * this->updateLSP(lsp, L1_TYPE);
     */
    /* 7.3.15.1. */
    unsigned char *lspID;
    int circuitType = this->getLevel(lsp);
    int gateIndex = lsp->getArrivalGate()->getIndex();
    ISISinterface *iface = this->getIfaceByGateIndex(gateIndex);
    int interfaceIndex = this->getIfaceIndex(iface);
    /* 7.3.15.1. a) 6) */
    //returns true if for source-id in LSP there is adjacency with matching MAC address
    if (!this->isAdjUp(lsp, circuitType))
    {
        //no adjacency for source-id => discard lsp
        //generate event?

        EV<< "ISIS: Warning: Discarding LSP: didn't find adjacency in state UP for this LSP" << endl;
        delete lsp;
        return;
    }
//    this->schedulePeriodicSend(circuitType);

    lspID = this->getLspID(lsp);

    /* 7.3.15.1. b */
    if (lsp->getRemLifeTime() == 0)
    {

        this->purgeLSP(lsp, circuitType);
        /* lsp is already deleted in purgeLSP */
        //delete lsp;
        delete[] lspID;
        return;

    }
    LSPRecord *lspRec;
    //is it my LSP?
    if (this->compareArrays(lspID, (unsigned char*) this->sysId, ISIS_SYSTEM_ID))
    {

        //if i don't have it anymore
        /* 7.3.15.1 c) i don't have it in DB */
        if ((lspRec = this->getLSPFromDbByID(lspID, circuitType)) == NULL)
        {
            //init network wide purge
            this->purgeLSP(lsp, circuitType);
            //delete lsp;
            delete lspID;
            return;
        }
        else
        {
            /* 7.3.15.1 c) OR no longer in the set of LSPs generated by this system */
            if (lspRec->deadTimer->getTimerKind() == LSP_DELETE_TIMER) //TODO improve this
            {/*if the deadTimer is set to xxLSP_DELETE, then it's already purged
             * and in database is kept just header.
             */
                /* 7.3.15.1 c) */
                this->purgeLSP(lsp, circuitType);
                delete lspID;
                return;
            }
            /* 7.3.15.1 d) */
            //if we have it
            /* 7.3.16.1 sequence numbers */
            if (lsp->getSeqNumber() > lspRec->LSP->getSeqNumber())
            {
//                std::cout<<"handle seqNum: "<< lspRec->LSP->getSeqNumber() <<endl;
                lspRec->LSP->setSeqNumber(lsp->getSeqNumber() + 1); //TODO handle overflow of seqNumer
//                std::cout<<"handle seqNum: "<< lspRec->LSP->getSeqNumber() <<endl;
                this->setSRMflags(lspRec, circuitType);
                //TODO set new remaining lifetime
                //TODO reschedule deadTimer

            }
            else
            {

                delete lsp;
                delete lspID;
                return;
            }

        }
    }
    else
    {
        /* 7.3.15.1 e) 1) */
        lspRec = this->getLSPFromDbByID(lspID, circuitType);
        if (lspRec == NULL)
        {
            /* 7.3.15.1 e) 1) i. */
            this->installLSP(lsp, circuitType);
            this->schedulePeriodicSend(circuitType);
            delete[]   lspID;
            return;

        }
        else
        {
            if (lsp->getSeqNumber() > lspRec->LSP->getSeqNumber())
            {
                /* 7.3.15.1 e) 1) i. */
                this->replaceLSP(lsp, lspRec, circuitType);
                this->schedulePeriodicSend(circuitType);
                delete[] lspID;
                return;

            }
            /* 7.3.15.1 e) 2) */
            /* should check also lsp->getRemLifetime != 0  OR both zero, but this is handled in purgeLSP*/
            else if (lsp->getSeqNumber() == lspRec->LSP->getSeqNumber() && lspRec->LSP->getRemLifeTime() != 0)
            {

                /* 7.3.15.1 e) 2) i. */
                this->clearSRMflag(lspRec, interfaceIndex, circuitType);

                if (!this->ISISIft.at(interfaceIndex).network)
                {
                    /* 7.3.15.1 e) 2) ii. */
                    this->setSSNflag(lspRec, interfaceIndex, circuitType);
                }

            }
            /* 7.3.15.1 e) 3) */
            else if (lsp->getSeqNumber() < lspRec->LSP->getSeqNumber())
            {
                /* 7.3.15.1 e) 3) i. */
                this->setSRMflag(lspRec, interfaceIndex, circuitType);
                /* 7.3.15.1 e) 3) ii. */
                this->clearSSNflag(lspRec, interfaceIndex, circuitType);
            }
        }

    }
    delete lsp;
    delete lspID;
}



/*
 * Create and send CSNP message to DIS interface.
 * @param timer is specific CSNP timer
 */
void ISIS::sendCsnp(ISISTimer *timer)
{
    //TODO don't know how to handle csnp over PtP yet (there is no periodic sending, but initial csnp is sent)
    /* Maybe send CSNP during some initial interval (or number of times, or just once) and then just don't re-schedule timer for this interface */
    if (this->ISISIft.at(timer->getInterfaceIndex()).network && (false && !this->ISISIft.at(timer->getInterfaceIndex()).network || !this->amIDIS(timer->getInterfaceIndex(), timer->getIsType())))
    {
        this->schedule(timer);
        return;
    }

//    ISISinterface *iface = &(this->ISISIft.at(timer->getInterfaceIndex()));
//    unsigned char * disID;
    short circuitType = timer->getIsType();

    std::vector<LSPRecord *> *lspDb = this->getLSPDb(circuitType);
    std::vector<LSPRecord *>::iterator it = lspDb->begin();
    for (int fragment = 0; it != lspDb->end(); fragment++)

    {
        ISISCSNPPacket *packet = new ISISCSNPPacket("CSNP");
        if (circuitType == L1_TYPE)
        {
            packet->setType(L1_CSNP);
//            disID = iface->L1DIS;
        }
        else if (circuitType == L2_TYPE)
        {
            packet->setType(L2_CSNP);
//            disID = iface->L2DIS;
        }
        else
        {
            packet->setType(L1_CSNP);
//            disID = iface->L1DIS;
            EV<< "deviceId " << deviceId << ": ISIS: WARNING: Setting unknown CSNP packet type " << endl;
        }
        packet->setLength(0); //TODO set to length of header

//        //add Ethernet control info
//        Ieee802Ctrl *ctrl = new Ieee802Ctrl();
//
//        // set DSAP & NSAP fields
//        ctrl->setDsap(SAP_CLNS);
//        ctrl->setSsap(SAP_CLNS);
//
//        //set destination broadcast address
//        //It should be multicast 01-80-C2-00-00-14 MAC address, but it doesn't work in OMNeT
//        MACAddress ma;
//        ma.setAddress("ff:ff:ff:ff:ff:ff");
//        ctrl->setDest(ma);
//        packet->setControlInfo(ctrl);

        //set system ID field which consists of my system id + zero circuit id inc this case
        for (unsigned int i = 0; i < packet->getSourceIDArraySize() - 1; i++)
        {
            packet->setSourceID(i, this->sysId[i]);
        }
        //last octet has to be 0 see 9.11 at ISO 10589:2002(E)
        packet->setSourceID(6, 0);

        int lspCount = lspDb->end() - it;

        //TODO set start LSP-ID
        if (fragment != 0)
        {
            for (unsigned int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
            {
                packet->setStartLspID(i, (*it)->LSP->getLspID(i));
            }
        }
        else
        {
            for (unsigned int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
            {
                packet->setStartLspID(i, 0);
            }
        }

        for (; it != lspDb->end() && packet->getLength() < ISIS_LSP_MAX_SIZE;)
        {

            TLV_t myTLV;
            myTLV.type = LSP_ENTRIES;

            if (lspCount * 16 > 255)
            { //TODO replace "16" with something more appropriate
                myTLV.length = 255 - (255 % 16); //TODO minus size of header
            }
            else
            {
                myTLV.length = lspCount * 16;
            }
            myTLV.value = new unsigned char[myTLV.length];
            myTLV.length = 0;

            for (int i = 0;
                    (myTLV.length + 16) < 255 && it != lspDb->end()
                            && (packet->getLength() + (myTLV.length + 16) + 2) < ISIS_LSP_MAX_SIZE; ++it, i++)
            { //255 is maximum that can fit into Length field of TLV
              //add entry from lspDb
              //convert unsigned short to unsigned char array and insert to TLV
                myTLV.value[(i * 16)] = (((*it)->LSP->getRemLifeTime() >> 8) & 0xFF);
                myTLV.value[(i * 16) + 1] = ((*it)->LSP->getRemLifeTime() & 0xFF);

                //copy LSP ID to TLV
                unsigned char *lspId = this->getLspID((*it)->LSP);
                this->copyArrayContent(lspId, myTLV.value, ISIS_SYSTEM_ID + 2, 0, (i * 16) + 2);
                delete[] lspId;
                //convert unsigned long seq number to unsigned char array[4] and insert into TLV
                for (unsigned int j = 0; j < 4; j++)
                {
                    myTLV.value[(i * 16) + 10 + j] = ((*it)->LSP->getSeqNumber() >> (24 - (8 * j))) & 0xFF;
                }

                //set end LSP-ID

                myTLV.length += 16;
                //packet->setLength(packet->getLength() + 16);
            }
            //int tlvSize = packet->getTLVArraySize();
            this->addTLV(packet, &myTLV);
            packet->setLength(packet->getLength() + myTLV.length + 2);

            delete[] myTLV.value;

        }
        if (it != lspDb->end()) //if there is still another lsp in DB
        {
            for (unsigned int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
            {
                packet->setEndLspID(i, (*it)->LSP->getLspID(i));
            }
        }
        else
        //this was last lsp, so mark it as last with LSP-ID FFFFF...
        {
            for (unsigned int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
            {
                packet->setEndLspID(i, 255);
            }

        }

        if (timer->getInterfaceIndex() != this->getIfaceIndex(this->getIfaceByGateIndex(timer->getInterfaceIndex())))
        {
            EV<< "ISIS: Warning: Houston, we got a problem! A BIG ONE!" << endl;
        }

            //send only on interface specified in timer
        send(packet, "lowerLayerOut", timer->getGateIndex());
        EV<< "ISIS::sendCSNP: Source-ID: ";
        for (unsigned int i = 0; i < 6; i++)
            {
                EV << setfill('0') << setw(2) << dec << (unsigned int) this->sysId[i];
                if (i % 2 == 1)
                    EV << ".";
            }

        EV<< endl;

        /*
         //send packet on ALL interfaces with adjacency UP
         for(std::vector<ISISinterface>::iterator intIt = this->ISISIft.begin(); intIt != this->ISISIft.end(); ++intIt)
         {
         if(isUp((*intIt).gateIndex, circuitType) && (*intIt).network && this->compareArrays((unsigned char *)this->sysId, (*intIt).L1DIS, ISIS_SYSTEM_ID))//TODO L1DIS is not based on circuitType
         {
         ISISCSNPPacket *packetDup = packet->dup();
         Ieee802Ctrl *ctrlDup = ctrl->dup();
         packetDup->setControlInfo(ctrlDup);
         //set source LAN ID
         packetDup->setSourceID(ISIS_SYSTEM_ID, (*intIt).gateIndex);
         send(packetDup, "lowerLayerOut", (*intIt).gateIndex);
         }
         }
         delete ctrl;
         delete packet;
         */
    }

    //reschedule timer
    this->schedule(timer);

}

/*
 * Create and send PSNP message to DIS interface.
 * @param timer is specific PSNP timer
 */
void ISIS::sendPsnp(ISISTimer *timer)
{

    int gateIndex;
    int interfaceIndex;
    short circuitType;
//    unsigned char * disID;
    ISISinterface * iface;
    FlagRecQ_t *SSNQueue;

    gateIndex = timer->getGateIndex();
    interfaceIndex = timer->getInterfaceIndex();
    circuitType = timer->getIsType();
    iface = &(this->ISISIft.at(interfaceIndex));
    SSNQueue = this->getSSNQ(iface->network, interfaceIndex, circuitType);

    if (this->amIDIS(interfaceIndex, circuitType))
    {
        //reschedule OR make sure that during DIS election/resignation this timer is handled correctly
        //this is a safer, but dumber, solution
        //TODO see above
        this->schedule(timer);
        return;

    }

    //if queue is empty reschedule timer
    if (SSNQueue->empty())
    {
        this->schedule(timer);
        return;
    }

    ISISPSNPPacket *packet = new ISISPSNPPacket("PSNP");
    if (circuitType == L1_TYPE)
    {
        packet->setType(L1_PSNP);
//        disID = iface->L1DIS;
    }
    else if (circuitType == L2_TYPE)
    {
        packet->setType(L2_PSNP);
//        disID = iface->L2DIS;
    }
    else
    {
        packet->setType(L1_PSNP);
//        disID = iface->L1DIS;
        EV<< "deviceId " << deviceId << ": ISIS: WARNING: Setting unknown PSNP packet type " << endl;
    }
    packet->setLength(0); //TODO set to length of header

//    //add Ethernet control info
//    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
//
//    // set DSAP & NSAP fields
//    ctrl->setDsap(SAP_CLNS);
//    ctrl->setSsap(SAP_CLNS);
//
//    //set destination broadcast address
//    //It should be multicast 01-80-C2-00-00-14 MAC address, but it doesn't work in OMNeT
//    MACAddress ma;
//    ma.setAddress("ff:ff:ff:ff:ff:ff");
//    ctrl->setDest(ma);
//
//    packet->setControlInfo(ctrl);

    //set system ID field which consists of my system id + zero circuit id in this case
    for (unsigned int i = 0; i < packet->getSourceIDArraySize() - 1; i++)
    {
        packet->setSourceID(i, this->sysId[i]);
    }
    packet->setSourceID(6, 0);

    //TODO check that all fields of the packet are filled correctly

    unsigned int lspCount = SSNQueue->size();
    for (FlagRecQ_t::iterator it = SSNQueue->begin(); it != SSNQueue->end();)
    {

        TLV_t myTLV;
        myTLV.type = LSP_ENTRIES;

        if (lspCount * 16 > 255)
        { //TODO replace "16" with something more appropriate
            myTLV.length = 255 - (255 % 16); //TODO minus size of header
        }
        else
        {
            myTLV.length = lspCount * 16;
        }
        myTLV.value = new unsigned char[myTLV.length];
        myTLV.length = 0;

        for (int i = 0;
                (myTLV.length + 16) < 255 && it != SSNQueue->end()
                        && (packet->getLength() + (myTLV.length + 16) + 2) < ISIS_LSP_MAX_SIZE;
                it = SSNQueue->begin(), i++)
        { //255 is maximum that can fit into Length field of TLV
          //add entry from lspDb

            //convert unsigned short to unsigned char array and insert to TLV
            myTLV.value[(i * 16)] = (((*it)->lspRec->LSP->getRemLifeTime() >> 8) & 0xFF);
            myTLV.value[(i * 16) + 1] = ((*it)->lspRec->LSP->getRemLifeTime() & 0xFF);

            //copy LSP ID to TLV
            unsigned char *lspId = this->getLspID((*it)->lspRec->LSP);
            this->copyArrayContent(lspId, myTLV.value, ISIS_SYSTEM_ID + 2, 0, (i * 16) + 2);
            delete[] lspId;
            //convert unsigned long seq number to unsigned char array[4] and insert into TLV
            for (unsigned int j = 0; j < 4; j++)
            {
                myTLV.value[(i * 16) + 10 + j] = ((*it)->lspRec->LSP->getSeqNumber() >> (24 - (8 * j))) & 0xFF;
            }

            //set end LSP-ID

            myTLV.length += 16;
            //packet->setLength(packet->getLength() + 16);

            //it should wait after sending the lsp, but let's just assume it will work fine
            //clear SSNflag, this should also remove the flagRecord from correct queue(vector
            this->clearSSNflag((*it)->lspRec, (*it)->index, circuitType);
        }
        //int tlvSize = packet->getTLVArraySize();
        this->addTLV(packet, &myTLV);
        packet->setLength(packet->getLength() + myTLV.length + 2);
        delete[] myTLV.value;

    }

    if (timer->getInterfaceIndex() != this->getIfaceIndex(this->getIfaceByGateIndex(timer->getInterfaceIndex())))
    {
        EV<< "ISIS: Warning: Houston, we got a problem! A BIG ONE!" << endl;
    }

        //send only on interface specified in timer
    send(packet, "lowerLayerOut", gateIndex);
    EV<< "ISIS::sendPSNP: Source-ID: ";
    for (unsigned int i = 0; i < 6; i++)
        {
            EV << setfill('0') << setw(2) << dec << (unsigned int) this->sysId[i];
            if (i % 2 == 1)
                EV << ".";
        }

    EV<< endl;


    this->schedule(timer);
}

/*
 * Handle incomming psnp message according to ISO 10589 7.3.15.2
 * @param psnp is incomming PSNP message
 */
void ISIS::handlePsnp(ISISPSNPPacket *psnp)
{

    short circuitType = this->getLevel(psnp);
    int gateIndex = psnp->getArrivalGate()->getIndex();
    ISISinterface* iface = this->getIfaceByGateIndex(gateIndex);
    int interfaceIndex = this->getIfaceIndex(iface);

    /* 7.3.15.2. a) If circuit C is a broadcast .. */
    if (iface->network && !this->amIDIS(interfaceIndex, circuitType))
    {
        EV<< "ISIS: Warning: Discarding PSNP. Received on nonDIS interface." << endl;
        delete psnp;
        return;
    }

        /* 7.3.15.2 a) 6) */
        /* we handle broadcast and non-broadcast adjacencies same way */
    if (!this->isAdjUp(psnp, circuitType))
    {
        EV<< "ISIS: Warning: Discarding PSNP. Didn't find matching adjacency." << endl;
        delete psnp;
        return;
    }

        /* 7.3.15.2 a) 7) */
        /* 7.3.15.2 a) 8) */
        /* Authentication is omitted */

        /* 7.3.15.2 b) */
    TLV_t * tmpTlv;
    unsigned char *tmpLspID;
    for (int offset = 0; (tmpTlv = this->getTLVByType(psnp, LSP_ENTRIES, offset)) != NULL; offset++)
    {

        for (int i = 0; i < tmpTlv->length; i += 16) //TODO change 16 to something
        {
            tmpLspID = new unsigned char[ISIS_SYSTEM_ID + 2];
            this->copyArrayContent(tmpTlv->value, tmpLspID, (ISIS_SYSTEM_ID + 2), i + 2, 0);
            unsigned short remLife = tmpTlv->value[i] * 255 + tmpTlv->value[i + 1];
            /* this just makes me laugh */
            unsigned long seqNum = tmpTlv->value[i + 10] * 255 * 255 * 255 + tmpTlv->value[i + 11] * 255 * 255
                    + tmpTlv->value[i + 12] * 255 + tmpTlv->value[i + 13];
            //getLspBySysID
            LSPRecord *lspRec;
            lspRec = this->getLSPFromDbByID(tmpLspID, circuitType);
            if (lspRec != NULL)
            {
                /* 7.3.15.2 b) 2) */
                //if values are same
                if (seqNum == lspRec->LSP->getSeqNumber()
                        && (remLife == lspRec->LSP->getRemLifeTime()
                                || (remLife != 0 && lspRec->LSP->getRemLifeTime() != 0)))
                {
                    //if non-broadcast -> clear SRMflag for C
                    if (!iface->network)
                    {
                        this->clearSRMflag(lspRec, interfaceIndex, circuitType);
                    }
                }
                /* 7.3.15.2 b) 3) */
                //else if received is older
                else if (seqNum < lspRec->LSP->getSeqNumber())
                {
                    //clean SSN and set SRM
                    this->clearSSNflag(lspRec, interfaceIndex, circuitType);
                    this->setSRMflag(lspRec, interfaceIndex, circuitType);
                }
                /* 7.3.15.2 b) 4) */
                //else if newer
                else if (seqNum > lspRec->LSP->getSeqNumber()
                        || (seqNum == lspRec->LSP->getSeqNumber() && lspRec->LSP->getRemLifeTime() != 0))
                {
                    //setSSNflag AND if C is non-broadcast clearSRM
                    this->setSSNflag(lspRec, interfaceIndex, circuitType);
                    if (!iface->network)
                    {
                        this->clearSRMflag(lspRec, interfaceIndex, circuitType);
                    }
                }
            }
            /* 7.3.15.2 b) 5) */
            else
            {
                //if remLifetime, checksum, seqNum are all non-zero
                if (remLife != 0 && seqNum != 0)
                {
                    this->printSysId((unsigned char *) this->sysId);
//                    std::cout<<"Received new LSP in PSNP";
//                    this->printLspId(tmpLspID);
                    //create LSP with seqNum 0 and set SSNflag for C
                    //DO NOT SET SRMflag!!!!!!!
                    ISISLSPPacket *lsp;
                    if (circuitType == L1_TYPE)
                    {
                        lsp = new ISISLSPPacket("L1 LSP Packet");
                        lsp->setType(L1_LSP);
                        ///set PATTLSPDBOLIS
                        lsp->setPATTLSPDBOLIS(0x02 + (this->att * 0x10)); //only setting IS type = L1 (TODO A2)
                    }
                    else if (circuitType == L2_TYPE)
                    {
                        lsp = new ISISLSPPacket("L2 LSP Packet");
                        lsp->setType(L2_LSP);
                        //set PATTLSPDBOLIS
                        lsp->setPATTLSPDBOLIS(0x0A); //bits 1(2) and 3(8) => L2
                    }

                    //remLifeTime
                    lsp->setRemLifeTime(remLife);
                    //lspID[8]
                    this->setLspID(lsp, tmpLspID);

                    //set seqNum
                    lsp->setSeqNumber(0);

                    //install new "empty" LSP and set SSNflag
                    this->setSSNflag(this->installLSP(lsp, circuitType), interfaceIndex, circuitType);

                }
            }
            delete[] tmpLspID;
        }

    }
    //if lsp-entry equals
    delete psnp;
}

/*
 * Handles incomming CSNP message according to ISO 10589 7.3.15.2
 * @param csnp is incomming CSNP message
 */
void ISIS::handleCsnp(ISISCSNPPacket *csnp)
{

    short circuitType = this->getLevel(csnp); //L1_TYPE; //TODO get circuitType from csnp
    int gateIndex = csnp->getArrivalGate()->getIndex();
    ISISinterface* iface = this->getIfaceByGateIndex(gateIndex);
    int interfaceIndex = this->getIfaceIndex(iface);

    /* 7.3.15.2 a) 6) */
    /* we handle broadcast and non-broadcast adjacencies same way */
    if (!this->isAdjUp(csnp, circuitType))
    {
        EV<< "ISIS: Warning: Discarding CSNP. Didn't find matching adjacency." << endl;
        delete csnp;
        return;
    }

        /* 7.3.15.2 a) 7) */
        /* 7.3.15.2 a) 8) */
        /* Authentication is omitted */

    std::vector<unsigned char *>* lspRange;
    lspRange = this->getLspRange(this->getStartLspID(csnp), this->getEndLspID(csnp), circuitType);

    //for -> iterate over tlvArraySize and then iterate over TLV
//    for(int i = 0; i < csnp->getTLVArraySize(); i++)
    /* 7.3.15.2 b) */
    TLV_t * tmpTlv;
    unsigned char *tmpLspID;
    for (int offset = 0; (tmpTlv = this->getTLVByType(csnp, LSP_ENTRIES, offset)) != NULL; offset++)
    {

        for (int i = 0; i + 16 <= tmpTlv->length; i += 16) //TODO change 16 to something
        {
            tmpLspID = new unsigned char[ISIS_SYSTEM_ID + 2];
            this->copyArrayContent(tmpTlv->value, tmpLspID, (ISIS_SYSTEM_ID + 2), i + 2, 0);
            unsigned short remLife = tmpTlv->value[i] * 255 + tmpTlv->value[i + 1];
            /* this just makes me laugh */
            unsigned long seqNum = tmpTlv->value[i + 10] * 255 * 255 * 255 + tmpTlv->value[i + 11] * 255 * 255
                    + tmpTlv->value[i + 12] * 255 + tmpTlv->value[i + 13];

            /* 7.3.15.2 c) */
            if (!lspRange->empty())
            {
                while (memcmp(lspRange->front(), tmpLspID, ISIS_SYSTEM_ID + 2) < 0)
                {
                    this->setSRMflag(this->getLSPFromDbByID(lspRange->front(), circuitType), interfaceIndex,
                            circuitType);
                    delete lspRange->front();
                    lspRange->erase(lspRange->begin());
                }

                if (memcmp(lspRange->front(), tmpLspID, ISIS_SYSTEM_ID + 2) == 0)
                {
                    delete lspRange->front();
                    lspRange->erase(lspRange->begin());
                }
            }

            //getLspBySysID
            LSPRecord *lspRec;
            lspRec = this->getLSPFromDbByID(tmpLspID, circuitType);
            if (lspRec != NULL)
            {
                //if values are same
                if (seqNum == lspRec->LSP->getSeqNumber()
                        && (remLife == lspRec->LSP->getRemLifeTime()
                                || (remLife != 0 && lspRec->LSP->getRemLifeTime() != 0)))
                {
                    //if non-broadcast -> clear SRMflag for C
                    if (!iface->network)
                    {
                        this->clearSRMflag(lspRec, interfaceIndex, circuitType);
                    }
                }
                //else if received is older
                else if (seqNum < lspRec->LSP->getSeqNumber())
                {
                    //clean SSN and set SRM
                    this->clearSSNflag(lspRec, interfaceIndex, circuitType);
                    this->setSRMflag(lspRec, interfaceIndex, circuitType);
                }
                //else if newer
                else if (seqNum > lspRec->LSP->getSeqNumber()
                        || (seqNum == lspRec->LSP->getSeqNumber() && lspRec->LSP->getRemLifeTime() != 0))
                {
                    //setSSNflag AND if C is non-broadcast clearSRM
                    this->setSSNflag(lspRec, interfaceIndex, circuitType);
                    if (!iface->network)
                    {
                        this->clearSRMflag(lspRec, interfaceIndex, circuitType);
                    }
                }
            }
            /* 7.3.15.2 b) 5) */
            else
            {
                //if remLifetime, checksum, seqNum are all non-zero
                if (remLife != 0 && seqNum != 0)
                {
                    this->printSysId((unsigned char *) this->sysId);
//                    std::cout << "Received new LSP in CSNP";
//                    this->printLspId(tmpLspID);
                    //create LSP with seqNum 0 and set SSNflag for C
                    //DO NOT SET SRMflag!!!!!!!
                    ISISLSPPacket *lsp;
                    if (circuitType == L1_TYPE)
                    {
                        lsp = new ISISLSPPacket("L1 LSP Packet");
                        lsp->setType(L1_LSP);
                        ///set PATTLSPDBOLIS
                        lsp->setPATTLSPDBOLIS(0x02 + (this->att * 0x10)); //only setting IS type = L1 (TODO A2)
                    }
                    else if (circuitType == L2_TYPE)
                    {
                        lsp = new ISISLSPPacket("L2 LSP Packet");
                        lsp->setType(L2_LSP);
                        //set PATTLSPDBOLIS
                        lsp->setPATTLSPDBOLIS(0x0A); //bits 1(2) and 3(8) => L2
                    }
                    //remLifeTime
                    lsp->setRemLifeTime(remLife);
                    //lspID[8]
                    this->setLspID(lsp, tmpLspID);

                    //set seqNum
                    lsp->setSeqNumber(0);

                    //install new "empty" LSP and set SSNflag
                    this->setSSNflag(this->installLSP(lsp, circuitType), interfaceIndex, circuitType);

                }
            }
            //TODO A! delete[] tmpLspID;
        }

    }

    while (!lspRange->empty())
    {
        LSPRecord *tmpRec = this->getLSPFromDbByID(lspRange->front(), circuitType);
        if(tmpRec != NULL){
            this->setSRMflag(this->getLSPFromDbByID(lspRange->front(), circuitType), interfaceIndex, circuitType);
        }else{
            EV<<"ISIS: ERROR: run for your Life!!!!!"<<endl;
        }
//TODO A!        delete lspRange->front();
        lspRange->erase(lspRange->begin());

    }
//TODO A!    delete lspRange;
    //if lsp-entry equals
    delete csnp;
}

/*
 * Returns set of LSP-IDs in range from startLspID to endLspID
 * @param startLspID beginning of range
 * @param endLspID end of range
 * @return range of LSP-IDs
 */
std::vector<unsigned char *>* ISIS::getLspRange(unsigned char *startLspID, unsigned char * endLspID, short circuitType)
{

    int res1, res2;
    unsigned char * lspID;
    std::vector<LSPRecord *> *lspDb = this->getLSPDb(circuitType);
    std::vector<unsigned char*> *lspRange = new std::vector<unsigned char *>;
    std::sort(lspDb->begin(), lspDb->end(), cmpLSPRecord());
    //TODO we can end the search before hitting lspDb->end when DB is sorted
    for (std::vector<LSPRecord *>::iterator it = lspDb->begin(); it != lspDb->end(); ++it)
    {
        lspID = this->getLspID((*it)->LSP);
        res1 = memcmp(startLspID, lspID, ISIS_SYSTEM_ID + 2);
        res2 = memcmp(lspID, endLspID, ISIS_SYSTEM_ID + 2);
        if (res1 <= 0 && res2 >= 0)
        {
            lspRange->push_back(lspID);
        }
        delete[] lspID;
    }

    return lspRange;

}

/*
 * Extracts Start LSP-ID from CSNP message.
 * @param csnp incoming CSNP message.
 * @return start LSP-ID.
 */
unsigned char * ISIS::getStartLspID(ISISCSNPPacket *csnp)
{

    unsigned char *lspId = new unsigned char[ISIS_SYSTEM_ID + 2];

    for (int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
    {
        lspId[i] = csnp->getStartLspID(i);
    }

    return lspId;
}

/*
 * Extracts End LSP-ID from CSNP message.
 * @param csnp incoming CSNP message.
 * @return end LSP-ID.
 */
unsigned char * ISIS::getEndLspID(ISISCSNPPacket *csnp)
{

    unsigned char *lspId = new unsigned char[ISIS_SYSTEM_ID + 2];

    for (int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
    {
        lspId[i] = csnp->getEndLspID(i);
    }

    return lspId;
}



/**
 * Check sysId from received hello packet for duplicity.
 * @return True if neighbour's sysId is the same as mine, false otherwise.
 */
bool ISIS::checkDuplicateSysID(ISISMessage * msg)
{

    bool test = false;

    if (msg->getType() == LAN_L1_HELLO || msg->getType() == LAN_L2_HELLO || msg->getType() == TRILL_HELLO)
    {

        // typecast for L1 hello; L1 and L2 hellos differ only in "type" field
        ISISLANHelloPacket *hello = check_and_cast<ISISLANHelloPacket *>(msg);

        //check for area address tlv and compare with my area id
        for (unsigned int j = 0; j < hello->getTLVArraySize(); j++)
        {
            if (hello->getTLV(j).type == AREA_ADDRESS
                    && this->compareArrays((unsigned char *) this->areaId, &(hello->getTLV(j).value[1]),
                            hello->getTLV(j).value[0]))
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
        EV<< this->deviceId << ": IS-IS WARNING: possible duplicate system ID ";
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


void ISIS::schedulePeriodicSend(short circuitType){
    ISISTimer *timer;
    if(circuitType == L1_TYPE){
        timer = this->periodicL1Timer;
    }else{
        timer = this->periodicL2Timer;
    }

    cancelEvent(timer);
    scheduleAt(simTime() + 0.33, timer);
}

/*
 * Sends LSPs that have set SRM flag.
 * For PtP interfaces sends all messages in queue.
 * For broadcast sends only one random LSP.
 * Method is called per interface.
 * @param timer incomming timer
 * @param circuiType specify level for which send should be performed.
 */
void ISIS::periodicSend(ISISTimer* timer, short circuitType)
{
    /*
     * TODO improvement: don't scan whole database, but instead
     *  create queue "toBeSend" with lspRec* and index to SRMflags vector
     *  and when setting SRMflag (in any method) put a record in mentioned queue
     *  When doing periodicSend we don't have to check whole database (but should
     *   at least in the beginning to check that it works correctly), but just this queue
     *  We could have two queues (for PTP and broadcast).
     *  PTP queue would be simply send out.
     *  Broadcast queue would be subject of random picking.
     *
     */
    /*
     std::vector<LSPRecord *> *lspDb = this->getLSPDb(circuitType);
     std::vector<ISISadj> * adjTable = this->getAdjTab(circuitType);
     std::vector<ISISinterface>::iterator itIface = this->ISISIft.begin();
     */

    std::vector<std::vector<FlagRecord*>*> * SRMPTPQueue = getSRMPTPQueue(circuitType);
    std::vector<std::vector<FlagRecord*>*> * SRMBQueue = getSRMBQueue(circuitType);

    //std::cout << "Starting Periodic send for: ";
    //this->printSysId((unsigned char *) this->sysId);
    //std::cout << endl;

    //PTP circuits
    for (std::vector<std::vector<FlagRecord*>*>::iterator it = SRMPTPQueue->begin(); it != SRMPTPQueue->end(); ++it)
    {
        for (std::vector<FlagRecord*>::iterator itRec = (*it)->begin(); itRec != (*it)->end(); ++itRec)
        {

            //std::cout<<"sendLsp to:" << (*itRec)->index <<" : ";
            //this->printLspId(this->getLspID((*itRec)->lspRec->LSP));

            this->sendLSP((*itRec)->lspRec, this->ISISIft.at((*itRec)->index).gateIndex);

            //DON'T clear SRMflag for PtP

            /* when the below code is commented:
             * it might be necessary to incorporate retransmit interval and
             * when checking if the SRM flag is not already set.
             * setting the flag multiple times may cause bloating appropriate flag queue.
             */
            //delete (*itRec);
            //itRec = (*it)->erase(itRec); //instead of ++itRec in for()!!!!!!
        }

    }

    //broadcast circuits
    //for each queue(interface) pick one LSP and send it
    for (std::vector<std::vector<FlagRecord*>*>::iterator it = SRMBQueue->begin(); it != SRMBQueue->end(); ++it)
    {
        int queueSize = (*it)->size();
        if (queueSize == 0)
        {
            continue;
        }
        int index = floor(uniform(0, queueSize)); /*!< Index to circuit's SRMQueue */

        //send random LSP from queue
        //TODO if queue.size() > 10 pick two LSPs (or something like it)
        //TODO maybe? better version would be with this->ISISIft.at((*it)->at(index)->index)
        this->sendLSP((*it)->at(index)->lspRec, this->ISISIft.at((*it)->at(index)->index).gateIndex);

        //clear SRMflag
        this->clearSRMflag((*it)->at(index)->lspRec, (*it)->at(index)->index, circuitType);
        /*        (*it)->at(index)->lspRec->SRMflags.at((*it)->at(index)->index);

         delete (*it)->at(index);
         //and remove FlagRecord from queue
         (*it)->erase((*it)->begin() + index);
         */
        queueSize = (*it)->size();


        //TODO A2 Code below has been added just to speed up lsp redistribution on broadcast links, but it needs more dynamic solution
        if (queueSize == 0)
        {
            continue;
        }
        index = floor(uniform(0, queueSize)); /*!< Index to circuit's SRMQueue */

        //send random LSP from queue
        //TODO if queue.size() > 10 pick two LSPs (or something like it)
        //TODO maybe? better version would be with this->ISISIft.at((*it)->at(index)->index)
        this->sendLSP((*it)->at(index)->lspRec, this->ISISIft.at((*it)->at(index)->index).gateIndex);

        //clear SRMflag
        this->clearSRMflag((*it)->at(index)->lspRec, (*it)->at(index)->index, circuitType);
        /*        (*it)->at(index)->lspRec->SRMflags.at((*it)->at(index)->index);

         delete (*it)->at(index);
         //and remove FlagRecord from queue
         (*it)->erase((*it)->begin() + index);
         */
        queueSize = (*it)->size();

    }
    //reschedule PERIODIC_SEND timer
    this->schedule(timer);
}

/*
 * This method actually sends the LSP out of this module to specified interface.
 * @param lspRec specify record in LSP database that needs to be send
 * @param gateIndex specify interface to which the lsp should be send
 */
void ISIS::sendLSP(LSPRecord *lspRec, int gateIndex)
{

    /* TODO C1
     * incorporate this->lspInterval ... Minimum delay in ms between sending two LSPs.
     *
     */
    /*    std::cout<<"Sending LSP from: ";
     this->printSysId((unsigned char *)this->sysId);
     std::cout<< endl;
     std::cout<<"sendLsp to:" << gateIndex <<" : ";
     this->printLspId(this->getLspID(lspRec->LSP));

     this->printLSP(lspRec->LSP, "sendLSP");*/
    //update remainingLifeTime
    //TODO check if new remLifeTime is not 0 or smaller
    if (lspRec->deadTimer->getTimerKind() == LSP_DELETE_TIMER)
    {

    }
    else
    {
        double remLife = lspRec->simLifetime - simTime().dbl();
        if (remLife < 0)
        {
            EV<< "ISIS: Warning: Remaining lifetime smaller than zero in sendLSP" << endl;
            lspRec->LSP->setRemLifeTime(1);
        }
        else
        {
            unsigned short rem = floor(remLife);
            lspRec->LSP->setRemLifeTime(rem);
        }
    }

    ISISLSPPacket *tmpLSP = lspRec->LSP->dup();
//    //TODO add proper control Info for point-to-point
//    Ieee802Ctrl *tmpCtrl = new Ieee802Ctrl();
//
//    // set DSAP & NSAP fields
//    tmpCtrl->setDsap(SAP_CLNS);
//    tmpCtrl->setSsap(SAP_CLNS);
//
//    //set destination broadcast address
//    //It should be multicast 01-80-C2-00-00-14 MAC address, but it doesn't work in OMNeT
//    MACAddress ma;
//    ma.setAddress("ff:ff:ff:ff:ff:ff");
//    tmpCtrl->setDest(ma);
//
//    tmpLSP->setControlInfo(tmpCtrl);

    send(tmpLSP, "lowerLayerOut", gateIndex);
    EV<< "ISIS::sendLSP: Source-ID: ";
    for (unsigned int i = 0; i < 6; i++)
        {
            EV << setfill('0') << setw(2) << dec << (unsigned int) this->sysId[i];
            if (i % 2 == 1)
                EV << ".";
        }

    EV<< endl;

}

/**
 * Create or update my own LSP.
 * @param circuiType specify level e.g. L1 or L2
 * @return vector of generated LSPs
 */
std::vector<ISISLSPPacket *>* ISIS::genLSP(short circuitType)
{
    unsigned char *myLSPID = this->getLSPID();
    ISISLSPPacket* LSP; // = new ISISLSPL1Packet;

//    if ((LSP = this->getLSPFromDbByID(myLSPID, circuitType)) != NULL)
    //we have at least "System-ID.00-00" LSP

    //generate LSP and then compare it with one found in database
    //if they differ get sequence number from the one found in database, increment it by one
    //and replace the original LSP entry -> if only one LSP is present in DB then it's easy,
    //but what to do when there are multiple fragments

    //how to compare two LSP and recognize

    //generate LSP only up to ISIS_LSP_MAX_SIZE
    /* after reaching ISIS_LSP_MAX_SIZE (minus few %) (and comparing the two LSPs and such)
     * 1.if there's more -> create new LSP
     * 2.if not, check if there's another fragment in DB (it should be enough to check only next fragment)
     *
     */

    //create new LSP
    std::vector<TLV_t *>* tlvTable = new std::vector<TLV_t *>;

    //if sizeOfNeighboursUp == 0 then return NULL
    if (!this->isAdjUp(circuitType))
    {
        //FIXME
        return new std::vector<ISISLSPPacket *>;
    }

    //TLV AREA ADDRESS
    this->addTLV(tlvTable, AREA_ADDRESS, circuitType);

    //TLV IS_NEIGHBOURS
    this->addTLV(tlvTable, IS_NEIGHBOURS_LSP, circuitType, NULL);

    //add other TLVs

    //now start putting informations from tlvTable into LSP Packets

    /*
     * while(! tlvTable->empty())
     *   get first tlv from tlvTable
     *   if tlv_entry.length > max_lsp_size (minus header)
     *     we won't fit it into single lsp
     *
     *   else if tlv_entry.length > available_space in current LSP
     *     allocate new lsp
     *   else
     *     start putting it in current LSP
     *     remove it from vector
     *     DONT FORGET to PUT EVERY GENERATED LSP INTO returning vector
     *     start new iteration
     */

    TLV_t * tmpTlv;
//    unsigned int tlvSize; //tmp variable
    unsigned int availableSpace = ISIS_LSP_MAX_SIZE; //available space in LSP

    //deleted at the end of generateLSP
    std::vector<ISISLSPPacket *>* tmpLSPDb = new std::vector<ISISLSPPacket *>;
    for (unsigned char fragment = 0; !tlvTable->empty(); fragment++)
    {
        availableSpace = ISIS_LSP_MAX_SIZE;
        if (circuitType == L1_TYPE)
        {
            LSP = new ISISLSPPacket("L1 LSP");
            LSP->setType(L1_LSP);
            //set PATTLSPDBOLIS
            LSP->setPATTLSPDBOLIS(0x02 + (this->att * 0x10)); //only setting IS type = L1 (TODO A2)
        }
        else if (circuitType == L2_TYPE)
        {
            LSP = new ISISLSPPacket("L2 LSP");
            LSP->setType(L2_LSP);
            //set PATTLSPDBOLIS
            LSP->setPATTLSPDBOLIS(0x0A); //bits 1(2) and 3(8) => L2
        }
        else
        {
            EV<< "ISIS: ERROR: Wrong circuitType in genLSP()" << endl;
        }

        //set pduLength

        //set remLifeTime
        LSP->setRemLifeTime(this->lspMaxLifetime);

        //set lspID[8];
        myLSPID[ISIS_SYSTEM_ID + 1] = fragment;
        this->setLspID(LSP, myLSPID);

        //set seqNum
        LSP->setSeqNumber(1);

        //set checksum

        //set TLV

        //TLV_t tmpTLV;

        for (; !tlvTable->empty();)
        {
            tmpTlv = tlvTable->at(0);

            //TODO incorporate header size and mostly get actual MTU something like this->ISISIft.at(0).entry->getMTU()
            if (tmpTlv->length > ISIS_LSP_MAX_SIZE)
            {
                //tlv won't fit into single message (this shouldn't happen)
                EV << "ISIS: Warning: TLV won't fit into single message (this shouldn't happen)" << endl;
                tlvTable->erase(tlvTable->begin());
            }
            else if (tmpTlv->length > availableSpace)
            {
                //tlv won't fit into this LSP, so break cycle and create new LSP
//                tmpLSPDb->push_back(LSP);
                EV << "ISIS: Info: This TLV is full." << endl;
                break;//ends inner cycle

            }
            else
            {
                this->addTLV(LSP, tmpTlv);
                //tlvSize = LSP->getTLVArraySize();
                //LSP->setTLVArraySize(tlvSize + 1);

                //update availableSpace
                availableSpace = availableSpace - (2 + tmpTlv->length);// "2" means Type and Length fields

                //clean up
                delete[] tmpTlv->value;
                delete tmpTlv;
                tlvTable->erase(tlvTable->begin());

            }
        }
        //this->printLSP(LSP, "genLSP, non-pseudo");
        tmpLSPDb->push_back(LSP);

    }
            //what about pseudonode?
            /*
             * H   H  EEEEE  RRRR   EEEEE
             * H   H  E      R   R  E
             * HHHHH  EEEEE  RRRR   EEEEE
             * H   H  E      R  R   E
             * H   H  EEEEE  R   R  EEEEE
             */

    std::vector<bool> activeIface; //interfaces with adjacency in state UP

    for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
    {
        activeIface.push_back(this->isUp((*it).gateIndex, circuitType));

    }

    for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
    {
        //if at least one adjacency is UP and network type is broadcast (there is no DIS on PTP) AND I am DIS on this interface
        if (this->isUp((*it).gateIndex, circuitType) && (*it).network
                && this->amIDIS(this->getIfaceIndex(&(*it)), circuitType))
        {
            tlvTable->clear();
            //tmpLSPDb->clear(); //Why? Why? Why, you fucked-up crazy-ass weirdo beaver?!?
            availableSpace = ISIS_LSP_MAX_SIZE;
//            unsigned char nsel;

            if (circuitType == L1_TYPE)
            {
                myLSPID[ISIS_SYSTEM_ID] = (*it).L1DIS[ISIS_SYSTEM_ID];
//                nsel = (*it).L1DIS[ISIS_SYSTEM_ID];
            }
            else if (circuitType == L2_TYPE)
            {
                myLSPID[ISIS_SYSTEM_ID] = (*it).L2DIS[ISIS_SYSTEM_ID];
//                nsel = (*it).L2DIS[ISIS_SYSTEM_ID];
            }
            else
            {
                EV<< "ISIS: ERROR: Wrong circuitType in genLSP()" << endl;
            }

                //TLV IS_NEIGHBOURS
            this->addTLV(tlvTable, IS_NEIGHBOURS_LSP, circuitType, (*it).entry);

            //TLV AREA_ADDRESS
            this->addTLV(tlvTable, AREA_ADDRESS, circuitType);

            for (unsigned char fragment = 0; !tlvTable->empty(); fragment++)
            {
                availableSpace = ISIS_LSP_MAX_SIZE;
                if (circuitType == L1_TYPE)
                {
                    LSP = new ISISLSPPacket("L1 LSP"); //TODO based on circuitType
                    LSP->setType(L1_LSP);
                    //set PATTLSPDBOLIS
                    LSP->setPATTLSPDBOLIS(0x02 + (this->att * 0x10)); //setting L1 + Attached flag (bit 4) if set
                }
                else if (circuitType == L2_TYPE)
                {
                    LSP = new ISISLSPPacket("L2 LSP"); //TODO based on circuitType
                    LSP->setType(L2_LSP);
                    //set PATTLSPDBOLIS
                    LSP->setPATTLSPDBOLIS(0x0A); //bits 1(2) and 3(8) => L2
                }
                else
                {
                    EV<< "ISIS: ERROR: Wrong circuitType in genLSP()" << endl;
                }

                //set pduLength

                //set remLifeTime
                LSP->setRemLifeTime(this->lspMaxLifetime);

                //set lspID[8];
                myLSPID[ISIS_SYSTEM_ID + 1] = fragment;
                this->setLspID(LSP, myLSPID);

                //set seqNum
                LSP->setSeqNumber(1);

                //set checksum

                //set TLV
                //TLV_t tmpTLV;

                for (; !tlvTable->empty();)
                {
                    tmpTlv = tlvTable->at(0);

                    //TODO incorporate header size and mostly get actual MTU something like this->ISISIft.at(0).entry->getMTU()
                    if (tmpTlv->length > ISIS_LSP_MAX_SIZE)
                    {
                        //tlv won't fit into single message (this shouldn't happen)
                        EV << "ISIS: Warning: TLV won't fit into single message (this shouldn't happen)" << endl;
                        tlvTable->erase(tlvTable->begin());
                    }
                    else if (tmpTlv->length > availableSpace)
                    {
                        //tlv won't fit into this LSP, so break cycle and create new LSP
                        //                tmpLSPDb->push_back(LSP);
                        EV << "ISIS: This TLV is full." << endl;
                        break;//ends inner cycle

                    }
                    else
                    {
                        this->addTLV(LSP, tmpTlv);
                        //tlvSize = LSP->getTLVArraySize();
                        //LSP->setTLVArraySize(tlvSize + 1);

                        //update availableSpace
                        //2Bytes are Type and Length fields
                        availableSpace = availableSpace - (2 + tmpTlv->length);

                        //clean up
                        delete[] tmpTlv->value;
                        delete tmpTlv;
                        tlvTable->erase(tlvTable->begin());

                    }
                }
                //this->printLSP(LSP, "genLSP, pseudo");
                tmpLSPDb->push_back(LSP);

            }

        }
    }

    delete[] myLSPID;
    return tmpLSPDb;
}

/*
 * Calls refresh for appropriate level specified by timer.
 * @timer incomming timer
 */
void ISIS::refreshLSP(ISISTimer *timer)
{
    //this->printLSPDB();

    if (this->isType == L1_TYPE || this->isType == L1L2_TYPE)
    {
        this->refreshLSP(L1_TYPE);
    }
    if (this->isType == L2_TYPE || this->isType == L1L2_TYPE)
    {
        this->refreshLSP(L2_TYPE);
    }
    //this->printLSPDB();

    this->schedule(timer);

}

/*
 * Increment sequence number for LSPs that belongs to this IS and set new remaining lifetime.
 * But only if their remaining lifetime is not already zero, or deadTimer set to LSP_DELETE_TIMER.
 * @param circuitType is level
 */
void ISIS::refreshLSP(short circuitType)
{
    std::vector<LSPRecord*>* lspDb = this->getLSPDb(circuitType);
    std::vector<LSPRecord*>::iterator it = lspDb->begin();
    unsigned char *lspId;
    for (; it != lspDb->end(); ++it)
    {
        lspId = this->getLspID((*it)->LSP);
        if (this->compareArrays(lspId, (unsigned char *) this->sysId, ISIS_SYSTEM_ID)
                && (*it)->deadTimer->getTimerKind() != LSP_DELETE_TIMER && (*it)->LSP->getRemLifeTime() != 0)
        {
            //this->printLSP((*it)->LSP, "print from refreshLSP");
            //std::cout<<"RefreshLSP: seqNum: " <<(*it)->LSP->getSeqNumber() << endl;
            (*it)->LSP->setSeqNumber((*it)->LSP->getSeqNumber() + 1);
            //std::cout<<"RefreshLSP: seqNum: " <<(*it)->LSP->getSeqNumber() << endl;
            // this->printLSP((*it)->LSP, "print from refreshLSP");
            (*it)->LSP->setRemLifeTime(this->lspMaxLifetime);
            //cancel original deadTimer
            cancelEvent((*it)->deadTimer);

            this->setSRMflags((*it), circuitType);
            //TODO what about SSNflags?
            //this->clearSSNflags((*it), circuitType);

            (*it)->simLifetime = simTime().dbl() + (*it)->LSP->getRemLifeTime();

            //re-schedule dead timer
            this->schedule((*it)->deadTimer, (*it)->LSP->getRemLifeTime());
        }
        delete lspId;
    }
}

/*
 * Call generateLSP for appropriate level.
 * @param timer is incomming timer.
 */
void ISIS::generateLSP(ISISTimer *timer)
{
    cancelEvent(timer);
    if (this->isType == L1_TYPE || this->isType == L1L2_TYPE)
    {
        this->generateLSP(L1_TYPE);
    }
    if (this->isType == L2_TYPE || this->isType == L1L2_TYPE)
    {
        this->generateLSP(L2_TYPE);
    }


    this->schedule(timer);

}

/*
 * Generate and if necessary replace existing LSPs.
 * @param circuitType is level.
 */
void ISIS::generateLSP(short circuitType)
{
    /*
     * I am not 100% sure that this method correctly handle situation in which:
     * There are DIS's LSPs in DB, but when the DIS resign, and we don't generate any DIS's LSPs
     * we don't even know that we should purge something.
     * In worst case scenario these LSPs age out.
     */

    unsigned char *lspId;
    std::vector<LSPRecord *> *lspDb = this->getLSPDb(circuitType);
    //don't forget to delete it
    std::vector<ISISLSPPacket *>* tmpLSPDb;

    tmpLSPDb = this->genLSP(circuitType);

    if (lspDb->empty())
    {
        //our database is empty, so put everything from tmpLSPDb into L1LSPDb
        //LSPRecord* tmpLSPRecord;
        for (std::vector<ISISLSPPacket *>::iterator it = tmpLSPDb->begin(); it != tmpLSPDb->end(); ++it)
        {
            this->installLSP((*it), circuitType);

        }
        tmpLSPDb->clear();
    }

    else if (tmpLSPDb->empty())
    {
        //nothing generated so purge all existing LSPs
        this->purgeMyLSPs(circuitType);
    }
    else
    {
        //something in between
        /*
         * 1. get System-ID + NSEL from LSP-ID
         * 2. after processing, check if the next generated (if any exist) has the same LAN-ID
         *      if not -> purge all remaining LSP fragments from database with same LAN-ID
         *      if they have same LAN-ID OR there is not another LSP fragment, continue in processing
         */

        //lspId = this->getLspID(tmpLSPDb->at(0));
        //we need to compare each LSP
        //because of the way we create LSP, from the first one that doesn't match we can discard/replace all successive
        LSPRecord * tmpLSPRecord; // = this->getLSPFromDbByID(this->getLspID(tmpLSPDb->at(0)), circuitType);
        for (; !tmpLSPDb->empty();)
        {
            lspId = this->getLspID(tmpLSPDb->front());
            if ((tmpLSPRecord = this->getLSPFromDbByID(this->getLspID(tmpLSPDb->at(0)), circuitType)) != NULL)
            {
                if (this->compareLSP(tmpLSPRecord->LSP, tmpLSPDb->at(0)))
                {
                    //LSP match we don't need to do anything
                    delete tmpLSPDb->at(0);
                    tmpLSPDb->erase(tmpLSPDb->begin());
                    tmpLSPRecord = NULL;
                    //continue;

                }
                else
                {
                    //install tmpLSPDb->at(0) into this->L1LSPDb
                    this->replaceLSP(tmpLSPDb->at(0), tmpLSPRecord, circuitType);
                    //set SRMflag on all circuits (that have at least one adjacency UP)
                    //done in replaceLSP

                    //erase
                    tmpLSPDb->erase(tmpLSPDb->begin());

                    //set tmpLSPRecord to NULL
                    tmpLSPRecord = NULL;
                    //continue;
                }
            }
            else
            {
                //install tmpLSPDb->at(0)
                this->installLSP(*tmpLSPDb->begin(), circuitType);

                //erase
                tmpLSPDb->erase(tmpLSPDb->begin());
                //break;
            }
            /* if there is any generated LSP in tmpLSPDb, and the current LSP-ID
             *  and next LSP's LSP-ID doesn't match (without fragment-ID) => we don't have any LSPs from this LAN-ID,
             *  so purge all remaining LSPs with matching LAN-ID (System-ID + NSEL)
             */
            if (!tmpLSPDb->empty()
                    && !this->compareArrays(lspId, this->getLspID((*tmpLSPDb->begin())), ISIS_SYSTEM_ID + 1))
            {
                //purge all remaining fragments with lspId (without fragment-ID)
                lspId[ISIS_SYSTEM_ID + 1] = lspId[ISIS_SYSTEM_ID + 1] + 1;
                this->purgeRemainLSP(lspId, circuitType);
            }
            //tmpLSPRecord = this->getLSPFromDbByID(this->getLspID(tmpLSPDb->at(0)), circuitType);

            delete[] lspId;
        }

    }

    delete tmpLSPDb;
}

/*
 * Purge successive LSPs for lspId. (lspId itself is not purged)
 * @param lspId is first LSP that should be purged.
 * @param circuitType is level
 */
void ISIS::purgeRemainLSP(unsigned char *lspId, short circuitType)
{

    //lspId[ISIS_SYSTEM_ID + 1] = lspId[ISIS_SYSTEM_ID + 1] + 1;

    while (this->getLSPFromDbByID(lspId, circuitType) != NULL)
    {
        this->purgeLSP(lspId, circuitType);
        //this should increment fragment-ID for the next round check
        lspId[ISIS_SYSTEM_ID + 1] = lspId[ISIS_SYSTEM_ID + 1] + 1;

    }
}

/*
 * Initiate purge of ALL LSPs generated by this IS on specified level.
 * @param circuitType is level.
 */
void ISIS::purgeMyLSPs(short circuitType)
{

    std::vector<LSPRecord *>* lspDb;

    lspDb = this->getLSPDb(circuitType);
    unsigned char *lspId;
    for (std::vector<LSPRecord*>::iterator it = lspDb->begin(); it != lspDb->end(); ++it)
    {
        lspId = this->getLspID((*it)->LSP);
        if (this->compareArrays(lspId, (unsigned char *) this->sysId, ISIS_SYSTEM_ID))
        {
            this->purgeLSP(lspId, circuitType);
        }
        delete lspId;
    }
}

/*
 * This method is used for purging in-memory LSP's when the remaining Lifetime becomes zero.
 */
void ISIS::purgeLSP(unsigned char *lspId, short circuitType)
{

    LSPRecord * lspRec;
    if ((lspRec = this->getLSPFromDbByID(lspId, circuitType)) != NULL)
    {
        //std::cout<<"purgeLSP seqNum: " << lspRec->LSP->getSeqNumber() <<endl;
        //lspRec->LSP->setSeqNumber(0);
        //TODO do we need to increment seqNum or equal seqNum with remLife == 0 will replace that LSP?
        //lspRec->LSP->setSeqNumber(lspRec->LSP->getSeqNumber() + 1);
        //std::cout<<"purgeLSP seqNum: " << lspRec->LSP->getSeqNumber() <<endl;
        /* 7.3.16.4.  */
        lspRec->LSP->setRemLifeTime(0);
        //delete all TLV from lspRec-LSP and set TLVArraySize = 0
        //TODO check if area address TLV is necessary
        //add only Area Address
        /*        for(unsigned int i = 0; i < lspRec->LSP->getTLVArraySize(); i++){
         if(lspRec->LSP->getTLV(i).value != NULL){
         delete lspRec->LSP->getTLV(i).value;
         }
         }*/

        lspRec->LSP->setTLVArraySize(0);
        //if TLV Area Addresses should be present even in purge LSP than add it
        //this->addTLV(lspRec->LSP, AREA_ADDRESS);

        this->setSRMflags(lspRec, circuitType);
        //TODO What about SSN?? ->clearSSNflags?

        //simLifetime now express the time when the LSP has been purged so after "N" seconds it can be removed completely
        lspRec->simLifetime = simTime().dbl();

        //remove the deadTimer from future events
        cancelEvent(lspRec->deadTimer);
        //change timer type
        lspRec->deadTimer->setTimerKind(LSP_DELETE_TIMER);
        //and reschedule it again
        this->schedule(lspRec->deadTimer);
    }

}

/*
 * Purge of an received LSP with zero Remaining Lifetime according to 7.3.16.4
 * Received lsp is deleted at the end. Only exception is when replacing current LSP then
 * early return is executed to avoid deleting received lsp.
 *
 */
void ISIS::purgeLSP(ISISLSPPacket *lsp, short circuitType)
{
    LSPRecord *lspRec;
    int gateIndex = lsp->getArrivalGate()->getIndex();
    ISISinterface* iface = this->getIfaceByGateIndex(gateIndex);
    int interfaceIndex = this->getIfaceIndex(iface);
    //std::vector<LSPRecord *> * lspDb = this->getLSPDb(circuitType);
    unsigned char * lspId = this->getLspID(lsp);

    //if lsp is not in memory
    if ((lspRec = this->getLSPFromDbByID(lspId, circuitType)) == NULL)
    {

        //send an ack of the LSP on circuit C
        if (!this->getIfaceByGateIndex(gateIndex)->network)
        {
            /* TODO uncomment warning when sending ack for lsp NOT in db is implemented */
            EV<< "ISIS: Warning: Should send ACK for purged LSP over non-broadcast circuit. (Not yet implemented)."
            << endl;
            //explicit ack are only on non-broadcast circuits
            //this->setSSNflag(lspRec, gateIndex, circuitType);
        }
    }
    else

    { /*7.3.16.4. b) LSP from S is in the DB */
        if (!this->compareArrays(this->getSysID(lsp), (unsigned char *) this->sysId, ISIS_SYSTEM_ID))
        {
            //if received LSP is newer
            //7.3.16.4 b) 1)
            if (lsp->getSeqNumber() > lspRec->LSP->getSeqNumber()
                    || (lsp->getSeqNumber() == lspRec->LSP->getSeqNumber() && lspRec->LSP->getRemLifeTime() != 0))
            {
                //store the new LSP in DB, overwriting existing LSP
                //replaceLSP
                this->replaceLSP(lsp, lspRec, circuitType);
                //lsp is installed in DB => don't delete it, so early return
                delete[] lspId;
                return;
            }
            /* 7.3.16.4. b) 2)*/
            else if (lsp->getSeqNumber() == lspRec->LSP->getSeqNumber() && lsp->getRemLifeTime() == 0
                    && lspRec->LSP->getRemLifeTime() == 0)
            {
                /* clear SRMflag for C and
                 * if C is non-broadcast circuit, set SSNflag for C
                 */
                /* 7.3.16.4. b) 2) i.*/
                this->clearSRMflag(lspRec, interfaceIndex, circuitType);
                if (!this->getIfaceByGateIndex(gateIndex)->network)
                {
                    /* 7.3.16.4. b) 2) ii.*/
                    //explicit ack are only on non-broadcast circuits
                    this->setSSNflag(lspRec, interfaceIndex, circuitType);
                }

            }
            /* 7.3.16.4. b) 3).*/
            else if (lsp->getSeqNumber() < lspRec->LSP->getSeqNumber())
            {
                /* 7.3.16.4. b) 3) i.*/
                this->setSRMflag(lspRec, interfaceIndex, circuitType);
                /* 7.3.16.4. b) 3) ii.*/
                this->clearSSNflag(lspRec, interfaceIndex, circuitType);
            }

        }
        /*7.3.16.4. c) it's our own LSP */
        else
        {
            if (lspRec->LSP->getRemLifeTime() != 0)
            {
                if (lsp->getSeqNumber() >= lspRec->LSP->getSeqNumber())
                {
                    //std::cout<<"purgeLSP seqNum: " << lspRec->LSP->getSeqNumber() <<endl;
                    lspRec->LSP->setSeqNumber(lsp->getSeqNumber() + 1);
                    //std::cout<<"purgeLSP seqNum: " << lspRec->LSP->getSeqNumber() <<endl;
                }
                /*7.3.16.4. c) 4) set SRMflags for all circuits */
                this->setSRMflags(lspRec, circuitType);
            }
        }
    }
    delete lsp;
    delete[] lspId;
}

/*
 * This method permanently removes LSP from DB.
 * @timer is timer.
 */
void ISIS::deleteLSP(ISISTimer *timer)
{
    short circuitType = timer->getIsType();
    unsigned char* lspId = this->getLspID(timer);
    std::vector<LSPRecord *> *lspDb = this->getLSPDb(circuitType);
    unsigned char * tmpLspId;

    for (std::vector<LSPRecord*>::iterator it = lspDb->begin(); it != lspDb->end(); ++it)
    {
        tmpLspId = this->getLspID((*it)->LSP);
        if (this->compareArrays(tmpLspId, lspId, ISIS_SYSTEM_ID + 2))
        {
            //clearing *flags shoudn't be necessary, but just to be sure
            this->clearSRMflags((*it), timer->getIsType());
            this->clearSSNflags((*it), timer->getIsType());
            delete (*it);
            lspDb->erase(it);
            delete lspId;
            delete tmpLspId;
            return;
        }
        delete tmpLspId;
    }

    delete lspId;
}

/*
 * Returns pointer to LSP database specified by level
 * @param circuitType is level.
 * @return pointer to LSP database.
 */
std::vector<LSPRecord *> * ISIS::getLSPDb(short circuitType)
{

    if (circuitType == L1_TYPE)
    {
        return this->L1LSPDb;
    }
    else if (circuitType == L2_TYPE)
    {
        return this->L2LSPDb;
    }

    return NULL;
}

/*
 * Return pointer to adjacency table specified by level.
 * @param circuitType is level
 * @return pointer to adjacency table.
 */
std::vector<ISISadj> * ISIS::getAdjTab(short circuitType)
{

    if (circuitType == L1_TYPE)
    {
        return &(this->adjL1Table);
    }
    else if (circuitType == L2_TYPE)
    {
        return &(this->adjL2Table);
    }

    return NULL;
}

/*
 * Return pointer to ISO paths specified by level.
 * @param circuitType is level
 * @return pointer to ISO Paths.
 */
ISISPaths_t* ISIS::getPathsISO(short circuitType)
{

    if (circuitType == L1_TYPE)
    {
        return this->L1ISISPathsISO;
    }
    else if (circuitType == L2_TYPE)
    {
        return this->L2ISISPathsISO;
    }

    return NULL;
}

/*
 * Returns appropriate SRM Queues for point-to-point link.
 * @param circuitType is level.
 * @return vector (of vector) of SRM flags.
 */
FlagRecQQ_t * ISIS::getSRMPTPQueue(short circuitType)
{

    if (circuitType == L1_TYPE)
    {
        return (this->L1SRMPTPQueue);
    }
    else if (circuitType == L2_TYPE)
    {
        return (this->L2SRMPTPQueue);
    }

    return NULL;
}

FlagRecQQ_t * ISIS::getSRMBQueue(short circuitType)
{

    if (circuitType == L1_TYPE)
    {
        return (this->L1SRMBQueue);
    }
    else if (circuitType == L2_TYPE)
    {
        return (this->L2SRMBQueue);
    }

    return NULL;
}

FlagRecQQ_t * ISIS::getSSNPTPQueue(short circuitType)
{

    if (circuitType == L1_TYPE)
    {
        return (this->L1SSNPTPQueue);
    }
    else if (circuitType == L2_TYPE)
    {
        return (this->L2SSNPTPQueue);
    }

    return NULL;
}

FlagRecQQ_t * ISIS::getSSNBQueue(short circuitType)
{

    if (circuitType == L1_TYPE)
    {
        return (this->L1SSNBQueue);
    }
    else if (circuitType == L2_TYPE)
    {
        return (this->L2SSNBQueue);
    }

    return NULL;
}

/*
 * @param circuitType is probably redundant and will be removed
 */

void ISIS::setSRMflag(LSPRecord * lspRec, int interfaceIndex, short circuitType)
{
    ISISinterface * iface = &(this->ISISIft.at(interfaceIndex));
    std::vector<FlagRecord*> *SRMQueue = this->getSRMQ(iface->network, interfaceIndex, circuitType);
    if (lspRec->SRMflags.size() == 0)
    {
        this->addFlags(lspRec, circuitType);
    }
    /* This should prevent from adding multiple FlagRecord for the same LSP-gateIndex pair */
    if (lspRec->SRMflags.at(interfaceIndex) == true)
    {
        return;
    }

    //set SRMflag on interface only if there is  at least one adjacency UP
    if (this->isUp(iface->gateIndex, circuitType))
    {
        lspRec->SRMflags.at(interfaceIndex) = true; //setting true (Send Routing Message) on every interface
        FlagRecord *tmpSRMRec = new FlagRecord;
        tmpSRMRec->lspRec = lspRec;
        tmpSRMRec->index = interfaceIndex;
        SRMQueue->push_back(tmpSRMRec);
        /*
         if (this->ISISIft.at(index).network)
         {
         //TODO how do you know it's L1 and not L2
         this->L1SRMBQueue->at(index)->push_back(tmpSRMRec);
         }
         else
         {
         this->L1SRMPTPQueue->at(index)->push_back(tmpSRMRec);
         }*/
    }

}

void ISIS::setSRMflags(LSPRecord *lspRec, short circuitType)
{
    if (lspRec->SRMflags.size() == 0)
    {
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SRMflags.size(); i++)
    {
        this->setSRMflag(lspRec, i, circuitType);
    }

}

void ISIS::setSRMflagsBut(LSPRecord *lspRec, unsigned int interfaceIndex, short circuitType)
{
    if (lspRec->SRMflags.size() == 0)
    {
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SRMflags.size(); i++)
    {
        if (i == interfaceIndex)
        {
            this->clearSRMflag(lspRec, i, circuitType);
        }
        else
        {
            this->setSRMflag(lspRec, i, circuitType);
        }
    }

}

void ISIS::clearSRMflag(LSPRecord *lspRec, int interfaceIndex, short circuitType)
{

    std::vector<FlagRecord*>* srmQ;
    srmQ = this->getSRMQ(this->ISISIft.at(interfaceIndex).network, interfaceIndex, circuitType);

    //clear flag
    lspRec->SRMflags.at(interfaceIndex) = false;

    //and remove FlagRecord from Queue
    std::vector<FlagRecord*>::iterator it = srmQ->begin();
    for (; it != srmQ->end();)
    {
        if ((*it)->index == interfaceIndex && (*it)->lspRec == lspRec)
        {
            it = srmQ->erase(it);
            //break;
        }
        else
        {
            ++it;
        }
    }
}

void ISIS::clearSRMflags(LSPRecord *lspRec, short circuitType)
{

    if (lspRec->SRMflags.size() == 0)
    {
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SRMflags.size(); i++)
    {
        this->clearSRMflag(lspRec, i, circuitType);
    }

}

void ISIS::clearSRMflagsBut(LSPRecord *lspRec, unsigned int interfaceIndex, short circuitType)
{
    if (lspRec->SRMflags.size() == 0)
    {
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SRMflags.size(); i++)
    {
        if (i == interfaceIndex)
        {
            this->setSRMflag(lspRec, i, circuitType);
        }
        else
        {
            this->clearSRMflag(lspRec, i, circuitType);
        }
    }

}

std::vector<FlagRecord*>* ISIS::getSRMQ(bool network, int interfaceIndex, short circuitType)
{
    if (circuitType == L1_TYPE)
    {
        if (network)
        {
            return this->L1SRMBQueue->at(interfaceIndex);
        }
        else
        {
            return this->L1SRMPTPQueue->at(interfaceIndex);
        }

    }
    else if (circuitType == L2_TYPE)
    {
        if (network)
        {
            return this->L2SRMBQueue->at(interfaceIndex);
        }
        else
        {
            return this->L2SRMPTPQueue->at(interfaceIndex);
        }
    }
    return NULL;
}

void ISIS::setSSNflag(LSPRecord * lspRec, int interfaceIndex, short circuitType)
{

    ISISinterface * iface = &(this->ISISIft.at(interfaceIndex));
    std::vector<FlagRecord*> *SSNQueue = this->getSSNQ(iface->network, interfaceIndex, circuitType);

    if (lspRec->SSNflags.size() == 0)
    {
        this->addFlags(lspRec, circuitType);
    }

    /* This should prevent from adding multiple FlagRecord for the same LSP-gateIndex pair */
    if (lspRec->SSNflags.at(interfaceIndex) == true)
    {
        return;
    }
    //set SSNflag on interface only if there is  at least one adjacency UP
    if (this->isUp(iface->gateIndex, circuitType))
    {

        lspRec->SSNflags.at(interfaceIndex) = true; //setting true (Send Routing Message) on every interface
        FlagRecord *tmpSSNRec = new FlagRecord;
        tmpSSNRec->lspRec = lspRec;
        tmpSSNRec->index = interfaceIndex;
        SSNQueue->push_back(tmpSSNRec);
//        if (this->ISISIft.at(index).network)
//        {
//            this->L1SSNBQueue->at(index)->push_back(tmpSSNRec);
//        }
//        else
//        {
//            this->L1SSNPTPQueue->at(index)->push_back(tmpSSNRec);
//        }
    }

}

void ISIS::setSSNflags(LSPRecord *lspRec, short circuitType)
{
    if (lspRec->SSNflags.size() == 0)
    {
        this->addFlags(lspRec, circuitType);
    }

    for (unsigned int i = 0; i < lspRec->SSNflags.size(); i++)
    {
        this->setSSNflag(lspRec, i, circuitType);
    }

}

void ISIS::setSSNflagsBut(LSPRecord *lspRec, unsigned int interfaceIndex, short circuitType)
{
    if (lspRec->SSNflags.size() == 0)
    {
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SSNflags.size(); i++)
    {
        if (i == interfaceIndex)
        {
            this->clearSSNflag(lspRec, i, circuitType);
        }
        else
        {
            this->setSSNflag(lspRec, i, circuitType);
        }
    }

}

void ISIS::clearSSNflag(LSPRecord *lspRec, int interfaceIndex, short circuitType)
{

    std::vector<FlagRecord*>* ssnQ;
    ssnQ = this->getSSNQ(this->ISISIft.at(interfaceIndex).network, interfaceIndex, circuitType);

    //clear flag
    lspRec->SSNflags.at(interfaceIndex) = false;

    //and remove FlagRecord from Queue
    std::vector<FlagRecord*>::iterator it = ssnQ->begin();
    for (; it != ssnQ->end();)
    {
        if ((*it)->index == interfaceIndex && (*it)->lspRec == lspRec)
        {
            it = ssnQ->erase(it);
            //break;
        }
        else
        {
            ++it;
        }
    }
}

void ISIS::clearSSNflags(LSPRecord *lspRec, short circuitType)
{

    if (lspRec->SSNflags.size() == 0)
    {
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SSNflags.size(); i++)
    {
        this->clearSSNflag(lspRec, i, circuitType);
    }

}

void ISIS::clearSSNflagsBut(LSPRecord *lspRec, unsigned int interfaceIndex, short circuitType)
{
    if (lspRec->SSNflags.size() == 0)
    {
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SSNflags.size(); i++)
    {
        if (i == interfaceIndex)
        {
            this->setSSNflag(lspRec, i, circuitType);
        }
        else
        {
            this->clearSSNflag(lspRec, i, circuitType);
        }
    }

}

std::vector<FlagRecord*>* ISIS::getSSNQ(bool network, int interfaceIndex, short circuitType)
{
    if (circuitType == L1_TYPE)
    {
        if (network)
        {
            return this->L1SSNBQueue->at(interfaceIndex);
        }
        else
        {
            return this->L1SSNPTPQueue->at(interfaceIndex);
        }

    }
    else if (circuitType == L2_TYPE)
    {
        if (network)
        {
            return this->L2SSNBQueue->at(interfaceIndex);
        }
        else
        {
            return this->L2SSNPTPQueue->at(interfaceIndex);
        }
    }
    return NULL;
}
/**
 * Replaces existing lsp in LSP record
 * @param lsp is received LSP
 * @param is local lsp Record
 * @param circuitType is circuit type
 */
void ISIS::replaceLSP(ISISLSPPacket *lsp, LSPRecord *lspRecord, short circuitType)
{

    //increase sequence number
    //std::cout<<"replaceLSP seqNum: "<< lspRecord->LSP->getSeqNumber() <<endl;
    /*if(lsp->getSeqNumber() < lspRecord->LSP->getSeqNumber()){
     lsp->setSeqNumber(lspRecord->LSP->getSeqNumber() + 1);
     }else{
     lsp->setSeqNumber(lsp->getSeqNumber() + 1);
     }*/
    lsp->setSeqNumber(lspRecord->LSP->getSeqNumber() + 1);
    //now we can delete the previous LSP

    delete lspRecord->LSP;

    //set new lsp
    lspRecord->LSP = lsp;
    //std::cout<<"replaceLSP seqNum: "<< lspRecord->LSP->getSeqNumber() <<endl;
    //create new timer
    //lspRecord->deadTimer = new ISISTimer("LSP Dead Timer");
    //select appropriate type (this could be performed based on lsp)
    this->cancelEvent(lspRecord->deadTimer);

    lspRecord->deadTimer->setTimerKind(LSP_DEAD_TIMER);

    lspRecord->deadTimer->setIsType(circuitType);
    for (unsigned int i = 0; i < lspRecord->deadTimer->getLSPidArraySize(); i++)
    {
        lspRecord->deadTimer->setLSPid(i, lspRecord->LSP->getLspID(i));
    }

    /* need to check  this for replacing LSP from genLSP */
    if (lsp->getArrivalGate() != NULL)
    {
        //received so don't set SRM flag on that interface
//        int gateIndex = lsp->getArrivalGate()->getIndex();
//        /* 7.3.15.1 e) 1) ii. and iii. */ /* 7.3.16.4 b) 1) ii. and iii. */
//        this->setSRMflagsBut(lspRecord, gateIndex , circuitType);
//        /* 7.3.16.4 b) 1) v.  MODIFIED*/
//        this->clearSSNflags(lspRecord, circuitType);
//        //for non-broadcast /* 7.3.16.4 b) 1) iv. */
//        if(!this->ISISIft.at(gateIndex).network){
//            this->setSSNflag(lspRecord, gateIndex, circuitType);
//        }
        //received so don't set SRM flag on that interface
        int gateIndex = lsp->getArrivalGate()->getIndex();
        ISISinterface* iface = this->getIfaceByGateIndex(gateIndex);
        int interfaceIndex = this->getIfaceIndex(iface);
        this->setSRMflagsBut(lspRecord, interfaceIndex, circuitType);
        //for non-broadcast /* 7.3.16.4 b) 1) iv. */
        if (!this->ISISIft.at(interfaceIndex).network)
        {
            this->setSSNflag(lspRecord, interfaceIndex, circuitType);
        }
        /* 7.3.16.4 b) 1) v. */
        this->clearSSNflagsBut(lspRecord, interfaceIndex, circuitType);

    }
    else
    {
        //generated -> set SRM on all interfaces
        this->setSRMflags(lspRecord, circuitType);
        //TODO what with SSN?
        //this->clearSSNflags(lspRecord, circuitType);
    }

    //set simulation time when the lsp will expire
    lspRecord->simLifetime = simTime().dbl() + lspRecord->LSP->getRemLifeTime();

    this->schedule(lspRecord->deadTimer, lsp->getRemLifeTime());
    //TODO B1 use fireNotification
        this->runSPF(circuitType);

}

void ISIS::addFlags(LSPRecord *lspRec, short circuitType)
{

    //add flags
    if (!lspRec->SRMflags.empty() || !lspRec->SSNflags.empty())
    {
        EV<< "ISIS: Warning: Adding *flags to non-empty vectors." << endl;
        return;

    }
    for (std::vector<ISISinterface>::iterator intIt = this->ISISIft.begin(); intIt != this->ISISIft.end(); ++intIt)
    {

        lspRec->SRMflags.push_back(false);
        lspRec->SSNflags.push_back(false);

    }
    //set SRM
    //this->setSRMflags(lspRec, circuitType);
}

/**
 * Installs @param lsp in local database.
 * @param lsp is received LSP
 * @param circuitType is circuit type
 */
LSPRecord * ISIS::installLSP(ISISLSPPacket *lsp, short circuitType)
{

    LSPRecord * tmpLSPRecord = new LSPRecord;
    std::vector<LSPRecord*> * lspDb = this->getLSPDb(circuitType);

    tmpLSPRecord->LSP = lsp;
    //TODO B1
    /* if lsp is my LSP
     *   then set all flags
     * else
     *   set all BUT the received circuts
     */

    if (lsp->getArrivalGate() != NULL)
    {
        int gateIndex = lsp->getArrivalGate()->getIndex();
        ISISinterface* iface = this->getIfaceByGateIndex(gateIndex);
        int interfaceIndex = this->getIfaceIndex(iface);
        /* 7.3.15.1 e) 1) ii. */
        this->setSRMflagsBut(tmpLSPRecord, interfaceIndex, circuitType);
        /*TODO A2 7.3.15.1 e) 1) iii. -> redundant?? */
        //this->clearSRMflag(tmpLSPRecord, lsp->getArrivalGate()->getIndex(), circuitType);
        /* change from specification */
        /* iv. and v. are "switched */
        /* 7.3.15.1 e) 1) v. */
        this->clearSSNflags(tmpLSPRecord, circuitType);

        if (!this->ISISIft.at(interfaceIndex).network)
        {
            /* 7.3.15.1 e) 1) iv. */
            this->setSSNflag(tmpLSPRecord, interfaceIndex, circuitType);
        }

    }
    else
    {
        /* Never set SRM flag for lsp with seqNum == 0 */
        if (lsp->getSeqNumber() > 0)
        {
            /* installLSP is called from genLSP */
            this->setSRMflags(tmpLSPRecord, circuitType);
        }
        //TODO what about SSNflags?
        //this->clearSSNflags(tmpLSPRecord, circuitType);
    }

    tmpLSPRecord->deadTimer = new ISISTimer("LSP Dead Timer");
    tmpLSPRecord->deadTimer->setTimerKind(LSP_DEAD_TIMER);
    tmpLSPRecord->deadTimer->setIsType(circuitType);
    for (unsigned int i = 0; i < tmpLSPRecord->deadTimer->getLSPidArraySize(); i++)
    {
        tmpLSPRecord->deadTimer->setLSPid(i, tmpLSPRecord->LSP->getLspID(i));
    }

    tmpLSPRecord->simLifetime = simTime().dbl() + tmpLSPRecord->LSP->getRemLifeTime();

    this->schedule(tmpLSPRecord->deadTimer, lsp->getRemLifeTime());

    lspDb->push_back(tmpLSPRecord);

    return lspDb->back();

    //TODO B1 use fireNotification
    this->runSPF(circuitType);
}

/*
 * Returns true if packets match
 */
bool ISIS::compareLSP(ISISLSPPacket *lsp1, ISISLSPPacket *lsp2)
{

    //pduLength
    //so far this field is not used, or at least not properly

    //remLifeTime
    //newly generated LSP will have different remaining time

    //lspID
    //lspIDs have to match, because lsp2 is selected based on matching LSP-ID

    //seqNumber
    //newly generated LSP will have seqNumber set to 1

    //checksum
    //not used

    //PATTLSPDBOLIS
    //so far this field is set only to 0x01 = only IS Type = L1, but check anyway
    if (lsp1->getPATTLSPDBOLIS() != lsp2->getPATTLSPDBOLIS())
    {
        return false;
    }

    if (lsp1->getTLVArraySize() != lsp2->getTLVArraySize())
    {
        return false;
    }

    for (unsigned int i = 0; i < lsp1->getTLVArraySize(); i++)
    {
        if (lsp1->getTLV(i).type != lsp2->getTLV(i).type || lsp1->getTLV(i).length != lsp2->getTLV(i).length
                || !this->compareArrays(lsp1->getTLV(i).value, lsp2->getTLV(i).value, lsp1->getTLV(i).length))
        {
            return false;
        }
    }

    //packets match
    return true;
}

/*
 * Returns LSPRecord from LSP database specified by LSP-ID.
 * @param LSPID is ID of LSP to be returned.
 * @param circuitType is level.
 */
LSPRecord* ISIS::getLSPFromDbByID(unsigned char *LSPID, short circuitType)
{

    std::vector<LSPRecord *> *lspDb;

    if (LSPID == NULL)
    {
        return NULL;
    }

    lspDb = this->getLSPDb(circuitType);
    unsigned char *lspId;
    for (std::vector<LSPRecord*>::iterator it = lspDb->begin(); it != lspDb->end(); ++it)
    {
        lspId = this->getLspID((*it)->LSP);
        if (this->compareArrays(lspId, LSPID, ISIS_SYSTEM_ID + 2))
        {
            delete[] lspId;
            return (*it);
        }
        delete[] lspId;
    }

    return NULL;
}

/*
 * Returns "first" LSP-ID of this system eg. SYSTEM-ID.00-00, because this LSP-I
 */
unsigned char* ISIS::getLSPID()
{

    unsigned char *myLSPID = new unsigned char[8];

    this->copyArrayContent((unsigned char*) this->sysId, myLSPID, ISIS_SYSTEM_ID, 0, 0);
    myLSPID[6] = 0;
    myLSPID[7] = 0;
    return myLSPID;
}


/**
 * Since this method is used for point-to-point links only one adjacency is expected
 * @param gateIndex index to global interface table
 */

ISISadj* ISIS::getAdjByGateIndex(int gateIndex, short circuitType, int offset)
{

    if (circuitType == L1_TYPE || circuitType == L1L2_TYPE) //L1L2_TYPE option is there for L1L2 PTP links, because for such links there should be adjacency in both tables
    {
        for (std::vector<ISISadj>::iterator it = this->adjL1Table.begin(); it != this->adjL1Table.end(); ++it)
        {
            if ((*it).gateIndex == gateIndex)
            {
                if (offset == 0)
                {
                    return &(*it);
                }
                else
                {
                    offset--;
                }

            }
        }
    }
    else if (circuitType == L2_TYPE)
    {
        for (std::vector<ISISadj>::iterator it = this->adjL2Table.begin(); it != this->adjL2Table.end(); ++it)
        {
            if ((*it).gateIndex == gateIndex)
            {
                if (offset == 0)
                {
                    return &(*it);
                }
                else
                {
                    offset--;
                }
            }
        }
    }
    return NULL;

}

/**
 * Returns interface for provided gate index.
 * @param gateIndex index to global interface table
 */

ISISinterface* ISIS::getIfaceByGateIndex(int gateIndex)
{
    /*
     * gateIndex != interfaceIndex therefore we cannot use just return &(this->ISISIft.at(gateIndex));
     */

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
unsigned short ISIS::getHoldTime(int interfaceIndex, short circuitType)
{

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
        EV<< "deviceId " << deviceId << ": Warning: Unrecognized circuitType used in ISIS::getHoldTime()\n";
    }
    return 1; //this is here just so OMNeT wouldn't bother me with "no return in non-void method" warning
}

/*
 * Returns Hello interval for interface and level.
 * If this IS is DIS on specified interface, returns only one third of the value.
 * @param interfaceIndex is index to ISISIft
 * @param circuitType is level
 * @return number of seconds.
 */
double ISIS::getHelloInterval(int interfaceIndex, short circuitType)
{

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
        EV<< "deviceId "
        << deviceId
        << ": Warning: Are you sure you want to know Hello interval for L1L2_TYPE ?!? in ISIS::getHelloInterval()\n";
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
/**
 * Return true if this system is DIS for interface specified by @param interfaceIndex for specified level.
 * @param entry Pointer to interface record in interfaceTable
 * @param intElement XML element of current interface in XML config file
 */
bool ISIS::amIDIS(int interfaceIndex, short circuitType)
{

    if (circuitType == L1_TYPE)
    {
        return this->amIL1DIS(interfaceIndex);
    }
    else if (circuitType == L2_TYPE)
    {
        return this->amIL2DIS(interfaceIndex);
    }
    else
    {
        return this->amIL1DIS(interfaceIndex) || this->amIL2DIS(interfaceIndex);;
        EV<< "deviceId " << deviceId << ": ISIS: WARNING: amIDIS for unknown circuitType " << endl;
    }
    return false;
}
        /*
         * Determines if this IS is DIS on specified interface for L1
         * @param interfaceIndex is index to ISISIft
         * return true if this IS is DIS on specified interface for L1.
         */
bool ISIS::amIL1DIS(int interfaceIndex)
{
    //if this is not broadcast interface then no DIS is elected
    if (!this->ISISIft.at(interfaceIndex).network)
    {
        return false;
    }

    return (compareArrays((unsigned char *) this->sysId, this->ISISIft.at(interfaceIndex).L1DIS, ISIS_SYSTEM_ID));

}

/*
 * Determines if this IS is DIS on specified interface for L2
 * @param interfaceIndex is index to ISISIft
 * return true if this IS is DIS on specified interface for L2.
 */
bool ISIS::amIL2DIS(int interfaceIndex)
{
    //if this is not broadcast interface then no DIS is elected
    if (!this->ISISIft.at(interfaceIndex).network)
    {
        return false;
    }

    return (compareArrays((unsigned char *) this->sysId, this->ISISIft.at(interfaceIndex).L2DIS, ISIS_SYSTEM_ID));

}

/*
 * Returns pointer to TLV array in ISISMessage specified by tlvType.
 * @param inMsg message to be parsed
 * @param tlvType is type of TLV
 * @param offset is n-th found TLV
 * @return pointer to TLV array.
 */
TLV_t* ISIS::getTLVByType(ISISMessage *inMsg, enum TLVtypes tlvType, int offset)
{

    for (unsigned int i = 0; i < inMsg->getTLVArraySize(); i++)
    {
        if (inMsg->getTLV(i).type == tlvType)
        {
            if (offset == 0)
            {
                return &(inMsg->getTLV(i));
            }
            else
            {
                offset--;
            }
        }
    }

    return NULL;
}

/*
 * Returns pointer to TLV_t::value within TLV where starts specified subTLV
 * @param TLV_t* TLV to be parsed
 * @param subTLVType is type of subTLV to find
 * @param offset is n-th found sub-TLV
 * @return pointer to TLV::value.
 */
unsigned char* ISIS::getSubTLVByType(TLV_t *tlv, enum TLVtypes subTLVType, int offset)
{
    /* Implementation assumes only correctly formated TLVs eg
     * if minimal length of such TLV is 2B (for example TLV_MT_PORT_CAP)
     * and specified lenght is more than 2B then there has to be sub-TLV that is at least
     * 2B long (type + length, value can be zero). If such sub-TLV would have only 1B eg
     * malformed, memory corruption could occur.
     */

    int skip = 0;
    if (tlv->type == TLV_MT_PORT_CAP)
    {
        skip = 2; //TLV_MT_PORT_CAP is itself 2B long

        for (skip = 2; tlv->length > skip;)
        {
            if (tlv->value[skip] != subTLVType)
            {
                skip += 2 + tlv->value[skip + 1];

            }
            else
            {
                return &(tlv->value[skip]);
            }
        }
    }

//    for (unsigned int i = 0; i < inMsg->getTLVArraySize(); i++)
//    {
//        if (inMsg->getTLV(i).type == subTLVType)
//        {
//            if (offset == 0)
//            {
//                return &(inMsg->getTLV(i));
//            }
//            else
//            {
//                offset--;
//            }
//        }
//    }

    return NULL;
}

/*


 TLV_t* ISIS::getTLVByType(ISISL1HelloPacket *msg, enum TLVtypes tlvType, int offset)
 {

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

 TLV_t* ISIS::getTLVByType(ISISL2HelloPacket *msg, enum TLVtypes tlvType, int offset)
 {

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

 TLV_t* ISIS::getTLVByType(ISISPTPHelloPacket *msg, enum TLVtypes tlvType, int offset)
 {

 for (unsigned int i = 0; i < msg->getTLVArraySize(); i++)
 {
 if (msg->getTLV(i).type == tlvType)
 {
 if (offset == 0)
 {
 return &(msg->getTLV(i));
 }
 else
 {
 offset--;
 }
 }
 }

 return NULL;
 }
 */
/*
 * Checks if the System-ID length and Maximum Areas fields in incomming message are supported.
 * @pram inMsg is incomming message
 * @return true if message is OK.
 */
bool ISIS::isMessageOK(ISISMessage *inMsg)
{

    if (inMsg->getIdLength() != ISIS_SYSTEM_ID && inMsg->getIdLength() != 0)
    {
        return false;
    }

    if (this->mode == L2_ISIS_MODE)
    {
        if (inMsg->getMaxAreas() != 1)
        {
            return false;
        }
    }
    else
    {
        if (inMsg->getMaxAreas() != ISIS_MAX_AREAS && inMsg->getMaxAreas() != 0)
        {
            return false;
        }
    }

    return true;
}

/*
 * Compares Area-ID of this IS with Area-ID TLV.
 * @param areaAddressTLV is TLV with area address TLV.
 * @return true if at least one area-id match.
 */
bool ISIS::isAreaIDOK(TLV_t *areaAddressTLV, unsigned char *compare)
{
    if (compare == NULL)
    {
        compare = (unsigned char *) this->areaId;
    }

    if (areaAddressTLV->length == 0)
    {
        return false;
    }

    for (unsigned int i = 0; i < areaAddressTLV->length;)
    {
        if (this->compareArrays(compare, &areaAddressTLV->value[i + 1], areaAddressTLV->value[i]))
        {
            //if one address match return false
            return true;
        }
        i += areaAddressTLV->value[i] + 1;

    }
    //not even single areaID match, so return false
    return false;
}

/*
 * Returns index to ISISIft for specified interface.
 * @param interface is pointer to interface
 * @return index to ISISIft vector
 */
int ISIS::getIfaceIndex(ISISinterface *interface)
{

    for (unsigned int i = 0; i < this->ISISIft.size(); i++)
    {

        if (interface == &(ISISIft.at(i)))
        {
            return i;
        }
    }
    return 0;
}

/*
 * Generates and adds specified TLV to the message.
 * @param inMsg message
 * @param tlvType is TLV type to be generated.
 */
void ISIS::addTLV(ISISMessage *inMsg, enum TLVtypes tlvType, short circuitType, int gateIndex)
{
    std::vector<TLV_t*> tmpTLV;
    switch (tlvType)
        {
        case (AREA_ADDRESS):

            tmpTLV = this->genTLV(AREA_ADDRESS, circuitType); //second parameter doesn't matter for Area Address
//            this->addTLV(inMsg, tmpTLV);

            break;
        case (IS_NEIGHBOURS_HELLO):
            tmpTLV = this->genTLV(IS_NEIGHBOURS_HELLO, circuitType, gateIndex); //second parameter doesn't matter for Area Address
//            this->addTLV(inMsg, tmpTLV);

            break;

        case (TLV_TRILL_NEIGHBOR):
            tmpTLV = this->genTLV(TLV_TRILL_NEIGHBOR, circuitType, gateIndex);
//            this->addTLV(inMsg, tmpTLV);

            break;

        case (PTP_HELLO_STATE):
            tmpTLV = this->genTLV(PTP_HELLO_STATE, circuitType, gateIndex); //second parameter doesn't matter for Area Address
//            this->addTLV(inMsg, tmpTLV);

            break;

        default:
            EV<< "ISIS: ERROR: This TLV type is not (yet) implemented in addTLV(ISISMessage*, enum TLVtypes)" << endl;
            break;

        }

    for (std::vector<TLV_t *>::iterator it = tmpTLV.begin(); it != tmpTLV.end(); ++it)
    {
        this->addTLV(inMsg, (*it));
    }

}

/*
 * Copy already generated TLV to message and sets correct TLV array size.
 * @param inMsg message
 * @param tlv pointer to TLV to be added.
 */
void ISIS::addTLV(ISISMessage* inMsg, TLV_t *tlv)
{
    TLV_t tmpTlv;
    unsigned int tlvSize;

    tmpTlv.type = tlv->type; // set type
    tmpTlv.length = tlv->length; //set length
    tmpTlv.value = new unsigned char[tlv->length]; //allocate appropriate space
    this->copyArrayContent(tlv->value, tmpTlv.value, tlv->length, 0, 0); //copy it

    tlvSize = inMsg->getTLVArraySize(); //get array size
    inMsg->setTLVArraySize(tlvSize + 1); // increase it by one
    inMsg->setTLV(tlvSize, tmpTlv); //finally add TLV at the end
}

std::vector<TLV_t *> ISIS::genTLV(enum TLVtypes tlvType, short circuitType, int gateIndex, enum TLVtypes subTLVType,
        InterfaceEntry *ie)
{
    TLV_t * myTLV;
    std::vector<TLV_t *> myTLVVector;
    AdjTab_t * adjTab;
    TRILLInterfaceData *d;
    int count = 0;
    if (tlvType == AREA_ADDRESS)
    {
        /*************************************
         * TLV #1 Area Addreses              *
         * Code 1                            *
         * Length: length of the value field *
         * Value:                            *
         * ***********************************
         * Address length       *     1B     *
         * Area Address   * Address Length   *
         *************************************
         * Address length       *     1B     *
         * Area Address   * Address Length   *
         *************************************
         */
        /* RFC 6326 4.2. */
        if (this->mode == ISIS::L2_ISIS_MODE)
        {
            myTLV = new TLV_t;
            myTLV->type = AREA_ADDRESS;
            myTLV->length = 2;
            myTLV->value = new unsigned char[myTLV->length];
            myTLV->value[0] = 1;
            myTLV->value[1] = 0;
            myTLVVector.push_back(myTLV);

        }
        else
        {
            //TODO add support for multiple Area Addresses
            myTLV = new TLV_t;
            myTLV->type = AREA_ADDRESS;
            myTLV->length = ISIS_AREA_ID + 1;
            myTLV->value = new unsigned char[ISIS_AREA_ID + 1];
            myTLV->value[0] = ISIS_AREA_ID; //first byte is length of area address
            this->copyArrayContent((unsigned char*) this->areaId, myTLV->value, ISIS_AREA_ID, 0, 1);
            myTLVVector.push_back(myTLV);
        }
    }
    else if (tlvType == IS_NEIGHBOURS_HELLO)
    {
        /* IS Neighbours in state "UP" or "Initializing" eg. ALL in adjTab. */
        /*************************************
         * TLV #6 IS Neighbours              *
         * Code 6                            *
         * Length: length of the value field *
         * Value:                            *
         * ***********************************
         * LAN Address (MAC)    *     6B     *
         * LAN Address (MAC)    *     6B     *
         * LAN Address (MAC)    *     6B     *
         *************************************
         */
        //TODO there could more neighbours than could fit in one TLV
        adjTab = this->getAdjTab(circuitType);

        /* stupid solution */
        for (unsigned int h = 0; h < adjTab->size(); h++)
        {
            if (adjTab->at(h).gateIndex == gateIndex)
            {
                count++;
            }
        }

        myTLV = new TLV_t;
        myTLV->type = IS_NEIGHBOURS_HELLO;
        myTLV->length = count * 6; //number of records * 6 (6 is size of system ID/MAC address)
        myTLV->value = new unsigned char[myTLV->length];
        count = 0;
        for (unsigned int h = 0; h < adjTab->size(); h++)
        {
            if (adjTab->at(h).gateIndex == gateIndex)
            {
//                    this->copyArrayContent(adjTab->at(h).mac.getAddressBytes(), myTLV->value, 6, 0, h * 6);
                adjTab->at(h).mac.getAddressBytes(&(myTLV->value[count * 6]));
                count++;
            }
        }
        myTLVVector.push_back(myTLV);

    }
    else if (tlvType == TLV_TRILL_NEIGHBOR)
    {
        /* TRILL Neighbor TLV to be used in TRILL IIH PDUs */
        /*************************************
         * TLV #145 TRILL Neighbor           *
         * Code 145                          *
         * Length: 1 + 9 * n                 *
         * Value:                            *
         * ***********************************
         * S|L| Reserved        *     1B     *
         *************************************
         * F| Reserved          *     1B     *
         * MTU                  *     2B     *
         * MAC Address          *     6B     *
         *************************************
         */

        //TODO A2 check length and if necessary, create new TLV_t
        adjTab = this->getAdjTab(circuitType);

        /* stupid solution */
        for (unsigned int h = 0; h < adjTab->size(); h++)
        {
            if (adjTab->at(h).gateIndex == gateIndex)
            {
                count++;
            }
        }
        myTLV = new TLV_t;
        myTLV->type = TLV_TRILL_NEIGHBOR;
        myTLV->length = 1 + count * 9;
        myTLV->value = new unsigned char[myTLV->length];
        //TODO A2 S and L bit should be dynamic
        myTLV->value[0] = 192; //both bits set
        count = 0;
        for (unsigned int h = 0; h < adjTab->size(); h++)
        {
            if (adjTab->at(h).gateIndex == gateIndex)
            {
                //TODO MTU test
                myTLV->value[1 + count * 9] = 0; // F | Reserved
                myTLV->value[1 + count * 9 + 1] = 0; //MTU
                myTLV->value[1 + count * 9 + 2] = 0; //MTU

                adjTab->at(h).mac.getAddressBytes(&(myTLV->value[1 + (count * 9) + 3]));
                count++;
            }
        }

        myTLVVector.push_back(myTLV);
    }
    else if (tlvType == PTP_HELLO_STATE)
    {
        //ptp adjacency state TLV #240
        myTLV = new TLV_t;
        myTLV->type = PTP_HELLO_STATE;
        myTLV->length = 1;
        myTLV->value = new unsigned char[myTLV->length];
        ISISadj* tempAdj;
        //if adjacency for this interface exists, then its state is either UP or INIT
        //we also assumes that on point-to-point link only one adjacency can exist
        //TODO we check appropriate level adjacency table, but what to do for L1L2? In such case there's should be adjacency in both tables so we check just L1
        if ((tempAdj = this->getAdjByGateIndex(gateIndex, circuitType)) != NULL)
        {
            if (tempAdj->state == ISIS_ADJ_DETECT)
            {
                myTLV->value[0] = PTP_INIT;
                EV<< "ISIS::sendPTPHello: Source-ID: ";
                for (unsigned int i = 0; i < 6; i++)
                {
                    EV<< setfill('0') << setw(2) << dec << (unsigned int) this->sysId[i];
                    if (i % 2 == 1)
                    EV << ".";
                }
                EV<<"sending state PTP_INIT " << endl;
            }
            else
            {
                myTLV->value[0] = PTP_UP;
                EV<< "ISIS::sendPTPHello: Source-ID: ";

                for (unsigned int i = 0; i < 6; i++)
                {
                    EV << setfill('0') << setw(2) << dec << (unsigned int) this->sysId[i];
                    if (i % 2 == 1)
                    EV << ".";
                }
                EV <<"sending state PTP_UP " << endl;
                EV << "ISIS::sendPTPHello: sending state PTP_UP " << endl;
            }

        }
        else
        {
            //if adjacency doesn't exist yet, then it's for sure down
            myTLV->value[0] = PTP_DOWN;
            EV<< "ISIS::sendPTPHello: Source-ID: ";
            for (unsigned int i = 0; i < 6; i++)
            {
                EV << setfill('0') << setw(2) << dec << (unsigned int) this->sysId[i];
                if (i % 2 == 1)
                EV << ".";
            }
            EV <<"sending state PTP_DOWN " << endl;

        }

        myTLVVector.push_back(myTLV);

    }
    else if (tlvType == TLV_MT_PORT_CAP)
    {

        /* Multi-Topology-aware Port Capability */
        /*************************************
         * TLV #143 MTPORTCAP                *
         * Code 143                          *
         * Length: 2 + sub-TLVs length       *
         * Value:                            *
         * ***********************************
         * R|R|R|R| Topology Id *     2B     *
         *************************************
         *   Sub-TLVs           * variable B *
         *************************************
         */

        if (ie != NULL)
        {
            d = ie->trillData();
        }

        myTLV = new TLV_t;
        myTLV->type = TLV_MT_PORT_CAP;
        myTLV->length = 0;
        myTLV->value = new unsigned char[255];
        myTLV->value[0] = 0; //reserved + zero topology Id
        myTLV->value[1] = 0;
        myTLV->length = 2;

        /* Special VLANS and Flags Sub-TLV  */
        /*************************************
         * TLV #143 MTPORTCAP                *
         * subTLV Code 1                     *
         * Length: 8                         *
         * Value:                            *
         * ***********************************
         * Port ID              *     2B     *
         * Sender Nickname      *     2B     *
         * AF|AC|VM|BY| Outer.VLAN *  2B     *
         * TR| R| R| R| Desig.VLAN *  2B     *
         *************************************
         */

        myTLV->value[myTLV->length] = TLV_MT_PORT_CAP_VLAN_FLAG;
        myTLV->value[myTLV->length + 1] = 8;
        myTLV->length += 2; //size of "head" of subTLV
        //TODO change it interfaceId
        //Port ID
        myTLV->value[myTLV->length] = gateIndex & 0xFF;
        myTLV->value[myTLV->length + 1] = gateIndex >> 8;

        //TODO B1 Nickname
//        memcpy(&(myTLV->value[myTLV->length + 2]), &(this->sysId[ISIS_SYSTEM_ID - 3]), 2);
        myTLV->value[myTLV->length + 2] = this->nickname & 0xFF;
        myTLV->value[myTLV->length + 3] = (this->nickname >> 8) & 0xFF;
        //Outer.VLAN
        //TODO B1 what to do when the port is configured to strip VLAN tag
        //TODO B1 in some cases one hello is sent on EVERY VLAN (0 - 4096)
        myTLV->value[myTLV->length + 4] = d->getVlanId() & 0xFF;
        myTLV->value[myTLV->length + 5] = (d->getVlanId() >> 8) & 0x0F;
        //AF
        myTLV->value[myTLV->length + 5] |= (d->isAppointedForwarder(d->getVlanId(), this->nickname) << 7);
        //AC
        myTLV->value[myTLV->length + 5] |= (d->isAccess() << 6);
        //VM VLAN Mapping TODO unable to detect so setting false
        myTLV->value[myTLV->length + 5] |= (d->isVlanMapping() << 5);
        //BY Bypass pseudonode
        myTLV->value[myTLV->length + 5] |= (d->isVlanMapping() << 4);

        //Design.VLAN
        myTLV->value[myTLV->length + 6] = d->getDesigVlan() && 0xFF;
        myTLV->value[myTLV->length + 7] = (d->getDesigVlan() >> 8) && 0x0F;
        //TR flag
        myTLV->value[myTLV->length + 7] |= (d->isTrunk() << 7);

        myTLV->length += 8;

        /* Appointed Forwarders Sub-TLV     */
        /*************************************
         * TLV #143 MTPORTCAP                *
         * subTLV Code 3                     *
         * Length: n * 6B                    *
         * Value:                            *
         * ***********************************
         * Appointee Nickname   *     2B     *
         * R|R|R|R| Start. VLAN *     2B     *
         * R|R|R|R| End. VLAN   *     2B     *
         *************************************
         * ...                               *
         */
        //TODO A2

        if(amIL1DIS(this->getIfaceIndex(this->getIfaceByGateIndex(gateIndex)))){
            //TODO A2 before appointing someone, DRB should wait at least Holding timer interval (to ensure it is the DRB).
            myTLV->value[myTLV->length] = TLV_MT_PORT_CAP_APP_FWD;
            myTLV->value[myTLV->length + 1] = 1 * 6;
            myTLV->length += 2; //size of subTLV header (type, length)
            //Appointee Nickname

            myTLV->value[myTLV->length + 2] = this->nickname & 0xFF; //Appointee Nickname
            myTLV->value[myTLV->length + 2] = (this->nickname >> 8) & 0xFF; //Appointee Nickname
            myTLV->value[myTLV->length + 2] = 1; //Start VLAN
            myTLV->value[myTLV->length + 3] = 0; //Start VLAN
            myTLV->value[myTLV->length + 4] = 1; //End VLAN
            myTLV->value[myTLV->length + 5] = 0; //End VLAN

            myTLV->length += 6; //size of one record

        }

        //end of subTLV

        myTLVVector.push_back(myTLV);
    }
    else
    {
        EV<< "ISIS: ERROR: This TLV type is not (yet) implemented in genTLV(enum TLVtypes tlvType, short circuitType, int gateIndex, enum TLVtypes subTLVType, InterfaceEntry *ie)" << endl;

    }
    return myTLVVector;
}

/*
 * Generates and adds specified TLV type to the set of TLVs
 * @param tlvTable set of TLVs
 * @param tlvType TLV type to be generated
 * @param circuitType is level.
 */
void ISIS::addTLV(std::vector<TLV_t *> *tlvTable, enum TLVtypes tlvType, short circuitType, InterfaceEntry *ie)
{
    std::vector<TLV_t *> myTLVVector;
    TLV_t * myTLV;

    if (tlvType == AREA_ADDRESS)
    {
        myTLVVector = this->genTLV(AREA_ADDRESS, circuitType);
        for (std::vector<TLV_t *>::iterator it = myTLVVector.begin(); it != myTLVVector.end(); ++it)
        {
            tlvTable->push_back((*it));
        }

    }
    else if (tlvType == IS_NEIGHBOURS_LSP)
    {

        if (ie == NULL)
        {
            std::vector<LSPneighbour> neighbours;
            ISISadj *tmpAdj;
            for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
            {
                LSPneighbour neighbour;
                tmpAdj = this->getAdjByGateIndex((*it).gateIndex, circuitType);
                //if there's not adjacency in state "UP" for specified interface, then skip this interface
                if (tmpAdj == NULL || tmpAdj->state != ISIS_ADJ_REPORT)
                {
                    continue;
                }

                if ((*it).network)
                {
                    unsigned char *DIS;
                    if (circuitType == L1_TYPE)
                    {
                        DIS = (*it).L1DIS;
                    }
                    else
                    {
                        DIS = (*it).L2DIS;
                    }

                    this->copyArrayContent(DIS, neighbour.LANid, ISIS_SYSTEM_ID + 1, 0, 0);
                }
                else
                {
                    this->copyArrayContent(tmpAdj->sysID, neighbour.LANid, ISIS_SYSTEM_ID, 0, 0);
                    neighbour.LANid[ISIS_SYSTEM_ID] = 0;
                }

                //find interface which is DIS connected to and set metrics
                neighbour.metrics.defaultMetric = (*it).metric; //default = 10
                neighbour.metrics.delayMetric = 128; //disabled;
                neighbour.metrics.expenseMetric = 128; //disabled
                neighbour.metrics.errortMetric = 128; //disabled

                neighbours.push_back(neighbour);

            }
            //we have vector of neighbours

            //now compute size needed in tlv
            // virtual link + ((metrics + System-ID + 1) * number_of_neighbours

            int entrySize = (4 + ISIS_SYSTEM_ID + 1);
            //run until there's any neighbour
            for (; !neighbours.empty();)
            {
                myTLV = new TLV_t;
                myTLV->type = IS_NEIGHBOURS_LSP;
                myTLV->length = 1 + ((4 + ISIS_SYSTEM_ID + 1) * neighbours.size());
                myTLV->value = new unsigned char[ISIS_LSP_MAX_SIZE];

                //virtualFlag
                myTLV->value[0] = 40; //TODO should be 0
                //inner loop for separate TLV; after reaching ISIS_LSP_MAX_SIZE or empty neighbours stop filling this tlv
                // 2 bytes for type and length fields and 1 byte for virtual circuit
                for (unsigned int i = 0;
                        ((i + 1) * entrySize) + 1 < 255 && 2 + 1 + (entrySize * i) + entrySize < ISIS_LSP_MAX_SIZE
                                && !neighbours.empty(); i++)
                {
                    myTLV->value[(i * 11) + 1] = neighbours.at(0).metrics.defaultMetric;
                    myTLV->value[(i * 11) + 2] = neighbours.at(0).metrics.delayMetric;
                    myTLV->value[(i * 11) + 3] = neighbours.at(0).metrics.expenseMetric;
                    myTLV->value[(i * 11) + 4] = neighbours.at(0).metrics.errortMetric;
                    this->copyArrayContent(neighbours.at(0).LANid, myTLV->value, ISIS_SYSTEM_ID + 1, 0, (i * 11) + 5); //set system ID
                    myTLV->length = 1 + ((4 + ISIS_SYSTEM_ID + 1) * (i + 1));
                    //delete first entry
                    neighbours.erase(neighbours.begin());
                }
                //this tlv is full or no other neighbour entry is present
                tlvTable->push_back(myTLV);
                //TODO do i need to create new myTLV and allocate value?
                //myTLV = new TLV_t;

            }
            return;
        }
        else
        //pseudonode
        {
            //let's assume that nsel is interfaceIndex +1
//            int interfaceIndex = nsel - 1;
            ISISinterface *iface = this->getIfaceByGateIndex(ie->getNetworkLayerGateIndex());

            std::vector<LSPneighbour> neighbours;

            ISISadj *tmpAdj;
//            for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
//            {

            for (int offset = 0; (tmpAdj = this->getAdjByGateIndex(iface->gateIndex, circuitType, offset)) != NULL;
                    offset++)
            {
                LSPneighbour neighbour;
                if (tmpAdj->state < ISIS_ADJ_2WAY || !iface->network)
                {
                    continue;
                }

                this->copyArrayContent(tmpAdj->sysID, neighbour.LANid, ISIS_SYSTEM_ID, 0, 0);
                neighbour.LANid[ISIS_SYSTEM_ID] = 0;

                neighbour.metrics.defaultMetric = 0; //metric from DIS is 0
                neighbour.metrics.delayMetric = 128; //disabled;
                neighbour.metrics.expenseMetric = 128; //disabled
                neighbour.metrics.errortMetric = 128; //disabled

                neighbours.push_back(neighbour);
            }
//            }
            //add also mine non-pseudonode interface as neighbour
            LSPneighbour neighbour;
            this->copyArrayContent((unsigned char*) this->sysId, neighbour.LANid, ISIS_SYSTEM_ID, 0, 0);
            neighbour.LANid[ISIS_SYSTEM_ID] = 0;
            neighbour.metrics.defaultMetric = 0; //metric to every neighbour in pseudonode LSP is always zero!!!
            neighbour.metrics.delayMetric = 128; //disabled;
            neighbour.metrics.expenseMetric = 128; //disabled
            neighbour.metrics.errortMetric = 128; //disabled

            neighbours.push_back(neighbour);

            //we have vector neighbours for pseudo

            int entrySize = (4 + ISIS_SYSTEM_ID + 1);
            //run until there's any neighbour
            for (; !neighbours.empty();)
            {
                myTLV = new TLV_t;
                myTLV->type = IS_NEIGHBOURS_LSP;
                myTLV->length = 1 + ((4 + ISIS_SYSTEM_ID + 1) * neighbours.size());
                myTLV->value = new unsigned char[ISIS_LSP_MAX_SIZE];

                //virtualFlag
                myTLV->value[0] = 40; //TODO should be 0
                //inner loop for separate TLV; after reaching ISIS_LSP_MAX_SIZE or empty neighbours stop filling this tlv
                // 2 bytes for type and length fields and 1 byte for virtual circuit
                for (unsigned int i = 0;
                        ((i + 1) * entrySize) + 1 < 255 && 2 + 1 + (entrySize * i) + entrySize < ISIS_LSP_MAX_SIZE
                                && !neighbours.empty(); i++)
                {
                    myTLV->value[(i * 11) + 1] = neighbours.at(0).metrics.defaultMetric;
                    myTLV->value[(i * 11) + 2] = neighbours.at(0).metrics.delayMetric;
                    myTLV->value[(i * 11) + 3] = neighbours.at(0).metrics.expenseMetric;
                    myTLV->value[(i * 11) + 4] = neighbours.at(0).metrics.errortMetric;
                    this->copyArrayContent(neighbours.at(0).LANid, myTLV->value, ISIS_SYSTEM_ID + 1, 0, (i * 11) + 5); //set system ID
                    myTLV->length = 1 + ((4 + ISIS_SYSTEM_ID + 1) * (i + 1));
                    //delete first entry
                    neighbours.erase(neighbours.begin());
                }
                //this tlv is full or no other neighbour entry is present
                tlvTable->push_back(myTLV);
                //TODO do i need to create new myTLV and allocate value?
                //myTLV = new TLV_t;

            }
        }

        //end of TLV IS_NEIGHBOURS_LSP
    }
    else if (tlvType == TLV_MT_PORT_CAP)
    {

        //subTLV VLAN FLAGS
        myTLVVector = genTLV(TLV_MT_PORT_CAP, circuitType, ie->getNetworkLayerGateIndex(), TLV_MT_PORT_CAP_VLAN_FLAG,
                ie);
        for (std::vector<TLV_t *>::iterator it = myTLVVector.begin(); it != myTLVVector.end();)
        {
            tlvTable->push_back((*it));
            it = myTLVVector.erase(it);
        }

        //end of TLV_MT_PORT_CAP
    }
    else if (tlvType == TLV_TRILL_NEIGHBOR)
    {

        myTLVVector = genTLV(TLV_TRILL_NEIGHBOR, circuitType, ie->getNetworkLayerGateIndex());
        for (std::vector<TLV_t *>::iterator it = myTLVVector.begin(); it != myTLVVector.end();)
        {
            tlvTable->push_back((*it));
            it = myTLVVector.erase(it);
        }
//        myTLVVector.clear();
    }
    else
    {
        EV<< "ISIS: ERROR: This TLV type is not (yet) implemented in addTLV" << endl;
    }
}

        /*
         * Returns true if this System have ANY adjacency UP (now with extension 2WAY or REPORT) on level specified by circuitType
         * @param circuitType is level.
         */
bool ISIS::isAdjUp(short circuitType)
{
    std::vector<ISISadj>* adjTable = this->getAdjTab(circuitType);

    for (std::vector<ISISadj>::iterator it = adjTable->begin(); it != adjTable->end(); ++it)
    {
        if ((*it).state >= ISIS_ADJ_2WAY) //TODO A1
        {
            return true;
        }
    }
    return false;
}

/*
 * This method returns true if there is ANY adjacency on interface identified by gateIndex in state UP (now with extension 2WAY or REPORT)
 * Method does NOT check SNPA (aka MAC).
 * Can be used to check if we should send LSP on such interface.
 * @param gateIndex is gate index
 * @param circuitType is level
 */
bool ISIS::isUp(int gateIndex, short circuitType)
{
    std::vector<ISISadj> *adjTable;

    adjTable = this->getAdjTab(circuitType);
    for (std::vector<ISISadj>::iterator it = adjTable->begin(); it != adjTable->end(); ++it)
    {
        if ((*it).gateIndex == gateIndex && (*it).state >= ISIS_ADJ_2WAY)
        {
            return true;
        }
    }
    return false;

}

/*
 * Verify if adjacency for specified message is in state UP.
 * @param msg incomming message
 * @param circuitType is level.
 */
bool ISIS::isAdjUp(ISISMessage *msg, short circuitType)
{
    /* Pretty messy code, please clean up */
    std::vector<ISISadj> *adjTable = this->getAdjTab(circuitType);
    int gateIndex = msg->getArrivalGate()->getIndex();

    //TODO for truly point-to-point link there would not be MAC address
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->getControlInfo());
    MACAddress tmpMac = ctrl->getSrc();
    unsigned char * systemID;

    systemID = this->getSysID(msg);
    for (std::vector<ISISadj>::iterator it = adjTable->begin(); it != adjTable->end(); ++it)
    {
        //System-ID match?
        /* Exception for LSP is here because to satisfy  7.3.15.1 a) 6) is enough to check SNPA and interface e.g gateIndex */
        if (compareArrays(systemID, (*it).sysID, ISIS_SYSTEM_ID) || msg->getType() == L1_LSP
                || msg->getType() == L2_LSP)
        {
            //MAC Address and gateIndex
            //we need to check source (tmpMac) and destination interface thru we received this hello
            if (tmpMac.compareTo((*it).mac) == 0 && gateIndex == (*it).gateIndex && (*it).state >= ISIS_ADJ_2WAY)
            {

                delete[] systemID;
                return true;

            }
        }
    }
    delete[] systemID;

    return false;
}

/**
 * Computes distribution tree for each system in LSP DB.
 * @param circuitType is circuit type
 */
void ISIS::spfDistribTrees(short int circuitType){

//    std::map<int, ISISPaths_t *> distribTrees;
    //    std::vector<unsigned char *> idVector;
    this->distribTrees.clear();
        std::map<std::string, int> systemIdVector;  //vector of all ISs's systemId

    systemIdVector = this->getAllSystemIdsFromLspDb(circuitType);

    for (std::map<std::string, int>::iterator iter = systemIdVector.begin(); iter != systemIdVector.end(); ++iter)
    {

        ISISCons_t initial;
        ISISPaths_t *ISISPaths = new ISISPaths_t; //best paths
        ISISPaths_t ISISTent; //
        ISISPath * tmpPath;
//        short circuitType;

        //let's fill up the initial paths with supported-protocol's reachability informations

        //fill ISO
        bool result;
//        circuitType = timer->getIsType();
        result = this->extractISO(&initial, circuitType);
        if (!result)
        {
            //there was an error during extraction so cancel SPF
            //TODO B5 reschedule
//            this->schedule(timer);
            //TODO B5 clean
            return;
        }

        //put current systemId from systemIdVector on TENT list
        unsigned char *lspId = (unsigned char *)iter->first.c_str(); //returns sysId + 00

        tmpPath = new ISISPath;
        tmpPath->to = new unsigned char[ISIS_SYSTEM_ID + 2];
        this->copyArrayContent(lspId, tmpPath->to, ISIS_SYSTEM_ID, 0, 0);
        tmpPath->to[ISIS_SYSTEM_ID] = 0;
        tmpPath->to[ISIS_SYSTEM_ID + 1] = 0;

        tmpPath->metric = 0;

        ISISNeighbour *neighbour = new ISISNeighbour;
        neighbour->id = new unsigned char[ISIS_SYSTEM_ID + 2];
        this->copyArrayContent(lspId, neighbour->id, ISIS_SYSTEM_ID, 0, 0);
        neighbour->id[ISIS_SYSTEM_ID] = 0;
        neighbour->id[ISIS_SYSTEM_ID + 1] = 0;
        neighbour->type = false; //not a leaf
        tmpPath->from.push_back(neighbour);

        ISISTent.push_back(tmpPath);



        //TODO shoudn't i put myself in PATH list directly?
        for (; !ISISTent.empty();)
        {
            //tmpPath = this->getBestPath(&(this->ISISTent));

            //this->moveToPath(tmpPath);
            this->bestToPathDT(&initial, &ISISTent, ISISPaths);

        }
        std::sort(ISISPaths->begin(), ISISPaths->end(), ISISPath());
        for(ISISPaths_t::iterator it = ISISPaths->begin(); it != ISISPaths->end(); ){
            if((*it)->to[ISIS_SYSTEM_ID] != 0){
                it = ISISPaths->erase(it);
            }else{
                ++it;
            }
        }

        this->distribTrees.insert(std::make_pair(lspId[ISIS_SYSTEM_ID - 1] + lspId[ISIS_SYSTEM_ID - 2] * 0xFF, ISISPaths));

    }

}



/*
 * Moves best path from ISISTent to ISISPaths and initiate move of appropriate connections from init to ISISTent
 * @param initial is set of connections
 * @param ISISTent is set of tentative paths
 * @param ISISPaths is set of best paths from this IS
 */
void ISIS::bestToPathDT(ISISCons_t *init, ISISPaths_t *ISISTent, ISISPaths_t *ISISPaths)
{

    ISISPath *path;
    ISISPath *tmpPath;
    //sort it
    std::sort(ISISTent->begin(), ISISTent->end(), ISISPath());
    //save best in path
    path = ISISTent->front();
    //mov
    this->moveToTentDT(init, path, path->to, path->metric, ISISTent);

    ISISTent->erase(ISISTent->begin());

    if ((tmpPath = this->getPath(ISISPaths, path->to)) == NULL)
    {
        //path to this destination doesn't exist, so simply push

        ISISPaths->push_back(path);
    }
    else
    {
        if (tmpPath->metric > path->metric)
        {
            if (tmpPath->metric > path->metric)
            {
                EV<< "ISIS: Error during SPF. We got better metric than the one PATHS." << endl;
                //we got better metric so clear "from" neighbours
                tmpPath->from.clear();
            }
            EV << "ISIS: Error during SPF. I think we shouldn't have neither same metric." << endl;
            //append
            tmpPath->metric = path->metric;
            cout << "pathb metric: " << tmpPath->metric << endl;
            tmpPath->from.clear();
            for (ISISNeighbours_t::iterator it = path->from.begin(); it != path->from.end(); ++it)
            {
                tmpPath->from.push_back((*it));
            }

        }
    }

}

/* Moves connections with matching "from" from init to Tent
 * @param initial is set of connections
 * @param from specify which connections to move
 * @param metric is metric to get to "from" node
 * @param ISISTent is set of tentative paths
 */
void ISIS::moveToTentDT(ISISCons_t *initial, ISISPath *path, unsigned char *from, uint32_t metric, ISISPaths_t *ISISTent)
{

    ISISPath *tmpPath;
    ISISCons_t *cons = this->getCons(initial, from);
    /*       if(cons->empty()){
     EV <<"ISIS: Error during SPF. Didn't find my own LSP"<<endl;
     //TODO clean
     //           delete cons;
     //         return;
     }*/

    for (ISISCons_t::iterator consIt = cons->begin(); consIt != cons->end(); ++consIt)
    {
        if ((tmpPath = this->getPath(ISISTent, (*consIt)->to)) == NULL)
        {
            //path to this destination doesn't exist, so create new
            tmpPath = new ISISPath;
            tmpPath->to = new unsigned char[ISIS_SYSTEM_ID + 2];
            this->copyArrayContent((*consIt)->to, tmpPath->to, ISIS_SYSTEM_ID + 2, 0, 0);
            tmpPath->metric = (*consIt)->metric + metric;
//               cout << "patha metric: " << tmpPath->metric << endl;
            if((*consIt)->from[ISIS_SYSTEM_ID] != 0){
                //"from" is pseudonode and i guess that's not desirable
                for (ISISNeighbours_t::iterator nIt = path->from.begin(); nIt != path->from.end(); ++nIt)
                {
                    //if nextHop (from) should be pseudonode, set nextHop as the "to" identifier
                    if ((*nIt)->id[ISIS_SYSTEM_ID] != 0)
                    {
                        ASSERT(false); //this basically says if you get to this part of the code, it's trouble
                        ISISNeighbour *neigh = (*nIt)->copy();
                        memcpy(neigh->id, (*consIt)->to, ISIS_SYSTEM_ID + 2);
                        tmpPath->from.push_back(neigh);
                    }
                    else
                    {
                        //                       if(this->compareArrays((*nIt)->id, neighbour->id, ISIS_SYSTEM_ID + 2)){
                        //this neighbour is already there
                        //                           delete neighbour;
                        tmpPath->from.push_back((*nIt)->copy());
                        //                           return;
                        //                       }
                    }
                }
            }
            else
            {
                ISISNeighbour *neighbour = new ISISNeighbour;
                neighbour->id = new unsigned char[ISIS_SYSTEM_ID + 2];
                this->copyArrayContent((*consIt)->from, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
                neighbour->entry = (*consIt)->entry;
                neighbour->type = false; //not a leaf
                tmpPath->from.push_back(neighbour);
            }


            ////// END
            /* if @param from is THIS IS then next hop (neighbour->id) will be that next hop */
//            if (this->compareArrays((*consIt)->from, (unsigned char *) this->sysId, ISIS_SYSTEM_ID)
//                    && (*consIt)->from[ISIS_SYSTEM_ID] == 0)
//            {
//                this->copyArrayContent((*consIt)->to, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
//                neighbour->entry = (*consIt)->entry;
////                ASSERT(neighbour->entry != NULL);
//                tmpPath->from.push_back(neighbour);
//            }
//            else
//            {
//                //TODO neighbour must be THIS IS or next hop, therefore we need to check whether directly connected
////                   this->copyArrayContent(path->from, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
//                for (ISISNeighbours_t::iterator nIt = path->from.begin(); nIt != path->from.end(); ++nIt)
//                {
//                    //if nextHop (from) should be pseudonode, set nextHop as the "to" identifier
//                    if ((*nIt)->id[ISIS_SYSTEM_ID] != 0)
//                    {
//
//                        ISISNeighbour *neigh = (*nIt)->copy();
//                        memcpy(neigh->id, (*consIt)->to, ISIS_SYSTEM_ID + 2);
//                        tmpPath->from.push_back(neigh);
//                    }
//                    else
//                    {
////                       if(this->compareArrays((*nIt)->id, neighbour->id, ISIS_SYSTEM_ID + 2)){
//                        //this neighbour is already there
////                           delete neighbour;
//                        tmpPath->from.push_back((*nIt)->copy());
////                           return;
////                       }
//                    }
//                }
//            }

//               tmpPath->from = neighbour;

            ISISTent->push_back(tmpPath);
        }
        else
        {
            if (tmpPath->metric > (*consIt)->metric + metric)
            {
                /* true in expression below makes the result tree and not a graph (only single "from" allowed) */
                if (tmpPath->metric > (*consIt)->metric + metric || true)
                {
                    //we got better metric so clear "from" neighbours
                    tmpPath->from.clear();
                }
                //append
                tmpPath->metric = (*consIt)->metric + metric;
                cout << "path metric: " << tmpPath->metric << endl;

                if((*consIt)->from[ISIS_SYSTEM_ID] != 0){
                    for (ISISNeighbours_t::iterator nIt = path->from.begin(); nIt != path->from.end(); ++nIt)
                    {
                        //if nextHop (from) should be pseudonode, set nextHop as the "to" identifier
                        if ((*nIt)->id[ISIS_SYSTEM_ID] != 0)
                        {
                            ASSERT(false); //this basically says if you get to this part of the code, it's trouble
                            ISISNeighbour *neigh = (*nIt)->copy();
                            memcpy(neigh->id, (*consIt)->to, ISIS_SYSTEM_ID + 2);
                            tmpPath->from.push_back(neigh);
                        }
                        else
                        {
                            //                       if(this->compareArrays((*nIt)->id, neighbour->id, ISIS_SYSTEM_ID + 2)){
                            //this neighbour is already there
                            //                           delete neighbour;
                            tmpPath->from.push_back((*nIt)->copy());
                            //                           return;
                            //                       }
                        }
                    }
                }else{
                    /* the @param from is regular IS so create classic neighbor */
                    ISISNeighbour *neighbour = new ISISNeighbour;
                    neighbour->id = new unsigned char[ISIS_SYSTEM_ID + 2];
                    this->copyArrayContent((*consIt)->from, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
                    neighbour->type = false; //not a leaf

                    for (ISISNeighbours_t::iterator nIt = tmpPath->from.begin(); nIt != tmpPath->from.end(); ++nIt)
                    {
                        if (this->compareArrays((*nIt)->id, neighbour->id, ISIS_SYSTEM_ID + 2))
                        {
                            //this neighbour is already there
                            delete neighbour;
                            return;
                        }
                    }
                    tmpPath->from.push_back(neighbour);
                }

//                for (ISISNeighbours_t::iterator nIt = path->from.begin(); nIt != path->from.end(); ++nIt)
//                {
//                    bool found = false;
//                    for (ISISNeighbours_t::iterator neIt = tmpPath->from.begin(); neIt != tmpPath->from.end(); ++neIt)
//                    {
//                        //is such nextHop with matching ID AND entry (we may have adjacency with same IS over multiple interfaces)
//                        if (this->compareArrays((*neIt)->id, (*nIt)->id, ISIS_SYSTEM_ID + 2)
//                                && (*neIt)->entry == (*nIt)->entry)
//                        {
//                            //this neighbour is already there
////                             delete neighbour;
////                             continue;
//                            found = true;
//                            break;
//                        }
//
//                    }
//                    if (!found)
//                    {
//                        //if such nextHop from @param path is not already present (not found), add it to nextHops(from)
//                        tmpPath->from.push_back((*nIt)->copy());
//                    }
//
////
//                }

            }

        }

    }
}

std::vector<unsigned char *> *ISIS::getSystemIDsFromTreeOnlySource(int nickname, const unsigned char *systemId){

    std::vector<unsigned char *> *systemIDs = this->getSystemIDsFromTree(nickname, systemId);



//    it = this->distribTrees.find(nickname);

        for(std::vector<unsigned char *>::iterator it = systemIDs->begin(); it != systemIDs->end();){
            if(memcmp((*it), systemId, ISIS_SYSTEM_ID) == 0){
                it = systemIDs->erase(it);
            }else{
                ++it;
            }
//            for(ISISNeighbours_t::iterator neighIt = (*pathIt)->from.begin(); neighIt != (*pathIt)->from.end(); ++neighIt){
//                if (memcmp(systemId, (*neighIt)->id, ISIS_SYSTEM_ID) == 0)
//                {
//                    treePaths->push_back((*pathIt)->copy());
//                    break;
//                }
//            }


        }


        return systemIDs;

}

/**
 * Returns system-ids of all neighbours for in distribution tree for specified nickname
 * @param nickname specify tree
 * @param system-id to look for in distribution tree
 * @return vector of system-ids
 */
std::vector<unsigned char *> *ISIS::getSystemIDsFromTree(int nickname, const unsigned char *systemId){

    std::vector<unsigned char *> *systemIDs = new std::vector<unsigned char *>;

    std::map<int, ISISPaths_t *>::iterator it;
    if(this->distribTrees.find(nickname) == this->distribTrees.end()){
        this->spfDistribTrees(L1_TYPE);
    }

    it = this->distribTrees.find(nickname);
    if(it != this->distribTrees.end()){
        for(ISISPaths_t::iterator pathIt = it->second->begin(); pathIt != it->second->end(); ++pathIt){
            //if destination match -> push_back
            if(memcmp((*pathIt)->to, systemId, ISIS_SYSTEM_ID) == 0){
                for (ISISNeighbours_t::iterator neighIt = (*pathIt)->from.begin(); neighIt != (*pathIt)->from.end();
                        ++neighIt)
                {
                    unsigned char *tmpSysId = new unsigned char[ISIS_SYSTEM_ID];
                    memcpy(tmpSysId, (*neighIt)->id, ISIS_SYSTEM_ID);
                    systemIDs->push_back(tmpSysId);


                }

                continue;
            }
            //if at least one "from" match -> push_back (actually there should be only one "from")
            for(ISISNeighbours_t::iterator neighIt = (*pathIt)->from.begin(); neighIt != (*pathIt)->from.end(); ++neighIt){
                if (memcmp(systemId, (*neighIt)->id, ISIS_SYSTEM_ID) == 0)
                {
                    unsigned char *tmpSysId = new unsigned char[ISIS_SYSTEM_ID];
                    memcpy(tmpSysId, (*pathIt)->to, ISIS_SYSTEM_ID);
                    systemIDs->push_back(tmpSysId);

                    break;
                }
            }


        }

    }
        return systemIDs;

}

/**
 * Returns all system-id from LSP database
 * @param circuitType is circuit type.
 */
std::map<std::string, int> ISIS::getAllSystemIdsFromLspDb(short circuitType){
    LSPRecQ_t * lspDb = this->getLSPDb(circuitType);
    std::map<std::string, int> systemIdMap;
    for(LSPRecQ_t::iterator it = lspDb->begin(); it!=lspDb->end(); ++it){
        unsigned char *systemId;
        systemId = this->getSysID((*it)->LSP);
        std::string sysId((char *)systemId, ISIS_SYSTEM_ID);

        systemIdMap[sysId] = 1;
    }

    return systemIdMap;
}


void ISIS::runSPF(short circuitType, ISISTimer *timer){

    if(timer != NULL){
        circuitType = timer->getIsType();
    }

    if(circuitType == L1_TYPE){
        timer = this->spfL1Timer;
    }else if(circuitType == L2_TYPE){
        timer = this->spfL2Timer;
    }

    cancelEvent(timer);
    fullSPF(timer);
//    this->schedule(timer);
}


/*
 * Performs full run of SPF algorithm. For L2 ISIS also computes distribution trees.
 * @param timer is initiating timer.
 */
void ISIS::fullSPF(ISISTimer *timer)
{
    if(this->mode == L2_ISIS_MODE){
        this->spfDistribTrees(timer->getIsType());
    }

    ISISCons_t initial;
    ISISPaths_t *ISISPaths = new ISISPaths_t; //best paths
    ISISPaths_t ISISTent; //
    ISISPath * tmpPath;
    short circuitType;

    //let's fill up the initial paths with supported-protocol's reachability informations

    //fill ISO
    bool result;
    circuitType = timer->getIsType();
    result = this->extractISO(&initial, circuitType);
    if (!result)
    {
        //there was an error during extraction so cancel SPF
        //TODO B5 reschedule
        this->schedule(timer);
        //TODO B5 clean
        return;
    }

    //put myself (this IS) on TENT list
    unsigned char *lspId = this->getLSPID(); //returns sysId + 00

    tmpPath = new ISISPath;
    tmpPath->to = new unsigned char[ISIS_SYSTEM_ID + 2];
    this->copyArrayContent(lspId, tmpPath->to, ISIS_SYSTEM_ID + 2, 0, 0);

    tmpPath->metric = 0;

    ISISNeighbour *neighbour = new ISISNeighbour;
    neighbour->id = new unsigned char[ISIS_SYSTEM_ID + 2];
    this->copyArrayContent(lspId, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
    neighbour->type = false; //not a leaf
    tmpPath->from.push_back(neighbour);

    ISISTent.push_back(tmpPath);

//    ISISCons_t *cons = this->getCons(&initial, lspId);
//    if(cons->empty()){
//        EV <<"ISIS: Error during SPF. Didn't find my own LSP"<<endl;
//        //TODO clean
//        delete cons;
//        return;
//    }
//

    //add my connections as a starting point
    /*  for(ISISCons_t::iterator it = cons->begin(); it != cons->end(); ++it){
     if ((tmpPath = this->getPath(&(ISISTent), (*it)->to)) == NULL)
     {
     //path to this destination doesn't exist, co create new
     tmpPath = new ISISPath;
     tmpPath->to = new unsigned char[ISIS_SYSTEM_ID + 2];
     this->copyArrayContent((*it)->to, tmpPath->to, ISIS_SYSTEM_ID + 2, 0, 0);
     tmpPath->metric = (*it)->metric;

     ISISNeighbour *neighbour = new ISISNeighbour;
     neighbour->id = new unsigned char[ISIS_SYSTEM_ID + 2];
     this->copyArrayContent((*it)->from, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
     neighbour->type = false; //not a leaf
     tmpPath->from.push_back(neighbour);

     ISISTent.push_back(tmpPath);
     }
     else
     {
     if(tmpPath->metric >= (*it)->metric){
     if(tmpPath->metric > (*it)->metric){
     //we got better metric so clear "from" neighbours
     tmpPath->from.clear();
     }
     //append
     tmpPath->metric = (*it)->metric;
     ISISNeighbour *neighbour = new ISISNeighbour;
     neighbour->id = new unsigned char[ISIS_SYSTEM_ID + 2];
     this->copyArrayContent((*it)->from, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
     neighbour->type = false; //not a leaf
     tmpPath->from.push_back(neighbour);

     }

     }

     }*/

    //TODO shoudn't i put myself in PATH list directly?
    for (; !ISISTent.empty();)
    {
        //tmpPath = this->getBestPath(&(this->ISISTent));

        //this->moveToPath(tmpPath);
        this->bestToPath(&initial, &ISISTent, ISISPaths);

    }
    std::sort(ISISPaths->begin(), ISISPaths->end(), ISISPath());



    this->printPaths(ISISPaths);

    //find shortest metric in TENT

    /*  ISISPaths_t *ISISPathsISO = getPathsISO(circuitType);
     if(ISISPathsISO != NULL){
     delete ISISPathsISO;
     }

     ISISPathsISO = ISISPaths;
     */
    if (circuitType == L1_TYPE)
    {
//        if (this->L1ISISPathsISO != NULL)
//        {
//            this->L1ISISPathsISO->clear();
//            delete this->L1ISISPathsISO;
//            this->L1ISISPathsISO->swap(*ISISPaths);
        //delete ISISPaths;
//        }else{
        this->L1ISISPathsISO = ISISPaths;
//        }
    }
    else if (circuitType == L2_TYPE)
    {
        if (this->L2ISISPathsISO != NULL)
        {
            delete this->L2ISISPathsISO;
        }
        this->L2ISISPathsISO = ISISPaths;
    }

    ISISPaths_t *areas = new ISISPaths_t;
    ISISPaths_t *ISISPathsISO = getPathsISO(circuitType);
    if (circuitType == L2_TYPE)
    {
        this->extractAreas(ISISPathsISO, areas, circuitType);
        std::cout << "Print Areas\n";
        this->printPaths(areas);

        for (ISISPaths_t::iterator it = areas->begin(); it != areas->end(); ++it)
            {

                if ((*it)->to[ISIS_SYSTEM_ID] != 0)
                {
                    //skip all pseudonodes, put only real nodes to routing table
                    continue;
                }
                //for every neighbour (nextHop)
                for (ISISNeighbours_t::iterator nIt = (*it)->from.begin(); nIt != (*it)->from.end(); ++nIt)
                {
                    //getAdjacency by systemID and circuitType, then find iface by gateIndex and finaly get the InterfaceEntry
                    //if it's a pseudonode then find interface based on matching DIS
                    if ((*nIt)->id[ISIS_SYSTEM_ID] != 0)
                    {
                        for (ISISInterTab_t::iterator tabIt = this->ISISIft.begin(); tabIt != this->ISISIft.end(); ++tabIt)
                        {
                            if (circuitType == L1_TYPE)
                            {
                                if (memcmp((*tabIt).L1DIS, (*nIt)->id, ISIS_SYSTEM_ID + 1) == 0)
                                {
                                    (*nIt)->entry = (*tabIt).entry;
                                }
                            }
                            else
                            {
                                if (memcmp((*tabIt).L2DIS, (*nIt)->id, ISIS_SYSTEM_ID + 1) == 0)
                                {
                                    (*nIt)->entry = (*tabIt).entry;
                                }
                            }
                        }

                    }
                    else
                    {

                        ISISadj *tmpAdj = getAdjBySystemID((*nIt)->id, circuitType);
                        if (tmpAdj != NULL)
                        {
                            (*nIt)->entry = this->getIfaceByGateIndex(tmpAdj->gateIndex)->entry;
                        }
                        else
                        {
                            (*nIt)->entry = NULL;
                        }
                    }
        //            (*nIt)->entry = this->getIfaceByGateIndex((this->getAdjBySystemID((*nIt)->id, circuitType))->gateIndex)->entry;
                }

                this->clnsTable->addRecord(new CLNSRoute((*it)->to, ISIS_SYSTEM_ID + 1, (*it)->from, (*it)->metric));

            }
    }

    //initiate search for closest attached L1_L2 IS
    if (!this->att)
    {
        this->setClosestAtt();
    }
//    this->clnsTable->dropTable();
    //TODO B1 rewrite this mess

    for (ISISPaths_t::iterator it = ISISPaths->begin(); it != ISISPaths->end(); ++it)
    {

        if ((*it)->to[ISIS_SYSTEM_ID] != 0)
        {
            //skip all pseudonodes, put only real nodes to routing table
            continue;
        }
        //for every neighbour (nextHop)
        for (ISISNeighbours_t::iterator nIt = (*it)->from.begin(); nIt != (*it)->from.end(); ++nIt)
        {
            //getAdjacency by systemID and circuitType, then find iface by gateIndex and finaly get the InterfaceEntry
            //if it's a pseudonode then find interface based on matching DIS
            if ((*nIt)->id[ISIS_SYSTEM_ID] != 0)
            {
                for (ISISInterTab_t::iterator tabIt = this->ISISIft.begin(); tabIt != this->ISISIft.end(); ++tabIt)
                {
                    if (circuitType == L1_TYPE)
                    {
                        if (memcmp((*tabIt).L1DIS, (*nIt)->id, ISIS_SYSTEM_ID + 1) == 0)
                        {
                            (*nIt)->entry = (*tabIt).entry;
                        }
                    }
                    else
                    {
                        if (memcmp((*tabIt).L2DIS, (*nIt)->id, ISIS_SYSTEM_ID + 1) == 0)
                        {
                            (*nIt)->entry = (*tabIt).entry;
                        }
                    }
                }

            }
            else
            {

                ISISadj *tmpAdj = getAdjBySystemID((*nIt)->id, circuitType);
                if (tmpAdj != NULL)
                {
                    (*nIt)->entry = this->getIfaceByGateIndex(tmpAdj->gateIndex)->entry;
                }
                else
                {
                    (*nIt)->entry = NULL;
                }
            }
//            (*nIt)->entry = this->getIfaceByGateIndex((this->getAdjBySystemID((*nIt)->id, circuitType))->gateIndex)->entry;
        }

        this->clnsTable->addRecord(new CLNSRoute((*it)->to, ISIS_SYSTEM_ID + 1, (*it)->from, (*it)->metric));

    }

    this->schedule(timer);
}
/**
 * Extracts areas from LSP DB for every path in @param paths
 * @param paths is set of paths to be searched
 * @param areas output variable to store extracted paths
 * @param circuitType is either L1 or L2
 */
void ISIS::extractAreas(ISISPaths_t* paths, ISISPaths_t* areas, short circuitType)
{

    ISISLSPPacket* lsp;
    LSPRecord* lspRec;
    TLV_t* tmpTLV;
    ISISPath *tmpPath;

    for (ISISPaths_t::iterator pathIt = paths->begin(); pathIt != paths->end(); ++pathIt)
    {
        //for each record in best paths extract connected Areas
        lspRec = this->getLSPFromDbByID((*pathIt)->to, circuitType);
        if(lspRec == NULL){
            continue;
        }
        lsp = lspRec->LSP;
        for (int offset = 0; (tmpTLV = this->getTLVByType(lsp, AREA_ADDRESS, offset)) != NULL; offset++)
        {
            for (unsigned int i = 0; i + ISIS_AREA_ID + 1 <= tmpTLV->length; i = +ISIS_AREA_ID + 1)
            {
                if ((tmpPath = this->getPath(areas, (*pathIt)->to)) == NULL)
                {
                    //path to this address doesn't exist, so create new
                    tmpPath = new ISISPath;
                    tmpPath->to = new unsigned char[ISIS_SYSTEM_ID + 2];
                    this->copyArrayContent(tmpTLV->value, tmpPath->to, ISIS_AREA_ID, i + 1, 0);
                    for (unsigned int b = ISIS_AREA_ID; b < ISIS_SYSTEM_ID + 2; b++)
                    {
                        tmpPath->to[b] = 0;
                    }
                    tmpPath->metric = (*pathIt)->metric;

                    //copy all neighbours from 'from'
                    for (ISISNeighbours_t::iterator nIt = (*pathIt)->from.begin(); nIt != (*pathIt)->from.end(); ++nIt)
                    {
                        tmpPath->from.push_back((*nIt)->copy());
                    }

                    areas->push_back(tmpPath);

                }
                else
                {
                    //path to this address already exists, so check metric
                    if (tmpPath->metric >= (*pathIt)->metric)
                    {
                        if (tmpPath->metric > (*pathIt)->metric)
                        {
                            //we got better metric for this area so clear 'from' neighbours
                            tmpPath->from.clear();
                        }
                        tmpPath->metric = (*pathIt)->metric;
                        //copy all neighbours from 'from'
                        for (ISISNeighbours_t::iterator nIt = (*pathIt)->from.begin(); nIt != (*pathIt)->from.end();
                                ++nIt)
                        {
                            tmpPath->from.push_back((*nIt)->copy());
                        }

                    }
                }
            }
        }
    }

}
/**
 * Sets attached system to closest system that advertise att flag
 */
void ISIS::setClosestAtt(void)
{

    uint32_t metric = UINT32_MAX;
    ISISNeighbours_t * attIS;

    if (this->att || this->L1ISISPathsISO == NULL)
    {
        return;
    }

    attIS = new ISISNeighbours_t;
    for (LSPRecQ_t::iterator it = this->L1LSPDb->begin(); it != this->L1LSPDb->end(); ++it)
    {
        if ((*it)->LSP->getPATTLSPDBOLIS() && 0x10)
        {
            //we found LSP with Attached flag set
            //find originator of this LSP in bestPaths
            for (ISISPaths_t::iterator pIt = this->L1ISISPathsISO->begin(); pIt != this->L1ISISPathsISO->end(); ++pIt)
            {
                unsigned char *lspID = this->getLspID((*it)->LSP);
                /* if LSP-ID match AND metric is smaller or same as the current one, add that IS as closest L1L2 IS */
                if (memcmp(lspID, (*pIt)->to, ISIS_SYSTEM_ID) == 0)
                {
                    if ((*pIt)->metric < metric)
                    {
                        //clear attIS
                        //TODO deallocate neighbours ID
                        attIS->clear();
                    }

                    if ((*pIt)->metric <= metric)
                    {
                        //TODO replace NULL with interfaceEntry
                        ISISNeighbour *att = new ISISNeighbour(lspID, false, NULL);
                        attIS->push_back(att);
//                        metric = (*pIt)->metric = metric;
                        metric = (*pIt)->metric;
                    }
                }

                delete[] lspID;
            }
        }
    }

    if (this->attIS != NULL && !this->attIS->empty())
    {
        this->attIS->clear();
    }
    this->attIS = attIS;

}

/*
 * Print best paths informations to stdout
 * @param paths is set of best paths.
 */
void ISIS::printPaths(ISISPaths_t *paths)
{

    std::cout << "Best paths of IS: ";
    //print area id
    for (unsigned int i = 0; i < ISIS_AREA_ID; i++)
    {
        std::cout << setfill('0') << setw(2) << dec << (unsigned int) this->areaId[i];
        if (i % 2 == 0)
            std::cout << ".";

    }

    //print system id
    for (unsigned int i = 0; i < 6; i++)
    {
        std::cout << setfill('0') << setw(2) << dec << (unsigned int) this->sysId[i];
        if (i % 2 == 1)
            std::cout << ".";
    }

    //print NSEL
    std::cout << setfill('0') << setw(2) << dec << (unsigned int) this->NSEL[0] << "\tNo. of paths: " << paths->size()
            << endl;
    for (ISISPaths_t::iterator it = paths->begin(); it != paths->end(); ++it)
    {
        std::cout << "To: ";
//        this->printSysId((*it)->to);

        for (unsigned int i = 0; i < 6; i++)
        {
            std::cout << setfill('0') << setw(2) << dec << (unsigned int) (*it)->to[i];
            if (i % 2 == 1)
                std::cout << ".";
        }


        std::cout << setfill('0') << setw(2) << dec << (unsigned short) (*it)->to[6];
        std::cout << "\t\t metric: " << (*it)->metric << "\t via: ";
        if ((*it)->from.empty())
        {
            std::cout << "EMPTY";
        }
        for (ISISNeighbours_t::iterator nIt = (*it)->from.begin(); nIt != (*it)->from.end(); ++nIt)
        {
//            std::cout << "\t\t\t\t\t";
            this->printSysId((*nIt)->id);
            std::cout << setfill('0') << setw(2) << dec << (unsigned short) (*nIt)->id[6];
            std::cout << " ";
        }
        std::cout << endl;
    }
    std::cout << endl;
}

/*
 * Moves best path from ISISTent to ISISPaths and initiate move of appropriate connections from init to ISISTent
 * @param initial is set of connections
 * @param ISISTent is set of tentative paths
 * @param ISISPaths is set of best paths from this IS
 */
void ISIS::bestToPath(ISISCons_t *init, ISISPaths_t *ISISTent, ISISPaths_t *ISISPaths)
{

    ISISPath *path;
    ISISPath *tmpPath;
    //sort it
    std::sort(ISISTent->begin(), ISISTent->end(), ISISPath());
    //save best in path
    path = ISISTent->front();
    //mov
    this->moveToTent(init, path, path->to, path->metric, ISISTent);

    ISISTent->erase(ISISTent->begin());

    if ((tmpPath = this->getPath(ISISPaths, path->to)) == NULL)
    {
        //path to this destination doesn't exist, so simply push

        ISISPaths->push_back(path);
    }
    else
    {
        if (tmpPath->metric >= path->metric)
        {
            if (tmpPath->metric > path->metric)
            {
                EV<< "ISIS: Error during SPF. We got better metric than the one PATHS." << endl;
                //we got better metric so clear "from" neighbours
                tmpPath->from.clear();
            }
            EV << "ISIS: Error during SPF. I think we shouldn't have neither same metric." << endl;
            //append
            tmpPath->metric = path->metric;
            cout << "pathb metric: " << tmpPath->metric << endl;
            for (ISISNeighbours_t::iterator it = path->from.begin(); it != path->from.end(); ++it)
            {
                tmpPath->from.push_back((*it));
            }

        }
    }

}

/* Moves connections with matching "from" from init to Tent
 * @param initial is set of connections
 * @param from specify which connections to move
 * @param metric is metric to get to "from" node
 * @param ISISTent is set of tentative paths
 */
void ISIS::moveToTent(ISISCons_t *initial, ISISPath *path, unsigned char *from, uint32_t metric, ISISPaths_t *ISISTent)
{

    ISISPath *tmpPath;
    ISISCons_t *cons = this->getCons(initial, from);
    /*       if(cons->empty()){
     EV <<"ISIS: Error during SPF. Didn't find my own LSP"<<endl;
     //TODO clean
     //           delete cons;
     //         return;
     }*/

    for (ISISCons_t::iterator consIt = cons->begin(); consIt != cons->end(); ++consIt)
    {
        if ((tmpPath = this->getPath(ISISTent, (*consIt)->to)) == NULL)
        {
            //path to this destination doesn't exist, so create new
            tmpPath = new ISISPath;
            tmpPath->to = new unsigned char[ISIS_SYSTEM_ID + 2];
            this->copyArrayContent((*consIt)->to, tmpPath->to, ISIS_SYSTEM_ID + 2, 0, 0);
            tmpPath->metric = (*consIt)->metric + metric;
//               cout << "patha metric: " << tmpPath->metric << endl;

            ISISNeighbour *neighbour = new ISISNeighbour;

            neighbour->id = new unsigned char[ISIS_SYSTEM_ID + 2];
            /* if @param from is THIS IS then next hop (neighbour->id) will be that next hop */
            if (this->compareArrays((*consIt)->from, (unsigned char *) this->sysId, ISIS_SYSTEM_ID)
                    && (*consIt)->from[ISIS_SYSTEM_ID] == 0)
            {
                this->copyArrayContent((*consIt)->to, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
                neighbour->entry = (*consIt)->entry;
                ASSERT(neighbour->entry != NULL);
                tmpPath->from.push_back(neighbour);
            }
            else
            {
                //TODO neighbour must be THIS IS or next hop, therefore we need to check whether directly connected
//                   this->copyArrayContent(path->from, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
                for (ISISNeighbours_t::iterator nIt = path->from.begin(); nIt != path->from.end(); ++nIt)
                {
                    //if nextHop (from) should be pseudonode, set nextHop as the "to" identifier
                    if ((*nIt)->id[ISIS_SYSTEM_ID] != 0)
                    {

                        ISISNeighbour *neigh = (*nIt)->copy();
                        memcpy(neigh->id, (*consIt)->to, ISIS_SYSTEM_ID + 2);
                        tmpPath->from.push_back(neigh);
                    }
                    else
                    {
//                       if(this->compareArrays((*nIt)->id, neighbour->id, ISIS_SYSTEM_ID + 2)){
                        //this neighbour is already there
//                           delete neighbour;
                        tmpPath->from.push_back((*nIt)->copy());
//                           return;
//                       }
                    }
                }
            }
            neighbour->type = false; //not a leaf
//               tmpPath->from = neighbour;

            ISISTent->push_back(tmpPath);
        }
        else
        {
            if (tmpPath->metric >= (*consIt)->metric + metric)
            {
                if (tmpPath->metric > (*consIt)->metric + metric)
                {
                    //we got better metric so clear "from" neighbours
                    tmpPath->from.clear();
                }
                //append
                tmpPath->metric = (*consIt)->metric + metric;
                cout << "path metric: " << tmpPath->metric << endl;
//                   ISISNeighbour *neighbour = new ISISNeighbour;
//                   neighbour->id = new unsigned char[ISIS_SYSTEM_ID + 2];
//                   this->copyArrayContent((*consIt)->from, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
//                   neighbour->type = false; //not a leaf

//                   for(ISISNeighbours_t::iterator nIt = tmpPath->from.begin(); nIt != tmpPath->from.end(); ++nIt){
//                       if(this->compareArrays((*nIt)->id, neighbour->id, ISIS_SYSTEM_ID + 2)){
//                           //this neighbour is already there
//                           delete neighbour;
//                           return;
//                       }
//                   }
//                   tmpPath->from.push_back(neighbour);

                for (ISISNeighbours_t::iterator nIt = path->from.begin(); nIt != path->from.end(); ++nIt)
                {
                    bool found = false;
                    for (ISISNeighbours_t::iterator neIt = tmpPath->from.begin(); neIt != tmpPath->from.end(); ++neIt)
                    {
                        //is such nextHop with matching ID AND entry (we may have adjacency with same IS over multiple interfaces)
                        if (this->compareArrays((*neIt)->id, (*nIt)->id, ISIS_SYSTEM_ID + 2)
                                && (*neIt)->entry == (*nIt)->entry)
                        {
                            //this neighbour is already there
//                             delete neighbour;
//                             continue;
                            found = true;
                            break;
                        }

                    }
                    if (!found)
                    {
                        //if such nextHop from @param path is not already present (not found), add it to nextHops(from)
                        tmpPath->from.push_back((*nIt)->copy());
                    }

//
                }

            }

        }

    }
}

/*
 * Returns pointer to besh path in provided vector.
 * @param paths is vector of paths
 * @return pointer to path with best metric
 */
ISISPath * ISIS::getBestPath(ISISPaths_t *paths)
{

    std::sort(paths->begin(), paths->end(),ISISPath());
    return paths->front();

}

void ISIS::getBestMetric(ISISPaths_t *paths)
{

    for (ISISPaths_t::iterator it = paths->begin(); it != paths->end(); ++it)
    {

    }
}

/*
 * This methods extract ISO information from LSP database.
 * @param initial is set of unidirectional connections
 * @return false if extractict fails
 */
bool ISIS::extractISO(ISISCons_t *initial, short circuitType)
{

  ISISLspDb_t *lspDb = this->getLSPDb(circuitType);
  unsigned char *lspId;

  ISISCon* connection;

  for (ISISLspDb_t::iterator it = lspDb->begin(); it != lspDb->end(); ++it)
  {

    //TODO if LSP's remaining time equals to zero then continue to next lsp;
    if ((*it)->LSP->getRemLifeTime() == 0 || (*it)->LSP->getSeqNumber() == 0)
    {
      continue;
    }
    //getLspId
    lspId = this->getLspID((*it)->LSP);

    //check if it's a zero-th fragment. if not try to find it -> getLspFromDbByID

    if (lspId[ISIS_SYSTEM_ID + 1] != 0)
    {
      unsigned char backup = lspId[ISIS_SYSTEM_ID + 1];
      lspId[ISIS_SYSTEM_ID + 1] = 0;
      //if doesn't exist -> continue to next lsp
      if (this->getLSPFromDbByID(lspId, circuitType) == NULL)
      {
        continue;
      }
      lspId[ISIS_SYSTEM_ID + 1] = backup;

    }
    //else


    TLV_t *tmpTLV;
    for (int offset = 0; (tmpTLV = this->getTLVByType((*it)->LSP, IS_NEIGHBOURS_LSP, offset)) != NULL; offset++)
    {
      for (unsigned int i = 1; i + 11 <= tmpTLV->length; i += 11)
      {
        connection = new ISISCon;
        connection->from = new unsigned char[ISIS_SYSTEM_ID + 2];
        this->copyArrayContent(lspId, connection->from, ISIS_SYSTEM_ID + 1, 0, 0);
        connection->from[ISIS_SYSTEM_ID + 1] = 0;
        connection->to = new unsigned char[ISIS_SYSTEM_ID + 2];
        this->copyArrayContent(tmpTLV->value, connection->to, ISIS_SYSTEM_ID + 1, i + 4, 0);
        connection->to[ISIS_SYSTEM_ID + 1] = 0;
        connection->metric = tmpTLV->value[i]; //default metric
        //                    cout << "connection metric : " << connection->metric << endl;
        connection->type = false; //it's not a leaf

        //if this LSP has been generated by this IS (lspId == sysId)
        if (memcmp(lspId, this->sysId, ISIS_SYSTEM_ID) == 0)
        {
          //TODO FIX
          //this destroys everything i was trying to accomplish by ingoring multiple adjacencies between two ISs
          //if destination of this connection (entry in LSP) is to Pseudonode
          if (connection->to[ISIS_SYSTEM_ID] != 0)
          {
            //if System-ID part of Pseudonode's LAN-ID belongs to this system
            if(memcmp(connection->to, this->sysId, ISIS_SYSTEM_ID) == 0){
              //then the connection entry index equals to Pseudonode-ID byte minus 1
              connection->entry = this->ISISIft.at(connection->to[ISIS_SYSTEM_ID] - 1).entry;
            }else{
              //if not, we have too search through Adjacency table to find adjacency with matching destination
              ISISadj *tmpAdj = getAdjBySystemID(connection->to, circuitType);
              if(!tmpAdj){
                return false;
              }
              connection->entry = getIfaceByGateIndex(tmpAdj->gateIndex)->entry;

              //                            connection->entry = this->ISISIft.at(connection->to[ISIS_SYSTEM_ID] - 1).entry;
            }
          }
          //if it is Pseudonode's LSP
          else if (lspId[ISIS_SYSTEM_ID] != 0)
          {
            connection->entry = this->ISISIft.at(lspId[ISIS_SYSTEM_ID] - 1).entry;
          }
          else
          {
            ISISadj *tmpAdj = getAdjBySystemID(connection->to, circuitType);
            connection->entry = getIfaceByGateIndex(tmpAdj->gateIndex)->entry;
          }
        }
        else
        {
          connection->entry = NULL;
        }

        initial->push_back(connection);
        //path->neighbours.push_back(neighbour);
      }

    }


  }

  this->twoWayCheck(initial);

  return true;
}

/*
 * This methods performs two-way check of reported connections.
 * @param cons is vector of connections.
 */
void ISIS::twoWayCheck(ISISCons_t *cons)
{
//    ISISCons_t *tmpCons;
    for (ISISCons_t::iterator it = cons->begin(); it != cons->end();)
    {
        //if there is not reverse connection
        //TODO is this enough? there could be two one-way connections between two ISs
        if (!this->isCon(cons, (*it)->to, (*it)->from))
        {
            it = cons->erase(it);
        }
        else
        {
            ++it;
        }
    }
}

/*
 * @return vector of connections with matching from in @param cons.
 */
ISISCons_t * ISIS::getCons(ISISCons_t *cons, unsigned char *from)
{

    ISISCons_t *retCon = new ISISCons_t;
    for (ISISCons_t::iterator it = cons->begin(); it != cons->end();)
    {
        if (this->compareArrays((*it)->from, from, 8))
        {
            retCon->push_back((*it));
            it = cons->erase(it);
        }
        else
        {
            ++it;
        }
    }

    return retCon;
}
/*
 * Check if connection from @param from to @param to exists.
 * @return true if there is connection with matching field from and to
 *
 */
bool ISIS::isCon(ISISCons_t *cons, unsigned char *from, unsigned char *to)
{

    for (ISISCons_t::iterator it = cons->begin(); it != cons->end(); ++it)
    {
        if (this->compareArrays((*it)->from, from, ISIS_SYSTEM_ID + 2)
                && this->compareArrays((*it)->to, to, ISIS_SYSTEM_ID + 2))
        {
            return true;
        }
    }
    return false;
}

/*
 * Returns path with specified id.
 * @param paths vector of paths
 * @param id is identificator of desired path
 * @return path
 */
ISISPath * ISIS::getPath(ISISPaths_t *paths, unsigned char *id)
{

    for (ISISPaths_t::iterator it = paths->begin(); it != paths->end(); ++it)
    {
        if (this->compareArrays((*it)->to, id, 8))
        {
            return (*it);
        }
    }

    return NULL;
}

void ISIS::setHelloCounter(unsigned long helloCounter)
{
    this->helloCounter = helloCounter;
}

void ISIS::setL1CSNPInterval(int l1CsnpInterval)
{
    L1CSNPInterval = l1CsnpInterval;
}

void ISIS::setL1HelloInterval(int l1HelloInterval)
{
    L1HelloInterval = l1HelloInterval;
}

void ISIS::setL1HelloMultiplier(short l1HelloMultiplier)
{
    L1HelloMultiplier = l1HelloMultiplier;
}

void ISIS::setL1LspGenInterval(int l1LspGenInterval)
{
    L1LspGenInterval = l1LspGenInterval;
}

void ISIS::setL1LspInitWait(int l1LspInitWait)
{
    L1LspInitWait = l1LspInitWait;
}

void ISIS::setL1LspSendInterval(int l1LspSendInterval)
{
    L1LspSendInterval = l1LspSendInterval;
}

void ISIS::setL1PSNPInterval(int l1PsnpInterval)
{
    L1PSNPInterval = l1PsnpInterval;
}

void ISIS::setL1SpfFullInterval(int l1SpfFullInterval)
{
    L1SPFFullInterval = l1SpfFullInterval;
}

void ISIS::setL2CSNPInterval(int l2CsnpInterval)
{
    L2CSNPInterval = l2CsnpInterval;
}

void ISIS::setL2HelloInterval(int l2HelloInterval)
{
    L2HelloInterval = l2HelloInterval;
}

void ISIS::setL2HelloMultiplier(short l2HelloMultiplier)
{
    L2HelloMultiplier = l2HelloMultiplier;
}

void ISIS::setL2LspGenInterval(int l2LspGenInterval)
{
    L2LspGenInterval = l2LspGenInterval;
}

void ISIS::setL2LspInitWait(int l2LspInitWait)
{
    L2LspInitWait = l2LspInitWait;
}

void ISIS::setL2LspSendInterval(int l2LspSendInterval)
{
    L2LspSendInterval = l2LspSendInterval;
}

void ISIS::setL2PSNPInterval(int l2PsnpInterval)
{
    L2PSNPInterval = l2PsnpInterval;
}

void ISIS::setL2SpfFullInterval(int l2SpfFullInterval)
{
    L2SPFFullInterval = l2SpfFullInterval;
}

void ISIS::setLspInterval(int lspInterval)
{
    this->lspInterval = lspInterval;
}

void ISIS::setLspMaxLifetime(int lspMaxLifetime)
{
    this->lspMaxLifetime = lspMaxLifetime;
}

void ISIS::appendISISInterface(ISISinterface iface)
{
    this->ISISIft.push_back(iface);
}

int ISIS::getL1HelloInterval() const
{
    return L1HelloInterval;
}

short ISIS::getL1HelloMultiplier() const
{
    return L1HelloMultiplier;
}

int ISIS::getL2HelloInterval() const
{
    return L2HelloInterval;
}

short ISIS::getL2HelloMultiplier() const
{
    return L2HelloMultiplier;
}

int ISIS::getL1CsnpInterval() const
{
    return L1CSNPInterval;
}

int ISIS::getL1PsnpInterval() const
{
    return L1PSNPInterval;
}

int ISIS::getL2CsnpInterval() const
{
    return L2CSNPInterval;
}

int ISIS::getL2PsnpInterval() const
{
    return L2PSNPInterval;
}

void ISIS::setL1CsnpInterval(int l1CsnpInterval)
{
    L1CSNPInterval = l1CsnpInterval;
}

void ISIS::setL1PsnpInterval(int l1PsnpInterval)
{
    L1PSNPInterval = l1PsnpInterval;
}

void ISIS::setL2CsnpInterval(int l2CsnpInterval)
{
    L2CSNPInterval = l2CsnpInterval;
}

ISIS::ISIS_MODE ISIS::getMode() const
{
    return mode;
}

void ISIS::setL2PsnpInterval(int l2PsnpInterval)
{
    L2PSNPInterval = l2PsnpInterval;
}

int ISIS::getLspInterval() const
{
    return lspInterval;
}

short ISIS::getIsType() const
{
    return isType;
}

const unsigned char *ISIS::getSysId() const
{
    return sysId;
}

void ISIS::setLspRefreshInterval(int lspRefreshInterval)
{
    this->lspRefreshInterval = lspRefreshInterval;
}

void ISIS::setIsType(short isType)
{
    this->isType = isType;
}

int ISIS::getNickname() const
{
    return nickname;
}

void ISIS::setAtt(bool att)
{
    this->att = att;
}

int ISIS::getISISIftSize(void)
{
    return this->ISISIft.size();
}

/**
 * Generate NET address based on first non-zero MAC address it could find in
 * interface table. In case there's not any non-zero MAC address, one is generated.
 * This method is for ISIS::L2_ISIS_MODE
 */
void ISIS::generateNetAddr()
{
    ift = InterfaceTableAccess().get();
//    unsigned char *a = new unsigned char[6];
    char *tmp = new char[25];
    MACAddress address;

    for (int i = 0; i < ift->getNumInterfaces(); i++)
    {
        if ((address = ift->getInterface(i)->getMacAddress()).getInt() != 0)
        {
            break;
        }

    }
    /* If there's not any interface with non-zero MAC address then generate one.
     * This is not likely to happen.*/
    if (address.getInt() == 0)
    {
        cout << "Warning: didn't get non-zero MAC address for NET generating" << endl;
        address.generateAutoAddress();
    }
//
//    this->areaId = new unsigned char[3];
//    this->sysId = new unsigned char[6];
//    this->NSEL = new unsigned char[1];

    this->netAddr = std::string(tmp);
    stringstream addressStream;
    addressStream << hex << address;
    string aS = addressStream.str();

//    this->areaId[0] = (unsigned char)atoi("49");
//    this->areaId[1] = (unsigned char)atoi("00");
//    this->areaId[2] = (unsigned char)atoi("01");

//    address.getAddressBytes(this->sysId);

//    this->NSEL[0] = (unsigned char)atoi("00");
//    this->sysId = string(aS.substr(0,2) + aS.substr(3,2) + aS.substr(0,2) + aS.substr(3,2)  ).c_str();
    this->netAddr = "00.0000." + aS.substr(0, 2) + aS.substr(3, 2) + "." + aS.substr(6, 2) + aS.substr(9, 2) + "."
            + aS.substr(12, 2) + aS.substr(15, 2) + ".00";

    if (!this->parseNetAddr())
    {
        throw cRuntimeError("Unable to parse auto-generated NET address.");
    }

}

