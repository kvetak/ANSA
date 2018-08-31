//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
* @file LLDPAgentTable.cc
* @author Tomas Rajca
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#include "ansa/linklayer/lldp/tables/LLDPAgentTable.h"
#include "ansa/linklayer/lldp/LLDPMain.h"

#include <string>

namespace inet {
Register_Abstract_Class(LLDPAgent);
Define_Module(LLDPAgentTable);

std::string LLDPStatistics::info() const
{
    std::stringstream string;
    string << "ageouts total:" << ageoutsTotal << ", frames discarded total:" << framesDiscardedTotal;
    string << ", frames in errors total:" << framesInErrorsTotal << ", frames in total:" << framesInTotal;
    string << ", frames out total: " << framesInTotal << ", frames out total: " << framesOutTotal;
    string << ", tlvs discarded total: " << tlvsDiscardedTotal << ", tlvs unrecognized total" << tlvsUnrecognizedTotal;
    string << ", lldpdu length errors: " << lldpduLengthErrors << endl;
    return string.str();
}

std::string LLDPAgent::info() const
{
    std::stringstream string;

    string << getIfaceName();
    string << ":";
    if(adminStatus == enabledRxTx)
        string << "RxTx";
    else if(adminStatus == enabledRxOnly)
        string << "Rx";
    else if(adminStatus == enabledTxOnly)
        string << "Tx";
    else if(adminStatus == disabled)
        string << "dis";
    string << ", txCredit:" << (uint32_t)txCredit;
    string << ", txFast:" << (uint32_t)txFast;
    string << ", msgFastTx:" << msgFastTx;
    string << ", msgTxHold:" << (uint32_t)msgTxHold << "," << endl;
    string << "msgTxInterval:" << msgTxInterval;
    string << ", reinitDelay:" << reinitDelay;
    string << ", txFastInit:" << txFastInit;
    string << ", txCreditMax:" << (uint32_t)txCreditMax;
    return string.str();
}

LLDPAgent::LLDPAgent(
        InterfaceEntry *iface,
        LLDPMain *c,
        uint8_t msgFastTxDef,
        uint8_t msgTxHoldDef,
        uint16_t msgTxIntervalDef,
        uint16_t reinitDelayDef,
        uint16_t txFastInitDef,
        uint8_t txCreditMaxDef,
        AS adminStatusDef)
{
    interface = iface;
    core = c;

    txTTR = new LLDPTimer();
    txTTR->setTimerType(TTR);
    txTTR->setContextPointer(this);
    txShutdownWhile = new LLDPTimer();
    txShutdownWhile->setTimerType(ShutdownWhile);
    txShutdownWhile->setContextPointer(this);

    txCredit = txCreditMaxDef;
    txCreditMax = txCreditMaxDef;
    txFast = 0;
    msgFastTx = msgFastTxDef;
    msgTxHold = msgTxHoldDef;
    msgTxInterval = msgTxIntervalDef;
    reinitDelay = reinitDelayDef;
    txFastInit = txFastInitDef;
    adminStatus = adminStatusDef;

    txTTROwner = dynamic_cast<cSimpleModule *>(txTTR->getOwner());
    lnt = core->getLnt();
    reinitDelaySet = false;
}

LLDPAgent::~LLDPAgent()
{
    if(txTTROwner != nullptr)
    {
        txTTROwner->cancelAndDelete(txTTR);
        txTTROwner = nullptr;
    }
    //if is scheduled, get his sender module, otherwise get owner module
    cSimpleModule *owner = dynamic_cast<cSimpleModule *>((txShutdownWhile->isScheduled()) ? txShutdownWhile->getSenderModule() : txShutdownWhile->getOwner());
    if(owner != nullptr)
    {// owner is cSimpleModule object -> can call his methods
        owner->cancelAndDelete(txShutdownWhile);
        txShutdownWhile = nullptr;
    }
}

void LLDPAgent::txAddCredit()
{
    if(txCredit < txCreditMax)
        txCredit++;
}

void LLDPAgent::dec(uint8_t *var)
{
    if((*var) > 0)
        (*var)--;
}

void LLDPAgent::startAgent()
{
    if(reinitDelaySet)
        return;

    if(adminStatus == enabledRxTx || adminStatus == enabledTxOnly)
        txSchedule(true);
}

void LLDPAgent::stopAgent()
{
    txTTROwner->cancelEvent(txTTR);
    txShutdownFrame();
}

void LLDPAgent::txSchedule(bool noDelay)
{
    if(adminStatus != enabledRxTx && adminStatus != enabledTxOnly)
        return;

    uint16_t delay = 0;
    if(!noDelay)
    {
        if(txFast > 0)
            delay = msgFastTx;
        else
            delay = msgTxInterval;
    }

    txTTROwner->scheduleAt(simTime() + delay, txTTR);
}

void LLDPAgent::neighbourUpdate(LLDPUpdate *msg)
{
    LLDPNeighbour *neighbour = lnt->findNeighbourByMSAP(msg->getMsap());

    // shutdown packet
    if(msg->getTtl() == 0)
    {
        lnt->removeNeighbour(msg->getMsap());
        if(neighbour != nullptr)
            EV_INFO << "Neighbour " << neighbour->getSystemName() << " go down. Delete from table" << endl;
        return;
    }

    std::string chassisId = msg->getChassisId();
    std::string portId = msg->getPortId();
    if(neighbour == nullptr)
    {// new neighbour
        neighbour = lnt->addNeighbour(this, chassisId, portId);
        EV_INFO << "New neighbour " << neighbour->getSystemName() << ". Chassis ID: " << neighbour->getChassisId() << ", Port ID: " << neighbour->getPortId() << endl;

        // fast start
        txFastStart();
    }
    else
    {
        EV_INFO << "Update neighbour " << neighbour->getSystemName() << ". Chassis ID: " << neighbour->getChassisId() << ", Port ID: " << neighbour->getPortId() << endl;
    }

    // mark all neighbour information as unchanged
    bool updatedTlv[128];
    memset(&updatedTlv, 0, 128 * sizeof(bool));
    bool mtu = false;
    neighbour->getManagementAdd().setAllUnchanged();

    lnt->restartRxInfoTtl(neighbour, msg->getTtl());
    neighbour->setLastUpdate(simTime());
    neighbour->setTtl(msg->getTtl());

    for(unsigned int i=3; i < msg->getOptionArraySize(); i++)
    {
        const TlvOptionBase *option = msg->getOption(i);
        switch(msg->getOption(i)->getType())
        {
            case LLDPTLV_PORT_DES: {
                const LLDPOptionPortDes *opt = check_and_cast<const LLDPOptionPortDes *> (option);
                neighbour->setPortDes(opt->getValue());
                updatedTlv[LLDPTLV_PORT_DES] = true;
                break;
            }

            case LLDPTLV_SYSTEM_NAME: {
                const LLDPOptionSystemName *opt = check_and_cast<const LLDPOptionSystemName *> (option);
                neighbour->setSystemName(opt->getValue());
                updatedTlv[LLDPTLV_SYSTEM_NAME] = true;
                break;
            }

            case LLDPTLV_SYSTEM_DES: {
                const LLDPOptionSystemDes *opt = check_and_cast<const LLDPOptionSystemDes *> (option);
                neighbour->setSystemDes(opt->getValue());
                updatedTlv[LLDPTLV_SYSTEM_DES] = true;
                break;
            }

            case LLDPTLV_SYSTEM_CAP: {
                const LLDPOptionCap *opt = check_and_cast<const LLDPOptionCap *> (option);
                neighbour->setSystemCap(capabilitiesConvert(opt->getSysCap(0), opt->getSysCap(1)));
                neighbour->setEnabledCap(capabilitiesConvert(opt->getEnCap(0), opt->getEnCap(1)));
                updatedTlv[LLDPTLV_SYSTEM_CAP] = true;
                break;
            }

            case LLDPTLV_MAN_ADD: {
                const LLDPOptionManAdd *opt = check_and_cast<const LLDPOptionManAdd *> (option);
                LLDPManAdd *manAdd = neighbour->getManagementAdd().findManAdd(opt->getAddSubtype());

                if(manAdd == nullptr)
                { // add new
                    manAdd = new LLDPManAdd();
                    neighbour->getManagementAdd().addManAdd(manAdd);
                }
                manAdd->setAddLength(opt->getAddLength());
                manAdd->setAddSubtype(opt->getAddSubtype());
                manAdd->setAddress(opt->getAddress());
                manAdd->setIfaceSubtype(opt->getIfaceSubtype());
                manAdd->setIfaceNum(opt->getIfaceNum());
                manAdd->setOidLength(opt->getOidLength());
                manAdd->setOid(opt->getOid());

                manAdd->setUpdated(true);

                updatedTlv[LLDPTLV_SYSTEM_CAP] = true;
                break;
            }

            case LLDPTLV_ORG_SPEC: {
                const LLDPOptionOrgSpec *opt = check_and_cast<const LLDPOptionOrgSpec *> (option);
                neighbour->setMtu(std::stoi(opt->getValue()));
                mtu = true;
                break;
            }

            default: {
                EV_INFO << "Unknown type of TLV " << msg->getOption(i)->getType() << endl;
            }
        }
    }

    // remove neighbour informations that was not changed
    for(int i = 0; i < 128; i++)
    {
        if(!updatedTlv[i])
        {
            switch(i)
            {
                case LLDPTLV_PORT_DES:
                    neighbour->setPortDes(nullptr);
                    break;
                case LLDPTLV_SYSTEM_NAME:
                    neighbour->setSystemName(nullptr);
                    break;
                case LLDPTLV_SYSTEM_DES:
                    neighbour->setSystemDes(nullptr);
                    break;
                case LLDPTLV_SYSTEM_CAP:
                    neighbour->setSystemCap(nullptr);
                    neighbour->setEnabledCap(nullptr);
                    break;
            }
        }
    }
    if(!mtu)
        neighbour->setMtu(0);
    neighbour->getManagementAdd().removeNotUpdated();
}

Ptr<LLDPUpdate> LLDPAgent::constrUpdateLLDPDU()
{
    auto update = makeShared<LLDPUpdate>();

    // mandatory
    setTlvChassisId(update);      // chassis ID
    setTlvPortId(update);         // port ID
    setTlvTtl(update, false);     // TTL

    // additional optional
    setTlvPortDes(update);        // port description
    setTlvSystemName(update);     // system name
    setTlvSystemDes(update);      // system description
    setTlvCap(update);            // system capabilites
    setTlvManAdd(update);         // management address

    // organizationally specific
    setTlvMtu(update);         // maximum frame size

    // optional (IEEE Std 802.1AB-2009)
    setTlvEndOf(update);          // end of lldpdu - must be at the end

    return update;
}

Ptr<LLDPUpdate> LLDPAgent::constrShutdownLLDPDU()
{
    auto update = makeShared<LLDPUpdate>();

    setTlvChassisId(update);      // chassis ID
    setTlvPortId(update);         // port ID
    setTlvTtl(update, true);      // TTL

    setTlvEndOf(update);          // end of LLDPDU - must be at the end
    return update;
}

void LLDPAgent::txFrame(const Ptr<LLDPUpdate>& update)
{
    if(adminStatus != enabledRxTx && adminStatus != enabledTxOnly)
    {
        EV_ERROR << "Trying send message with not transmitting agent - sending canceled" << endl;
        // delete update;
        return;
    }

    if (!interface->isUp())
    {// trying send message on DOWN iface -> cancel
        EV_ERROR << "Interface " << interface->getFullName() << " is down, discarding packet" << endl;
        // delete update;
        return;
    }

    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    controlInfo->setDestinationAddress(MacAddress("01-80-c2-00-00-0e"));
    controlInfo->setInterfaceId(interface->getInterfaceId());
    controlInfo->setEtherType(35020);       // 88-cc
    update->setControlInfo(controlInfo);

    st.framesOutTotal++;
    short length = 0;
    for(unsigned int i = 0; i < update->getOptionArraySize(); i++)
        length += update->getOptionLength(update->getOption(i));
    length += sizeof(length)*update->getOptionArraySize();
    update->setChunkLength(B(length));
    txTTROwner->send(update, "ifOut");
}

void LLDPAgent::txInfoFrame()
{
    if(txCredit > 0)
    {
        const Ptr<LLDPUpdate>& update = constrUpdateLLDPDU();
        txFrame(update);
#ifdef CREDIT
        dec(&txCredit);
#endif
        dec(&txFast);

        txSchedule(false);
    }
    else
    {
        txTTROwner->scheduleAt(simTime() + 1, txTTR);
    }
}

void LLDPAgent::txShutdownFrame()
{
    if(adminStatus != enabledRxTx && adminStatus != enabledTxOnly)
        return;

    const Ptr<LLDPUpdate>& update = constrUpdateLLDPDU();
    txFrame(update);
}

void LLDPAgent::txFastStart()
{
    if(adminStatus != enabledRxTx && adminStatus != enabledTxOnly)
        return;

    txTTROwner->cancelEvent(txTTR);

    txFast = txFastInit;

    txInfoFrame();
}



///************************* START OF TLV *****************************///


void LLDPAgent::setTlvChassisId(const Ptr<LLDPUpdate>& msg)
{
    LLDPOptionChassisId *tlv = new LLDPOptionChassisId();
    tlv->setSubtype((uint8_t)lcisMacAdd);

    std::string s = core->getChassisId();
    if(s.size() > 255)
        s.resize(255);

    tlv->setValue(s.c_str());
    msg->setOptionLength(tlv);
    msg->addOption(tlv, -1);
}

void LLDPAgent::setTlvPortId(const Ptr<LLDPUpdate>& msg)
{
    LLDPOptionPortId *tlv = new LLDPOptionPortId();
    tlv->setSubtype((uint8_t)pcisIfaceNam);

    std::string s = interface->getFullName();
    if(s.size() > 255)
        s.resize(255);

    tlv->setValue(s.c_str());
    msg->setOptionLength(tlv);
    msg->addOption(tlv, -1);
}

void LLDPAgent::setTlvTtl(const Ptr<LLDPUpdate>& msg, bool shutDown)
{
    LLDPOptionTTL *tlv = new LLDPOptionTTL();
    if(!shutDown)
        tlv->setTtl(msgTxInterval * msgTxHold);
    else
        tlv->setTtl(0);
    msg->setOptionLength(tlv);
    msg->addOption(tlv, -1);
}

void LLDPAgent::setTlvEndOf(const Ptr<LLDPUpdate>& msg)
{
    LLDPOptionEndOf *tlv = new LLDPOptionEndOf();
    msg->addOption(tlv, -1);
}

void LLDPAgent::setTlvPortDes(const Ptr<LLDPUpdate>& msg)
{
    LLDPOptionPortDes *tlv = new LLDPOptionPortDes();

    std::string s = interface->getInterfaceFullPath();
    if(s.size() > 255)
        s.resize(255);

    tlv->setValue(s.c_str());
    msg->setOptionLength(tlv);
    msg->addOption(tlv, -1);
}

void LLDPAgent::setTlvSystemName(const Ptr<LLDPUpdate>& msg)
{
    LLDPOptionSystemName *tlv = new LLDPOptionSystemName();

    std::string s = core->getContainingModule()->getFullName();
    if(s.size() > 255)
        s.resize(255);

    tlv->setValue(s.c_str());
    msg->setOptionLength(tlv);
    msg->addOption(tlv, -1);
}

void LLDPAgent::setTlvSystemDes(const Ptr<LLDPUpdate>& msg)
{
    LLDPOptionSystemDes *tlv = new LLDPOptionSystemDes();

    std::string s = core->getContainingModule()->getComponentType()->getFullName();
    if(s.size() > 255)
        s.resize(255);

    tlv->setValue(s.c_str());
    msg->setOptionLength(tlv);
    msg->addOption(tlv, -1);
}

void LLDPAgent::setTlvCap(const Ptr<LLDPUpdate>& msg)
{
    LLDPOptionCap *tlv = new LLDPOptionCap();

    const char* sysCap = core->getSystemCapabilites();
    const char* enCap = core->getEnabledCapabilites();

    tlv->setSysCap(0, sysCap[0]);
    tlv->setSysCap(1, sysCap[1]);
    tlv->setEnCap(0, enCap[0]);
    tlv->setEnCap(1, enCap[1]);
    tlv->setChasId((uint8_t)lcisMacAdd);
    msg->setOptionLength(tlv);
    msg->addOption(tlv, -1);
}

void LLDPAgent::setTlvManAdd(const Ptr<LLDPUpdate>& msg)
{
    std::stringstream string;
    bool l3Address = false;

    if(interface->ipv4Data() != nullptr && !interface->ipv4Data()->getIPAddress().isUnspecified())
    {
        l3Address = true;
        string << interface->ipv4Data()->getIPAddress().getInt();
        setTlvManAddSpec(msg, string.str());
    }
    //if(interface->ipv6Data() != nullptr && !interface->ipv6Data()->getLinkLocalAddress().isUnspecified())
    //{
    //    l3Address = true;
    //    setTlvManAddSpec(msg, interface->ipv6Data()->getLinkLocalAddress().str());
    //}
    if(!l3Address)
    {
        string << interface->getMacAddress().getInt();
        setTlvManAddSpec(msg, string.str());
    }
}

void LLDPAgent::setTlvManAddSpec(const Ptr<LLDPUpdate>& msg, std::string add)
{
    LLDPOptionManAdd *tlv = new LLDPOptionManAdd();
    tlv->setAddress(add.c_str());
    tlv->setAddLength(sizeof(tlv->getAddSubtype())+strlen(tlv->getAddress()));
    tlv->setIfaceNum(interface->getInterfaceId());
    tlv->setOidLength(0);
    msg->setOptionLength(tlv);
    msg->addOption(tlv, -1);
}

void LLDPAgent::setTlvMtu(const Ptr<LLDPUpdate>& msg)
{
    LLDPOptionOrgSpec *tlv = new LLDPOptionOrgSpec();
    std::stringstream string;
    string << interface->getMtu();

    tlv->setOui(OUI_IEEE_802_3);            // IEEE 802.3
    tlv->setSubtype(4);                     // subtype to MTU
    tlv->setValue(string.str().c_str());    // set MTU

    setTlvOrgSpec(msg, tlv);
}

void LLDPAgent::setTlvOrgSpec(const Ptr<LLDPUpdate>& msg, LLDPOptionOrgSpec *tlv)
{
    std::string s = tlv->getValue();
    if(strlen(tlv->getValue()) > 507)
    {
        EV_INFO << "Organizationally specific TLV is too long. TLV discarded." << endl;
        delete tlv;
        return;
    }

    msg->setOptionLength(tlv);
    msg->addOption(tlv, -1);
}

std::string LLDPAgent::capabilitiesConvert(char cap1, char cap2)
{
    std::string out;
    std::stringstream s;
    // O - Other, P - repeater, B - MAC Bridge, W - WLAN Access Point, R - Router, T - Telephone
    // C - DOCSIS cable device, S - Station Only, c - C-VLAN Component of a VLAN Bridge
    // s - S-VLAN Component of a VLAN Bridge, t - Two-port MAC Relay (TPMR)
    char capLabel[11] = {'O', 'P', 'B', 'W', 'R', 'T', 'C', 'S', 'c', 's', 't'};
    char capGet[2] = {cap1, cap2};

    for(int i=0; i < 8; i++)
        if(capGet[1-i/8] & (1 << i%8))
            s << " " << capLabel[i];

    out = s.str();
    if(out.length() > 0)    //delete first space
        out.erase(0, 1);
    return out;
}

///************************** END OF TLV ******************************///


///************************ LLDP AGENT TABLE ****************************///

void LLDPAgentTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH_PTRVECTOR(agents);
    }
}

void LLDPAgentTable::handleMessage(cMessage *)
{

}

LLDPAgent * LLDPAgentTable::findAgentById(const int ifaceId)
{
    for (auto & agent : agents)
        if(agent->getInterfaceId() == ifaceId)
            return agent;

    return nullptr;
}

void LLDPAgentTable::startAgents()
{
    for (auto agent : agents)
        agent->startAgent();
}


LLDPAgent * LLDPAgentTable::addAgent(LLDPAgent * agent)
{
    if(findAgentById(agent->getInterfaceId()) != nullptr)
    {// agent already in table
        throw cRuntimeError("Adding to LLDPAgentTable agent, which is already in it - iface id %d", agent->getInterfaceId());
    }

    agents.push_back(agent);

    return agent;
}

void LLDPAgentTable::removeAgent(LLDPAgent * agent)
{
    std::vector<LLDPAgent *>::iterator it;

    for (it = agents.begin(); it != agents.end();)
    {// through all interfaces
        if((*it) == agent)
        {// found same
            delete (*it);
            it = agents.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

void LLDPAgentTable::removeAgent(int ifaceId)
{
    std::vector<LLDPAgent *>::iterator it;

    for (it = agents.begin(); it != agents.end();)
    {// through all agents
        if((*it)->getInterfaceId() == ifaceId)
        {// found same
            delete (*it);
            it = agents.erase(it);
            return;
        }        else
        {// do not delete -> get next
            ++it;
        }
    }
}

LLDPAgentTable::~LLDPAgentTable()
{
    std::vector<LLDPAgent *>::iterator it;

    for (it = agents.begin(); it != agents.end(); ++it)
    {// through all agents
        delete (*it);
    }
    agents.clear();
}

std::string LLDPAgentTable::printStats()
{
    std::stringstream string;
    std::vector<LLDPAgent *>::iterator it;
    LLDPStatistics *st;

    for (it = agents.begin(); it != agents.end(); ++it)
    {// through all agents
        if((*it)->getAdminStatus() != AS::disabled)
        {
            st = (*it)->getSt();
            string << "Agent on intereface " << (*it)->getIfaceName() << " statistics:" << endl;

            string << "Ageouts total:" << st->ageoutsTotal << ", Frames discarded total:" << st->framesDiscardedTotal << endl;
            string << "Frames in errors total:" << st->framesInErrorsTotal << ", Frames in total:" << st->framesInTotal << endl;
            string << "Frames out total: " << st->framesInTotal << ", Frames out total: " << st->framesOutTotal << endl;
            string << "Tlvs discarded total: " << st->tlvsDiscardedTotal << ", Tlvs unrecognized total" << st->tlvsUnrecognizedTotal << endl;
            string << "Lldpdu length errors: " << st->lldpduLengthErrors << endl;
        }
    }
    return string.str();
}

LLDPManAdd *LLDPManAddTab::findManAdd(uint8_t type)
{
    std::vector<LLDPManAdd *>::iterator it;

    for (it = manAddresses.begin(); it != manAddresses.end(); ++it)
    {// through all management addresses search for same type
        if((*it)->getAddSubtype() == type)
        {// found same
            return (*it);
        }
    }

    return nullptr;
}

void LLDPManAddTab::addManAdd(LLDPManAdd *manAdd)
{
    if(findManAdd(manAdd->getAddSubtype()) == nullptr)
        manAddresses.push_back(manAdd);
}

void LLDPManAddTab::removeManAdd(LLDPManAdd *manAdd)
{
    std::vector<LLDPManAdd *>::iterator it;

    for (it = manAddresses.begin(); it != manAddresses.end();)
    {// through all interfaces
        if((*it) == manAdd)
        {// found same
            delete (*it);
            it = manAddresses.erase(it);
            return;
        }
        else
        {// do not delete -> get next
            ++it;
        }
    }
}

void LLDPManAddTab::setAllUnchanged()
{
    std::vector<LLDPManAdd *>::iterator it;

    for (it = manAddresses.begin(); it != manAddresses.end(); ++it)
    {// through all managements addresses
        (*it)->setUpdated(false);
    }
}

void LLDPManAddTab::removeNotUpdated()
{
    std::vector<LLDPManAdd *>::iterator it, lIt;

    lIt = it = manAddresses.begin();
    for (it = manAddresses.begin(); it != manAddresses.end(); )
    {// through all manAddresses
        if(!(*it)->getUpdated())
        {
            lIt = ++it;
            delete (*it);
            it = manAddresses.erase(it);
            it = lIt;
        }
        else
            ++it;
    }
}

} /* namespace inet */
