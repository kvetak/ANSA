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

ISIS::ISIS(){
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

}


ISIS::~ISIS()
{
    if(this->L1LSPDb != NULL){
        std::vector<LSPRecord *>::iterator it = this->L1LSPDb->begin();
        for(; it != this->L1LSPDb->end(); ++it){
            //delete (*it)->LSP;
            //delete (*it)->deadTimer;
            delete (*it);
        }
        this->L1LSPDb->clear();
        delete this->L1LSPDb;
    }

    if(this->L2LSPDb != NULL){
        std::vector<LSPRecord *>::iterator it = this->L2LSPDb->begin();
            for(; it != this->L2LSPDb->end(); ++it){
                //delete (*it)->LSP;
                //delete (*it)->deadTimer;
                delete (*it);
            }
            this->L2LSPDb->clear();
            delete this->L2LSPDb;
        }

    /* delete SRM */
    if(this->L1SRMBQueue != NULL){
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L1SRMBQueue->begin();
        for(; qIt != this->L1SRMBQueue->end(); ++qIt){
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for(; it != (*qIt)->end(); ++it){
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L1SRMBQueue->clear();
        delete this->L1SRMBQueue;
    }

    if(this->L1SRMPTPQueue != NULL){
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L1SRMPTPQueue->begin();
        for(; qIt != this->L1SRMPTPQueue->end(); ++qIt){
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for(; it != (*qIt)->end(); ++it){
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L1SRMPTPQueue->clear();
        delete this->L1SRMPTPQueue;
    }

    if(this->L2SRMBQueue != NULL){
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L2SRMBQueue->begin();
        for(; qIt != this->L2SRMBQueue->end(); ++qIt){
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for(; it != (*qIt)->end(); ++it){
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L2SRMBQueue->clear();
        delete this->L2SRMBQueue;
    }

    if(this->L2SRMPTPQueue != NULL){
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L2SRMPTPQueue->begin();
        for(; qIt != this->L2SRMPTPQueue->end(); ++qIt){
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for(; it != (*qIt)->end(); ++it){
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
    if(this->L1SSNBQueue != NULL){
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L1SSNBQueue->begin();
        for(; qIt != this->L1SSNBQueue->end(); ++qIt){
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for(; it != (*qIt)->end(); ++it){

                delete (*it);

            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L1SSNBQueue->clear();
        delete this->L1SSNBQueue;
    }

    if(this->L1SSNPTPQueue != NULL){
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L1SSNPTPQueue->begin();
        for(; qIt != this->L1SSNPTPQueue->end(); ++qIt){
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for(; it != (*qIt)->end(); ++it){
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L1SSNPTPQueue->clear();
        delete this->L1SSNPTPQueue;
    }

    if(this->L2SSNBQueue != NULL){
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L2SSNBQueue->begin();
        for(; qIt != this->L2SSNBQueue->end(); ++qIt){
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for(; it != (*qIt)->end(); ++it){
                delete (*it);
            }
            (*qIt)->clear();
            delete (*qIt);
        }
        this->L2SSNBQueue->clear();
        delete this->L2SSNBQueue;
    }

    if(this->L2SSNPTPQueue != NULL){
        std::vector<std::vector<FlagRecord*> *>::iterator qIt = this->L2SSNPTPQueue->begin();
        for(; qIt != this->L2SSNPTPQueue->end(); ++qIt){
            std::vector<FlagRecord*>::iterator it = (*qIt)->begin();
            for(; it != (*qIt)->end(); ++it){
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

        //set lspInterval
        cXMLElement *cxlspInt = isisRouting->getFirstChildWithTag("LSP-Interval");
        if (cxlspInt == NULL || cxlspInt->getNodeValue() == NULL)
        {
            this->lspInterval = ISIS_LSP_INTERVAL;
        }
        else
        {
            this->lspInterval = atoi(cxlspInt->getNodeValue());
        }

        //set lspRefreshInterval
        cXMLElement *cxlspRefInt = isisRouting->getFirstChildWithTag("LSP-Refresh-Interval");
        if (cxlspRefInt == NULL || cxlspRefInt->getNodeValue() == NULL)
        {
            this->lspRefreshInterval = ISIS_LSP_REFRESH_INTERVAL;
        }
        else
        {
            this->lspRefreshInterval = atoi(cxlspRefInt->getNodeValue());
        }

        //set lspMaxLifetime
        cXMLElement *cxlspMaxLife = isisRouting->getFirstChildWithTag("LSP-Max-Lifetime");
        if (cxlspMaxLife == NULL || cxlspMaxLife->getNodeValue() == NULL)
        {
            this->lspMaxLifetime = ISIS_LSP_MAX_LIFETIME;
        }
        else
        {
            this->lspMaxLifetime = atoi(cxlspMaxLife->getNodeValue());
        }

        //set L1LspGenInterval (CISCO's
        cXMLElement *cxL1lspGenInt = isisRouting->getFirstChildWithTag("L1-LSP-Gen-Interval");
        if (cxL1lspGenInt == NULL || cxL1lspGenInt->getNodeValue() == NULL)
        {
            this->L1LspGenInterval = ISIS_LSP_GEN_INTERVAL;
        }
        else
        {
            this->L1LspGenInterval = atoi(cxL1lspGenInt->getNodeValue());
        }

        //set L2LspGenInterval
        cXMLElement *cxL2lspGenInt = isisRouting->getFirstChildWithTag("L2-LSP-Gen-Interval");
        if (cxL2lspGenInt == NULL || cxL2lspGenInt->getNodeValue() == NULL)
        {
            this->L2LspGenInterval = ISIS_LSP_GEN_INTERVAL;
        }
        else
        {
            this->L2LspGenInterval = atoi(cxL2lspGenInt->getNodeValue());
        }

        //set L1LspSendInterval
        cXMLElement *cxL1lspSendInt = isisRouting->getFirstChildWithTag("L1-LSP-Send-Interval");
        if (cxL1lspSendInt == NULL || cxL1lspSendInt->getNodeValue() == NULL)
        {
            this->L1LspSendInterval = ISIS_LSP_SEND_INTERVAL;
        }
        else
        {
            this->L1LspSendInterval = atoi(cxL1lspSendInt->getNodeValue());
        }

        //set L2LspSendInterval
        cXMLElement *cxL2lspSendInt = isisRouting->getFirstChildWithTag("L2-LSP-Send-Interval");
        if (cxL2lspSendInt == NULL || cxL2lspSendInt->getNodeValue() == NULL)
        {
            this->L2LspSendInterval = ISIS_LSP_SEND_INTERVAL;
        }
        else
        {
            this->L2LspSendInterval = atoi(cxL2lspSendInt->getNodeValue());
        }

        //set L1LspInitWait
        cXMLElement *cxL1lspInitWait = isisRouting->getFirstChildWithTag("L1-LSP-Init-Wait");
        if (cxL1lspInitWait == NULL || cxL1lspInitWait->getNodeValue() == NULL)
        {
            this->L1LspInitWait = ISIS_LSP_INIT_WAIT;
        }
        else
        {
            this->L1LspInitWait = atoi(cxL1lspInitWait->getNodeValue());
        }
        //set L2LspInitWait
        cXMLElement *cxL2lspInitWait = isisRouting->getFirstChildWithTag("L2-LSP-Init-Wait");
        if (cxL2lspInitWait == NULL || cxL2lspInitWait->getNodeValue() == NULL)
        {
            this->L2LspInitWait = ISIS_LSP_INIT_WAIT;
        }
        else
        {
            this->L2LspInitWait = atoi(cxL2lspInitWait->getNodeValue());
        }

        //set L1CsnpInterval
        cXMLElement *cxL1CsnpInt = isisRouting->getFirstChildWithTag("L1-CSNP-Interval");
        if (cxL1CsnpInt == NULL || cxL1CsnpInt->getNodeValue() == NULL)
        {
            this->L1CsnpInterval = ISIS_CSNP_INTERVAL;
        }
        else
        {
            this->L1CsnpInterval = atoi(cxL1CsnpInt->getNodeValue());
        }

        //set L2PsnpInterval
        cXMLElement *cxL2CsnpInt = isisRouting->getFirstChildWithTag("L2-CSNP-Interval");
        if (cxL2CsnpInt == NULL || cxL2CsnpInt->getNodeValue() == NULL)
        {
            this->L2CsnpInterval = ISIS_CSNP_INTERVAL;
        }
        else
        {
            this->L2CsnpInterval = atoi(cxL2CsnpInt->getNodeValue());
        }

        //set L1PsnpInterval
        cXMLElement *cxL1PsnpInt = isisRouting->getFirstChildWithTag("L1-PSNP-Interval");
        if (cxL1PsnpInt == NULL || cxL1PsnpInt->getNodeValue() == NULL)
        {
            this->L1PsnpInterval = ISIS_CSNP_INTERVAL;
        }
        else
        {
            this->L1PsnpInterval = atoi(cxL1PsnpInt->getNodeValue());
        }

        //set L2PsnpInterval
        cXMLElement *cxL2PsnpInt = isisRouting->getFirstChildWithTag("L2-PSNP-Interval");
        if (cxL2PsnpInt == NULL || cxL2PsnpInt->getNodeValue() == NULL)
        {
            this->L2PsnpInterval = ISIS_CSNP_INTERVAL;
        }
        else
        {
            this->L2PsnpInterval = atoi(cxL2PsnpInt->getNodeValue());
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


        //create SRMQueue for each interface (even though it would be used only for broadcast interfaces)
        for(std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it){
            this->L1SRMBQueue->push_back(new std::vector<FlagRecord *>);
        }

        for(std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it){
            this->L1SRMPTPQueue->push_back(new std::vector<FlagRecord *>);
        }

        //SSNflags
        for(std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it){
            this->L1SSNBQueue->push_back(new std::vector<FlagRecord *>);
        }

        for(std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it){
            this->L1SSNPTPQueue->push_back(new std::vector<FlagRecord *>);
        }


        //create SRMQueue for each interface (even though it would be used only for broadcast interfaces)
        for(std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it){
            this->L2SRMBQueue->push_back(new std::vector<FlagRecord *>);
        }

        for(std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it){
            this->L2SRMPTPQueue->push_back(new std::vector<FlagRecord *>);
        }

        //SSNflags
        for(std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it){
            this->L2SSNBQueue->push_back(new std::vector<FlagRecord *>);
        }

        for(std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it){
            this->L2SSNPTPQueue->push_back(new std::vector<FlagRecord *>);
        }

        //TODO
        this->L1SPFFullInterval = ISIS_SPF_FULL_INTERVAL;
        this->L2SPFFullInterval = ISIS_SPF_FULL_INTERVAL;


    }else if(stage == 4){
        this->initISIS();
        //        ISISTimer *timerMsg = new ISISTimer("ISIS Start", ISIS_START);
        //        this->schedule(timerMsg);
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

        cXMLElement *metric = intElement->getFirstChildWithTag("ISIS-Metric");
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
        default: {

            newIftEntry.circuitType = L1L2_TYPE;

            cXMLElement *circuitType = intElement->getFirstChildWithTag("ISIS-Circuit-Type");
            if (circuitType != NULL && circuitType->getNodeValue() != NULL)
            {
                if (strcmp(circuitType->getNodeValue(), "L2") == 0){
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
    cXMLElement *L1HelloInt = intElement->getFirstChildWithTag(
            "ISIS-L1-Hello-Interval");
    if (L1HelloInt == NULL || L1HelloInt->getNodeValue() == NULL) {
        newIftEntry.L1HelloInterval = this->L1HelloInterval;
    } else {
        newIftEntry.L1HelloInterval = atoi(L1HelloInt->getNodeValue());
    }

    //set L1 hello multiplier
    cXMLElement *L1HelloMult = intElement->getFirstChildWithTag(
            "ISIS-L1-Hello-Multiplier");
    if (L1HelloMult == NULL || L1HelloMult->getNodeValue() == NULL) {
        newIftEntry.L1HelloMultiplier = this->L1HelloMultiplier;
    } else {
        newIftEntry.L1HelloMultiplier = atoi(L1HelloMult->getNodeValue());
    }

    //set L2 hello interval in seconds
    cXMLElement *L2HelloInt = intElement->getFirstChildWithTag(
            "ISIS-L2-Hello-Interval");
    if (L2HelloInt == NULL || L2HelloInt->getNodeValue() == NULL) {
        newIftEntry.L2HelloInterval = this->L2HelloInterval;
    } else {
        newIftEntry.L2HelloInterval = atoi(L2HelloInt->getNodeValue());
    }

    //set L2 hello multiplier
    cXMLElement *L2HelloMult = intElement->getFirstChildWithTag(
            "ISIS-L2-Hello-Multiplier");
    if (L2HelloMult == NULL || L2HelloMult->getNodeValue() == NULL) {
        newIftEntry.L2HelloMultiplier = this->L2HelloMultiplier;
    } else {
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


    //TODO  priority is not needed for point-to-point
    //set priority of current DIS = me at start
    newIftEntry.L1DISpriority = newIftEntry.priority;
    newIftEntry.L2DISpriority = newIftEntry.priority;

    //set initial designated IS as himself
    this->copyArrayContent((unsigned char*)this->sysId, newIftEntry.L1DIS, ISIS_SYSTEM_ID, 0, 0);
    //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
    //newIftEntry.L1DIS[6] = entry->getInterfaceId() - 99;
    newIftEntry.L1DIS[ISIS_SYSTEM_ID] = newIftEntry.gateIndex + 1;
    //do the same for L2 DIS
    this->copyArrayContent((unsigned char*)this->sysId, newIftEntry.L2DIS, ISIS_SYSTEM_ID, 0, 0);
    //newIftEntry.L2DIS[6] = entry->getInterfaceId() - 99;
    newIftEntry.L2DIS[ISIS_SYSTEM_ID] = newIftEntry.gateIndex + 1;

    newIftEntry.passive = false;
    newIftEntry.entry = entry;
    this->ISISIft.push_back(newIftEntry);
}




/**
 * Schedule various per interface timers.
 */
void ISIS::initISIS()
{
    this->initHello();
    this->initGenerate();
    this->initRefresh(); //this could be called after at least one adjcency becomes UP
    this->initPeriodicSend();
    this->initCsnp(); //this could be called within initRefresh();
    this->initPsnp(); //see above
    this->initSPF();

/*
    ISISTimer *periodicSendL1 = new ISISTimer("Periodic send L1", PERIODIC_SEND);
    periodicSendL1->setIsType(L1_TYPE);
    this->schedule(periodicSendL1);

    ISISTimer *periodicSendL2 = new ISISTimer("Periodic send L2", PERIODIC_SEND);
    periodicSendL1->setIsType(L2_TYPE);
    this->schedule(periodicSendL2);

    //give network time to converge before first LSP is sent out (15 seconds)
    ISISTimer *LSPtimer = new ISISTimer("Send first LSP timer");
    LSPtimer->setTimerKind(L1LSP_REFRESH);
    scheduleAt(simTime() + 15.0, LSPtimer);

    //10 sec after first LSPs sent should be flooded CNSP packets
    ISISTimer *CSNPtimer = new ISISTimer("Send first CSNP packets");
    CSNPtimer->setTimerKind(CSNP_TIMER);
    scheduleAt(simTime() + 25.0, CSNPtimer);*/
}

void ISIS::initHello(){
    ISISTimer *timerMsg;
    ISISinterface *iface;
    for (unsigned int k = 0; k < this->ISISIft.size(); k++)
    {

        //schedule Hello timer per level => check if L1L2 on broadcast => schedule two timers
        //on PTP is L1L2 Hello valid timer
        iface = &(this->ISISIft.at(k));

        //don't schedule sending hello PDU on passive or not ISIS-enabled interface
        if(!iface->ISISenabled || iface->passive){
            continue;
        }


        if (iface->network && iface->circuitType == L1L2_TYPE)
        {
            timerMsg = new ISISTimer("Hello_L1_timer");
            timerMsg->setTimerKind(HELLO_TIMER);
            timerMsg->setIsType(L1_TYPE);
            timerMsg->setInterfaceIndex(k);
            this->schedule(timerMsg);

            timerMsg = new ISISTimer("Hello_L2_timer");
            timerMsg->setTimerKind(HELLO_TIMER);
            timerMsg->setIsType(L2_TYPE);
            timerMsg->setInterfaceIndex(k);
            this->schedule(timerMsg);

        }
        else
        {
            timerMsg = new ISISTimer("Hello_timer");
            timerMsg->setTimerKind(HELLO_TIMER);
            timerMsg->setIsType(iface->circuitType);
            timerMsg->setInterfaceIndex(k);
            this->schedule(timerMsg);

        }
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
        this->schedule(timerMsg);

        timerMsg = new ISISTimer("Generate LSPs timer");
        timerMsg->setTimerKind(GENERATE_LSP_TIMER);
        timerMsg->setIsType(L2_TYPE);
        this->schedule(timerMsg);

    }
    else
    {
        timerMsg = new ISISTimer("Generate LSPs timer");
        timerMsg->setTimerKind(GENERATE_LSP_TIMER);
        timerMsg->setIsType(this->isType);
        this->schedule(timerMsg);

    }
    EV << "ISIS: initGenerate()" << endl;

}
/*
 * Initial schedule of timer for refreshing LSPs
 */
void ISIS::initRefresh()
{
    ISISTimer *timerMsg;

    timerMsg = new ISISTimer("Refresh LSPs timer");
    timerMsg->setTimerKind(LSP_REFRESH);
    timerMsg->setIsType(this->isType);
    this->schedule(timerMsg);
    EV << "ISIS: initRefresh()" << endl;

}

void ISIS::initPeriodicSend()
{
    ISISTimer *timerMsg;

    if (this->isType == L1L2_TYPE)
    {
        timerMsg = new ISISTimer("Periodic send");
        timerMsg->setTimerKind(PERIODIC_SEND);
        timerMsg->setIsType(L1_TYPE);
        this->schedule(timerMsg);

        timerMsg = new ISISTimer("Periodic send");
        timerMsg->setTimerKind(PERIODIC_SEND);
        timerMsg->setIsType(L2_TYPE);
        this->schedule(timerMsg);
    }
    else
    {
        timerMsg = new ISISTimer("Periodic send");
        timerMsg->setTimerKind(PERIODIC_SEND);
        timerMsg->setIsType(this->isType);
        this->schedule(timerMsg);
    }
}


void ISIS::initCsnp()
{

    ISISTimer *timerMsg;
    ISISinterface *iface;
    for (unsigned int k = 0; k < this->ISISIft.size(); k++)
    {
        iface = &(this->ISISIft.at(k));

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
            this->schedule(timerMsg);

            timerMsg = new ISISTimer("CSNP L2");
            timerMsg->setTimerKind(CSNP_TIMER);
            timerMsg->setIsType(L2_TYPE);
            timerMsg->setInterfaceIndex(k);
            this->schedule(timerMsg);

        }
        else
        {
            timerMsg = new ISISTimer("CSNP");
            timerMsg->setTimerKind(CSNP_TIMER);
            timerMsg->setIsType(iface->circuitType);
            timerMsg->setInterfaceIndex(k);
            this->schedule(timerMsg);

        }
    }
}

void ISIS::initPsnp()
{

    ISISTimer *timerMsg;
    ISISinterface *iface;
    for (unsigned int k = 0; k < this->ISISIft.size(); k++)
    {
        iface = &(this->ISISIft.at(k));

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
            this->schedule(timerMsg);

            timerMsg = new ISISTimer("PSNP L2");
            timerMsg->setTimerKind(PSNP_TIMER);
            timerMsg->setIsType(L2_TYPE);
            timerMsg->setInterfaceIndex(k);
            this->schedule(timerMsg);

        }
        else
        {
            timerMsg = new ISISTimer("PSNP");
            timerMsg->setTimerKind(PSNP_TIMER);
            timerMsg->setIsType(iface->circuitType);
            timerMsg->setInterfaceIndex(k);
            this->schedule(timerMsg);

        }
    }
}


void ISIS::initSPF()
{
    ISISTimer *timerMsg;

    if (this->isType == L1L2_TYPE)
    {
        timerMsg = new ISISTimer("L1 SPF Full");
        timerMsg->setTimerKind(SPF_FULL);
        timerMsg->setIsType(L1_TYPE);
        this->schedule(timerMsg);

        timerMsg = new ISISTimer("L2 SPF Full");
        timerMsg->setTimerKind(SPF_FULL);
        timerMsg->setIsType(L2_TYPE);
        this->schedule(timerMsg);
    }
    else
    {
        timerMsg = new ISISTimer("SPF Full");
        timerMsg->setTimerKind(SPF_FULL);
        timerMsg->setIsType(this->isType);
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
            case (ISIS_START):
                this->initISIS();
                delete timer;
                break;

            case (HELLO_TIMER):
                this->sendHelloMsg(timer);
                break;

            case (NEIGHBOUR_DEAD):
                this->removeDeadNeighbour(timer);
                delete timer;
                break;

            case (GENERATE_LSP_TIMER):
                this->generateLSP(timer);
                break;

            case (LSP_REFRESH):
                this->refreshLSP(timer);
                break;

            case (LSP_DEAD):
                this->purgeLSP(this->getLspID(timer), timer->getIsType());

                //delete timer; don't delete timer, it's re-used for LSP_DELETE
                break;

            case (LSP_DELETE):
                this->deleteLSP(timer);
                this->drop(timer);
                delete timer;
                break;

            case (CSNP_TIMER):
                if (timer->getIsType() == L1_TYPE)
                {
                    this->sendL1Csnp(timer);
                }
                else if (timer->getIsType() == L2_TYPE)
                {
                    EV << "ISIS: Warning: Discarding CSNP_TIMER for level 2" << endl;
                    delete timer;
                }
                else
                {
                    EV << "ISIS: Warning: Discarding CSNP_TIMER for L1L2." << endl;
                    delete timer;
                }
                break;

            case (PSNP_TIMER):
                if (timer->getIsType() == L1_TYPE)
                {
                    this->sendL1Psnp(timer);
                }
                else if (timer->getIsType() == L2_TYPE)
                {
                    delete timer;
                    EV << "ISIS: Warning: Discarding PSNP_TIMER for level 2" << endl;
                }
                else
                {
                    delete timer;
                    EV << "ISIS: Warning: Discarding PSNP_TIMER for L1L2." << endl;
                }
                break;

            case (PERIODIC_SEND):
                this->periodicSend(timer, timer->getIsType());
                break;

            case (SPF_FULL):
                    this->fullSPF(timer);
            break;

            default:
                EV<< "ISIS: Warning: Received unsupported Timer type in handleMessage" <<endl;
                delete timer;
                break;
        }
    }
    else
    {

        //every (at least all Hello) message should be checked for matching system-ID length

        //shouldn't check this->isType, but rather circuitType on incoming interface
        ISISMessage *inMsg = check_and_cast<ISISMessage *>(msg);
        if(!this->isMessageOK(inMsg)){
            EV<< "ISIS: Warning: discarding message" <<endl;
            //TODO schedule event discarding message
            delete msg;
            return;
        }

        //get arrival interface
        int gateIndex = inMsg->getArrivalGate()->getIndex();
        ISISinterface * tmpIntf = this->getIfaceByGateIndex(gateIndex);
        if(tmpIntf == NULL){
            EV << "ISIS: ERROR: Couldn't find interface by gate index when ISIS::handleMessage" << endl;
            delete msg;
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
                                          << ": ISIS: WARNING: Discarding LAN_L1_HELLO message on unsupported circuit type interface "<< inMsg->getId() << endl;
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

            case (L1_LSP):
                if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
                {
                    //this->handleL1LSP(inMsg);
                    //this->printLSPDB();
//                    std::cout<<"Handle LSP on: ";
//                        this->printSysId((unsigned char *)this->sysId);
//                        std::cout<< endl;
                    //this->printLSP(check_and_cast<ISISLSPPacket *>(msg), "Handle L1_LSP");
                    this->handleL1Lsp(check_and_cast<ISISLSPPacket *>(msg));


                    //comment if printing of the link-state database is too disturbing
                    this->printLSPDB();
                }
                else
                {
                    EV
                                    << "deviceId "
                                    << deviceId
                                    << ": ISIS: WARNING: Discarding LAN_L1_HELLO message on unsupported circuit type interface "
                                    << inMsg->getId() << endl;
                }
                //delete inMsg; //TODO don't delete inMsg in new version
                break;

            case (L1_CSNP):
                if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
                {
                    //this->handleL1CSNP(inMsg);
                    this->handleL1Csnp(check_and_cast<ISISCSNPPacket *>(msg));
                    this->printLSPDB();
                }
                else
                {
                    EV
                                    << "deviceId "
                                    << deviceId
                                    << ": ISIS: WARNING: Discarding LAN_L1_HELLO message on unsupported circuit type interface\n"
                                    << inMsg->getId() << endl;
                }
                //delete inMsg;
                break;
            case (L1_PSNP):
                if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)
                {
                    this->handleL1Psnp(check_and_cast<ISISPSNPPacket *>(msg));
                    this->printLSPDB();
                    //delete inMsg;
                }
                break;
            default:
                EV  << "deviceId " << deviceId << ": ISIS: WARNING: Discarding unknown message type. Msg id: "
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
        EV << "ISIS: sendingBroadcastHello: " << endl;
        this->sendBroadcastHelloMsg(timer->getInterfaceIndex(), timer->getIsType());

    }
    else
    {
        EV << "ISIS: sendingPTPHello: " << endl;
        this->sendPTPHelloMsg(timer->getInterfaceIndex(), timer->getIsType());
    }
    //re-schedule timer
    this->schedule(timer);

}

/*
 * Send hello message on specified broadcast interface.
 * Packets contain IS_NEIGHBOURS_HELLO and AREA_ADDRESS TLVs.
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
    unsigned int tlvSize = helloL1->getTLVArraySize();
    helloL1->setTLVArraySize(tlvSize + 1); //resize array
    helloL1->setTLV(tlvSize, myTLV);

    tlvSize = helloL2->getTLVArraySize();
    helloL2->setTLVArraySize(tlvSize + 1); //resize array
    helloL2->setTLV(tlvSize, myTLV);


    //set NEIGHBOURS L1 TLV
    myTLV.type = IS_NEIGHBOURS_HELLO;
    myTLV.length = adjL1Table.size() * 6; //number of records * 6 (6 is size of system ID/MAC address)
    myTLV.value = new unsigned char[adjL1Table.size() * 6];

    for (unsigned int h = 0; h < adjL1Table.size(); h++)
    {
        this->copyArrayContent(adjL1Table.at(h).mac.getAddressBytes(), myTLV.value, 6, 0, h * 6);
    }

    tlvSize = helloL1->getTLVArraySize();
    helloL1->setTLVArraySize(tlvSize + 1);
    helloL1->setTLV(tlvSize, myTLV);

    //L2 neighbours
    myTLV.type = IS_NEIGHBOURS_HELLO;
    myTLV.length = adjL2Table.size() * 6; //number of records * 6 (6 is size of system ID/MAC address)
    myTLV.value = new unsigned char[adjL2Table.size() * 6];
    //L2 neighbours
    for (unsigned int h = 0; h < adjL2Table.size(); h++)
    {
        this->copyArrayContent(adjL2Table.at(h).mac.getAddressBytes(), myTLV.value, 6, 0, h * 6);
    }

    tlvSize = helloL2->getTLVArraySize();
    helloL2->setTLVArraySize(tlvSize + 1);
    helloL2->setTLV(tlvSize, myTLV);


    //PADDING TLV is omitted

    //TODO Authentication TLV

    //TODO add eventually another TLVs (eg. from RFC 1195)

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
                        << "'devideId :" << deviceId << " ISIS: L1 Hello packet was sent from "
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
                * Mainly in initISIS and rescheduling new events.
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

    delete helloL1;
    delete helloL2; //TODO
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
    //TODO we check appropriate level adjacency table, but what to do for L1L2? In such case there's should be adjacency in both tables so we check just L1
    if ((tempAdj = this->getAdjByGateIndex(ISISIft.at(k).gateIndex, circuitType) ) != NULL)
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


    this->send(ptpHello,"ifOut", ISISIft.at(k).gateIndex);


}

void ISIS::schedule(ISISTimer *timer, double timee)
{

    double timeAt;
    double randomTime;


    switch (timer->getTimerKind())
    {
        case (ISIS_START):
            this->scheduleAt(0, timer);
            break;

        case (HELLO_TIMER):
            timeAt = this->getHelloInterval(timer->getInterfaceIndex(), timer->getIsType());
            randomTime = uniform(0, 0.25 * timeAt);
            this->scheduleAt(simTime() + timeAt - randomTime, timer);
            EV << "ISIS::schedule: timeAt: " << timeAt << " randomTime: " << randomTime << endl;
            break;

        case (NEIGHBOUR_DEAD):
        if(timee < 0){
                            EV << "ISIS: Warning: You forgot provide additional time to schedule for NEIGHBOUR_DEAD" << endl;
                            timee = 0;
                        }
            timeAt = timee;
            randomTime = uniform(0, 0.25 * timeAt);
            this->scheduleAt(simTime() + timeAt - randomTime, timer);
            break;

        case (PERIODIC_SEND):
            if (timer->getIsType() == L1_TYPE)
            {
                timeAt = this->L1LspSendInterval;
            }
            else if (timer->getIsType() == L2_TYPE)
            {
                timeAt = this->L2LspSendInterval;
            }
            else
            {
                EV << "ISIS: Warning: Unsupported IS-Type in schedule for PERIODIC_SEND" << endl;
            }

            randomTime = uniform(0, 0.25 * timeAt);
            this->scheduleAt(simTime() + timeAt - randomTime, timer);
            break;

        case (LSP_REFRESH):
            timeAt = this->lspRefreshInterval;
            randomTime = uniform(0, 0.25 * timeAt);
            this->scheduleAt(simTime() + timeAt - randomTime, timer);
            break;

        case (LSP_DEAD):
                if(timee < 0){
                    EV << "ISIS: Warning: You forgot provide additional time to schedule for LSP_DEAD" << endl;
                    timee = 0;
                }
            timeAt = timee;
            //randomTime = uniform(0, 0.25 * timeAt);
            this->scheduleAt(simTime() + timeAt, timer);
            break;

        case (LSP_DELETE):
            timeAt = this->lspMaxLifetime * 2;
            this->scheduleAt(simTime() + timeAt, timer);
            break;

        case (CSNP_TIMER):
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
                this->scheduleAt(simTime() + timeAt - randomTime, timer);
                break;
        case (PSNP_TIMER):
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
                this->scheduleAt(simTime() + timeAt - randomTime, timer);
                break;

        case (L1_CSNP):
            timeAt = this->ISISIft.at(timer->getInterfaceIndex()).L1CsnpInterval;
            randomTime = uniform(0, 0.25 * timeAt);
            this->scheduleAt(simTime() + timeAt - randomTime, timer);

            break;

        case (L1_PSNP):
            timeAt = this->ISISIft.at(timer->getInterfaceIndex()).L1PsnpInterval;
            randomTime = uniform(0, 0.25 * timeAt);
            this->scheduleAt(simTime() + timeAt - randomTime, timer);

            break;

        case (GENERATE_LSP_TIMER):
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
            }else{
                EV << "ISIS: ERROR: Unsupported IS-Type in PERIODIC_SEND timer" << endl;
            }
            randomTime = uniform(0, 0.25 * timeAt);
            this->scheduleAt(simTime() + timeAt - randomTime, timer);

            break;

        case (SPF_FULL):
                if(timer->getIsType() == L1_TYPE){
                   timeAt = this->L1SPFFullInterval;
                }else if(timer->getIsType() == L2_TYPE){
                    timeAt = this->L2SPFFullInterval;
                }else{
                    EV<<"ISIS: Error Unsupported IS-Type in SPF_FULL timer" <<endl;
                }
                randomTime = uniform(0, 0.25 * timeAt);
                this->scheduleAt(simTime() + timeAt - randomTime, timer);
                break;

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
        //scheduleAt(simTime() + msg->getHoldTime(), tmpAdj->timer);
        this->schedule(tmpAdj->timer, msg->getHoldTime());

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
                        //this->sendMyL1LSPs();
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
        for (unsigned int i = 0; i < ISIS_AREA_ID; i++){
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
                //TODO set interfaceIndex
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

                //set network type
                neighbour.network = this->getIfaceByGateIndex(neighbour.gateIndex)->network;

                this->schedule(neighbour.timer, msg->getHoldTime());
                //scheduleAt(simTime() + msg->getHoldTime(), neighbour.timer);

                //insert neighbour into adjL1Table
                adjL2Table.push_back(neighbour);
                std::sort(this->adjL2Table.begin(), this->adjL2Table.end());

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
                            //this->sendMyL1LSPs();
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
                                //this->sendMyL2LSPs();
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
        adjTable = &(this->adjL2Table);
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
                if(circuitType != L2_TYPE){ //shoudn't it be "!="
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
                delete systemID;
                return &(*it);
            }
        }
    }
    delete systemID;
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
    if (msg->getType() == LAN_L1_HELLO)
    {
        ISISL1HelloPacket *l1hello = check_and_cast<ISISL1HelloPacket *>(msg);
        for (int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            systemID[i] = l1hello->getSourceID(i);
        }
    }
    else if (msg->getType() == LAN_L2_HELLO)
    {
        ISISL2HelloPacket *l2hello = check_and_cast<ISISL2HelloPacket *>(msg);
        for (int i = 0; i < ISIS_SYSTEM_ID; i++)
        {
            systemID[i] = l2hello->getSourceID(i);
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
    else if (msg->getType() == L1_CSNP)
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
        EV << "ISIS: ERROR: getSysID for this message type is not implemented (yet?): "<< msg->getType() << endl;
    }


    return systemID;

}

unsigned char* ISIS::getSysID(ISISTimer *timer){
    unsigned char *systemID = new unsigned char[ISIS_SYSTEM_ID];

    for (int i = 0; i < ISIS_SYSTEM_ID; i++)
    {
        systemID[i] = timer->getSysID(i);
    }

    return systemID;
}

unsigned char* ISIS::getLspID(ISISTimer *timer){
    unsigned char *lspID = new unsigned char[ISIS_SYSTEM_ID + 2];

    for (int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
    {
        lspID[i] = timer->getLSPid(i);
    }

    return lspID;
}

/*

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
*/

unsigned char* ISIS::getLspID(ISISLSPPacket *msg){

    unsigned char *lspId = new unsigned char[8]; //TODO change back to ISIS_SYSTEM_ID + 2

    for (int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
    {
        lspId[i] = msg->getLspID(i);
    }

    return lspId;

}

void ISIS::setLspID(ISISLSPPacket *msg, unsigned char * lspId){

    for (int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
    {
       msg->setLspID(i, lspId[i]);
    }

}

/**
 * Print L1 and L2 adjacency tables to EV.
 * This function is currently called every time hello packet is received and processed.
 * @see handleMessage(cMessage* msg)
 */
void ISIS::printAdjTable()
{
    std::sort(this->adjL1Table.begin(), this->adjL1Table.end());
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
    short circuitType = L1_TYPE;
    std::vector<LSPRecord *> *lspDb = this->getLSPDb(circuitType);
    EV << "L1 LSP database of IS ";

    //print area id
    for (unsigned int i = 0; i < 3; i++)
    {
        EV << setfill('0') << setw(2) << dec << (unsigned int) areaId[i];
        if (i % 2 == 0)
            EV << ".";

    }

    //print system id
    for (unsigned int i = 0; i < 6; i++)
    {
        EV << setfill('0') << setw(2) << dec << (unsigned int) sysId[i];
        if (i % 2 == 1)
            EV << ".";
    }

    //print NSEL
    EV
            << setfill('0') << setw(2) << dec << (unsigned int) NSEL[0] << "\tNo. of records in database: "
                    << lspDb->size() << endl;
    unsigned char *lspId;
    std::vector<LSPRecord *>::iterator it = lspDb->begin();
    for (; it != lspDb->end(); ++it)
    {
        EV << "\t";
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
        EV<< "\t" << setfill('0') << setw(5) << dec << (*it)->LSP->getRemLifeTime() <<endl;
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

                EV
                        << "\tmetric: " << setfill('0') << setw(2) << dec
                                << (unsigned int) tmpTlv->value[m + 0] << endl;
            }

        }

    }

    /* EV << "L1 LSP database of IS ";

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


 *
 */

    //TODO print L2 LSP DB
}

void ISIS::printLSP(ISISLSPPacket *lsp, char *from){
    std::cout<<"PrintLSP called from: " << from << endl;
    unsigned char * lspId = this->getLspID(lsp);
    std::cout << "Printing LSP: ";
    this->printLspId(lspId);
    std::cout << "Print LSP->test:";
    this->printLspId((unsigned char *)lsp->getTest());
    std::cout << "seqNum: " << lsp->getSeqNumber() <<endl;
    std::cout << "Length of TLVarray: " << lsp->getTLVArraySize()<<endl;
    std:: cout << "TLV: " <<endl;
    for(unsigned int i = 0; i < lsp->getTLVArraySize(); i++){
        std::cout<< "Type: "<< (unsigned short) lsp->getTLV(i).type <<endl;
        std::cout<< "Length: "<< (unsigned short) lsp->getTLV(i).length <<endl;
    }


    TLV_t *tmpTlv;
    for (unsigned int k = 0; (tmpTlv = this->getTLVByType(lsp, IS_NEIGHBOURS_LSP, k)) != NULL; k++)
    {
        std::cout <<"Start printing TLV of length: "<< (unsigned int)tmpTlv->length << endl;
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

void ISIS::printLspId(unsigned char *lspId){

    this->printSysId(lspId);
    std::cout << setfill('0') << setw(2) << dec << (unsigned int) lspId[ISIS_SYSTEM_ID] ;
    std::cout << "-";
    std::cout << setfill('0') << setw(2) << dec << (unsigned int) lspId[ISIS_SYSTEM_ID + 1] << endl ;
}

void ISIS::printSysId(unsigned char *sysId){


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

    /* Check it and rewrite it */
    EV << "ISIS: Warning: RemoveDeadNeighbour: If everything is working correctly, this method shoudn't be called" <<endl;

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
                adjL1Table.erase(adjL1Table.begin() + i);
            }else{
                i++;
            }
        }
        std::sort(this->adjL1Table.begin(), this->adjL1Table.end());

        unsigned char *lspId = this->getLspID(msg);
        this->purgeRemainLSP(lspId, msg->getIsType());


        if(this->ISISIft.at(msg->getInterfaceIndex()).network){
            //lspId has two bytes more than we need (that does no harm)
            this->resetDIS(lspId, msg->getInterfaceIndex(), msg->getIsType());
        }
        delete lspId;

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

            // WHY must area ID also match??

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
        std::sort(this->adjL2Table.begin(), this->adjL2Table.end());

        unsigned char *lspId = this->getLspID(msg);
        this->purgeRemainLSP(lspId, msg->getIsType());

        if(this->ISISIft.at(msg->getInterfaceIndex()).network){
            //lspId has two bytes more than we need (that does no harm)
            this->resetDIS(lspId, msg->getInterfaceIndex(), msg->getIsType());
        }

        delete lspId;


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
    /* TODO Please rewrite this mess. */

//    Ieee802Ctrl *ctrl = check_and_cast <Ieee802Ctrl *> (msg->getControlInfo());
    short circuitType = L1_TYPE;
    unsigned int i;
    for(i=0; i<ISISIft.size(); i++)
    {
        if(ISISIft.at(i).gateIndex == msg->getArrivalGate()->getIndex())
        {
            if(ISISIft.at(i).gateIndex != i){
                EV << "ISIS: Warning: Houston, we got a problem! A BIG ONE!" << endl;
            }
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
            //break;
        }
    }
    if(equal){
        //DIS-ID from IIH and local DIS in interface->L1DIS is same so we don't need to elect anybody
        return;

    }

//    bool last = this->amIL1DIS(i);
    unsigned char* lastDIS = new unsigned char[ISIS_SYSTEM_ID + 1];
    for (unsigned int k = 0; k < msg->getLanIDArraySize(); k++)
    {
        lastDIS[k] = ISISIft.at(i).L1DIS[k];
    }

    MACAddress localDIS, receivedDIS;
    ISISadj *tmpAdj;

//    MACAddress tmpMAC = ISISIft.at(i).entry->getMacAddress();


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
        receivedDIS = MACAddress("000000000000");
    }


    //if announced DIS priority is higher then actual one or if they are equal and src MAC is higher than mine, then it's time to update DIS
    if ((!equal)
            && ((msg->getPriority() > ISISIft.at(i).L1DISpriority)
                    || (msg->getPriority() == ISISIft.at(i).L1DISpriority
                            && (receivedDIS.compareTo(localDIS) > 0) )))
    {
        unsigned char * disLspID = new unsigned char [ISIS_SYSTEM_ID + 2];
        this->copyArrayContent(lastDIS, disLspID, ISIS_SYSTEM_ID + 1, 0, 0);
        disLspID[ISIS_SYSTEM_ID + 1] = 0;//set fragment-ID
        //purge lastDIS's LSPs
        this->purgeRemainLSP(disLspID, circuitType);

        for (unsigned int j = 0; j < msg->getLanIDArraySize(); j++)
        {
            //set new DIS
            ISISIft.at(i).L1DIS[j] = msg->getLanID(j);
        }
        //and set his priority
        ISISIft.at(i).L1DISpriority = msg->getPriority();

        //purge DIS LSP
        //clear LSP containing dead neighbour

/*        for (unsigned int it = 0; it < L1LSP.size();)
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
        }*/

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
void ISIS::resetDIS(unsigned char* systemID, int gateIndex, short circuitType)
{

ISISinterface *iface = &(this->ISISIft.at(gateIndex));
    if(circuitType == L1_TYPE || circuitType == L1L2_TYPE)
    {
        //if the systemID was DIS, then set DIS = me. if dead neighbour wasn't DIS do nothing.
        if(this->compareArrays(systemID, iface->L1DIS, ISIS_SYSTEM_ID + 1)){
            //set myself as DIS
            iface->L1DISpriority = iface->priority;
            //set initial designated IS as himself
            this->copyArrayContent((unsigned char*)this->sysId, iface->L1DIS, ISIS_SYSTEM_ID, 0, 0);
            //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
            iface->L1DIS[ISIS_SYSTEM_ID] = iface->gateIndex + 1;
        }
    }

    if(circuitType == L2_TYPE || circuitType == L1L2_TYPE)
    {
        //if the systemID was DIS, then set DIS = me. if dead neighbour wasn't DIS do nothing.
        if(this->compareArrays(systemID, iface->L2DIS, ISIS_SYSTEM_ID + 1)){
            //set myself as DIS
            iface->L2DISpriority = iface->priority;
            //set initial designated IS as himself
            this->copyArrayContent((unsigned char*)this->sysId, iface->L2DIS, ISIS_SYSTEM_ID, 0, 0);
            //set LAN identifier; -99 is because, OMNeT starts numbering interfaces from 100 -> interfaceID 100 means LAN ID 0; and we want to start numbering from 1
            iface->L2DIS[ISIS_SYSTEM_ID] = iface->gateIndex + 1;
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
    EV << "ISIS: Warning: Running deprecated method" << endl;
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
    myTLV.length = ISIS_AREA_ID;
    myTLV.value = new unsigned char[ISIS_AREA_ID];
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
                if(this->compareArrays(L1LSP.at(i).LSPid, (unsigned char*)this->sysId, ISIS_SYSTEM_ID))
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
    timer->setTimerKind(LSP_REFRESH);
    scheduleAt(simTime() + 18.0, timer);//TODO

    delete LSP;
}

/**
 * Not implemented yet
 * @see sendMyL1LSPs()
 */
void ISIS::sendMyL2LSPs()
{
    EV << "ISIS: Warning: Running deprecated method" << endl;
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
    EV << "ISIS: Warning: Running deprecated method" << endl;
    ISISLSPL1Packet *msg = check_and_cast<ISISLSPL1Packet *>(inMsg);

    //TODO if the Remaining Life Time is 0 a.k.a purge LSP then no TLV is present and this check should be omitted
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
                    record.deadTimer->setTimerKind(LSP_DEAD);

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

void ISIS::handleL1Lsp(ISISLSPPacket *lsp){

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
    int circuitType = L1_TYPE;
    int gateIndex = lsp->getArrivalGate()->getIndex();
    /* 7.3.15.1. a) 6) */
    //returns true if for source-id in LSP there is adjacency with matching MAC address
    if(!this->isAdjUp(lsp, circuitType)){
        //no adjacency for source-id => discard lsp
        //generate event?

        EV <<"ISIS: Warning: Discarding LSP: didn't find adjacency in state UP for this LSP" <<endl;
        delete lsp;
        return;
    }

    lspID = this->getLspID(lsp);

    /* 7.3.15.1. b */
    if(lsp->getRemLifeTime() == 0){

        this->purgeLSP(lsp, circuitType);
        /* lsp is already deleted in purgeLSP */
        //delete lsp;
        delete lspID;
        return;


    }
    LSPRecord *lspRec;
    //is it my LSP?
    if(this->compareArrays(lspID, (unsigned char*) this->sysId, ISIS_SYSTEM_ID)){

        //if i don't have it anymore
        /* 7.3.15.1 c) i don't have it in DB */
        if((lspRec = this->getLSPFromDbByID(lspID, circuitType)) == NULL){
            //init network wide purge
            this->purgeLSP(lsp, circuitType);
            //delete lsp;
            delete lspID;
            return;
        }else{
            /* 7.3.15.1 c) OR no longer in the set of LSPs generated by this system */
            if (lspRec->deadTimer->getTimerKind() == LSP_DELETE) //TODO improve this
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
            if(lsp->getSeqNumber() > lspRec->LSP->getSeqNumber()){
//                std::cout<<"handle seqNum: "<< lspRec->LSP->getSeqNumber() <<endl;
                lspRec->LSP->setSeqNumber(lsp->getSeqNumber() + 1); //TODO handle overflow of seqNumer
//                std::cout<<"handle seqNum: "<< lspRec->LSP->getSeqNumber() <<endl;
                this->setSRMflags(lspRec, circuitType);
                //TODO set new remaining lifetime
                //TODO reschedule deadTimer



            }else{

                delete lsp;
                delete lspID;
                return;
            }


        }
    }else{
        /* 7.3.15.1 e) 1) */
        lspRec = this->getLSPFromDbByID(lspID, circuitType);
        if(lspRec == NULL ){
            /* 7.3.15.1 e) 1) i. */
            this->installLSP(lsp, circuitType);
            delete lspID;
            return;

        }else{
            if(lsp->getSeqNumber() > lspRec->LSP->getSeqNumber()){
                /* 7.3.15.1 e) 1) i. */
                this->replaceLSP(lsp, lspRec, circuitType);
                delete lspID;
                return;

            }
            /* 7.3.15.1 e) 2) */
            /* should check also lsp->getRemLifetime != 0  OR both zero, but this is handled in purgeLSP*/
            else if(lsp->getSeqNumber() == lspRec->LSP->getSeqNumber() && lspRec->LSP->getRemLifeTime() != 0){

                /* 7.3.15.1 e) 2) i. */
                this->clearSRMflag(lspRec, gateIndex, circuitType);

                if (!this->ISISIft.at(gateIndex).network)
                {
                    /* 7.3.15.1 e) 2) ii. */
                    this->setSSNflag(lspRec, gateIndex, circuitType);
                }


            }
            /* 7.3.15.1 e) 3) */
            else if (lsp->getSeqNumber() < lspRec->LSP->getSeqNumber()){
                /* 7.3.15.1 e) 3) i. */
                this->setSRMflag(lspRec, gateIndex, circuitType);
                /* 7.3.15.1 e) 3) ii. */
                this->clearSSNflag(lspRec, gateIndex, circuitType);
            }
        }


    }
    delete lsp;
    delete lspID;
}


/*
 * This method is not used and will be deleted.
 */
void ISIS::updateLSP(ISISLSPPacket *lsp, short circuitType){

    unsigned char *lspId;
    LSPRecord * tmpLSPRecord;
        lspId = this->getLspID(lsp);
    if((tmpLSPRecord = this->getLSPFromDbByID(lspId, circuitType)) == NULL){
        //installLSP
    }else{
        //we have that LSP
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

void ISIS::sendL1Csnp(ISISTimer *timer)
{
    //TODO don't know how to handle csnp over PtP yet (there is no periodic sending, but initial csnp is sent)
    /* Maybe send CSNP during some initial interval (or number of times, or just once) and then just don't re-schedule timer for this interface */
    if(!this->ISISIft.at(timer->getInterfaceIndex()).network || !this->amIL1DIS(timer->getInterfaceIndex())){
        this->schedule(timer);
        return;
    }
    short circuitType = L1_TYPE;

    std::vector<LSPRecord *> *lspDb = this->getLSPDb(circuitType);
    std::vector<LSPRecord *>::iterator it = lspDb->begin();
    for (int fragment = 0; it != lspDb->end(); fragment++)

    {
        ISISCSNPPacket *packet = new ISISCSNPPacket("L1 CSNP");
        packet->setType(L1_CSNP);
        packet->setLength(0); //TODO set to length of header

        //add Ethernet control info
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

        //set system ID field which consists of my system id + zero circuit id inc this case
        for (unsigned int i = 0; i < packet->getSourceIDArraySize() - 1; i++)
        {
            packet->setSourceID(i, this->sysId[i]);
        }
        packet->setSourceID(6, 0); //ORLY? have to be changed during sending according to interface this CSNP is being send on

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
                delete lspId;
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

            delete myTLV.value;

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
            EV << "ISIS: Warning: Houston, we got a problem! A BIG ONE!" << endl;
        }

        //send only on interface specified in timer
        send(packet, "ifOut", timer->getInterfaceIndex());

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
         send(packetDup, "ifOut", (*intIt).gateIndex);
         }
         }
         delete ctrl;
         delete packet;
         */
    }

    //reschedule timer
    this->schedule(timer);

}

void ISIS::sendL1Psnp(ISISTimer *timer){

    if(this->amIL1DIS(timer->getInterfaceIndex())){
        //reschedule OR make sure that during DIS election/resignation this timer is handled correctly
        //this is a safer, but dumber, solution
        //TODO see above
        this->schedule(timer);
        return;

    }
    short circuitType = L1_TYPE; //TODO get type from timer
    ISISinterface * iface = &(this->ISISIft.at(timer->getInterfaceIndex()));
    std::vector<FlagRecord*> *SSNQueue = this->getSSNQ(iface->network, iface->gateIndex, circuitType);

    //if queue is empty reschedule timer
    if(SSNQueue->empty()){
        this->schedule(timer);
        return;
    }



    ISISPSNPPacket *packet = new ISISPSNPPacket("L1 PSNP");
    packet->setType(L1_PSNP);

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
    for (unsigned int i = 0; i < packet->getSourceIDArraySize() - 1; i++)
    {
        packet->setSourceID(i, this->sysId[i]);
    }
    packet->setSourceID(6, 0);

    //TODO check that all fields of the packet are filled correctly

    unsigned int lspCount = SSNQueue->size();
    for(std::vector<FlagRecord*>::iterator it = SSNQueue->begin(); it != SSNQueue->end(); )
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
                        && (packet->getLength() + (myTLV.length + 16) + 2) < ISIS_LSP_MAX_SIZE; it = SSNQueue->begin(), i++)
        { //255 is maximum that can fit into Length field of TLV
            //add entry from lspDb

            //convert unsigned short to unsigned char array and insert to TLV
            myTLV.value[(i * 16)] = (((*it)->lspRec->LSP->getRemLifeTime() >> 8) & 0xFF);
            myTLV.value[(i * 16) + 1] = ((*it)->lspRec->LSP->getRemLifeTime() & 0xFF);


            //copy LSP ID to TLV
            unsigned char *lspId = this->getLspID((*it)->lspRec->LSP);
            this->copyArrayContent(lspId, myTLV.value, ISIS_SYSTEM_ID + 2, 0, (i * 16) + 2);
            delete lspId;
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
        delete myTLV.value;




    }

    if (timer->getInterfaceIndex() != this->getIfaceIndex(this->getIfaceByGateIndex(timer->getInterfaceIndex())))
    {
        EV << "ISIS: Warning: Houston, we got a problem! A BIG ONE!" << endl;
    }

    //send only on interface specified in timer
    send(packet, "ifOut", timer->getInterfaceIndex());

    this->schedule(timer);
}

void ISIS::handleL1Psnp(ISISPSNPPacket *psnp){

    short circuitType = L1_TYPE;
    int gateIndex = psnp->getArrivalGate()->getIndex();
    ISISinterface* intf = &(this->ISISIft.at(gateIndex));
    /* 7.3.15.2. a) If circuit C is a broadcast .. */
    if(intf->network && !this->amIL1DIS(gateIndex)){
        EV << "ISIS: Warning: Discarding PSNP. Received on nonDIS interface." <<endl;
        delete psnp;
        return;
    }
    
    /* 7.3.15.2 a) 6) */
    /* we handle broadcast and non-broadcast adjacencies same way */
    if (!this->isAdjUp(psnp, circuitType))
    {
        EV << "ISIS: Warning: Discarding PSNP. Didn't find matching adjacency." << endl;
        delete psnp;
        return;
    }

    /* 7.3.15.2 a) 7) */
    /* 7.3.15.2 a) 8) */
    /* Authentication is omitted */

    //for -> iterate over tlvArraySize and then iterate over TLV
//    for(int i = 0; i < psnp->getTLVArraySize(); i++)
    /* 7.3.15.2 b) */
    TLV_t * tmpTlv;
    unsigned char *tmpLspID;
    for(int offset = 0; (tmpTlv = this->getTLVByType(psnp, LSP_ENTRIES, offset)) != NULL; offset++){

        for(int i = 0; i < tmpTlv->length; i+=16)//TODO change 16 to something
        {
            tmpLspID = new unsigned char [ISIS_SYSTEM_ID + 2];
            this->copyArrayContent(tmpTlv->value, tmpLspID, (ISIS_SYSTEM_ID + 2), i + 2, 0);
            unsigned short remLife = tmpTlv->value[i] * 255 +tmpTlv->value[i + 1];
            /* this just makes me laugh */
            unsigned long seqNum = tmpTlv->value[i + 10] * 255 * 255 * 255 + tmpTlv->value[i + 11] * 255 *255 + tmpTlv->value[i + 12] * 255 + tmpTlv->value[i + 13];
            //getLspBySysID
            LSPRecord *lspRec;
            lspRec = this->getLSPFromDbByID(tmpLspID, circuitType);
            if(lspRec != NULL){
                /* 7.3.15.2 b) 2) */
                //if values are same
                if(seqNum == lspRec->LSP->getSeqNumber() && (remLife == lspRec->LSP->getRemLifeTime() || (remLife != 0 && lspRec->LSP->getRemLifeTime() !=0))){
                    //if non-broadcast -> clear SRMflag for C
                    if(!intf->network){
                        this->clearSRMflag(lspRec, gateIndex, circuitType);
                    }
                }
                /* 7.3.15.2 b) 3) */
                //else if received is older
                else if (seqNum < lspRec->LSP->getSeqNumber()){
                    //clean SSN and set SRM
                    this->clearSSNflag(lspRec, gateIndex, circuitType);
                    this->setSRMflag(lspRec, gateIndex, circuitType);
                }
                /* 7.3.15.2 b) 4) */
                //else if newer
                else if(seqNum > lspRec->LSP->getSeqNumber() || (seqNum == lspRec->LSP->getSeqNumber() && lspRec->LSP->getRemLifeTime() != 0) ){
                    //setSSNflag AND if C is non-broadcast clearSRM
                    this->setSSNflag(lspRec, gateIndex, circuitType);
                    if(!intf->network){
                        this->clearSRMflag(lspRec, gateIndex, circuitType);
                    }
                }
            }
            /* 7.3.15.2 b) 5) */
            else{
                //if remLifetime, checksum, seqNum are all non-zero
                if(remLife != 0 && seqNum != 0){
                    this->printSysId((unsigned char *)this->sysId);
                    std::cout<<"Received new LSP in PSNP";
                    this->printLspId(tmpLspID);
                    //create LSP with seqNum 0 and set SSNflag for C
                    //DO NOT SET SRMflag!!!!!!!
                    ISISLSPPacket *lsp = new ISISLSPPacket("LSP Packet");
                    lsp->setType(L1_LSP);
                    //remLifeTime
                    lsp->setRemLifeTime(remLife);
                    //lspID[8]
                    this->setLspID(lsp, tmpLspID);

                    //set seqNum
                    lsp->setSeqNumber(0);

                    //set PATTLSPDBOLIS
                    lsp->setPATTLSPDBOLIS(0x01);


                    //install new "empty" LSP and set SSNflag
                    this->setSSNflag( this->installLSP(lsp, circuitType), gateIndex, circuitType);

                }
            }
            delete tmpLspID;
        }

    }
    //if lsp-entry equals
    delete psnp;
}



void ISIS::handleL1Csnp(ISISCSNPPacket *csnp){

    short circuitType = L1_TYPE; //TODO get circuitType from csnp
    int gateIndex = csnp->getArrivalGate()->getIndex();
    ISISinterface* intf = &(this->ISISIft.at(gateIndex));


    /* 7.3.15.2 a) 6) */
    /* we handle broadcast and non-broadcast adjacencies same way */
    if (!this->isAdjUp(csnp, circuitType))
    {
        EV << "ISIS: Warning: Discarding CSNP. Didn't find matching adjacency." << endl;
        delete csnp;
        return;
    }

    /* 7.3.15.2 a) 7) */
    /* 7.3.15.2 a) 8) */
    /* Authentication is omitted */

    std::vector<unsigned char *>* lspRange;
    lspRange = this->getLspRange(this->getStartLspID(csnp),this->getEndLspID(csnp), circuitType);




    //for -> iterate over tlvArraySize and then iterate over TLV
//    for(int i = 0; i < csnp->getTLVArraySize(); i++)
    /* 7.3.15.2 b) */
    TLV_t * tmpTlv;
    unsigned char *tmpLspID;
    for(int offset = 0; (tmpTlv = this->getTLVByType(csnp, LSP_ENTRIES, offset)) != NULL; offset++)
    {

        for(int i = 0; i < tmpTlv->length; i+=16)//TODO change 16 to something
        {
            tmpLspID = new unsigned char [ISIS_SYSTEM_ID + 2];
            this->copyArrayContent(tmpTlv->value, tmpLspID, (ISIS_SYSTEM_ID + 2), i + 2, 0);
            unsigned short remLife = tmpTlv->value[i] * 255 +tmpTlv->value[i + 1];
            /* this just makes me laugh */
            unsigned long seqNum = tmpTlv->value[i + 10] * 255 * 255 * 255 + tmpTlv->value[i + 11] * 255 *255 + tmpTlv->value[i + 12] * 255 + tmpTlv->value[i + 13];

            /* 7.3.15.2 c) */
            if (!lspRange->empty())
            {
                while (memcmp(lspRange->front(), tmpLspID, ISIS_SYSTEM_ID + 2) < 0)
                {
                    this->setSRMflag(this->getLSPFromDbByID(lspRange->front(), circuitType), gateIndex, circuitType);
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
            if(lspRec != NULL){
                //if values are same
                if(seqNum == lspRec->LSP->getSeqNumber() && (remLife == lspRec->LSP->getRemLifeTime() || (remLife != 0 && lspRec->LSP->getRemLifeTime() !=0))){
                    //if non-broadcast -> clear SRMflag for C
                    if(!intf->network){
                        this->clearSRMflag(lspRec, gateIndex, circuitType);
                    }
                }
                //else if received is older
                else if (seqNum < lspRec->LSP->getSeqNumber()){
                    //clean SSN and set SRM
                    this->clearSSNflag(lspRec, gateIndex, circuitType);
                    this->setSRMflag(lspRec, gateIndex, circuitType);
                }
                //else if newer
                else if(seqNum > lspRec->LSP->getSeqNumber() || (seqNum == lspRec->LSP->getSeqNumber() && lspRec->LSP->getRemLifeTime() != 0) ){
                    //setSSNflag AND if C is non-broadcast clearSRM
                    this->setSSNflag(lspRec, gateIndex, circuitType);
                    if(!intf->network){
                        this->clearSRMflag(lspRec, gateIndex, circuitType);
                    }
                }
            }
            /* 7.3.15.2 b) 5) */
            else{
                //if remLifetime, checksum, seqNum are all non-zero
                if(remLife != 0 && seqNum != 0){
                    this->printSysId((unsigned char *) this->sysId);
                    std::cout << "Received new LSP in CSNP";
                    this->printLspId(tmpLspID);
                    //create LSP with seqNum 0 and set SSNflag for C
                    //DO NOT SET SRMflag!!!!!!!
                    ISISLSPPacket *lsp = new ISISLSPPacket("LSP Packet");
                    lsp->setType(L1_LSP);
                    //remLifeTime
                    lsp->setRemLifeTime(remLife);
                    //lspID[8]
                    this->setLspID(lsp, tmpLspID);

                    //set seqNum
                    lsp->setSeqNumber(0);

                    //set PATTLSPDBOLIS
                    lsp->setPATTLSPDBOLIS(0x01);


                    //install new "empty" LSP and set SSNflag
                    this->setSSNflag( this->installLSP(lsp, circuitType), gateIndex, circuitType);

                }
            }
            delete tmpLspID;
        }

    }

    while(!lspRange->empty()){
        this->setSRMflag(this->getLSPFromDbByID(lspRange->front(), circuitType), gateIndex, circuitType);
        delete lspRange->front();
        lspRange->erase(lspRange->begin());

    }
    delete lspRange;
    //if lsp-entry equals
    delete csnp;
}


std::vector<unsigned char *>* ISIS::getLspRange(unsigned char *startLspID, unsigned char * endLspID, short circuitType){

    int res1, res2;
    unsigned char * lspID;
    std::vector<LSPRecord *> *lspDb = this->getLSPDb(circuitType);
    std::vector<unsigned char*> *lspRange = new std::vector<unsigned char *>;
    std::sort(lspDb->begin(), lspDb->end());
    //TODO we can end the search before hitting lspDb->end when DB is sorted
    for (std::vector<LSPRecord *>::iterator it = lspDb->begin(); it != lspDb->end(); ++it)
    {
        lspID = this->getLspID((*it)->LSP);
        res1 = memcmp(startLspID, lspID, ISIS_SYSTEM_ID + 2);
        res2 = memcmp(lspID, endLspID, ISIS_SYSTEM_ID + 2);
        if(res1 <= 0 && res2 >= 0){
            lspRange->push_back(lspID);
        }
        delete lspID;
    }

    return lspRange;

}

unsigned char * ISIS::getStartLspID(ISISCSNPPacket *csnp){

        unsigned char *lspId = new unsigned char[ISIS_SYSTEM_ID + 2];

        for (int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
        {
            lspId[i] = csnp->getStartLspID(i);
        }

        return lspId;
}

unsigned char * ISIS::getEndLspID(ISISCSNPPacket *csnp){

        unsigned char *lspId = new unsigned char[ISIS_SYSTEM_ID + 2];

        for (int i = 0; i < ISIS_SYSTEM_ID + 2; i++)
        {
            lspId[i] = csnp->getEndLspID(i);
        }

        return lspId;
}

/**
 * Send CSNP packet to each LAN, where this router represents DIS.
 * Repeat every 10 seconds.
 */
void ISIS::sendL1CSNP()
{
    EV << "ISIS: Warning: Running deprecated method" << endl;
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
     * Value Multiples of LSP summaries, each consisting of the remaining lifetime (2 bytes), LSP ID (ID length + 2 bytes),
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
             * TLV Value Multiples of LSP summaries, each consisting of the remaining lifetime (2 bytes), LSP ID (ID length + 2 bytes),
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

    std::vector<std::vector<FlagRecord*>* > * SRMPTPQueue = getSRMPTPQueue(circuitType);
    std::vector<std::vector<FlagRecord*>* > * SRMBQueue = getSRMBQueue(circuitType);

    //std::cout << "Starting Periodic send for: ";
    //this->printSysId((unsigned char *) this->sysId);
    //std::cout << endl;

    //PTP circuits
    for(std::vector<std::vector<FlagRecord*>* >::iterator it = SRMPTPQueue->begin(); it != SRMPTPQueue->end(); ++it){
       for(std::vector<FlagRecord*>::iterator itRec = (*it)->begin(); itRec != (*it)->end(); ++itRec){

           //std::cout<<"sendLsp to:" << (*itRec)->index <<" : ";
               //this->printLspId(this->getLspID((*itRec)->lspRec->LSP));

               this->sendLSP((*itRec)->lspRec, (*itRec)->index);
               
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
    for(std::vector<std::vector<FlagRecord*>* >::iterator it = SRMBQueue->begin(); it != SRMBQueue->end(); ++it){
        int queueSize = (*it)->size();
        if(queueSize == 0){
            continue;
        }
        int index = floor(uniform(0, queueSize)); /*!< Index to circuit's SRMQueue */

        //send random LSP from queue
        //TODO if queue.size() > 10 pick two LSPs (or something like it)
        //TODO maybe? better version would be with this->ISISIft.at((*it)->at(index)->index)
        this->sendLSP((*it)->at(index)->lspRec, (*it)->at(index)->index);

        //clear SRMflag
        this->clearSRMflag((*it)->at(index)->lspRec, (*it)->at(index)->index, circuitType);
/*        (*it)->at(index)->lspRec->SRMflags.at((*it)->at(index)->index);

        delete (*it)->at(index);
        //and remove FlagRecord from queue
        (*it)->erase((*it)->begin() + index);
        */
        queueSize = (*it)->size();
        int a = 5;

    }
    //reschedule PERIODIC_SEND timer
    this->schedule(timer);
}


void ISIS::sendLSP(LSPRecord *lspRec, int gateIndex){

    /* TODO
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
    if(lspRec->deadTimer->getTimerKind() == LSP_DELETE){

    }else{
        double remLife = lspRec->simLifetime - simTime().dbl();
        if (remLife < 0)
        {
            EV << "ISIS: Warning: Remaining lifetime smaller than zero in senLSP" << endl;
            lspRec->LSP->setRemLifeTime(1);
        }
        else
        {
            unsigned short rem = floor(remLife);
            lspRec->LSP->setRemLifeTime(rem);
        }
    }

    ISISLSPPacket *tmpLSP = lspRec->LSP->dup();
    //TODO add proper control Info for point-to-point
    Ieee802Ctrl *tmpCtrl = new Ieee802Ctrl();

    // set DSAP & NSAP fields
    tmpCtrl->setDsap(SAP_CLNS);
    tmpCtrl->setSsap(SAP_CLNS);

    //set destination broadcast address
    //It should be multicast 01-80-C2-00-00-14 MAC address, but it doesn't work in OMNeT
    MACAddress ma;
    ma.setAddress("ff:ff:ff:ff:ff:ff");
    tmpCtrl->setDest(ma);

    tmpLSP->setControlInfo(tmpCtrl);

    send(tmpLSP, "ifOut", gateIndex);


}


/**
 * Create or update my own LSP.
 * @see ISIS::sendMyL1LSPs()
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
    //TLV IS_NEIGHBOURS
    this->addTLV(tlvTable, IS_NEIGHBOURS_LSP, circuitType, 0);

    //TLV AREA ADDRESS
    this->addTLV(tlvTable, AREA_ADDRESS, circuitType);

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
    unsigned int tlvSize; //tmp variable
    unsigned int availableSpace = ISIS_LSP_MAX_SIZE; //available space in LSP

    //deleted at the end of generateLSP
    std::vector<ISISLSPPacket *>* tmpLSPDb = new std::vector<ISISLSPPacket *>;
    for (unsigned char fragment = 0; !tlvTable->empty(); fragment++)
    {
        if (circuitType == L1_TYPE)
        {
            LSP = new ISISLSPPacket("L1 LSP");
            LSP->setType(L1_LSP);
        }
        else if (circuitType == L2_TYPE)
        {
            LSP = new ISISLSPPacket("L2 LSP");
            LSP->setType(L2_LSP);
        }
        else
        {
            EV << "ISIS: ERROR: Wrong circuitType in genLSP()" << endl;
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

        //set PATTLSPDBOLIS
        LSP->setPATTLSPDBOLIS(0x01); //only setting IS type = L1 (TODO)
        //set TLV
        LSP->setTest("ahoj");
                        const char * lspId = (const char *)this->getLspID(LSP);
                        LSP->setTest(lspId);
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
                this->addTLV(LSP,tmpTlv);
                //tlvSize = LSP->getTLVArraySize();
                //LSP->setTLVArraySize(tlvSize + 1);

                //update availableSpace
                availableSpace = availableSpace - (2 + tmpTlv->length);// "2" means Type and Length fields

                //clean up
                delete tmpTlv->value;
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


    for(std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it){
        activeIface.push_back(this->isUp((*it).gateIndex, circuitType));

    }

    for(std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
    {
        //if at least one adjacency is UP and network type is broadcast (there is no DIS on PTP) AND I am DIS on this interface
        if(this->isUp((*it).gateIndex, circuitType) && (*it).network && this->amIL1DIS(this->getIfaceIndex(&(*it))))
        {
            tlvTable->clear();
            //tmpLSPDb->clear(); //Why? Why? Why, you fucked-up crazy-ass weirdo beaver?!?
            availableSpace = ISIS_LSP_MAX_SIZE;
            unsigned char nsel;
            if(circuitType == L1_TYPE){
                myLSPID[ISIS_SYSTEM_ID] = (*it).L1DIS[ISIS_SYSTEM_ID];
                nsel = (*it).L1DIS[ISIS_SYSTEM_ID];
            }else if (circuitType == L2_TYPE){
                myLSPID[ISIS_SYSTEM_ID] = (*it).L2DIS[ISIS_SYSTEM_ID];
                nsel = (*it).L2DIS[ISIS_SYSTEM_ID];
            }else{
                EV <<"ISIS: ERROR: Wrong circuitType in genLSP()" << endl;
            }

            //TLV IS_NEIGHBOURS
            this->addTLV(tlvTable, IS_NEIGHBOURS_LSP, circuitType, nsel);

            //TLV AREA_ADDRESS
            this->addTLV(tlvTable, AREA_ADDRESS, circuitType);

            for (unsigned char fragment = 0; !tlvTable->empty(); fragment++)
                {

                if (circuitType == L1_TYPE)
                {
                    LSP = new ISISLSPPacket("L1 LSP"); //TODO based on circuitType
                    LSP->setType(L1_LSP);
                }
                else if (circuitType == L2_TYPE)
                {
                    LSP = new ISISLSPPacket("L2 LSP"); //TODO based on circuitType
                    LSP->setType(L2_LSP);
                }
                else
                {
                    EV << "ISIS: ERROR: Wrong circuitType in genLSP()" << endl;
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

                //set PATTLSPDBOLIS
                LSP->setPATTLSPDBOLIS(0x01); //only setting IS type = L1 (TODO)
                //set TLV
                LSP->setTest("ahoj");
                const char * lspId = (const char *)this->getLspID(LSP);
                LSP->setTest(lspId);
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
                        break; //ends inner cycle

                    }
                    else
                    {
                        this->addTLV(LSP, tmpTlv);
                        //tlvSize = LSP->getTLVArraySize();
                        //LSP->setTLVArraySize(tlvSize + 1);

                        //update availableSpace
                        availableSpace = availableSpace - (2 + tmpTlv->length);

                        //clean up
                        delete tmpTlv->value;
                        delete tmpTlv;
                        tlvTable->erase(tlvTable->begin());

                    }
                }
                //this->printLSP(LSP, "genLSP, pseudo");
                tmpLSPDb->push_back(LSP);

            }

        }
    }



    delete myLSPID;
    return tmpLSPDb;
}

void ISIS::refreshLSP(ISISTimer *timer){
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

void ISIS::refreshLSP(short circuitType)
{
    std::vector<LSPRecord*>* lspDb = this->getLSPDb(circuitType);
    std::vector<LSPRecord*>::iterator it = lspDb->begin();
    unsigned char *lspId;
    for(; it != lspDb->end(); ++it){
        lspId = this->getLspID((*it)->LSP);
        if(this->compareArrays(lspId, (unsigned char *) this->sysId, ISIS_SYSTEM_ID) && (*it)->deadTimer->getTimerKind() != LSP_DELETE && (*it)->LSP->getRemLifeTime() != 0 ){
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




void ISIS::generateLSP(ISISTimer *timer){

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
         * 2. after processing check if the next generated (if any exist) has the same LAN-ID
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
            if (!tmpLSPDb->empty() && !this->compareArrays(lspId, this->getLspID((*tmpLSPDb->begin())), ISIS_SYSTEM_ID + 1)) {
                //purge all remaining fragments with lspId (without fragment-ID)
                lspId[ISIS_SYSTEM_ID + 1] = lspId[ISIS_SYSTEM_ID + 1] + 1;
                this->purgeRemainLSP(lspId, circuitType);
            }
            //tmpLSPRecord = this->getLSPFromDbByID(this->getLspID(tmpLSPDb->at(0)), circuitType);

            delete lspId;
        }

    }

    delete tmpLSPDb;
}

/*
 * Purge successive LSPs for lspId. (lspId itself is not purged)
 *
 */
void ISIS::purgeRemainLSP(unsigned char *lspId, short circuitType){

    //lspId[ISIS_SYSTEM_ID + 1] = lspId[ISIS_SYSTEM_ID + 1] + 1;

    while(this->getLSPFromDbByID(lspId, circuitType) != NULL){
        this->purgeLSP(lspId, circuitType);
        //this should increment fragment-ID for the next round check
        lspId[ISIS_SYSTEM_ID + 1] = lspId[ISIS_SYSTEM_ID + 1] + 1;

    }
}

void ISIS::purgeMyLSPs(short circuitType){

    std::vector<LSPRecord *>* lspDb;

    lspDb = this->getLSPDb(circuitType);
    unsigned char *lspId;
    for(std::vector<LSPRecord*>::iterator it = lspDb->begin(); it != lspDb->end(); ++it){
        lspId = this->getLspID((*it)->LSP);
        if(this->compareArrays(lspId, (unsigned char *) this->sysId, ISIS_SYSTEM_ID)){
            this->purgeLSP(lspId, circuitType);
        }
        delete lspId;
    }
}

/*
 * This method is used for purging in-memory LSP's when the remaining Lifetime becomes zero.
 */
void ISIS::purgeLSP(unsigned char *lspId, short circuitType){

    LSPRecord * lspRec;
    if((lspRec = this->getLSPFromDbByID(lspId, circuitType)) != NULL){
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


        //simLifetime now express the time when the LSP has been purged so after "N" seconds it can be removed completely
        lspRec->simLifetime = simTime().dbl();

        //remove the deadTimer from future events
        cancelEvent(lspRec->deadTimer);
        //change timer type
        lspRec->deadTimer->setTimerKind(LSP_DELETE);
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
void ISIS::purgeLSP(ISISLSPPacket *lsp, short circuitType){
    LSPRecord *lspRec;
    int gateIndex = lsp->getArrivalGate()->getIndex();
    //std::vector<LSPRecord *> * lspDb = this->getLSPDb(circuitType);
    unsigned char * lspId = this->getLspID(lsp);

    //if lsp is not in memory
    if((lspRec = this->getLSPFromDbByID(lspId, circuitType)) == NULL){

        //send an ack of the LSP on circuit C
        if(!this->getIfaceByGateIndex(gateIndex)->network){
            /* TODO uncomment warning when sending ack for lsp NOT in db is implemented */
            EV<<"ISIS: Warning: Should send ACK for purged LSP over non-broadcast circuit. (Not yet implemented)."<<endl;
            //explicit ack are only on non-broadcast circuits
            //this->setSSNflag(lspRec, gateIndex, circuitType);
        }
    }else

    {   /*7.3.16.4. b) LSP from S is in the DB */
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
                delete lspId;
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
                this->clearSRMflag(lspRec, gateIndex, circuitType);
                if (!this->getIfaceByGateIndex(gateIndex)->network)
                {
                    /* 7.3.16.4. b) 2) ii.*/
                    //explicit ack are only on non-broadcast circuits
                    this->setSSNflag(lspRec, gateIndex, circuitType);
                }

            }
            /* 7.3.16.4. b) 3).*/
            else if (lsp->getSeqNumber() < lspRec->LSP->getSeqNumber())
            {
                /* 7.3.16.4. b) 3) i.*/
                this->setSRMflag(lspRec, gateIndex, circuitType);
                /* 7.3.16.4. b) 3) ii.*/
                this->clearSSNflag(lspRec, gateIndex, circuitType);
            }


        }
        /*7.3.16.4. c) it's our own LSP */
        else{
            if(lspRec->LSP->getRemLifeTime() != 0){
                if(lsp->getSeqNumber() >= lspRec->LSP->getSeqNumber()){
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
    delete lspId;
}

/*
 * This method deletes re
 */
void ISIS::deleteLSP(ISISTimer *timer)
{
    short circuitType = timer->getIsType();
    unsigned char* lspId = this->getLspID(timer);
    std::vector<LSPRecord *> *lspDb = this->getLSPDb(circuitType);
    unsigned char * tmpLspId;

    for(std::vector<LSPRecord*>::iterator it = lspDb->begin(); it != lspDb->end(); ++it){
        tmpLspId = this->getLspID((*it)->LSP);
        if(this->compareArrays(tmpLspId, lspId, ISIS_SYSTEM_ID + 2)){
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


std::vector<std::vector<FlagRecord*>* > * ISIS::getSRMPTPQueue(short circuitType){

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

std::vector<std::vector<FlagRecord*>* > * ISIS::getSRMBQueue(short circuitType){

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

std::vector<std::vector<FlagRecord*>* > * ISIS::getSSNPTPQueue(short circuitType){

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

std::vector<std::vector<FlagRecord*>* > * ISIS::getSSNBQueue(short circuitType){

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

void ISIS::setSRMflag(LSPRecord * lspRec, int index, short circuitType)
{
    ISISinterface * iface = &(this->ISISIft.at(index));
    std::vector<FlagRecord*> *SRMQueue = this->getSRMQ(iface->network, index, circuitType);
    if(lspRec->SRMflags.size() == 0 ){
        this->addFlags(lspRec, circuitType);
    }
    /* This should prevent from adding multiple FlagRecord for the same LSP-gateIndex pair */
    if(lspRec->SRMflags.at(index) == true){
        return;
    }

    //set SRMflag on interface only if there is  at least one adjacency UP
    if (this->isUp(index, circuitType))
    {
        lspRec->SRMflags.at(index) = true; //setting true (Send Routing Message) on every interface
        FlagRecord *tmpSRMRec = new FlagRecord;
        tmpSRMRec->lspRec = lspRec;
        tmpSRMRec->index = index;
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
    if(lspRec->SRMflags.size() == 0 ){
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SRMflags.size(); i++)
    {
        this->setSRMflag(lspRec, i, circuitType);
    }

}

void ISIS::setSRMflagsBut(LSPRecord *lspRec, unsigned int index, short circuitType)
{
    if(lspRec->SRMflags.size() == 0 ){
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SRMflags.size(); i++)
    {
        if(i == index){
            this->clearSRMflag(lspRec, i, circuitType);
        }else{
            this->setSRMflag(lspRec, i, circuitType);
        }
    }

}

void ISIS::clearSRMflag(LSPRecord *lspRec, int index, short circuitType){

    std::vector<FlagRecord*>* srmQ;
    srmQ =  this->getSRMQ(this->ISISIft.at(index).network, this->ISISIft.at(index).gateIndex, circuitType);

    //clear flag
    lspRec->SRMflags.at(index) = false;

    //and remove FlagRecord from Queue
    std::vector<FlagRecord*>::iterator it = srmQ->begin();
    for (; it != srmQ->end(); )
    {
        if ((*it)->index == index && (*it)->lspRec == lspRec)
        {
            it = srmQ->erase(it);
            //break;
        }else{
            ++it;
        }
    }
}

void ISIS::clearSRMflags(LSPRecord *lspRec, short circuitType){

    if(lspRec->SRMflags.size() == 0 ){
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SRMflags.size(); i++)
    {
        this->clearSRMflag(lspRec, i, circuitType);
    }

}

void ISIS::clearSRMflagsBut(LSPRecord *lspRec, unsigned int index, short circuitType)
{
    if(lspRec->SRMflags.size() == 0 ){
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SRMflags.size(); i++)
    {
        if(i == index){
            this->setSRMflag(lspRec, i, circuitType);
        }else{
            this->clearSRMflag(lspRec, i, circuitType);
        }
    }

}



std::vector<FlagRecord*>* ISIS::getSRMQ(bool network, int index, short circuitType){
    if(circuitType == L1_TYPE){
        if(network){
            return this->L1SRMBQueue->at(index);
        }else{
            return this->L1SRMPTPQueue->at(index);
        }

    }else if (circuitType == L2_TYPE){
        if(network){
                    return this->L2SRMBQueue->at(index);
                }else{
                    return this->L2SRMPTPQueue->at(index);
                }
    }
    return NULL;
}


void ISIS::setSSNflag(LSPRecord * lspRec, int index, short circuitType)
{

    ISISinterface * iface = &(this->ISISIft.at(index));
        std::vector<FlagRecord*> *SSNQueue = this->getSSNQ(iface->network, index, circuitType);

    if (lspRec->SSNflags.size() == 0)
        {
            this->addFlags(lspRec, circuitType);
        }

    /* This should prevent from adding multiple FlagRecord for the same LSP-gateIndex pair */
     if(lspRec->SSNflags.at(index) == true){
         return;
     }
    //set SSNflag on interface only if there is  at least one adjacency UP
    if (this->isUp(index, circuitType))
    {

        lspRec->SSNflags.at(index) = true; //setting true (Send Routing Message) on every interface
        FlagRecord *tmpSSNRec = new FlagRecord;
        tmpSSNRec->lspRec = lspRec;
        tmpSSNRec->index = index;
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

void ISIS::setSSNflagsBut(LSPRecord *lspRec, unsigned int index, short circuitType)
{
    if(lspRec->SSNflags.size() == 0 ){
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SSNflags.size(); i++)
    {
        if(i == index){
            this->clearSSNflag(lspRec, i, circuitType);
        }else{
            this->setSSNflag(lspRec, i, circuitType);
        }
    }

}


void ISIS::clearSSNflag(LSPRecord *lspRec, int index, short circuitType){

    std::vector<FlagRecord*>* ssnQ;
    ssnQ =  this->getSSNQ(this->ISISIft.at(index).network, this->ISISIft.at(index).gateIndex, circuitType);

    //clear flag
    lspRec->SSNflags.at(index) = false;

    //and remove FlagRecord from Queue
    std::vector<FlagRecord*>::iterator it = ssnQ->begin();
    for (; it != ssnQ->end();)
    {
        if ((*it)->index == index && (*it)->lspRec == lspRec)
        {
            it = ssnQ->erase(it);
            //break;
        }else{
            ++it;
        }
    }
}

void ISIS::clearSSNflags(LSPRecord *lspRec, short circuitType){

    if(lspRec->SSNflags.size() == 0 ){
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SSNflags.size(); i++)
    {
        this->clearSSNflag(lspRec, i, circuitType);
    }

}

void ISIS::clearSSNflagsBut(LSPRecord *lspRec, unsigned int index, short circuitType)
{
    if(lspRec->SSNflags.size() == 0 ){
        this->addFlags(lspRec, circuitType);
    }
    for (unsigned int i = 0; i < lspRec->SSNflags.size(); i++)
    {
        if(i == index){
            this->setSSNflag(lspRec, i, circuitType);
        }else{
            this->clearSSNflag(lspRec, i, circuitType);
        }
    }

}

std::vector<FlagRecord*>* ISIS::getSSNQ(bool network, int index, short circuitType){
    if(circuitType == L1_TYPE){
        if(network){
            return this->L1SSNBQueue->at(index);
        }else{
            return this->L1SSNPTPQueue->at(index);
        }

    }else if (circuitType == L2_TYPE){
        if(network){
                    return this->L2SSNBQueue->at(index);
                }else{
                    return this->L2SSNPTPQueue->at(index);
                }
    }
    return NULL;
}




void ISIS::replaceLSP(ISISLSPPacket *lsp, LSPRecord *lspRecord, short circuitType){

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

    lspRecord->deadTimer->setTimerKind(LSP_DEAD);


    lspRecord->deadTimer->setIsType(circuitType);
    for (unsigned int i = 0; i < lspRecord->deadTimer->getLSPidArraySize(); i++)
    {
        lspRecord->deadTimer->setLSPid(i, lspRecord->LSP->getLspID(i));
    }

    /* need to check  this for replacing LSP from genLSP */
    if(lsp->getArrivalGate() != NULL){
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
        this->setSRMflagsBut(lspRecord, gateIndex , circuitType);
        //for non-broadcast /* 7.3.16.4 b) 1) iv. */
        if(!this->ISISIft.at(gateIndex).network){
            this->setSSNflag(lspRecord, gateIndex, circuitType);
        }
        /* 7.3.16.4 b) 1) v. */
        this->clearSSNflagsBut(lspRecord, gateIndex, circuitType);




    }else{
        //generated -> set SRM on all interfaces
        this->setSRMflags(lspRecord, circuitType);
        //TODO what with SSN?
        //this->clearSSNflags(lspRecord, circuitType);
    }



    //set simulation time when the lsp will expire
    lspRecord->simLifetime = simTime().dbl() + lspRecord->LSP->getRemLifeTime();

    this->schedule(lspRecord->deadTimer, lsp->getRemLifeTime());


}

void ISIS::addFlags(LSPRecord *lspRec, short circuitType){

    //add flags
    if(!lspRec->SRMflags.empty() || !lspRec->SSNflags.empty()){
        EV <<"ISIS: Warning: Adding *flags to non-empty vectors." <<endl;
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


LSPRecord * ISIS::installLSP(ISISLSPPacket *lsp, short circuitType)
{

    LSPRecord * tmpLSPRecord = new LSPRecord;
    std::vector<LSPRecord*> * lspDb = this->getLSPDb(circuitType);

    tmpLSPRecord->LSP = lsp;
    //TODO
    /* if lsp is my LSP
     *   then set all flags
     * else
     *   set all BUT the received circuts
     */

    if(lsp->getArrivalGate() !=NULL){
        int gateIndex = lsp->getArrivalGate()->getIndex();
        /* 7.3.15.1 e) 1) ii. */
        this->setSRMflagsBut(tmpLSPRecord, gateIndex, circuitType);
        /*TODO 7.3.15.1 e) 1) iii. -> redundant?? */
        //this->clearSRMflag(tmpLSPRecord, lsp->getArrivalGate()->getIndex(), circuitType);

        /* change from specification */
        /* iv. and v. are "switched */
        /* 7.3.15.1 e) 1) v. */
        this->clearSSNflags(tmpLSPRecord, circuitType);

        if (!this->ISISIft.at(gateIndex).network)
        {
            /* 7.3.15.1 e) 1) iv. */
            this->setSSNflag(tmpLSPRecord, gateIndex, circuitType);
        }

    }else{
        /* Never set SRM flag for lsp with seqNum == 0 */
        if(lsp->getSeqNumber() > 0){
        /* installLSP is called from genLSP */
            this->setSRMflags(tmpLSPRecord, circuitType);
        }
        //TODO what about SSNflags?
        //this->clearSSNflags(tmpLSPRecord, circuitType);
    }


    tmpLSPRecord->deadTimer = new ISISTimer("LSP Dead Timer");
    tmpLSPRecord->deadTimer->setTimerKind(LSP_DEAD);
    tmpLSPRecord->deadTimer->setIsType(circuitType);
    for (unsigned int i = 0; i < tmpLSPRecord->deadTimer->getLSPidArraySize(); i++)
    {
        tmpLSPRecord->deadTimer->setLSPid(i, tmpLSPRecord->LSP->getLspID(i));
    }

    tmpLSPRecord->simLifetime = simTime().dbl() + tmpLSPRecord->LSP->getRemLifeTime();

    this->schedule(tmpLSPRecord->deadTimer, lsp->getRemLifeTime());

    lspDb->push_back(tmpLSPRecord);

    return lspDb->back();
}

/*
 * Returns true if packets match
 */
bool ISIS::compareLSP(ISISLSPPacket *lsp1, ISISLSPPacket *lsp2){

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
    if(lsp1->getPATTLSPDBOLIS() != lsp2->getPATTLSPDBOLIS()){
        return false;
    }

    if(lsp1->getTLVArraySize() != lsp2->getTLVArraySize()){
        return false;
    }

    for(unsigned int i = 0; i < lsp1->getTLVArraySize(); i++){
        if(lsp1->getTLV(i).type != lsp2->getTLV(i).type || lsp1->getTLV(i).length != lsp2->getTLV(i).length || !this->compareArrays(lsp1->getTLV(i).value, lsp2->getTLV(i).value, lsp1->getTLV(i).length) ){
            return false;
        }
    }

    //packets match
    return true;
}


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
            delete lspId;
            return (*it);
        }
        delete lspId;
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
                    ISISadj *tmpAdj = this->getAdjByGateIndex(ISISIft.at(i).gateIndex, L1_TYPE);
                    //if there's not adjacency in state "UP" for specified interface, then skip this interface
                    if(tmpAdj == NULL || !tmpAdj->state){
                        continue;
                    }
                    //for broadcast network use DIS's System-ID
                    if (ISISIft.at(i).network)
                    {
                        this->copyArrayContent(ISISIft.at(i).L1DIS, neighbour.LANid, 7, 0, 0);
                    }
                    else
                        //for point-to-point links use actual System-ID padded with 0 byte
                    {
                        this->copyArrayContent(tmpAdj->sysID, neighbour.LANid, ISIS_SYSTEM_ID, 0, 0);
                        neighbour.LANid[ISIS_SYSTEM_ID] = 0;
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
        record.deadTimer->setTimerKind(LSP_DEAD);
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
        //check if this interface is DIS for LAN AND if it is broadcast interface
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
                record.deadTimer->setTimerKind(LSP_DEAD);
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

ISISadj* ISIS::getAdjByGateIndex(int gateIndex, short circuitType, int offset)
{

    if (circuitType == L1_TYPE || circuitType == L1L2_TYPE)//L1L2_TYPE option is there for L1L2 PTP links, because for such links there should be adjacency in both tables
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
    return NULL;


}

/**
 *
 * @param gateIndex index to global interface table
 */

ISISinterface* ISIS::getIfaceByGateIndex(int gateIndex)
{

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

bool ISIS::amIL1DIS(int interfaceIndex)
{
    //if this is not broadcast interface then no DIS is elected
    if (!this->ISISIft.at(interfaceIndex).network)
    {
        return false;
    }

    return (compareArrays((unsigned char *) this->sysId, this->ISISIft.at(interfaceIndex).L1DIS, ISIS_SYSTEM_ID));

}


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
 * NOT ALL MESSAGE TYPES IMPLEMENTED YET
 */
TLV_t* ISIS::getTLVByType(ISISMessage *inMsg, enum TLVtypes tlvType, int offset)
{

    for(unsigned int i = 0; i < inMsg->getTLVArraySize(); i++){
           if(inMsg->getTLV(i).type == tlvType){
               if(offset == 0){
                   return &(inMsg->getTLV(i));
               }else{
                   offset--;
               }
           }
       }

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

bool ISIS::isMessageOK(ISISMessage *inMsg)
{

    if (inMsg->getIdLength() != ISIS_SYSTEM_ID && inMsg->getIdLength() != 0)
    {
        return false;
    }

    if (inMsg->getMaxAreas() != ISIS_MAX_AREAS && inMsg->getMaxAreas() != 0)
    {
        return false;
    }

    return true;
}

bool ISIS::isAreaIDOK(TLV_t *areaAddressTLV, unsigned char *compare)
{
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


void ISIS::addTLV(ISISMessage *inMsg, enum TLVtypes tlvType)
{
    unsigned int tlvSize;
    switch (tlvType)
    {
        case (AREA_ADDRESS):
            TLV_t myTLV;
            myTLV.type = AREA_ADDRESS;
            myTLV.length = ISIS_AREA_ID;
            myTLV.value = new unsigned char[ISIS_AREA_ID];
            this->copyArrayContent((unsigned char*) this->areaId, myTLV.value, 3, 0, 0);

            tlvSize = inMsg->getTLVArraySize();
            inMsg->setTLVArraySize(tlvSize + 1);
            inMsg->setTLV(tlvSize, myTLV);
            break;

        default:
            EV <<"ISIS: ERROR: This TLV type is not (yet) implemented in addTLV(ISISMessage*, enum TLVtypes)" << endl;
            break;

    }

}

void ISIS::addTLV(ISISMessage* inMsg, TLV_t *tlv){
    TLV_t tmpTlv;
    unsigned int tlvSize;

    tmpTlv.type = tlv->type; // set type
    tmpTlv.length = tlv->length; //set length
    tmpTlv.value = new unsigned char [tlv->length]; //allocate appropriate space
    this->copyArrayContent(tlv->value, tmpTlv.value, tlv->length, 0, 0); //copy it


    tlvSize = inMsg->getTLVArraySize(); //get array size
    inMsg->setTLVArraySize(tlvSize + 1);// increase it by one
    inMsg->setTLV(tlvSize, tmpTlv); //finally add TLV at the end
}

void ISIS::addTLV(std::vector<TLV_t *> *tlvTable, enum TLVtypes tlvType, short circuitType, unsigned char nsel)
{

    TLV_t * myTLV;


    if (tlvType == AREA_ADDRESS)
    {
        myTLV = new TLV_t;
        myTLV->type = AREA_ADDRESS;
        myTLV->length = ISIS_AREA_ID;
        myTLV->value = new unsigned char[ISIS_AREA_ID];
        this->copyArrayContent((unsigned char*) this->areaId, myTLV->value, 3, 0, 0);
        tlvTable->push_back(myTLV);

    }
    else if (tlvType == IS_NEIGHBOURS_LSP)
    {

        if (nsel == 0)
        {
            std::vector<LSPneighbour> neighbours;
            ISISadj *tmpAdj;
            for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
            {
                LSPneighbour neighbour;
                tmpAdj = this->getAdjByGateIndex((*it).gateIndex, circuitType);
                //if there's not adjacency in state "UP" for specified interface, then skip this interface
                if (tmpAdj == NULL || !tmpAdj->state)
                {
                    continue;
                }

                if ((*it).network)
                {
                    //TODO how do you know it's L1DIS and not L2
                    this->copyArrayContent((*it).L1DIS, neighbour.LANid, ISIS_SYSTEM_ID + 1, 0, 0);
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
                myTLV->value[0] = 40;//TODO should be 0
                //inner loop for separate TLV; after reaching ISIS_LSP_MAX_SIZE or empty neighbours stop filling this tlv
                // 2 bytes for type and length fields and 1 byte for virtual circuit
                for (unsigned int i = 0; ((i + 1) * entrySize) + 1 < 255 && 2 + 1 + (entrySize * i) + entrySize < ISIS_LSP_MAX_SIZE && !neighbours.empty();
                        i++)
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
            std::vector<LSPneighbour> neighbours;

            ISISadj *tmpAdj;
            for (std::vector<ISISinterface>::iterator it = this->ISISIft.begin(); it != this->ISISIft.end(); ++it)
            {

                for (int offset = 0; (tmpAdj = this->getAdjByGateIndex((*it).gateIndex, circuitType, offset)) != NULL;
                        offset++)
                {
                    LSPneighbour neighbour;
                    if (!tmpAdj->state)
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
            }
            //add also mine non-pseudonode interface as neighbour
                        LSPneighbour neighbour;
                        this->copyArrayContent((unsigned char*) this->sysId, neighbour.LANid, ISIS_SYSTEM_ID, 0, 0);
                        neighbour.LANid[ISIS_SYSTEM_ID] = 0;
                        neighbour.metrics.defaultMetric = 0;  //metric to every neighbour in pseudonode LSP is always zero!!!
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
    else
    {
        EV << "ISIS: ERROR: This TLV type is not (yet) implemented in addTLV" << endl;
    }
}

/*
 * Returns true if this System have ANY adjacency UP on level specified by circuitType
 */
bool ISIS::isAdjUp(short circuitType)
{
    std::vector<ISISadj>* adjTable = this->getAdjTab(circuitType);

    for (std::vector<ISISadj>::iterator it = adjTable->begin(); it != adjTable->end(); ++it)
    {
        if ((*it).state)
        {
            return true;
        }
    }
    return false;
}

/*
 * This method returns true if there is ANY adjacency on interface identified by gateIndex in state UP
 * Method does NOT check SNPA (aka MAC).
 * Can be used to check if we should send LSP on such interface.
 */
bool ISIS::isUp(int gateIndex, short circuitType)
{
    std::vector<ISISadj> *adjTable;

    adjTable = this->getAdjTab(circuitType);
    for (std::vector<ISISadj>::iterator it = adjTable->begin(); it != adjTable->end(); ++it)
    {
        if ((*it).gateIndex == gateIndex && (*it).state)
        {
            return true;
        }
    }
    return false;

}

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
        if (compareArrays(systemID, (*it).sysID, ISIS_SYSTEM_ID) || msg->getType() == L1_LSP || msg->getType() == L2_LSP)
        {
            //MAC Address and gateIndex
            //we need to check source (tmpMac) and destination interface thru we received this hello
            if (tmpMac.compareTo((*it).mac) == 0 && gateIndex == (*it).gateIndex && (*it).state)
            {

                delete systemID;
                return true;

            }
        }
    }
    delete systemID;

    return false;
}


void ISIS::fullSPF(ISISTimer *timer){

    ISISCons_t initial;
    ISISPaths_t ISISPaths;
    ISISPaths_t ISISTent;
    ISISPath * tmpPath;

    //let's fill up the initial paths with supported-protocol's reachability informations

    //fill ISO
    bool result;
    result = this->extractISO(&initial, timer->getIsType());
    if(!result){
        //there was an error during extraction so cancel SPF
        //todo reschedule
        this->schedule(timer);
        //TODO clean
        return;
    }

    //put myself (this IS) on TENT list
    unsigned char *lspId = this->getLSPID();//returns sysId + 00


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

    //TODO shoudn't i put myself in PATH list?

    for(;!ISISTent.empty();)
    {
        //tmpPath = this->getBestPath(&(this->ISISTent));

        //this->moveToPath(tmpPath);
        this->bestToPath(&initial, &ISISTent, &ISISPaths);

    }

    this->printPaths(&ISISPaths);

    //find shortest metric in TENT


    this->schedule(timer);
}

void ISIS::printPaths(ISISPaths_t *paths){


        std::cout << "Best paths of IS: ";
        //print area id
            for (unsigned int i = 0; i < 3; i++)
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
            std::cout
                    << setfill('0') << setw(2) << dec << (unsigned int) this->NSEL[0] << "\tNo. of paths: "
                            << paths->size() << endl;
    for(ISISPaths_t::iterator it = paths->begin(); it != paths->end(); ++it){
        this->printSysId((*it)->to);
        std::cout << setfill('0') << setw(2) << dec <<(unsigned short)(*it)->to[6]<< endl;;
        std::cout <<"\t\t metric: "<< (*it)->metric <<"\t via:"<<endl;;
        for(ISISNeighbours_t::iterator nIt = (*it)->from.begin(); nIt != (*it)->from.end(); ++nIt){
            std::cout<<"\t\t\t\t\t";
            this->printSysId((*nIt)->id);
            std::cout  << setfill('0') << setw(2) << dec <<(unsigned short)(*nIt)->id[6]<< endl;;
        }
    }
}

/*void ISIS::moveToPath(ISISPath* path){

    std::sort(this->ISISTent.begin(), this->ISISTent.end());
    //move connections from init to tent, as a "from" put path->to
    this->moveToTent(&(this->ISISInit), path->to, path->metric);


}*/
void ISIS::bestToPath(ISISCons_t *init, ISISPaths_t *ISISTent, ISISPaths_t *ISISPaths){

    ISISPath *path;
    ISISPath *tmpPath;
    //sort it
    std::sort(ISISTent->begin(), ISISTent->end());
    //save best in path
    path = ISISTent->front();
    //mov
    this->moveToTent(init, path->to, path->metric, ISISTent);

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
                EV <<"ISIS: Error during SPF. We got better metric than the one PATHS."<<endl;
                //we got better metric so clear "from" neighbours
                tmpPath->from.clear();
            }
            EV <<"ISIS: Error during SPF. I think we shouldn't have neither same metric."<<endl;
            //append
            tmpPath->metric = path->metric;
            for(ISISNeighbours_t::iterator it = path->from.begin(); it != path->from.end(); ++it){
                tmpPath->from.push_back((*it));
            }


        }

    }



    //

}

void ISIS::moveToTent(ISISCons_t *initial, unsigned char *from, uint32_t metric, ISISPaths_t *ISISTent){

    ISISPath *tmpPath;
    ISISCons_t *cons = this->getCons(initial, from);
/*       if(cons->empty()){
           EV <<"ISIS: Error during SPF. Didn't find my own LSP"<<endl;
           //TODO clean
//           delete cons;
  //         return;
       }*/


       //add my connections as a starting point
       for(ISISCons_t::iterator it = cons->begin(); it != cons->end(); ++it){
           if ((tmpPath = this->getPath(ISISTent, (*it)->to)) == NULL)
           {
               //path to this destination doesn't exist, co create new
               tmpPath = new ISISPath;
               tmpPath->to = new unsigned char[ISIS_SYSTEM_ID + 2];
               this->copyArrayContent((*it)->to, tmpPath->to, ISIS_SYSTEM_ID + 2, 0, 0);
               tmpPath->metric = (*it)->metric + metric;

               ISISNeighbour *neighbour = new ISISNeighbour;

               neighbour->id = new unsigned char[ISIS_SYSTEM_ID + 2];
               if(this->compareArrays((*it)->from,(unsigned char *) this->sysId, ISIS_SYSTEM_ID)){
                   this->copyArrayContent((*it)->to, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
               }else{
                   this->copyArrayContent((*it)->from, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
               }
               neighbour->type = false; //not a leaf
               tmpPath->from.push_back(neighbour);

               ISISTent->push_back(tmpPath);
           }
           else
           {
               if(tmpPath->metric >= (*it)->metric + metric){
                   if(tmpPath->metric > (*it)->metric + metric){
                       //we got better metric so clear "from" neighbours
                       tmpPath->from.clear();
                   }
                   //append
                   tmpPath->metric = (*it)->metric + metric;
                   ISISNeighbour *neighbour = new ISISNeighbour;
                   neighbour->id = new unsigned char[ISIS_SYSTEM_ID + 2];
                   this->copyArrayContent((*it)->from, neighbour->id, ISIS_SYSTEM_ID + 2, 0, 0);
                   neighbour->type = false; //not a leaf
                   tmpPath->from.push_back(neighbour);

               }

           }

       }
}



ISISPath * ISIS::getBestPath(ISISPaths_t *paths){

    std::sort(paths->begin(), paths->end());
    return paths->front();

}


void ISIS::getBestMetric(ISISPaths_t *paths){

    for(ISISPaths_t::iterator it = paths->begin(); it != paths->end(); ++it){

    }
}



bool ISIS::extractISO(ISISCons_t *initial, short circuitType){

    ISISLspDb_t *lspDb = this->getLSPDb(circuitType);
    unsigned char *lspId;

    ISISCon* connection;

    for(ISISLspDb_t::iterator it = lspDb->begin(); it != lspDb->end(); ++it){
        //getLspId
        lspId = this->getLspID((*it)->LSP);

        //check if it's zero-th fragment. if not try to find it -> getLspFromDbByID

        if(lspId[ISIS_SYSTEM_ID + 1] != 0){
            unsigned char backup = lspId[ISIS_SYSTEM_ID + 1];
            lspId[ISIS_SYSTEM_ID + 1] = 0;
            //if doesn't exist -> continue to next lsp
            if(this->getLSPFromDbByID(lspId, circuitType) == NULL){
                continue;
            }
            lspId[ISIS_SYSTEM_ID + 1] = backup;

        }
        //else
        else{
            //try to find id in "initial"
            //if found
//            if((path = this->getPath(initial, lspId)) == NULL){
//               path = new ISISPath;
//               path->neighbour.id = lspId;
//               path->neighbour.metric = 0;
//               path->neighbour.type = false;
//
//               initial->push_back(path);//put path to initial paths
//            }

            TLV_t *tmpTLV;
            for(int offset = 0; (tmpTLV = this->getTLVByType((*it)->LSP, IS_NEIGHBOURS_LSP, offset)) != NULL; offset++){
                for(unsigned int i = 1; i + 11 <= tmpTLV->length; i += 11)
                {
                    connection = new ISISCon;
                    connection->from = new unsigned char [ISIS_SYSTEM_ID + 2];
                    this->copyArrayContent(lspId, connection->from, ISIS_SYSTEM_ID + 1, 0, 0);
                    connection->from[ISIS_SYSTEM_ID + 1] = 0;
                    connection->to = new unsigned char [ISIS_SYSTEM_ID + 2];
                    this->copyArrayContent(tmpTLV->value, connection->to, ISIS_SYSTEM_ID +1, i + 4, 0);
                    connection->to[ISIS_SYSTEM_ID + 1] = 0;
                    connection->metric = tmpTLV->value[i];//default metric
                    connection->type = false;//it's not a leaf

                    initial->push_back(connection);
                    //path->neighbours.push_back(neighbour);
                }


            }



            //add information


            //else
                //create new record in initial and add information from lsp
                //..scan TLVs and process information like in printLSPDb
                //so far just TLV_ENTRIES with lsp-id and metric
        }
    }

    /*TODO perform two-way check (it should be done later when moving node from
     * initial conns to TENT paths, but it shoudn't affect the algorithm
     */
    this->twoWayCheck(initial);

    return true;
}

void ISIS::twoWayCheck(ISISCons_t *cons){
    ISISCons_t *tmpCons;
    for(ISISCons_t::iterator it = cons->begin(); it != cons->end(); ){
        //if there is not reverse connection
        //TODO is this enough? there could be two one-way connections between two ISs
        if(!this->isCon(cons, (*it)->to, (*it)->from)){
            it = cons->erase(it);
        }else{
            ++it;
        }
    }
}

ISISCons_t * ISIS::getCons(ISISCons_t *cons, unsigned char *from){

    ISISCons_t *retCon = new ISISCons_t;
    for(ISISCons_t::iterator it = cons->begin(); it != cons->end(); ){
        if(this->compareArrays((*it)->from, from, 8)){
            retCon->push_back((*it));
            it = cons->erase(it);
        }else{
            ++it;
        }
    }

    return retCon;
}

bool ISIS::isCon(ISISCons_t *cons, unsigned char *from, unsigned char *to){

    for (ISISCons_t::iterator it = cons->begin(); it != cons->end(); ++it)
    {
        if(this->compareArrays((*it)->from, from, ISIS_SYSTEM_ID + 2) && this->compareArrays((*it)->to, to, ISIS_SYSTEM_ID + 2)){
            return true;
        }
    }
    return false;
}



ISISPath * ISIS::getPath(ISISPaths_t *paths, unsigned char *id){

    for(ISISPaths_t::iterator it = paths->begin(); it != paths->end(); ++it){
        if(this->compareArrays((*it)->to, id, 8)){
            return (*it);
        }
    }

    return NULL;
}

